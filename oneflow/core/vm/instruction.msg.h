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
#ifndef ONEFLOW_CORE_VM_VPU_INSTRUCTION_MSG_H_
#define ONEFLOW_CORE_VM_VPU_INSTRUCTION_MSG_H_

#include <cstring>
#include <mutex>
#include "oneflow/core/job/parallel_desc.h"
#include "oneflow/core/object_msg/flat_msg.h"
#include "oneflow/core/object_msg/object_msg.h"
#include "oneflow/core/vm/stream_desc.msg.h"
#include "oneflow/core/vm/vm_object.msg.h"
#include "oneflow/core/vm/stream_type.h"
#include "oneflow/core/vm/instr_type_id.h"
#include "oneflow/core/vm/id_util.h"
#include "oneflow/core/vm/interpret_type.h"
#include "oneflow/core/vm/instruction_operand.msg.h"
#include "oneflow/core/vm/instruction.pb.h"
#include "oneflow/core/vm/instruction.cfg.h"

namespace oneflow {
namespace vm {

// clang-format off
OBJECT_MSG_BEGIN(InstructionOperandList);
 public:
  void __Init__() {}
  // Getters
  const std::vector<FlatMsg<InstructionOperand>>& operand() const { return operand_; }
  // Setters
  std::vector<FlatMsg<InstructionOperand>>* mut_operand() { return &operand_; }
  std::vector<FlatMsg<InstructionOperand>>* mutable_operand() { return &operand_; }

  OBJECT_MSG_FIELD(std::vector<FlatMsg<InstructionOperand>>, operand_);
OBJECT_MSG_END(InstructionOperandList);

OBJECT_MSG_BEGIN(InstructionMsg);
 public:
  // Getters
  bool has_parallel_desc_symbol_id() const { return 0 != parallel_desc_symbol_id_; }
  int64_t parallel_desc_symbol_id() const { return parallel_desc_symbol_id_; }
  const InstructionOperandList& operand_list() const {
    if (operand_list_) { return operand_list_.Get(); }
    static const auto default_val = ObjectMsgPtr<InstructionOperandList>::New();
    return default_val.Get();
  }
  const std::string& instr_type_name() const { return instr_type_name_; }
  const InstrTypeId& instr_type_id() const { return instr_type_id_; }
  const std::shared_ptr<const ParallelDesc>& parallel_desc() const { return parallel_desc_; }
  const std::shared_ptr<PhyInstrOperand>& phy_instr_operand() const { return phy_instr_operand_; }
  // Setters
  void set_parallel_desc_symbol_id(int64_t val) { parallel_desc_symbol_id_ = val; }
  InstructionOperandList* mut_operand_list() { return mutable_operand_list(); }
  InstructionOperandList* mutable_operand_list() {
    if (!operand_list_) { operand_list_ = ObjectMsgPtr<InstructionOperandList>::New(); }
    return operand_list_.Mutable();
  }
  void reset_operand_list(const InstructionOperandList& other) {
    operand_list_.Reset(const_cast<InstructionOperandList*>(&other));
  }
  std::string* mut_instr_type_name() { return &instr_type_name_; }
  std::string* mutable_instr_type_name() { return &instr_type_name_; }
  InstrTypeId* mut_instr_type_id() { return &instr_type_id_; }
  InstrTypeId* mutable_instr_type_id() { return &instr_type_id_; }
  std::shared_ptr<const ParallelDesc>* mut_parallel_desc() { return &parallel_desc_; }
  std::shared_ptr<const ParallelDesc>* mutable_parallel_desc() { return &parallel_desc_; }
  std::shared_ptr<PhyInstrOperand>* mut_phy_instr_operand() { return &phy_instr_operand_; }
  std::shared_ptr<PhyInstrOperand>* mutable_phy_instr_operand() { return &phy_instr_operand_; }

