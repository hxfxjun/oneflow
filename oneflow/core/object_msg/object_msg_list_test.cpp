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
// include sstream first to avoid some compiling error
// caused by the following trick
// reference: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=65899
#include <sstream>
#define private public
#include "oneflow/core/common/util.h"
#include "oneflow/core/object_msg/object_msg.h"

namespace oneflow {

namespace test {

namespace {

// clang-format off
OBJECT_MSG_BEGIN(TestListItem)
 public:
  void __Init__() { clear_cnt(); }
  // Getters
  bool has_cnt() const { return cnt_ != nullptr; }
  int cnt() const { return *cnt_; }
  bool is_foo_list_empty() const { return foo_list_.empty(); }

  // Setters
  void set_cnt(int* val) { cnt_ = val; }
  void clear_cnt() { cnt_ = nullptr; }
  int* mut_cnt() { return cnt_; }
  int* mutable_cnt() { return cnt_; }

  OBJECT_MSG_DEFINE_FIELD(intrusive::ListEntry, foo_list_);
  OBJECT_MSG_DEFINE_FIELD(int*, cnt_);

 public:
  void __Delete__() {
    if (has_cnt()) { --*mutable_cnt(); }
  }
OBJECT_MSG_END(TestListItem)
// clang-format on

TEST(List, empty) {
  OBJECT_MSG_LIST(TestListItem, foo_list) foo_list;
  ASSERT_TRUE(foo_list.empty());
  ASSERT_EQ(foo_list.size(), 0);
}

TEST(List, empty_Begin) {
  OBJECT_MSG_LIST(TestListItem, foo_list) foo_list;
  ObjectMsgPtr<TestListItem> obj_ptr;
  obj_ptr = foo_list.Begin();
  ASSERT_TRUE(!obj_ptr);
  ObjectMsgPtr<TestListItem> next;
  obj_ptr = foo_list.Begin();
  next = foo_list.Next(obj_ptr.Mutable());
  ASSERT_TRUE(!obj_ptr);
}

TEST(List, empty_Next) {
  OBJECT_MSG_LIST(TestListItem, foo_list) foo_list;
  ObjectMsgPtr<TestListItem> obj_ptr;
  ObjectMsgPtr<TestListItem> next;
  obj_ptr = foo_list.Begin();
  next = foo_list.Next(obj_ptr.Mutable());
  ASSERT_TRUE(!obj_ptr);
  ASSERT_TRUE(!next);
  obj_ptr = foo_list.Next(obj_ptr.Mutable());
  ASSERT_TRUE(!obj_ptr);
  obj_ptr = next;
  next = foo_list.Next(next.Mutable());
  ASSERT_TRUE(!obj_ptr);
  ASSERT_TRUE(!next);
}

TEST(List, PushFront) {
  OBJECT_MSG_LIST(TestListItem, foo_list) foo_list;
  auto item0 = ObjectMsgPtr<TestListItem>::New();
  auto item1 = ObjectMsgPtr<TestListItem>::New();
  foo_list.PushFront(item0.Mutable());
  foo_list.PushFront(item1.Mutable());
  ObjectMsgPtr<TestListItem> obj_ptr;
  ObjectMsgPtr<TestListItem> next;
  obj_ptr = foo_list.Begin();
  next = foo_list.Next(obj_ptr.Mutable());
  ASSERT_TRUE(obj_ptr == item1);
  ASSERT_TRUE(next == item0);
}

TEST(List, destructor) {
  int elem_cnt = 2;
  {
    OBJECT_MSG_LIST(TestListItem, foo_list) foo_list;
    auto item0 = ObjectMsgPtr<TestListItem>::New();
    item0->set_cnt(&elem_cnt);
    auto item1 = ObjectMsgPtr<TestListItem>::New();
    item1->set_cnt(&elem_cnt);
    foo_list.PushFront(item0.Mutable());
    foo_list.PushFront(item1.Mutable());
  }
  ASSERT_EQ(elem_cnt, 0);
  elem_cnt = 2;
  auto item0 = ObjectMsgPtr<TestListItem>::New();
  {
    OBJECT_MSG_LIST(TestListItem, foo_list) foo_list;
    item0->set_cnt(&elem_cnt);
    auto item1 = ObjectMsgPtr<TestListItem>::New();
    item1->set_cnt(&elem_cnt);
    foo_list.PushFront(item0.Mutable());
    foo_list.PushFront(item1.Mutable());
  }
  ASSERT_EQ(elem_cnt, 1);
}

TEST(List, PushBack) {
  OBJECT_MSG_LIST(TestListItem, foo_list) foo_list;
  auto item0 = ObjectMsgPtr<TestListItem>::New();
  auto item1 = ObjectMsgPtr<TestListItem>::New();
  foo_list.PushBack(item0.Mutable());
  foo_list.PushBack(item1.Mutable());
  ObjectMsgPtr<TestListItem> obj_ptr;
  ObjectMsgPtr<TestListItem> next;
  obj_ptr = foo_list.Begin();
  next = foo_list.Next(obj_ptr.Mutable());
  ASSERT_TRUE(obj_ptr == item0);
  ASSERT_TRUE(next == item1);
}

TEST(List, Erase) {
  OBJECT_MSG_LIST(TestListItem, foo_list) foo_list;
  auto item0 = ObjectMsgPtr<TestListItem>::New();
  auto item1 = ObjectMsgPtr<TestListItem>::New();
  foo_list.PushBack(item0.Mutable());
  foo_list.PushBack(item1.Mutable());
  ASSERT_EQ(item1->ref_cnt(), 2);
  foo_list.Erase(item1.Mutable());
  ASSERT_EQ(item1->ref_cnt(), 1);
  ObjectMsgPtr<TestListItem> obj_ptr;
  ObjectMsgPtr<TestListItem> next;
  obj_ptr = foo_list.Begin();
  next = foo_list.Next(obj_ptr.Mutable());
  ASSERT_TRUE(obj_ptr == item0);
  ASSERT_TRUE(!next);
}

TEST(List, PopBack) {
  OBJECT_MSG_LIST(TestListItem, foo_list) foo_list;
  auto item0 = ObjectMsgPtr<TestListItem>::New();
  auto item1 = ObjectMsgPtr<TestListItem>::New();
  foo_list.PushBack(item0.Mutable());
  foo_list.PushBack(item1.Mutable());
  ASSERT_EQ(item1->ref_cnt(), 2);
  foo_list.PopBack();
  ASSERT_EQ(item1->ref_cnt(), 1);
  ObjectMsgPtr<TestListItem> obj_ptr;
  ObjectMsgPtr<TestListItem> next;
  obj_ptr = foo_list.Begin();
  next = foo_list.Next(obj_ptr.Mutable());
  ASSERT_TRUE(obj_ptr == item0);
  ASSERT_TRUE(!next);
}

TEST(List, PopFront) {
  OBJECT_MSG_LIST(TestListItem, foo_list) foo_list;
  auto item0 = ObjectMsgPtr<TestListItem>::New();
  auto item1 = ObjectMsgPtr<TestListItem>::New();
  foo_list.PushBack(item0.Mutable());
  foo_list.PushBack(item1.Mutable());
  ASSERT_EQ(item0->ref_cnt(), 2);
  foo_list.PopFront();
  ASSERT_EQ(item0->ref_cnt(), 1);
  ObjectMsgPtr<TestListItem> obj_ptr;
  ObjectMsgPtr<TestListItem> next;
  obj_ptr = foo_list.Begin();
  next = foo_list.Next(obj_ptr.Mutable());
  ASSERT_TRUE(!next);
}

TEST(List, Clear) {
  OBJECT_MSG_LIST(TestListItem, foo_list) foo_list;
  auto item0 = ObjectMsgPtr<TestListItem>::New();
  auto item1 = ObjectMsgPtr<TestListItem>::New();
  foo_list.PushBack(item0.Mutable());
  foo_list.PushBack(item1.Mutable());
  ASSERT_EQ(item0->ref_cnt(), 2);
  ASSERT_EQ(item1->ref_cnt(), 2);
  foo_list.Clear();
  ASSERT_TRUE(foo_list.empty());
  ASSERT_EQ(item0->ref_cnt(), 1);
  ASSERT_EQ(item1->ref_cnt(), 1);
}

TEST(List, UNSAFE_FOR_EACH_PTR) {
  OBJECT_MSG_LIST(TestListItem, foo_list) foo_list;
  auto item0 = ObjectMsgPtr<TestListItem>::New();
  auto item1 = ObjectMsgPtr<TestListItem>::New();
  foo_list.PushBack(item0.Mutable());
  foo_list.PushBack(item1.Mutable());
  int i = 0;
  OBJECT_MSG_LIST_UNSAFE_FOR_EACH_PTR(&foo_list, item) {
    if (i == 0) {
      ASSERT_TRUE(item == item0.Mutable());
    } else if (i == 1) {
      ASSERT_TRUE(item == item1.Mutable());
    }
    ++i;
  }
  ASSERT_EQ(i, 2);
}

TEST(List, FOR_EACH) {
  OBJECT_MSG_LIST(TestListItem, foo_list) foo_list;
  auto item0 = ObjectMsgPtr<TestListItem>::New();
  auto item1 = ObjectMsgPtr<TestListItem>::New();
  foo_list.PushBack(item0.Mutable());
  foo_list.PushBack(item1.Mutable());
  int i = 0;
  OBJECT_MSG_LIST_FOR_EACH(&foo_list, item) {
    if (i == 0) {
      ASSERT_TRUE(item == item0);
      foo_list.Erase(item.Mutable());
    } else if (i == 1) {
      ASSERT_TRUE(item == item1);
      foo_list.Erase(item.Mutable());
    }
    ++i;
  }
  ASSERT_EQ(i, 2);
  ASSERT_TRUE(foo_list.empty());
  ASSERT_EQ(item0->ref_cnt(), 1);
  ASSERT_EQ(item1->ref_cnt(), 1);
}

// clang-format off
OBJECT_MSG_BEGIN(TestObjectMsgListHead);
 public:
  TestObjectMsgListHead() = default;
  using FooList = intrusive::List<OBJECT_MSG_FIELD(TestListItem, foo_list_)>;
  // Getters
  const FooList& foo_list() const { return foo_list_; }
  // Setters
  FooList* mut_foo_list() { return &foo_list_; }
  FooList* mutable_foo_list() { return &foo_list_; }

