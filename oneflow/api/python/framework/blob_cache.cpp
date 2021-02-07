/*
Copyright 2020 The OneFlow Authors. All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include "oneflow/api/python/of_api_registry.h"
#include "oneflow/core/framework/blob_cache.h"

namespace py = pybind11;

namespace oneflow {

namespace compatible_py {

ONEFLOW_API_PYBIND11_MODULE("", m) {
  py::class_<BlobCache, std::shared_ptr<BlobCache>>(m, "BlobCache", py::dynamic_attr())
      .def(py::init([](const std::shared_ptr<BlobObject>& blob_object) {
        return std::make_shared<BlobCache>(blob_object);
      }))
      .def_property_readonly("blob_object", &BlobCache::blob_object)
      .def("GetHeaderCache", &BlobCache::GetHeaderCache)
      .def("GetCachedDelegateBlobObject", &BlobCache::GetCachedDelegateBlobObject);

  m.def("FindOrCreateBlobCache", [](const std::shared_ptr<BlobObject>& blob_object) {
    return FindOrCreateBlobCache(blob_object).GetPtrOrThrow();
  });
  m.def("TryDisableBlobCache", [](const std::shared_ptr<BlobObject>& blob_object) {
    return TryDisableBlobCache(blob_object).GetOrThrow();
  });

  m.def("FindOrCreateDelegateBlobObject", &FindOrCreateDelegateBlobObject);
}
}  // namespace compatible_py

}  // namespace oneflow
