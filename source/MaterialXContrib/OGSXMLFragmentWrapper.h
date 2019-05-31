#ifndef MATERIALX_OGSX_ML_FRAGMENT_WRAPPER_H
#define MATERIALX_OGSX_ML_FRAGMENT_WRAPPER_H

/// @file
/// Contribution utility methods

#include <MaterialXGenShader/Library.h>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Element.h>

namespace MaterialX
{
/// @class OGSXMLFragmentWrapper
/// Class representing an OGS XML Fragment Wrapper
/// Interfaces allow the wrapper to be generated based on an input node and generator
/// The wrapper can contain fragment descriptions for more than one node if desired.
class OGSXMLFragmentWrapper
{
  public:
    /// Default constructor
    OGSXMLFragmentWrapper(GenContext* context);

    /// Default desctructor
    ~OGSXMLFragmentWrapper();

    /// @name Generation
    /// @{

    /// Add a fragment wrapper for a shader generated from a given element
    void createWrapper(ElementPtr node);

    /// Get the contents of the cached XML document as a stream.
    void getDocument(std::ostream& stream);

    /// Set to output vertex shader code (if any)
    void setOutputVertexShader(bool val)
    {
        _outputVertexShader = val;
    }

    /// @}
    /// @name Query
    /// @{ 

    /// Query if we want to output vertex shader code
    bool getOutputVertexShader() const
    {
        return _outputVertexShader;
    }

    /// Get list of Element paths and corresponding fragment input names
    const StringMap& getPathInputMap() const
    {
        return _pathInputMap;
    }

    /// @}
    /// @name Load
    /// @{

    /// Utility to read in an XML fragment from an input stream and return in an output stream
    static void readDocument(std::istream& istream, std::ostream& ostream);

    /// @}

  protected:
    void getVertexUniformSemantic(const string& name, string& semantic) const;

    // Mapping from MTLX keywords to OGS fragment XML keywords
    StringMap _typeMap;

    bool _outputVertexShader = false;
    
    // Internally cache XML document
    void* _xmlDocument;

    // Mapping from MTLX Element paths to fragment input names
    StringMap _pathInputMap;

    // Context for generating shaders
    GenContext* _context;
};

} // namespace MaterialX

#endif
