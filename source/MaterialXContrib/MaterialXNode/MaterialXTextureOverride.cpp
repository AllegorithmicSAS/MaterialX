#include "MaterialXTextureOverride.h"
#include "MaterialXNode.h"
#include "Plugin.h"
#include "Util.h"

#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Util.h>
#include <MaterialXFormat/XmlIo.h>

#include <maya/MFnDependencyNode.h>
#include <maya/MFragmentManager.h>
#include <maya/MRenderUtil.h>
#include <maya/MShaderManager.h>
#include <maya/MTextureManager.h>

#include <fstream>

const MString MaterialXTextureOverride::REGISTRANT_ID = "materialXTexture";

MHWRender::MPxShadingNodeOverride* MaterialXTextureOverride::creator(const MObject& obj)
{
	std::cout.rdbuf(std::cerr.rdbuf());
	std::cout << "MaterialXTextureOverride::creator" << std::endl;
	return new MaterialXTextureOverride(obj);
}

MaterialXTextureOverride::MaterialXTextureOverride(const MObject& obj)
	: MPxShadingNodeOverride(obj)
	, _object(obj)
{
	std::cout << "MaterialXTextureOverride 0" << std::endl;

	MStatus status;
	MFnDependencyNode depNode(_object, &status);
	if (status)
	{
		depNode.findPlug(MaterialXNode::DOCUMENT_ATTRIBUTE_LONG_NAME, false, &status).getValue(_documentContent);
		depNode.findPlug(MaterialXNode::ELEMENT_ATTRIBUTE_LONG_NAME, false, &status).getValue(_element);
	}

    try
    {
		// Create document
		_document = MaterialX::createDocument();
		MaterialX::readFromXmlString(_document, _documentContent.asChar());

		// Load libraries
		MaterialX::FilePath libSearchPath = Plugin::instance().getLibrarySearchPath();
		const MaterialX::StringVec libraries = { "stdlib", "pbrlib" };
		MaterialX::loadLibraries(libraries, libSearchPath, _document);

		std::cout.rdbuf(std::cerr.rdbuf());
		std::cout << "MaterialXTextureOverride 1" << std::endl;
	
		std::vector<MaterialX::GenContext*> contexts;
        MaterialX::GenContext* glslContext = new MaterialX::GenContext(MaterialX::GlslShaderGenerator::create());

		std::cout << "MaterialXTextureOverride 2" << std::endl;

        // Stop emission of environment map lookups.
        glslContext->getOptions().hwSpecularEnvironmentMethod = MaterialX::SPECULAR_ENVIRONMENT_NONE;
        glslContext->registerSourceCodeSearchPath(libSearchPath);
        contexts.push_back(glslContext);

		std::cout << "MaterialXTextureOverride 3" << std::endl;

		_glslWrapper = new MaterialX::OGSXMLFragmentWrapper(glslContext);
		_glslWrapper->setOutputVertexShader(true);

		std::cout << "MaterialXTextureOverride 4" << std::endl;
		
		MaterialX::ElementPtr element = _document->getDescendant(_element.asChar());
        MaterialX::OutputPtr output = element->asA<MaterialX::Output>();
        MaterialX::NodePtr node;
		std::cout << "MaterialXTextureOverride 5" << std::endl;
		if (output)
        {
			std::cout << "MaterialXTextureOverride 6" << std::endl;
			_glslWrapper->createWrapper(output);
        }

		std::stringstream glslStream;
		_glslWrapper->getDocument(glslStream);
		std::stringstream xmlFileStream;
		std::ifstream xmlFile(Plugin::instance().getOGSXMLFragmentPath().asString());
		xmlFileStream << xmlFile.rdbuf();

		std::cout << "MaterialXTextureOverride 7" << std::endl;
		// Register fragments with the manager if needed
		//
		MHWRender::MRenderer* theRenderer = MHWRender::MRenderer::theRenderer();
		if (theRenderer)
		{
			MHWRender::MFragmentManager* fragmentMgr =
			theRenderer->getFragmentManager();
			if (fragmentMgr)
			{
				std::cout << "MaterialXTextureOverride 8" << std::endl;
				fragmentMgr->setEffectOutputDirectory("C:/tmp");
				fragmentMgr->setIntermediateGraphOutputDirectory("C:/tmp");
				_fragmentName = fragmentMgr->addShadeFragmentFromBuffer(xmlFileStream.str().c_str(), false);// glslStream.str().c_str(), false);
				std::cout << "MaterialXTextureOverride 9" << std::endl;
			}
		}
    }
    catch (MaterialX::Exception& e)
    {
		std::cout << "MaterialXTextureOverride BAD!!!" << std::endl;
        std::cerr << "Failed to generate OGS XML wrapper: " << e.what() << std::endl;
    }
}

MaterialXTextureOverride::~MaterialXTextureOverride()
{
	// TODO: Free sampler state here!

	if (_glslWrapper)
	{
		delete _glslWrapper;
	}
}

MHWRender::DrawAPI MaterialXTextureOverride::supportedDrawAPIs() const
{
	return MHWRender::kOpenGL | MHWRender::kOpenGLCoreProfile;
}

