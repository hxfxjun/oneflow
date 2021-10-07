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
#ifndef ONEFLOW_CORE_VM_SCHEDULER_MSG_H_
#define ONEFLOW_CORE_VM_SCHEDULER_MSG_H_

#include <mutex>
#include "oneflow/core/common/maybe.h"
#include "oneflow/core/vm/interpret_type.h"
#include "oneflow/core/vm/instruction.h"
#include "oneflow/core/vm/stream.h"
#include "oneflow/core/vm/stream_runtime_desc.h"
#include "oneflow/core/vm/thread_ctx.h"
#include "oneflow/core/vm/vm_object.h"
#include "oneflow/core/vm/vm_resource_desc.h"
#include "oneflow/core/common/range.h"
#include "oneflow/core/job/parallel_desc.h"
#include "oneflow/core/object_msg/mutexed_list.h"

namespace oneflow {

namespace vm {

struct VmDesc;
// clang-format off
OBJECT_MSG_BEGIN(VirtualMachine);
 public:
  // types
  using ActiveStreamList = intrusive::List<OBJECT_MSG_FIELD(Stream, active_stream_entry_)>;
  using ThreadCtxList = intrusive::List<OBJECT_MSG_FIELD(ThreadCtx, thread_ctx_entry_)>;
  using LogicalObjectDeleteList = intrusive::List<OBJECT_MSG_FIELD(LogicalObject, delete_entry_)>;
  using InstructionList = intrusive::List<OBJECT_MSG_FIELD(Instruction, instruction_entry_)>;
  using VmStatRunningInstructionList =
      intrusive::List<OBJECT_MSG_FIELD(Instruction, vm_stat_running_instruction_entry_)>;
  using FrontSeqInstructionList =
      intrusive::List<OBJECT_MSG_FIELD(Instruction, front_seq_compute_instr_entry_)>;
  using InstructionMsgMutextList =
      intrusive::MutexedList<OBJECT_MSG_FIELD(InstructionMsg, instr_msg_entry_)>;
  using StreamTypeId2StreamRtDesc = intrusive::SkipList<OBJECT_MSG_FIELD(StreamRtDesc, stream_type_id_)>;
  using Id2LogicalObject = intrusive::SkipList<OBJECT_MSG_FIELD(LogicalObject, logical_object_id_)>;

  // Getters
  const VmResourceDesc& vm_resource_desc() const {
    if (vm_resource_desc_) { return vm_resource_desc_.Get(); }
    static const auto default_val = intrusive::MakeShared<VmResourceDesc>();
    return default_val.Get();
  }
  const Range& machine_id_range() const { return machine_id_range_; }
  const std::atomic<int64_t>& flying_instruction_cnt() const { return flying_instruction_cnt_; }
  const ActiveStreamList& active_stream_list() const { return active_stream_list_; }
  const ThreadCtxList& thread_ctx_list() const { return thread_ctx_list_; }
  const LogicalObjectDeleteList& delete_logical_object_list() const { return delete_logical_object_list_; }
  const InstructionList& waiting_instruction_list() const { return waiting_instruction_list_; }
  const InstructionList& ready_instruction_list() const { return ready_instruction_list_; }
  const VmStatRunningInstructionList& vm_stat_running_instruction_list() const { return vm_stat_running_instruction_list_; }
  const FrontSeqInstructionList& front_seq_compute_instr_list() const { return front_seq_compute_instr_list_; }
  const InstructionMsgMutextList& pending_msg_list() const { return pending_msg_list_; }
  const StreamTypeId2StreamRtDesc& stream_type_id2stream_rt_desc() const { return stream_type_id2stream_rt_desc_; }
  const Id2LogicalObject& id2logical_object() const { return id2logical_object_; }
  //Setters
  VmResourceDesc* mut_vm_resource_desc() { return mutable_vm_resource_desc(); }
  VmResourceDesc* mutable_vm_resource_desc() {
    if (!vm_resource_desc_) { vm_resource_desc_ = intrusive::MakeShared<VmResourceDesc>(); }
    return vm_resource_desc_.Mutable();
  }
  Range* mut_machine_id_range() { return &machine_id_range_; }
  std::atomic<int64_t>* mut_flying_instruction_cnt() { return &flying_instruction_cnt_; }
  Range* mutable_machine_id_range() { return &machine_id_range_; }
  std::atomic<int64_t>* mutable_flying_instruction_cnt() { return &flying_instruction_cnt_; }
  ActiveStreamList* mut_active_stream_list() { return &active_stream_list_; }
  ThreadCtxList* mut_thread_ctx_list() { return &thread_ctx_list_; }
  LogicalObjectDeleteList* mut_delete_logical_object_list() { return &delete_logical_object_list_; }
  InstructionList* mut_waiting_instruction_list() { return &waiting_instruction_list_; }
  InstructionList* mut_ready_instruction_list() { return &ready_instruction_list_; }
  VmStatRunningInstructionList* mut_vm_stat_running_instruction_list() { return &vm_stat_running_instruction_list_; }
  FrontSeqInstructionList* mut_front_seq_compute_instr_list() { return &front_seq_compute_instr_list_; }
  ActiveStreamList* mutable_active_stream_list() { return &active_stream_list_; }
  ThreadCtxList* mutable_thread_ctx_list() { return &thread_ctx_list_; }
  LogicalObjectDeleteList* mutable_delete_logical_object_list() { return &delete_logical_object_list_; }
  InstructionList* mutable_waiting_instruction_list() { return &waiting_instruction_list_; }
  InstructionList* mutable_ready_instruction_list() { return &ready_instruction_list_; }
  VmStatRunningInstructionList* mutable_vm_stat_running_instruction_list() { return &vm_stat_running_instruction_list_; }
  FrontSeqInstructionList* mutable_front_seq_compute_instr_list() { return &front_seq_compute_instr_list_; }
  InstructionMsgMutextList* mut_pending_msg_list() { return &pending_msg_list_; }
  InstructionMsgMutextList* mutable_pending_msg_list() { return &pending_msg_list_; }
  StreamTypeId2StreamRtDesc* mut_stream_type_id2stream_rt_desc() { return &stream_type_id2stream_rt_desc_; }
  StreamTypeId2StreamRtDesc* mutable_stream_type_id2stream_rt_desc() { return &stream_type_id2stream_rt_desc_; }
  Id2LogicalObject* mut_id2logical_object() { return &id2logical_object_; }
  Id2LogicalObject* mutable_id2logical_object() { return &id2logical_object_; }