  OBJECT_MSG_DEFINE_FIELD(FooList, foo_list_);
OBJECT_MSG_END(TestObjectMsgListHead);
// clang-format on

TEST(List, object_msg_define_list_head) {
  auto foo_list_head = ObjectMsgPtr<TestObjectMsgListHead>::New();
  auto& foo_list = *foo_list_head->mutable_foo_list();
  auto item0 = ObjectMsgPtr<TestListItem>::New();
  auto item1 = ObjectMsgPtr<TestListItem>::New();
  foo_list.PushBack(item0.Mutable());
  foo_list.PushBack(item1.Mutable());
  ASSERT_EQ(item0->ref_cnt(), 2);
  ASSERT_EQ(item1->ref_cnt(), 2);
  int i = 0;
  OBJECT_MSG_LIST_FOR_EACH(&foo_list, item) {
    if (i == 0) {
      ASSERT_TRUE(item == item0);
      foo_list.Erase(item.Mutable());
    } else if (i == 1) {
      ASSERT_TRUE(item == item1);
      foo_list.Erase(item.Mutable());
    }
    ++i;
  }
  ASSERT_EQ(i, 2);
  ASSERT_TRUE(foo_list.empty());
  ASSERT_EQ(item0->ref_cnt(), 1);
  ASSERT_EQ(item1->ref_cnt(), 1);
}

// clang-format off
OBJECT_MSG_BEGIN(TestObjectMsgListHeadWrapper);
 public:
  // Getters
  const TestObjectMsgListHead& head() const {
    if (head_) { return head_.Get(); }
    static const auto default_val = ObjectMsgPtr<TestObjectMsgListHead>::New();
    return default_val.Get();
  }
  // Setters
  TestObjectMsgListHead* mut_head() { return mutable_head(); }
  TestObjectMsgListHead* mutable_head() {
    if (!head_) { head_ = ObjectMsgPtr<TestObjectMsgListHead>::New(); }
    return head_.Mutable();
  }
  void clear_head() {
    if (head_) { head_.Reset(); }
  }