  // methods
  OF_PUBLIC void __Init__();
  OF_PUBLIC void __Init__(const std::string& instr_type_name);
  OF_PUBLIC void __Init__(const InstructionProto& proto);
  OF_PUBLIC void __Init__(const cfg::InstructionProto& proto); 
  OF_PUBLIC void __Init__(const InstructionMsg& instr_msg);

  OF_PUBLIC void ToProto(InstructionProto* proto) const;
  OF_PUBLIC ObjectMsgPtr<InstructionMsg> add_parallel_desc(int64_t symbol_id);
  OF_PUBLIC ObjectMsgPtr<InstructionMsg> add_double_operand(double double_operand);
  OF_PUBLIC ObjectMsgPtr<InstructionMsg> add_int64_operand(int64_t int64_operand);
  OF_PUBLIC ObjectMsgPtr<InstructionMsg> add_uint64_operand(uint64_t uint64_operand);
  OF_PUBLIC ObjectMsgPtr<InstructionMsg> add_bool_operand(bool bool_operand);
  OF_PUBLIC ObjectMsgPtr<InstructionMsg> add_separator();
  OF_PUBLIC ObjectMsgPtr<InstructionMsg> add_const_operand(ObjectId logical_object_id);
  OF_PUBLIC ObjectMsgPtr<InstructionMsg> add_const_operand(ObjectId logical_object_id, const SoleMirroredObject&);
  OF_PUBLIC ObjectMsgPtr<InstructionMsg> add_const_operand(ObjectId logical_object_id, const AllMirroredObject&);
  OF_PUBLIC ObjectMsgPtr<InstructionMsg> add_symbol_operand(ObjectId logical_object_id);
  OF_PUBLIC ObjectMsgPtr<InstructionMsg> add_mut_operand(ObjectId logical_object_id);
  OF_PUBLIC ObjectMsgPtr<InstructionMsg> add_mut_operand(ObjectId logical_object_id, const SoleMirroredObject&);
  OF_PUBLIC ObjectMsgPtr<InstructionMsg> add_mut_operand(ObjectId logical_object_id, const AllMirroredObject&);
  OF_PUBLIC ObjectMsgPtr<InstructionMsg> add_init_symbol_operand(ObjectId logical_object_id);
  OF_PUBLIC ObjectMsgPtr<InstructionMsg> add_mut2_operand(ObjectId logical_object_id);
  OF_PUBLIC ObjectMsgPtr<InstructionMsg> add_mut2_operand(ObjectId logical_object_id, const SoleMirroredObject&);
  OF_PUBLIC ObjectMsgPtr<InstructionMsg> add_mut2_operand(ObjectId logical_object_id, const AllMirroredObject&);
  OF_PUBLIC ObjectMsgPtr<InstructionMsg> add_del_operand(ObjectId logical_object_id);
  OF_PUBLIC const std::vector<FlatMsg<InstructionOperand>>& operand() const {
    return operand_list().operand();
  }
  OF_PUBLIC std::vector<FlatMsg<InstructionOperand>>* mut_operand() {
    return mut_operand_list()->mut_operand();
  }
  OF_PUBLIC std::vector<FlatMsg<InstructionOperand>>* mutable_operand() {
    return mutable_operand_list()->mut_operand();
  }
  OF_PUBLIC ObjectMsgPtr<InstructionMsg> Clone() const;
  OF_PUBLIC ObjectMsgPtr<InstructionMsg> MakeInferInstrMsg() const;

  // fields
  OBJECT_MSG_FIELD(InstrTypeId, instr_type_id_);
  // instr_type_name is a necessary reduandant field for method ToProto
  OBJECT_MSG_FIELD(std::string, instr_type_name_);
  OBJECT_MSG_FIELD(int64_t, parallel_desc_symbol_id_);
  OBJECT_MSG_FIELD(std::shared_ptr<const ParallelDesc>, parallel_desc_);
  OBJECT_MSG_FIELD(ObjectMsgPtr<InstructionOperandList>, operand_list_);
  OBJECT_MSG_FIELD(std::shared_ptr<PhyInstrOperand>, phy_instr_operand_);


