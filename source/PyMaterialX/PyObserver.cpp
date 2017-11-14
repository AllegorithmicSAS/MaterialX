//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Observer.h>

#include <PyBind11/pybind11.h>
#include <PyBind11/operators.h>
#include <PyBind11/stl.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyObserver(py::module& mod)
{
    py::class_<mx::Observer, std::shared_ptr<mx::Observer> >(mod, "Observer")
        .def("onAddElement", &mx::Observer::onAddElement)
        .def("onRemoveElement", &mx::Observer::onRemoveElement)
        .def("onSetAttribute", &mx::Observer::onSetAttribute)
        .def("onRemoveAttribute", &mx::Observer::onSetAttribute)
        .def("onInitialize", &mx::Observer::onInitialize)
        .def("onRead", &mx::Observer::onRead)
        .def("onWrite", &mx::Observer::onWrite)
        .def("onBeginUpdate", &mx::Observer::onBeginUpdate)
        .def("onEndUpdate", &mx::Observer::onEndUpdate);
}

void bindPyObservedDocument(py::module& mod)
{
    mod.def("createObservedDocument", &mx::Document::createDocument<mx::ObservedDocument>);

    py::class_<mx::ObservedDocument, mx::ObservedDocumentPtr, mx::Document>(mod, "ObservedDocument")
        .def("copy", &mx::ObservedDocument::copy)
        .def("addObserver", &mx::ObservedDocument::addObserver)
        .def("removeObserver", &mx::ObservedDocument::removeObserver)
        .def("clearObservers", &mx::ObservedDocument::clearObservers);
}