  // methods
  OF_PUBLIC void __Init__(const VmDesc& vm_desc);
  OF_PUBLIC Maybe<void> Receive(InstructionMsgList* instr_list);
  OF_PUBLIC Maybe<void> Receive(intrusive::SharedPtr<InstructionMsg>&& instruction_msg);
  OF_PUBLIC void Schedule();
  OF_PUBLIC bool ThreadUnsafeEmpty() const;
  OF_PUBLIC bool Empty() const;
  OF_PUBLIC Maybe<const ParallelDesc> GetInstructionParallelDesc(const InstructionMsg&);
  OF_PUBLIC MirroredObject* MutMirroredObject(int64_t logical_object_id, int64_t global_device_id);
  OF_PUBLIC const MirroredObject* GetMirroredObject(int64_t logical_object_id,
                                                 int64_t global_device_id);

  OF_PUBLIC int64_t this_machine_id() const;
  OF_PUBLIC int64_t this_start_global_device_id() const {
    return this_machine_id() * vm_resource_desc().max_device_num_per_machine();
  }

  // fields
  OBJECT_MSG_DEFINE_FIELD(intrusive::SharedPtr<VmResourceDesc>, vm_resource_desc_);
  OBJECT_MSG_DEFINE_FIELD(Range, machine_id_range_);
  OBJECT_MSG_DEFINE_FIELD(std::atomic<int64_t>, flying_instruction_cnt_);

  // heads
  OBJECT_MSG_DEFINE_FIELD(ActiveStreamList, active_stream_list_);
  OBJECT_MSG_DEFINE_FIELD(ThreadCtxList, thread_ctx_list_);
  OBJECT_MSG_DEFINE_FIELD(StreamTypeId2StreamRtDesc, stream_type_id2stream_rt_desc_);
  OBJECT_MSG_DEFINE_FIELD(Id2LogicalObject, id2logical_object_);
  OBJECT_MSG_DEFINE_FIELD(LogicalObjectDeleteList, delete_logical_object_list_);

  OBJECT_MSG_DEFINE_FIELD(InstructionMsgMutextList, pending_msg_list_);
  OBJECT_MSG_DEFINE_FIELD(InstructionList, waiting_instruction_list_);
  OBJECT_MSG_DEFINE_FIELD(InstructionList, ready_instruction_list_);
  OBJECT_MSG_DEFINE_FIELD(VmStatRunningInstructionList, vm_stat_running_instruction_list_);
  OBJECT_MSG_DEFINE_FIELD(FrontSeqInstructionList, front_seq_compute_instr_list_);