  OBJECT_MSG_DEFINE_FIELD(ObjectMsgPtr<TestObjectMsgListHead>, head_);
OBJECT_MSG_END(TestObjectMsgListHeadWrapper);
// clang-format on

TEST(List, nested_list_delete) {
  auto foo_list_head = ObjectMsgPtr<TestObjectMsgListHeadWrapper>::New();
  auto& foo_list = *foo_list_head->mutable_head()->mutable_foo_list();
  auto item0 = ObjectMsgPtr<TestListItem>::New();
  auto item1 = ObjectMsgPtr<TestListItem>::New();
  foo_list.PushBack(item0.Mutable());
  foo_list.PushBack(item1.Mutable());
  ASSERT_EQ(item0->ref_cnt(), 2);
  ASSERT_EQ(item1->ref_cnt(), 2);
  int i = 0;
  OBJECT_MSG_LIST_UNSAFE_FOR_EACH_PTR(&foo_list, item) {
    if (i == 0) {
      ASSERT_TRUE(item == item0.Mutable());
    } else if (i == 1) {
      ASSERT_TRUE(item == item1.Mutable());
    }
    ++i;
  }
  ASSERT_EQ(i, 2);
  foo_list_head->clear_head();
  ASSERT_EQ(item0->ref_cnt(), 1);
  ASSERT_EQ(item1->ref_cnt(), 1);
}

TEST(List, MoveTo) {
  OBJECT_MSG_LIST(TestListItem, foo_list) foo_list;
  OBJECT_MSG_LIST(TestListItem, foo_list) foo_list0;
  auto item0 = ObjectMsgPtr<TestListItem>::New();
  auto item1 = ObjectMsgPtr<TestListItem>::New();
  ASSERT_EQ(item0->is_foo_list_empty(), true);
  ASSERT_EQ(item1->is_foo_list_empty(), true);
  foo_list.PushBack(item0.Mutable());
  foo_list.PushBack(item1.Mutable());
  ASSERT_EQ(item0->is_foo_list_empty(), false);
  ASSERT_EQ(item1->is_foo_list_empty(), false);
  ASSERT_EQ(foo_list.size(), 2);
  ASSERT_EQ(foo_list0.empty(), true);
  ASSERT_EQ(item0->ref_cnt(), 2);
  ASSERT_EQ(item1->ref_cnt(), 2);
  foo_list.MoveTo(&foo_list0);
  ASSERT_EQ(foo_list0.size(), 2);
  ASSERT_EQ(foo_list.empty(), true);
  ASSERT_TRUE(foo_list0.Begin() == item0.Mutable());
  ASSERT_TRUE(foo_list0.Last() == item1.Mutable());
  ASSERT_EQ(item0->ref_cnt(), 2);
  ASSERT_EQ(item1->ref_cnt(), 2);
}

// clang-format off
OBJECT_MSG_BEGIN(SelfLoopContainer);
 public:
  void __Init__() { clear_deleted(); }
  // Getters
  bool has_deleted() const { return deleted_ != nullptr; }
  bool deleted() const { return *deleted_; } 
  bool is_entry_empty() const { return entry_.empty(); }
  // Setters
  bool* mut_deleted() { return deleted_; }
  bool* mutable_deleted() { return deleted_; }
  void set_deleted(bool* val) { deleted_ = val; }
  void clear_deleted() { deleted_ = nullptr; }

