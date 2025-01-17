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
#include "oneflow/core/platform/include/pthread_fork.h"
#include "oneflow/core/common/util.h"

namespace oneflow {

namespace pthread_fork {

static bool is_fork = false;

bool IsForkedSubProcess() { return is_fork; }
static void SetIsForkedSubProcess() { is_fork = true; }

void RegisterForkCallback() { pthread_atfork(nullptr, nullptr, SetIsForkedSubProcess); }
COMMAND(RegisterForkCallback());

}  // namespace pthread_fork

}  // namespace oneflow
