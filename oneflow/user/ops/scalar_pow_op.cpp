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
#include "oneflow/core/framework/framework.h"

namespace oneflow {

namespace {

REGISTER_USER_OP("scalar_pow")
    .Input("in")
    .Attr<bool>("has_int_operand")
    .Attr<bool>("has_float_operand")
    .Attr<int64_t>("int_operand")
    .Attr<double>("float_operand")
    .Output("out")
    .SetTensorDescInferFn([](user_op::InferContext* ctx) -> Maybe<void> {
      *ctx->OutputShape("out", 0) = ctx->InputShape("in", 0);
      return Maybe<void>::Ok();
    })
    .SetGetSbpFn([](user_op::SbpContext* ctx) -> Maybe<void> {
      const user_op::TensorDesc& in_tensor = ctx->LogicalTensorDesc4InputArgNameAndIndex("in", 0);
      FOR_RANGE(int64_t, i, 0, in_tensor.shape().NumAxes()) {
        ctx->NewBuilder()
            .Split(user_op::OpArg("in", 0), i)
            .Split(user_op::OpArg("out", 0), i)
            .Build();
      }
      return Maybe<void>::Ok();
    })
    .SetDataTypeInferFn([](user_op::InferContext* ctx) -> Maybe<void> {
      *ctx->OutputDType("out", 0) = ctx->InputDType("in", 0);
      return Maybe<void>::Ok();
    });

REGISTER_USER_OP("scalar_pow_grad")
    .Input("x")
    .Input("dy")
    .Attr<bool>("has_int_operand")
    .Attr<bool>("has_float_operand")
    .Attr<int64_t>("int_operand")
    .Attr<double>("float_operand")
    .Output("dx")
    .SetTensorDescInferFn([](user_op::InferContext* ctx) -> Maybe<void> {
      *ctx->OutputShape("dx", 0) = ctx->InputShape("x", 0);
      return Maybe<void>::Ok();
    })
    .SetGetSbpFn([](user_op::SbpContext* ctx) -> Maybe<void> {
      const user_op::TensorDesc& x_tensor = ctx->LogicalTensorDesc4InputArgNameAndIndex("x", 0);
      FOR_RANGE(int64_t, i, 0, x_tensor.shape().NumAxes()) {
        ctx->NewBuilder()
            .Split(user_op::OpArg("x", 0), i)
            .Split(user_op::OpArg("dx", 0), i)
            .Split(user_op::OpArg("dy", 0), i)
            .Build();
      }
      return Maybe<void>::Ok();
    })
    .SetDataTypeInferFn([](user_op::InferContext* ctx) -> Maybe<void> {
      CHECK_EQ_OR_RETURN(ctx->InputDType("x", 0), ctx->InputDType("dy", 0));
      *ctx->OutputDType("dx", 0) = ctx->InputDType("x", 0);
      return Maybe<void>::Ok();
    });

REGISTER_USER_OP_GRAD("scalar_pow")
    .SetBackwardOpConfGenFn([](user_op::BackwardOpConfContext* ctx) -> Maybe<void> {
      const auto scalar_pow_grad_op_name = ctx->FwOp().op_name() + "_grad";
      ctx->DefineOp(scalar_pow_grad_op_name, [&ctx](user_op::BackwardOpBuilder& builder) {
        return builder.OpTypeName("scalar_pow_grad")
            .InputBind("x", ctx->FwOp().input("in", 0))
            .InputBind("dy", ctx->FwOp().output_grad("out", 0))
            .Attr<bool>("has_int_operand", ctx->FwOp().attr<bool>("has_int_operand"))
            .Attr<bool>("has_float_operand", ctx->FwOp().attr<bool>("has_float_operand"))
            .Attr<int64_t>("int_operand", ctx->FwOp().attr<int64_t>("int_operand"))
            .Attr<double>("float_operand", ctx->FwOp().attr<double>("float_operand"))
            .Output("dx")
            .Build();
      });
      ctx->FwOp().InputGradBind(user_op::OpArg("in", 0),
                                [&ctx, &scalar_pow_grad_op_name]() -> const std::string& {
                                  return ctx->GetOp(scalar_pow_grad_op_name).output("dx", 0);
                                });
      return Maybe<void>::Ok();
    });

}  // namespace

}  // namespace oneflow
