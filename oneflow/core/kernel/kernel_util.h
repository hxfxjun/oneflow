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
#ifndef ONEFLOW_CORE_KERNEL_KERNEL_UTIL_H_
#define ONEFLOW_CORE_KERNEL_KERNEL_UTIL_H_

#include "oneflow/core/common/blas.h"
#include "oneflow/core/common/data_type.h"
#include "oneflow/core/common/str_util.h"
#include "oneflow/core/device/cudnn_util.h"
#include "oneflow/core/kernel/kernel_context.h"
#include "oneflow/core/common/switch_func.h"
#include "oneflow/core/kernel/new_kernel_util.h"
#include "oneflow/core/register/blob.h"

namespace oneflow {

class Blob;
class InitializerConf;
class MemoryCase;
class StreamContext;

void AutoMemcpy(DeviceCtx* ctx, void* dst, const void* src, size_t sz,
                const MemoryCase& dst_mem_case, const MemoryCase& src_mem_case);
void AutoMemcpy(DeviceCtx* ctx, Blob* dst, const Blob* src);
void AutoMemcpy(StreamContext* stream_ctx, void* dst, const void* src, size_t sz,
                const MemoryCase& dst_mem_case, const MemoryCase& src_mem_case);
void AutoMemcpy(StreamContext* stream_ctx, Blob* dst, const Blob* src);
void SyncAutoMemcpy(DeviceCtx* ctx, void* dst, const void* src, size_t sz,
                    const MemoryCase& dst_mem_case, const MemoryCase& src_mem_case);
void AutoMemset(DeviceCtx* ctx, void* dst, const char value, size_t sz,
                const MemoryCase& dst_mem_case);
void AutoMemset(StreamContext* stream_ctx, void* dst, const char value, size_t sz,
                const MemoryCase& dst_mem_case);

template<DeviceType device_type, typename T, typename U = void>
struct KernelUtil;

// CPU, Integral, Floating
template<typename T, typename Derived>
struct CpuKernelUtilIf {
  static void Axpy(DeviceCtx* ctx, const int n, const T* alpha, const T* x, const int incx, T* y,
                   const int incy);
};

// CPU, Floating
template<typename T>
struct KernelUtil<DeviceType::kCPU, T, typename std::enable_if<IsFloating<T>::value>::type>
    : public CpuKernelUtilIf<T, KernelUtil<DeviceType::kCPU, T>> {
  static void Axpy(DeviceCtx* ctx, const int n, const T alpha, const T* x, const int incx, T* y,
                   const int incy);

  static void Addition(DeviceCtx* ctx, const int64_t n, T* out, const T* in_0);
  static void Addition(DeviceCtx* ctx, const int64_t n, T* out, const T* in_0, const T* in_1);
  static void Addition(DeviceCtx* ctx, const int64_t n, T* out, const T* in_0, const T* in_1,
                       const T* in_2);
  static void Addition(DeviceCtx* ctx, const int64_t n, T* out, const T* in_0, const T* in_1,
                       const T* in_2, const T* in_3);
  static void Addition(DeviceCtx* ctx, const int64_t n, T* out, const T* in_0, const T* in_1,
                       const T* in_2, const T* in_3, const T* in_4);
  static void Addition(DeviceCtx* ctx, const int64_t n, T* out, const T* in_0, const T* in_1,
                       const T* in_2, const T* in_3, const T* in_4, const T* in_5);
  static void Addition(DeviceCtx* ctx, const int64_t n, T* out, const T* in_0, const T* in_1,
                       const T* in_2, const T* in_3, const T* in_4, const T* in_5, const T* in_6);
  static void Addition(DeviceCtx* ctx, const int64_t n, T* out, const T* in_0, const T* in_1,
                       const T* in_2, const T* in_3, const T* in_4, const T* in_5, const T* in_6,
                       const T* in_7);
  static void Addition(DeviceCtx* ctx, const int64_t n, T* out, const T* in_0, const T* in_1,
                       const T* in_2, const T* in_3, const T* in_4, const T* in_5, const T* in_6,
                       const T* in_7, const T* in_8);

  static void InitializeWithConf(DeviceCtx* ctx, const InitializerConf& initializer_conf,
                                 uint32_t random_seed, Blob* blob);
};

// CPU, Integral
template<typename T>
struct KernelUtil<DeviceType::kCPU, T, typename std::enable_if<IsIntegral<T>::value>::type>
    : public CpuKernelUtilIf<T, KernelUtil<DeviceType::kCPU, T>> {
  static void Axpy(DeviceCtx* ctx, const int n, const T alpha, const T* x, const int incx, T* y,
                   const int incy);
  static void InitializeWithConf(DeviceCtx* ctx, const InitializerConf& initializer_conf,
                                 uint32_t random_seed, Blob* blob);
};

// GPU, Integral, Floating
template<typename T, typename Derived>
struct GpuKernelUtilIf {
  static void InitializeWithConf(DeviceCtx* ctx, const InitializerConf& initializer_conf,
                                 uint32_t random_seed, Blob* blob);
};

// GPU, Floating
template<typename T>
struct KernelUtil<DeviceType::kGPU, T, typename std::enable_if<IsFloating<T>::value>::type>
    : public GpuKernelUtilIf<T, KernelUtil<DeviceType::kGPU, T>> {
  static void Axpy(DeviceCtx* ctx, const int n, const T alpha, const T* x, const int incx, T* y,
                   const int incy);
  static void Axpy(DeviceCtx* ctx, const int n, const T* alpha, const T* x, const int incx, T* y,
                   const int incy);

  static void Addition(DeviceCtx* ctx, const int64_t n, T* out, const T* in_0);
  static void Addition(DeviceCtx* ctx, const int64_t n, T* out, const T* in_0, const T* in_1);
  static void Addition(DeviceCtx* ctx, const int64_t n, T* out, const T* in_0, const T* in_1,
                       const T* in_2);
  static void Addition(DeviceCtx* ctx, const int64_t n, T* out, const T* in_0, const T* in_1,
                       const T* in_2, const T* in_3);
  static void Addition(DeviceCtx* ctx, const int64_t n, T* out, const T* in_0, const T* in_1,
                       const T* in_2, const T* in_3, const T* in_4);
  static void Addition(DeviceCtx* ctx, const int64_t n, T* out, const T* in_0, const T* in_1,
                       const T* in_2, const T* in_3, const T* in_4, const T* in_5);
  static void Addition(DeviceCtx* ctx, const int64_t n, T* out, const T* in_0, const T* in_1,
                       const T* in_2, const T* in_3, const T* in_4, const T* in_5, const T* in_6);
  static void Addition(DeviceCtx* ctx, const int64_t n, T* out, const T* in_0, const T* in_1,
                       const T* in_2, const T* in_3, const T* in_4, const T* in_5, const T* in_6,
                       const T* in_7);
  static void Addition(DeviceCtx* ctx, const int64_t n, T* out, const T* in_0, const T* in_1,
                       const T* in_2, const T* in_3, const T* in_4, const T* in_5, const T* in_6,
                       const T* in_7, const T* in_8);
};

// GPU, Integral
template<typename T>
struct KernelUtil<DeviceType::kGPU, T, typename std::enable_if<IsIntegral<T>::value>::type>
    : public GpuKernelUtilIf<T, KernelUtil<DeviceType::kGPU, T>> {
  static void Axpy(DeviceCtx* ctx, const int n, const T alpha, const T* x, const int incx, T* y,
                   const int incy);
};

template<typename T, typename U>
typename std::enable_if<std::is_same<T, U>::value>::type CopyElem(const T* in_dptr, U* out_dptr,
                                                                  int64_t elem_num) {
  Memcpy<DeviceType::kCPU>(nullptr, out_dptr, in_dptr, elem_num * sizeof(T));
}

template<typename T, typename U>
typename std::enable_if<!std::is_same<T, U>::value>::type CopyElem(const T* in_dptr, U* out_dptr,
                                                                   int64_t elem_num) {
  FOR_RANGE(int64_t, i, 0, elem_num) { *(out_dptr++) = static_cast<U>(*(in_dptr++)); }
}

#ifdef WITH_CUDA
template<typename T, typename U>
void CopyElemOnGpu(DeviceCtx* ctx, const T* in_dptr, U* out_dptr, int64_t elem_num);
#endif

}  // namespace oneflow

#endif  // ONEFLOW_CORE_KERNEL_KERNEL_UTIL_H_
