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

#ifndef _ONEFLOW_USER_KERNELS_SCALAR_FMOD_KERNEL_H_
#define _ONEFLOW_USER_KERNELS_SCALAR_FMOD_KERNEL_H_
#include "oneflow/core/ndarray/binary_func.h"
#include "oneflow/core/ndarray/xpu_util.h"

namespace oneflow {

template<DeviceType device_type, template<typename T> class BIN_OP, typename T>
struct ScalarFmodFunctor final {
  void operator()(DeviceCtx* ctx, const int64_t elem_cnt, const T scalar, const T* in, T* out);
};

template<template<typename> class UnaryFunctor, typename T>
OF_DEVICE_FUNC void DoScalarFmod(const int64_t elem_cnt, const T scalar, const T* in, T* out) {
  XPU_1D_KERNEL_LOOP(idx, elem_cnt) { out[idx] = UnaryFunctor<T>::Invoke(in[idx], scalar); }
}

#define REGISTER_SCALAR_FMOD_KERNEL(device, dtype)                    \
  REGISTER_USER_KERNEL("scalar_fmod")                                 \
      .SetCreateFn<ScalarFmodKernel<device, BinaryFuncFMod, dtype>>() \
      .SetIsMatchedHob((user_op::HobDeviceTag() == device)            \
                       & (user_op::HobDataType("in", 0) == GetDataType<dtype>::value));

}  // namespace oneflow

#endif  //_ONEFLOW_USER_KERNELS_SCALAR_FMOD_KERNEL_H_
