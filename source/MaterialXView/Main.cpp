#include <MaterialXView/Viewer.h>

#include <iostream>

NANOGUI_FORCE_DISCRETE_GPU();

int main(int argc, char* const argv[])
{  
    std::vector<std::string> tokens;
    for (int i = 1; i < argc; i++)
    {
        tokens.push_back(std::string(argv[i]));
    }

    mx::StringVec libraryFolders = { "libraries/stdlib", "libraries/pbrlib", "libraries/stdlib/genglsl", "libraries/pbrlib/genglsl", "libraries/bxdf" };
    mx::FileSearchPath searchPath;
    std::string meshFilename = "resources/Geometry/shaderball.obj";
    std::string materialFilename = "resources/Materials/Examples/StandardSurface/standard_surface_default.mtlx";
    DocumentModifiers modifiers;
    int multiSampleCount = 0;
    mx::HwSpecularEnvironmentMethod specularEnvironmentMethod = mx::SPECULAR_ENVIRONMENT_FIS;

    for (size_t i = 0; i < tokens.size(); i++)
    {
        const std::string& token = tokens[i];
        const std::string& nextToken = i + 1 < tokens.size() ? tokens[i + 1] : mx::EMPTY_STRING;
        if (token == "--library" && !nextToken.empty())
        {
            libraryFolders.push_back(nextToken);
        }
        if (token == "--path" && !nextToken.empty())
        {
            searchPath = mx::FileSearchPath(nextToken);
        }
        if (token == "--mesh" && !nextToken.empty())
        {
            meshFilename = nextToken;
        }
        if (token == "--material" && !nextToken.empty())
        {
            materialFilename = nextToken;
        }
        if (token == "--remap" && !nextToken.empty())
        {
            mx::StringVec vec = mx::splitString(nextToken, ":");
            if (vec.size() == 2)
            {
                modifiers.remapElements[vec[0]] = vec[1];
            }
        }
        if (token == "--skip" && !nextToken.empty())
        {
            modifiers.skipElements.insert(nextToken);
        }
        if (token == "--terminator" && !nextToken.empty())
        {
            modifiers.filePrefixTerminator = nextToken;
        }
        if (token == "--envMethod" && !nextToken.empty())
        {
            if (std::stoi(nextToken) == 1)
            {
                specularEnvironmentMethod = mx::SPECULAR_ENVIRONMENT_PREFILTER;
            }
        }
        if (token == "--msaa" && !nextToken.empty())
        {
            multiSampleCount = std::stoi(nextToken);
        }
    }

    // Search current directory and parent directory if not found.
    mx::FilePath currentPath(mx::FilePath::getCurrentPath());
    mx::FilePath parentCurrentPath(currentPath);
    parentCurrentPath.pop();
    std::vector<mx::FilePath> libraryPaths =
    { 
        mx::FilePath("libraries")
    };
    for (auto libraryPath : libraryPaths)
    {
        mx::FilePath fullPath(currentPath / libraryPath);
        if (!fullPath.exists())
        {
            fullPath = parentCurrentPath / libraryPath;
            if (fullPath.exists())
            {
                searchPath.append(fullPath);
            }
        }
        else
        {
            searchPath.append(fullPath);
        }
    }
    searchPath.append(parentCurrentPath);

    try
    {
        ng::init();
        {
            ng::ref<Viewer> viewer = new Viewer(libraryFolders,
                                                searchPath,
                                                meshFilename,
                                                materialFilename,
                                                modifiers,
                                                specularEnvironmentMethod,
                                                multiSampleCount);
            viewer->setVisible(true);
            ng::mainloop();
        }
    
        ng::shutdown();
    }
    catch (const std::runtime_error& e)
    {
        std::string error_msg = std::string("Fatal error: ") + std::string(e.what());
        std::cerr << error_msg << std::endl;
        return -1;
    }

    return 0;
}
