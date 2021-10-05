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
#ifndef ONEFLOW_CORE_VM_THREAD_MSG_H_
#define ONEFLOW_CORE_VM_THREAD_MSG_H_

#include <functional>
#include "oneflow/core/vm/stream.msg.h"
#include "oneflow/core/vm/stream_runtime_desc.msg.h"

namespace oneflow {
namespace vm {

// clang-format off
OBJECT_MSG_BEGIN(ThreadCtx);
 public:
  void __Init__() { clear_stream_rt_desc(); }

  // Getters
  bool has_stream_rt_desc() const { return stream_rt_desc_ != nullptr; }
  const StreamRtDesc& stream_rt_desc() const { return *stream_rt_desc_; }

  // Setters
  void set_stream_rt_desc(const StreamRtDesc* val) { stream_rt_desc_ = val; }
  void clear_stream_rt_desc() { stream_rt_desc_ = nullptr; }

  // methods
  OF_PUBLIC void __Init__(const StreamRtDesc& stream_rt_desc) {
    __Init__();
    set_stream_rt_desc(&stream_rt_desc);
  }
  OF_PUBLIC void LoopRun(const std::function<void(ThreadCtx*)>& Initializer);
  // fields
  OBJECT_MSG_FIELD(const StreamRtDesc*, stream_rt_desc_); 

  // links
  OBJECT_MSG_DEFINE_LIST_LINK(thread_ctx_link);
  OBJECT_MSG_DEFINE_LIST_HEAD(Stream, thread_ctx_stream_link, stream_list);
  OBJECT_MSG_DEFINE_CONDITION_LIST_HEAD(Instruction, pending_instruction_link,
                                        pending_instruction_list);

  OF_PRIVATE ObjectMsgConditionListStatus ReceiveAndRun();
  OF_PUBLIC ObjectMsgConditionListStatus TryReceiveAndRun();
OBJECT_MSG_END(ThreadCtx);
// clang-format on

}  // namespace vm
}  // namespace oneflow

#endif  // ONEFLOW_CORE_VM_THREAD_MSG_H_