  // list entries
  OBJECT_MSG_DEFINE_LIST_LINK(instr_msg_entry);

  // private methods
  OF_PRIVATE InstructionOperand* add_instr_operand();
OBJECT_MSG_END(InstructionMsg);
// clang-format on

using InstructionMsgList = OBJECT_MSG_LIST(InstructionMsg, instr_msg_entry);

template<OperandMemZoneModifier mem_zone_modifier>
void CheckOperand(const Operand& operand);

static const int kInstructionStatusBufferBytes = 32;

// clang-format off
FLAT_MSG_BEGIN(InstructionDeleted);
FLAT_MSG_END(InstructionDeleted);

FLAT_MSG_BEGIN(InstructionStatusBuffer);
  FLAT_MSG_DEFINE_OPTIONAL(InstructionDeleted, instruction_deleted);
  FLAT_MSG_DEFINE_REPEATED(char, buffer, kInstructionStatusBufferBytes);
FLAT_MSG_END(InstructionStatusBuffer);
// clang-format on

struct Instruction;
// clang-format off
OBJECT_MSG_BEGIN(InstructionEdge);
 public:
  void __Init__() {
    clear_src_instruction();
    clear_dst_instruction();
  }
  // Getters
  bool has_src_instruction() const { return src_instruction_ != nullptr; } 
  bool has_dst_instruction() const { return dst_instruction_ != nullptr; } 
  const Instruction& src_instruction() const { return *src_instruction_; }
  const Instruction& dst_instruction() const { return *dst_instruction_; } 
  // Setters
  void set_src_instruction(Instruction* val) { src_instruction_ = val; } 
  void set_dst_instruction(Instruction* val) { dst_instruction_ = val; } 
  void clear_src_instruction() { src_instruction_ = nullptr; } 
  void clear_dst_instruction() { dst_instruction_ = nullptr; } 
  Instruction* mut_src_instruction() { return src_instruction_; } 
  Instruction* mut_dst_instruction() { return dst_instruction_; } 
  Instruction* mutable_src_instruction() { return src_instruction_; } 
  Instruction* mutable_dst_instruction() { return dst_instruction_; } 
  // methods
  OF_PUBLIC void __Init__(Instruction* src_instruction, Instruction* dst_instruction) {
    __Init__();
    set_src_instruction(src_instruction);
    set_dst_instruction(dst_instruction);
  }

  // fields
  OBJECT_MSG_FIELD(Instruction*, src_instruction_); 
  OBJECT_MSG_FIELD(Instruction*, dst_instruction_); 
  // list entries
  OBJECT_MSG_DEFINE_LIST_LINK(src_instruction_entry);
  OBJECT_MSG_DEFINE_LIST_LINK(dst_instruction_entry);
OBJECT_MSG_END(InstructionEdge);
// clang-format on

struct Stream;
// clang-format off
OBJECT_MSG_BEGIN(Instruction);
 public:
  // Getters
  void __Init__() { clear_stream(); }
  bool has_stream() const { return stream_ != nullptr;  }
  const Stream& stream() const { return *stream_;  }
  const InstructionMsg& instr_msg() const {
    if (instr_msg_) { return instr_msg_.Get(); }
    static const auto default_val = ObjectMsgPtr<InstructionMsg>::New();
    return default_val.Get();
  }
  const std::shared_ptr<const ParallelDesc>& parallel_desc() const { return parallel_desc_; }
  const InstructionStatusBuffer& status_buffer() const { return status_buffer_.Get(); }
  // Setters
  void set_stream(Stream* val) { stream_ = val; }
  void clear_stream() { stream_ = nullptr; }
  Stream* mut_stream() { return stream_; }
  Stream* mutable_stream() { return stream_; }
  InstructionMsg* mut_instr_msg() { return mutable_instr_msg(); }
  InstructionMsg* mutable_instr_msg() {
    if (!instr_msg_) { instr_msg_ = ObjectMsgPtr<InstructionMsg>::New(); }
    return instr_msg_.Mutable();
  }
  void reset_instr_msg(InstructionMsg* instr_msg) { instr_msg_.Reset(instr_msg); }
  void clear_instr_msg() { instr_msg_.Reset(); }
  std::shared_ptr<const ParallelDesc>* mut_parallel_desc() { return &parallel_desc_; }
  std::shared_ptr<const ParallelDesc>* mutable_parallel_desc() { return &parallel_desc_; }
  InstructionStatusBuffer* mut_status_buffer() { return status_buffer_.Mutable(); }
  InstructionStatusBuffer* mutable_status_buffer() { return status_buffer_.Mutable(); }