  // methods
  OF_PUBLIC void __Init__(bool* deleted) {
    __Init__();
    set_deleted(deleted);
  }
  OF_PUBLIC void __Delete__() { *mut_deleted() = true; }
  // fields
  OBJECT_MSG_DEFINE_FIELD(bool*, deleted_);
  // list entries
  OBJECT_MSG_DEFINE_FIELD(intrusive::ListEntry, entry_);
  OBJECT_MSG_DEFINE_LIST_HEAD(SelfLoopContainer, entry, head);
OBJECT_MSG_END(SelfLoopContainer);
// clang-format on

TEST(ObjectMsgSelfLoopList, __Init__) {
  bool deleted = false;
  auto self_loop_head = ObjectMsgPtr<SelfLoopContainer>::New(&deleted);
  ASSERT_EQ(self_loop_head->mut_head()->container_, self_loop_head.Mutable());
}

TEST(ObjectMsgSelfLoopList, PushBack) {
  bool deleted0 = false;
  bool deleted1 = false;
  {
    auto self_loop_head0 = ObjectMsgPtr<SelfLoopContainer>::New(&deleted0);
    auto self_loop_head1 = ObjectMsgPtr<SelfLoopContainer>::New(&deleted1);
    ASSERT_EQ(self_loop_head0->ref_cnt(), 1);
    ASSERT_EQ(self_loop_head1->ref_cnt(), 1);
    self_loop_head0->mut_head()->PushBack(self_loop_head0.Mutable());
    ASSERT_EQ(self_loop_head0->head().size(), 1);
    ASSERT_EQ(self_loop_head0->ref_cnt(), 1);
    self_loop_head0->mut_head()->PushBack(self_loop_head1.Mutable());
    ASSERT_EQ(self_loop_head1->ref_cnt(), 2);
    ASSERT_EQ(self_loop_head0->head().size(), 2);
  }
  ASSERT_TRUE(deleted0);
  ASSERT_TRUE(deleted1);
}

TEST(ObjectMsgSelfLoopList, PushFront) {
  bool deleted0 = false;
  bool deleted1 = false;
  {
    auto self_loop_head0 = ObjectMsgPtr<SelfLoopContainer>::New(&deleted0);
    auto self_loop_head1 = ObjectMsgPtr<SelfLoopContainer>::New(&deleted1);
    ASSERT_EQ(self_loop_head0->ref_cnt(), 1);
    ASSERT_EQ(self_loop_head1->ref_cnt(), 1);
    self_loop_head0->mut_head()->PushFront(self_loop_head0.Mutable());
    ASSERT_EQ(self_loop_head0->head().size(), 1);
    ASSERT_EQ(self_loop_head0->ref_cnt(), 1);
    self_loop_head0->mut_head()->PushFront(self_loop_head1.Mutable());
    ASSERT_EQ(self_loop_head1->ref_cnt(), 2);
    ASSERT_EQ(self_loop_head0->head().size(), 2);
  }
  ASSERT_TRUE(deleted0);
  ASSERT_TRUE(deleted1);
}

TEST(ObjectMsgSelfLoopList, EmplaceBack) {
  bool deleted0 = false;
  bool deleted1 = false;
  {
    auto self_loop_head0 = ObjectMsgPtr<SelfLoopContainer>::New(&deleted0);
    auto self_loop_head1 = ObjectMsgPtr<SelfLoopContainer>::New(&deleted1);
    ASSERT_EQ(self_loop_head0->ref_cnt(), 1);
    ASSERT_EQ(self_loop_head1->ref_cnt(), 1);
    self_loop_head0->mut_head()->EmplaceBack(ObjectMsgPtr<SelfLoopContainer>(self_loop_head0));
    ASSERT_EQ(self_loop_head0->head().size(), 1);
    ASSERT_EQ(self_loop_head0->ref_cnt(), 1);
    self_loop_head0->mut_head()->EmplaceBack(ObjectMsgPtr<SelfLoopContainer>(self_loop_head1));
    ASSERT_EQ(self_loop_head1->ref_cnt(), 2);
    ASSERT_EQ(self_loop_head0->head().size(), 2);
  }
  ASSERT_TRUE(deleted0);
  ASSERT_TRUE(deleted1);
}

TEST(ObjectMsgSelfLoopList, EmplaceFront) {
  bool deleted0 = false;
  bool deleted1 = false;
  {
    auto self_loop_head0 = ObjectMsgPtr<SelfLoopContainer>::New(&deleted0);
    auto self_loop_head1 = ObjectMsgPtr<SelfLoopContainer>::New(&deleted1);
    ASSERT_EQ(self_loop_head0->ref_cnt(), 1);
    ASSERT_EQ(self_loop_head1->ref_cnt(), 1);
    self_loop_head0->mut_head()->EmplaceFront(ObjectMsgPtr<SelfLoopContainer>(self_loop_head0));
    ASSERT_EQ(self_loop_head0->head().size(), 1);
    ASSERT_EQ(self_loop_head0->ref_cnt(), 1);
    self_loop_head0->mut_head()->EmplaceFront(ObjectMsgPtr<SelfLoopContainer>(self_loop_head1));
    ASSERT_EQ(self_loop_head1->ref_cnt(), 2);
    ASSERT_EQ(self_loop_head0->head().size(), 2);
  }
  ASSERT_TRUE(deleted0);
  ASSERT_TRUE(deleted1);
}

TEST(ObjectMsgSelfLoopList, Erase) {
  bool deleted0 = false;
  bool deleted1 = false;
  {
    auto self_loop_head0 = ObjectMsgPtr<SelfLoopContainer>::New(&deleted0);
    auto self_loop_head1 = ObjectMsgPtr<SelfLoopContainer>::New(&deleted1);
    self_loop_head0->mut_head()->PushBack(self_loop_head0.Mutable());
    self_loop_head0->mut_head()->PushBack(self_loop_head1.Mutable());
    self_loop_head0->mut_head()->Erase(self_loop_head0.Mutable());
    self_loop_head0->mut_head()->Erase(self_loop_head1.Mutable());
    ASSERT_EQ(self_loop_head0->ref_cnt(), 1);
    ASSERT_EQ(self_loop_head1->ref_cnt(), 1);
  }
  ASSERT_TRUE(deleted0);
  ASSERT_TRUE(deleted1);
}

TEST(ObjectMsgSelfLoopList, PopBack) {
  bool deleted0 = false;
  bool deleted1 = false;
  {
    auto self_loop_head0 = ObjectMsgPtr<SelfLoopContainer>::New(&deleted0);
    auto self_loop_head1 = ObjectMsgPtr<SelfLoopContainer>::New(&deleted1);
    self_loop_head0->mut_head()->PushBack(self_loop_head0.Mutable());
    self_loop_head0->mut_head()->PushBack(self_loop_head1.Mutable());
    self_loop_head0->mut_head()->PopBack();
    self_loop_head0->mut_head()->PopBack();
    ASSERT_EQ(self_loop_head0->ref_cnt(), 1);
    ASSERT_EQ(self_loop_head1->ref_cnt(), 1);
  }
  ASSERT_TRUE(deleted0);
  ASSERT_TRUE(deleted1);
}

TEST(ObjectMsgSelfLoopList, PopFront) {
  bool deleted0 = false;
  bool deleted1 = false;
  {
    auto self_loop_head0 = ObjectMsgPtr<SelfLoopContainer>::New(&deleted0);
    auto self_loop_head1 = ObjectMsgPtr<SelfLoopContainer>::New(&deleted1);
    self_loop_head0->mut_head()->PushBack(self_loop_head0.Mutable());
    self_loop_head0->mut_head()->PushBack(self_loop_head1.Mutable());
    self_loop_head0->mut_head()->PopFront();
    self_loop_head0->mut_head()->PopFront();
    ASSERT_EQ(self_loop_head0->ref_cnt(), 1);
    ASSERT_EQ(self_loop_head1->ref_cnt(), 1);
  }
  ASSERT_TRUE(deleted0);
  ASSERT_TRUE(deleted1);
}

TEST(ObjectMsgSelfLoopList, MoveTo) {
  bool deleted0 = false;
  bool deleted1 = false;
  {
    auto self_loop_head0 = ObjectMsgPtr<SelfLoopContainer>::New(&deleted0);
    auto self_loop_head1 = ObjectMsgPtr<SelfLoopContainer>::New(&deleted1);
    self_loop_head0->mut_head()->PushBack(self_loop_head0.Mutable());
    self_loop_head0->mut_head()->PushBack(self_loop_head1.Mutable());
    self_loop_head0->mut_head()->MoveTo(self_loop_head1->mut_head());
    ASSERT_EQ(self_loop_head0->ref_cnt(), 2);
    ASSERT_EQ(self_loop_head1->ref_cnt(), 1);
  }
  ASSERT_TRUE(deleted0);
  ASSERT_TRUE(deleted1);
}

TEST(ObjectMsgSelfLoopList, Clear) {
  bool deleted0 = false;
  bool deleted1 = false;
  {
    auto self_loop_head0 = ObjectMsgPtr<SelfLoopContainer>::New(&deleted0);
    auto self_loop_head1 = ObjectMsgPtr<SelfLoopContainer>::New(&deleted1);
    self_loop_head0->mut_head()->PushBack(self_loop_head0.Mutable());
    self_loop_head0->mut_head()->PushBack(self_loop_head1.Mutable());
    self_loop_head0->mut_head()->Clear();
    ASSERT_EQ(self_loop_head0->ref_cnt(), 1);
    ASSERT_EQ(self_loop_head1->ref_cnt(), 1);
  }
  ASSERT_TRUE(deleted0);
  ASSERT_TRUE(deleted1);
}

}  // namespace

}  // namespace test

}  // namespace oneflow
