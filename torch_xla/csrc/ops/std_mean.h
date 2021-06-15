#pragma once

#include <vector>

#include "tensorflow/compiler/xla/types.h"
#include "torch_xla/csrc/ops/mean.h"
#include "torch_xla/csrc/ops/std.h"
#include "torch_xla/csrc/ir.h"

namespace torch_xla {
namespace ir {
namespace ops {

class StdMean : public Node {
 public:
  StdMean(const Value& input,
          std::vector<xla::int64> dimensions,
          xla::int64 correction,
          bool keep_reduced_dimensions,
          c10::optional<at::ScalarType> dtype);

  std::string ToString() const override;

  NodePtr Clone(OpList operands) const override;

  XlaOpVector Lower(LoweringContext* loctx) const override;

  const std::vector<xla::int64>& dimensions() const { return dimensions_; }

  bool keep_reduced_dimensions() const { return keep_reduced_dimensions_; }

  xla::int64 correction() const { return correction_; }

 private:
  std::vector<xla::int64> dimensions_;
  bool keep_reduced_dimensions_;
  xla::int64 correction_;
};

}  // namespace ops
}  // namespace ir
}  // namespace torch_xla
