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
#ifndef ONEFLOW_CORE_PRIMITIVE_CUDA_CUDA_GRAPH_SUPPORT_H_
#define ONEFLOW_CORE_PRIMITIVE_CUDA_CUDA_GRAPH_SUPPORT_H_

#include "oneflow/core/primitive/include/primitive.h"

namespace oneflow {

namespace primitive {

class CudaGraphSupport {
 public:
  OF_DISALLOW_COPY_AND_MOVE(CudaGraphSupport);
  CudaGraphSupport() = default;
  virtual ~CudaGraphSupport() = default;

  virtual bool IsCudaGraphSupported() const { return true; }
};

bool IsCudaGraphPrimitive(const Primitive* primitive);

}  // namespace primitive

}  // namespace oneflow

#endif  // ONEFLOW_CORE_PRIMITIVE_CUDA_CUDA_GRAPH_SUPPORT_H_
