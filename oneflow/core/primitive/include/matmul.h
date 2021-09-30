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
#ifndef ONEFLOW_CORE_PRIMITIVE_MATMUL_H_
#define ONEFLOW_CORE_PRIMITIVE_MATMUL_H_

#include "oneflow/core/primitive/include/primitive.h"
#include "oneflow/core/common/scalar.h"

namespace oneflow {

namespace primitive {

enum class MatmulTransposeType {
  N = 0,
  T,
};

class Matmul : public Primitive {
 public:
  OF_DISALLOW_COPY_AND_MOVE(Matmul);
  Matmul() = default;
  ~Matmul() override = default;

  virtual DataType a_type() const = 0;
  virtual DataType b_type() const = 0;
  virtual DataType c_type() const = 0;
  virtual MatmulTransposeType transpose_a() const = 0;
  virtual MatmulTransposeType transpose_b() const = 0;

  virtual void Launch(StreamContext* stream_ctx, size_t m, size_t n, size_t k, Scalar alpha,
                      const void* a, const void* b, Scalar beta, void* c) = 0;
};

class MatmulFactory : public Factory<Matmul> {
 public:
  OF_DISALLOW_COPY_AND_MOVE(MatmulFactory);
  MatmulFactory() = default;
  ~MatmulFactory() override = default;

  virtual std::unique_ptr<Matmul> New(DataType data_type, MatmulTranposeType transpose_a,
                                      MatmulTranposeType transpose_b) = 0;
};

}  // namespace primitive

}  // namespace oneflow

#endif  // ONEFLOW_CORE_PRIMITIVE_MATMUL_H_