  // methods
  OF_PUBLIC void __Init__(InstructionMsg* instr_msg, Stream* stream, const std::shared_ptr<const ParallelDesc>& parallel_desc);
  OF_PUBLIC void __Delete__();
  OF_PUBLIC bool Done() const;
  OF_PUBLIC void set_has_event_record(bool val);
  OF_PUBLIC const StreamType& stream_type() const;

  OF_PUBLIC template<OperandMemZoneModifier mem_zone_modifier>
      const RwMutexedObject* operand_type(const Operand& operand) const {
    CheckOperand<mem_zone_modifier>(operand);
    return operand_type(operand, GetOperandDefaultGlobalDeviceId());
  }
  OF_PUBLIC template<OperandMemZoneModifier mem_zone_modifier>
      const RwMutexedObject* operand_value(const Operand& operand) const {
    CheckOperand<mem_zone_modifier>(operand);
    return operand_value(operand, GetOperandDefaultGlobalDeviceId());
  }
  OF_PUBLIC template<OperandMemZoneModifier mem_zone_modifier>
      RwMutexedObject* mut_operand_type(const Operand& operand) {
    CheckOperand<mem_zone_modifier>(operand);
    return mut_operand_type(operand, GetOperandDefaultGlobalDeviceId());
  }
  OF_PUBLIC template<OperandMemZoneModifier mem_zone_modifier>
      RwMutexedObject* mut_operand_value(const Operand& operand) {
    CheckOperand<mem_zone_modifier>(operand);
    return mut_operand_value(operand, GetOperandDefaultGlobalDeviceId());
  }

  OF_PUBLIC template<OperandAccessModifier access_modifier, OperandMemZoneModifier mem_zone_modifier>
  const RwMutexedObject* operand_type(
      const ModifiedOperand<access_modifier, mem_zone_modifier>& operand) const {
    return operand_type<mem_zone_modifier>(operand.operand());
  }
  OF_PUBLIC template<OperandAccessModifier access_modifier, OperandMemZoneModifier mem_zone_modifier>
  const RwMutexedObject* operand_value(
      const ModifiedOperand<access_modifier, mem_zone_modifier>& operand) const {
    return operand_value<mem_zone_modifier>(operand.operand());
  }
  OF_PUBLIC template<OperandAccessModifier access_modifier, OperandMemZoneModifier mem_zone_modifier>
  RwMutexedObject* mut_operand_type(
      const ModifiedOperand<access_modifier, mem_zone_modifier>& operand) {
    return mut_operand_type<mem_zone_modifier>(operand.operand());
  }
  OF_PUBLIC template<OperandAccessModifier access_modifier, OperandMemZoneModifier mem_zone_modifier>
  RwMutexedObject* mut_operand_value(
      const ModifiedOperand<access_modifier, mem_zone_modifier>& operand) {
    return mut_operand_value<mem_zone_modifier>(operand.operand());
  }

