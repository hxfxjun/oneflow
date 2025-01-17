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
#ifndef ONEFLOW_CORE_DEVICE_COLLECTIVE_BOXING_DEVICE_CONTEXT_H_
#define ONEFLOW_CORE_DEVICE_COLLECTIVE_BOXING_DEVICE_CONTEXT_H_

#include "oneflow/core/kernel/kernel_context.h"
#include "oneflow/core/device/event_record.h"
#include "oneflow/core/job/collective_boxing_device_ctx_poller.h"

namespace oneflow {

using namespace boxing::collective;

class CollectiveBoxingDeviceCtx final : public DeviceCtx, public EventRecordProvider {
 public:
  OF_DISALLOW_COPY_AND_MOVE(CollectiveBoxingDeviceCtx);
  CollectiveBoxingDeviceCtx() = default;
  ~CollectiveBoxingDeviceCtx() override = default;

  std::shared_ptr<CollectiveBoxingDeviceCtxCheckpoint> AddCheckpoint();
  void AddCallBack(std::function<void()> callback) const override;

  DeviceType device_type() const override {
    UNIMPLEMENTED();
    return DeviceType::kInvalidDevice;
  }

  std::shared_ptr<EventRecord> MakeEventRecord() override {
    UNIMPLEMENTED();
    return std::shared_ptr<EventRecord>();
  }

 private:
  std::shared_ptr<CollectiveBoxingDeviceCtxCheckpoint> current_checkpoint_;

};  // namespace oneflow

}  // namespace oneflow

#endif  // ONEFLOW_CORE_DEVICE_COLLECTIVE_BOXING_DEVICE_CONTEXT_H_
