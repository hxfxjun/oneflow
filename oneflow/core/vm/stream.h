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
#ifndef ONEFLOW_CORE_VM_STREAM_H_
#define ONEFLOW_CORE_VM_STREAM_H_

#include "oneflow/core/vm/stream_desc.h"
#include "oneflow/core/vm/instruction.h"
#include "oneflow/core/device/device_context.h"

namespace oneflow {
namespace vm {

struct ThreadCtx;

// clang-format off
INTRUSIVE_BEGIN(Stream);
 public:
  void __Init__();

  // types
  using InstructionList = intrusive::List<INTRUSIVE_FIELD(Instruction, instruction_entry_)>;

  // Getters
  int64_t max_device_num_per_machine() const { return max_device_num_per_machine_; }
  const ThreadCtx& thread_ctx() const { return *thread_ctx_; }
  bool has_thread_ctx() const { return thread_ctx_ != nullptr; }
  const std::unique_ptr<DeviceCtx>& device_ctx() const { return device_ctx_; }
  bool is_active_stream_entry_empty() const { return active_stream_entry_.empty(); }
  const InstructionList& free_instruction_list() const { return free_instruction_list_; }
  const InstructionList& zombie_instruction_list() const { return zombie_instruction_list_; }
  const InstructionList& running_instruction_list() const { return running_instruction_list_; }
  const StreamId& stream_id() const { return stream_id_.key(); }

  // Setters
  void set_max_device_num_per_machine(int64_t val) { max_device_num_per_machine_ = val; }
  ThreadCtx* mut_thread_ctx() { return thread_ctx_; }
  ThreadCtx* mutable_thread_ctx() { return thread_ctx_; }
  void set_thread_ctx(ThreadCtx* val) { thread_ctx_ = val; }
  void clear_thread_ctx() { thread_ctx_ = nullptr; }
  std::unique_ptr<DeviceCtx>* mut_device_ctx() { return &device_ctx_; }
  std::unique_ptr<DeviceCtx>* mutable_device_ctx() { return &device_ctx_; }
  InstructionList* mut_free_instruction_list() { return &free_instruction_list_; }
  InstructionList* mut_zombie_instruction_list() { return &zombie_instruction_list_; }
  InstructionList* mut_running_instruction_list() { return &running_instruction_list_; }
  InstructionList* mutable_free_instruction_list() { return &free_instruction_list_; }
  InstructionList* mutable_zombie_instruction_list() { return &zombie_instruction_list_; }
  InstructionList* mutable_running_instruction_list() { return &running_instruction_list_; }
  StreamId* mut_stream_id() { return stream_id_.mut_key(); }
  StreamId* mutable_stream_id() { return stream_id_.mut_key(); }

  // methods
  OF_PUBLIC void __Init__(ThreadCtx* thread_ctx, const StreamId& stream_id, const int64_t max_device_num_per_machine);
  OF_PUBLIC intrusive::SharedPtr<Instruction> NewInstruction(InstructionMsg* instr_msg, const std::shared_ptr<const ParallelDesc>& parallel_desc);
  OF_PUBLIC void DeleteInstruction(intrusive::SharedPtr<Instruction>&&);
  OF_PUBLIC int64_t global_device_id() const { return stream_id().global_device_id(); }
  OF_PUBLIC int64_t machine_id() const;
  OF_PUBLIC int64_t device_id() const;
  OF_PUBLIC const StreamType& stream_type() const;
  OF_PUBLIC const StreamTypeId& stream_type_id() const;
  OF_PRIVATE void MoveToFreeList(intrusive::SharedPtr<Instruction>&& instruction);
  OF_PRIVATE void MoveFromZombieListToFreeList();

  // fields
  INTRUSIVE_DEFINE_FIELD(ThreadCtx*, thread_ctx_); 
  INTRUSIVE_DEFINE_FIELD(std::unique_ptr<DeviceCtx>, device_ctx_);
  INTRUSIVE_DEFINE_FIELD(int64_t, max_device_num_per_machine_);
  
  // list entries
  INTRUSIVE_DEFINE_FIELD(intrusive::ListEntry, active_stream_entry_);
  INTRUSIVE_DEFINE_FIELD(intrusive::ListEntry, thread_ctx_stream_entry_);
  using StreamIdKey = intrusive::SkipListEntry<StreamId, 10>;
  INTRUSIVE_DEFINE_FIELD(StreamIdKey, stream_id_);

  // heads 
  INTRUSIVE_DEFINE_FIELD(InstructionList, free_instruction_list_);
  INTRUSIVE_DEFINE_FIELD(InstructionList, zombie_instruction_list_);
  INTRUSIVE_DEFINE_FIELD(InstructionList, running_instruction_list_);
INTRUSIVE_END(Stream);
// clang-format on

}  // namespace vm
}  // namespace oneflow

#endif  // ONEFLOW_CORE_VM_STREAM_H_
