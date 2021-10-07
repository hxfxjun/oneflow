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
#ifndef ONEFLOW_CORE_OBJECT_MSG_MUTEXED_LIST_H_
#define ONEFLOW_CORE_OBJECT_MSG_MUTEXED_LIST_H_

#include <mutex>
#include "oneflow/core/object_msg/list.h"

namespace oneflow {

namespace intrusive {

template<typename LinkField>
class MutexedList {
 public:
  using value_type = typename LinkField::struct_type;

  MutexedList(const MutexedList&) = delete;
  MutexedList(MutexedList&&) = delete;
  MutexedList() { this->__Init__(); }
  ~MutexedList() { this->Clear(); }

  std::size_t thread_unsafe_size() const { return list_head_.size(); }
  std::size_t size() const {
    std::unique_lock<std::mutex> lock(mutex_);
    return list_head_.size();
  }
  bool empty() const {
    std::unique_lock<std::mutex> lock(mutex_);
    return list_head_.empty();
  }

  void __Init__() {
    list_head_.__Init__();
    new (&this->mutex_) std::mutex();
  }

  void EmplaceBack(intrusive::SharedPtr<value_type>&& ptr) {
    std::unique_lock<std::mutex> lock(mutex_);
    return list_head_.EmplaceBack(std::move(ptr));
  }
  void EmplaceFront(intrusive::SharedPtr<value_type>&& ptr) {
    std::unique_lock<std::mutex> lock(mutex_);
    return list_head_.EmplaceFront(std::move(ptr));
  }
  void PushBack(value_type* ptr) { EmplaceBack(intrusive::SharedPtr<value_type>(ptr)); }
  void PushFront(value_type* ptr) { EmplaceFront(intrusive::SharedPtr<value_type>(ptr)); }
  intrusive::SharedPtr<value_type> PopBack() {
    std::unique_lock<std::mutex> lock(mutex_);
    return list_head_.PopBack();
  }
  intrusive::SharedPtr<value_type> PopFront() {
    std::unique_lock<std::mutex> lock(mutex_);
    return list_head_.PopFront();
  }

  void MoveFrom(List<LinkField>* src) {
    std::unique_lock<std::mutex> lock(mutex_);
    src->MoveToDstBack(&list_head_);
  }

  void MoveTo(List<LinkField>* dst) {
    std::unique_lock<std::mutex> lock(mutex_);
    list_head_.MoveToDstBack(dst);
  }

  void Clear() {
    std::unique_lock<std::mutex> lock(mutex_);
    list_head_.Clear();
  }

 private:
  List<LinkField> list_head_;
  mutable std::mutex mutex_;
};

}  // namespace intrusive

}  // namespace oneflow

#endif  // ONEFLOW_CORE_OBJECT_MSG_MUTEXED_LIST_H_