MString MaterialXTextureOverride::fragmentName() const
{
	return _fragmentName;
}

void MaterialXTextureOverride::getCustomMappings(MHWRender::MAttributeParameterMappingList& mappings)
{
	const MaterialX::StringMap& inputs = _glslWrapper->getPathInputMap();
	for (auto i : inputs)
	{
		std::cout.rdbuf(std::cerr.rdbuf());
		std::cout << "getCustomMappings: " << i.second.c_str() << std::endl;
		MHWRender::MAttributeParameterMapping mapping(i.second.c_str(), "", false, true);
		mappings.append(mapping);
	}
}

void MaterialXTextureOverride::updateDG()
{
}

void MaterialXTextureOverride::updateShader(MHWRender::MShaderInstance& shader, 
                                            const MHWRender::MAttributeParameterMappingList& /*mappings*/)
{
	const MaterialX::StringMap& inputs = _glslWrapper->getPathInputMap();
	for (auto i : inputs)
	{
		MaterialX::ElementPtr element = _document->getDescendant(i.first);
		if (!element) continue;
		MaterialX::ValueElementPtr valueElement = element->asA<MaterialX::ValueElement>();
		if (valueElement)
		{
			if (valueElement->getType() == MaterialX::FILENAME_TYPE_STRING)
			{
				std::cout << "updateShader (filename): " << i.second.c_str() << std::endl;
				MHWRender::MSamplerStateDesc desc;
				desc.filter = MHWRender::MSamplerState::kAnisotropic;
				desc.maxAnisotropy = 16;
				const MSamplerState* samplerState = MHWRender::MStateManager::acquireSamplerState(desc);
				
				if (samplerState)
				{
					shader.setParameter(i.second.c_str(), *samplerState);
				}
				/*
				// Set texture
				MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
				if (renderer)
				{
					MHWRender::MTextureManager* textureManager =
					renderer->getTextureManager();
					if (textureManager)
					{
						MHWRender::MTexture* texture =
						textureManager->acquireTexture(fFileName);
						if (texture)
						{
							MHWRender::MTextureAssignment textureAssignment;
							textureAssignment.texture = texture;
							shader.setParameter(fResolvedMapName, textureAssignment);

							// release our reference now that it is set on the shader
							textureManager->releaseTexture(texture);
						}
					}
				}
				*/
			}
			else if (valueElement->getType() == MaterialX::TypedValue<MaterialX::Vector2>::TYPE)
			{
				MaterialX::Vector2 vector2 = valueElement->getValue()->asA<MaterialX::Vector2>();
				shader.setArrayParameter(i.second.c_str(), vector2.data(), 2);
				std::cout << "updateShader (vector2): " << i.second.c_str() << std::endl;
			}
			else if (valueElement->getType() == MaterialX::TypedValue<MaterialX::Vector3>::TYPE)
			{
				MaterialX::Vector3 vector3 = valueElement->getValue()->asA<MaterialX::Vector3>();
				shader.setArrayParameter(i.second.c_str(), vector3.data(), 3);
				std::cout << "updateShader (vector3): " << i.second.c_str() << std::endl;
			}
			else if (valueElement->getType() == MaterialX::TypedValue<MaterialX::Vector4>::TYPE)
			{
				MaterialX::Vector4 vector4 = valueElement->getValue()->asA<MaterialX::Vector4>();
				shader.setArrayParameter(i.second.c_str(), vector4.data(), 4);
				std::cout << "updateShader (vector4): " << i.second.c_str() << std::endl;
			}
			else if (valueElement->getType() == MaterialX::TypedValue<MaterialX::Color2>::TYPE)
			{
				std::cout << "updateShader (color2): " << i.second.c_str() << std::endl;
				MaterialX::Color2 color2 = valueElement->getValue()->asA<MaterialX::Color2>();
				shader.setArrayParameter(i.second.c_str(), color2.data(), 2);
			}
			else if (valueElement->getType() == MaterialX::TypedValue<MaterialX::Color3>::TYPE)
			{
				std::cout << "updateShader (color3): " << i.second.c_str() << std::endl;
				MaterialX::Color3 color3 = valueElement->getValue()->asA<MaterialX::Color3>();
				shader.setArrayParameter(i.second.c_str(), color3.data(), 3);
			}
			else if (valueElement->getType() == MaterialX::TypedValue<MaterialX::Color4>::TYPE)
			{
				std::cout << "updateShader (color4): " << i.second.c_str() << std::endl;
				MaterialX::Color4 color4 = valueElement->getValue()->asA<MaterialX::Color4>();
				shader.setArrayParameter(i.second.c_str(), color4.data(), 4);
			}
			else if (valueElement->getType() == MaterialX::TypedValue<MaterialX::Matrix44>::TYPE)
			{
				std::cout << "updateShader (mat44): " << i.second.c_str() << std::endl;
				MaterialX::Matrix44 mat44 = valueElement->getValue()->asA<MaterialX::Matrix44>();
				shader.setArrayParameter(i.second.c_str(), mat44.data(), 16);
			}
		}
	}
}