  OF_PUBLIC template<InterpretType interpret_type>
         MirroredObject* MutMirroredObject(const MutOperand& mut_operand) {
    return MirroredObjectUtil<interpret_type>::Mut(this, mut_operand);
  }

  OF_PUBLIC template<InterpretType interpret_type>
         const MirroredObject* GetMirroredObject(const ConstOperand& const_operand) const {
    return MirroredObjectUtil<interpret_type>::Get(*this, const_operand);
  }

  OF_PUBLIC MirroredObject* mut_type_mirrored_object(const MutOperand& mut_operand);
  OF_PUBLIC MirroredObject* mut_value_mirrored_object(const MutOperand& mut_operand);

  // private methods
  OF_PRIVATE template<int64_t(*TransformLogicalObjectId)(int64_t)>
          MirroredObject* MutMirroredObject(const Operand& operand,
                                            int64_t default_global_device_id);
  OF_PRIVATE template<int64_t(*TransformLogicalObjectId)(int64_t)>
          const MirroredObject* GetMirroredObject(const Operand& operand,
                                                  int64_t default_global_device_id) const;
  OF_PRIVATE const RwMutexedObject* operand_type(const Operand& operand,
                                              int64_t default_global_device_id) const;
  OF_PRIVATE const RwMutexedObject* operand_value(const Operand& operand,
                                               int64_t default_global_device_id) const;
  OF_PRIVATE RwMutexedObject* mut_operand_type(const Operand& operand,
                                            int64_t default_global_device_id);
  OF_PRIVATE RwMutexedObject* mut_operand_value(const Operand& operand,  
                                             int64_t default_global_device_id);

  OF_PRIVATE MirroredObject* MutMirroredObject(const Operand& operand,
                                                     int64_t default_global_device_id) {
    return MutMirroredObject<&IdUtil::GetValueId>(operand, default_global_device_id);
  }

  OF_PRIVATE int64_t GetOperandDefaultGlobalDeviceId() const;

  template<InterpretType interpret_type>
  struct MirroredObjectUtil {
    static const MirroredObject* Get(const Instruction&, const ConstOperand&);
    static MirroredObject* Mut(Instruction*, const MutOperand&);
  };

  // fields
  OBJECT_MSG_FIELD(FlatMsg<InstructionStatusBuffer>, status_buffer_);
  OBJECT_MSG_FIELD(ObjectMsgPtr<InstructionMsg>, instr_msg_);
  OBJECT_MSG_FIELD(std::shared_ptr<const ParallelDesc>, parallel_desc_);
  OBJECT_MSG_FIELD(Stream*, stream_); 

  // list entries
  OBJECT_MSG_DEFINE_LIST_LINK(instruction_entry);
  // `vm_stat_running_instruction_entry` valid from instruction ready to instruction done 
  OBJECT_MSG_DEFINE_LIST_LINK(vm_stat_running_instruction_entry);
  OBJECT_MSG_DEFINE_LIST_LINK(pending_instruction_entry);
  OBJECT_MSG_DEFINE_LIST_LINK(front_seq_infer_instr_entry);
  OBJECT_MSG_DEFINE_LIST_LINK(front_seq_compute_instr_entry);
  OBJECT_MSG_DEFINE_LIST_HEAD(InstructionEdge, src_instruction_entry, in_edges);
  OBJECT_MSG_DEFINE_LIST_HEAD(InstructionEdge, dst_instruction_entry, out_edges);
  OBJECT_MSG_DEFINE_SKIPLIST_HEAD(RwMutexedObjectAccess, mirrored_object_id, mirrored_object_id2access);
  OBJECT_MSG_DEFINE_LIST_HEAD(RwMutexedObjectAccess, instruction_access_entry, access_list);
OBJECT_MSG_END(Instruction);
// clang-format on

}  // namespace vm
}  // namespace oneflow

#endif  // ONEFLOW_CORE_VM_VPU_INSTRUCTION_MSG_H_