  // methods
 private:
  using TmpPendingInstrMsgList = intrusive::List<OBJECT_MSG_FIELD(InstructionMsg, instr_msg_entry_)>;
  using NewInstructionList = InstructionList;
  using PrescheduledInstructionList = InstructionList;
  using WaitingInstructionList = InstructionList;
  using ReadyInstructionList = InstructionList;

  template<typename ContainerT>
  void TryRunFrontSeqInstruction(ContainerT* front_seq_list,
                                        /*out*/ ReadyInstructionList* ready_instruction_list);
  void TryRunFrontSeqInstruction(/*out*/ ReadyInstructionList* ready_instruction_list);
  void ReleaseInstruction(Instruction* instruction,
                            /*out*/ ReadyInstructionList* ready_instruction_list);
  void TryReleaseFinishedInstructions(
          Stream* stream, /*out*/ ReadyInstructionList* ready_instruction_list);
  void FilterAndRunInstructionsInAdvance(TmpPendingInstrMsgList* instr_msg_list);
  void MakeInstructions(TmpPendingInstrMsgList*, /*out*/ NewInstructionList* ret_instruction_list);
  template<int64_t (*TransformLogicalObjectId)(int64_t), typename DoEachT>
  void ForEachMirroredObject(Id2LogicalObject* id2logical_object,
                             const Operand& operand,
                             int64_t global_device_id, const DoEachT& DoEach);
  template<OperandMemZoneModifier mem_zone_modifier, typename DoEachT>
  void ForEachConstMirroredObject(const InterpretType interpret_type,
                                  Id2LogicalObject* id2logical_object,
                                  const ModifiedOperand<kConstModifier, mem_zone_modifier>& const_operand,
                                  int64_t global_device_id, const DoEachT& DoEach);
  template<OperandMemZoneModifier mem_zone_modifier, typename DoEachT>
  void ForEachConstMirroredObject(const InterpretType interpret_type,
                                  Id2LogicalObject* id2logical_object,
                                  const ModifiedOperand<kDataMutableModifier, mem_zone_modifier>& mutable_operand,
                                  int64_t global_device_id, const DoEachT& DoEach);
  template<OperandMemZoneModifier mem_zone_modifier, typename DoEachT>
  void ForEachMutMirroredObject(const InterpretType interpret_type,
                                Id2LogicalObject* id2logical_object,
                                const ModifiedOperand<kDataMutableModifier, mem_zone_modifier>& mutable_operand,
                                int64_t global_device_id, const DoEachT& DoEach);
  template<OperandMemZoneModifier mem_zone_modifier, typename DoEachT>
  void ForEachMutMirroredObject(const InterpretType interpret_type,
                                Id2LogicalObject* id2logical_object,
                                const ModifiedOperand<kTypeAndDataMutableModifier, mem_zone_modifier>& mut2_operand,
                                int64_t global_device_id, const DoEachT& DoEach);

  template<OperandMemZoneModifier mem_zone_modifier, typename DoEachT>
  void ForEachMutMirroredObject(const InterpretType interpret_type,
                                Id2LogicalObject* id2logical_object,
                                const ModifiedOperand<kDeleteModifier, mem_zone_modifier>& mut2_operand,
                                int64_t global_device_id, const DoEachT& DoEach);

  void ConnectInstruction(Instruction* src_instruction, Instruction* dst_instruction);
  RwMutexedObjectAccess* ConsumeMirroredObject(OperandAccessType access_type, MirroredObject* mirrored_object,
                             Instruction* instrution);
  void ConsumeMirroredObjects(Id2LogicalObject* id2logical_object,
                              NewInstructionList* new_instruction_list);
  void FilterReadyInstructions(NewInstructionList* new_instruction_list,
                         /*out*/ ReadyInstructionList* ready_instruction_list);
  void DispatchAndPrescheduleInstructions(ReadyInstructionList* ready_instruction_list);

  template<typename ReadyList, typename IsEdgeReadyT>
  void TryMoveWaitingToReady(Instruction* instruction, ReadyList* ready_list,
                             const IsEdgeReadyT& IsEdgeReady);

  void TryDeleteLogicalObjects();

OBJECT_MSG_END(VirtualMachine);
// clang-format on

}  // namespace vm

}  // namespace oneflow

#endif  // ONEFLOW_CORE_VM_SCHEDULER_MSG_H_
