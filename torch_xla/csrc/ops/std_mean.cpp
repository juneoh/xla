#include "torch_xla/csrc/ops/std_mean.h"

#include "absl/strings/str_join.h"
#include "tensorflow/compiler/xla/xla_client/util.h"
#include "torch_xla/csrc/lowering_context.h"
#include "torch_xla/csrc/ops/infer_output_shape.h"
#include "torch_xla/csrc/reduction.h"

namespace torch_xla {
namespace ir {
namespace ops {
namespace {

xla::XlaOp LowerStd(xla::XlaOp input,
                    const std::vector<xla::int64>& dimensions,
                    bool keep_reduced_dimensions,
                    xla::int64 correction) {
  return BuildStdDeviation(input, dimensions, keep_reduced_dimensions, correction);
}

xla::XlaOp LowerMean2(xla::XlaOp input,
                     const std::vector<xla::int64>& dimensions,
                     bool keep_reduced_dimensions) {
  return BuildMean(input, dimensions, keep_reduced_dimensions);
}

std::pair<xla::Shape, xla::Shape> NodeOutputShape(const Value& input,
                                                  std::vector<xla::int64>& dimensions,
                                                  bool keep_reduced_dimensions,
                                                  xla::int64 correction) {
  auto lower_for_shape_fn_std = [&](absl::Span<const xla::XlaOp> operands) -> xla::XlaOp {
    return LowerStd(operands[0], dimensions, keep_reduced_dimensions, correction);
  };
  auto lower_for_shape_fn_mean = [&](absl::Span<const xla::XlaOp> operands) -> xla::XlaOp {
    return LowerMean2(operands[0], dimensions, keep_reduced_dimensions);
  };
  result_std = InferOutputShape({input.shape()}, lower_for_shape_fn_std);
  result_mean = InferOutputShape({input.shape()}, lower_for_shape_fn_mean);
  return std::pair<xla::Shape, xla::Shape>(result_std, result_mean);
}

}  // namespace

StdMean::StdMean(const Value& input, std::vector<xla::int64> dimensions,
                 xla::int64 correction, bool keep_reduced_dimensions)
    : Node(ir::OpKind(at::aten::std_mean), {input},
           [&]() {
             return NodeOutputShape(input, dimensions, keep_reduced_dimensions, correction);
           },
           /*num_outputs=*/1,
           xla::util::MHash(dimensions, correction, keep_reduced_dimensions)),
      dimensions_(std::move(dimensions)),
      correction_(correction),
      keep_reduced_dimensions_(keep_reduced_dimensions) {}

NodePtr StdMean::Clone(OpList operands) const {
  return MakeNode<StdMean>(operands.at(0), dimensions_, correction_,
                           keep_reduced_dimensions_);
}

XlaOpVector StdMean::Lower(LoweringContext* loctx) const {
  xla::XlaOp input = loctx->GetOutputOp(operand(0));
  xla::XlaOp op_std = LowerStd(input, dimensions_, keep_reduced_dimensions_, correction_);
  xla::XlaOp op_mean = LowerMean2(input, dimensions_, keep_reduced_dimensions_);
  return ReturnOps({op_std, op_mean}, loctx);
}

std::string StdMean::ToString() const {
  std::stringstream ss;
  ss << Node::ToString() << ", dimensions=(" << absl::StrJoin(dimensions_, ", ")
     << "), keep_reduced_dimensions=" << keep_reduced_dimensions_
     << ", correction=" << correction_;
  return ss.str();
}

}  // namespace ops
}  // namespace ir
}  // namespace torch_xla

