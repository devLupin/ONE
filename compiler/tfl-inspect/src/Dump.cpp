/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd. All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Dump.h"
#include "Reader.h"

#include <string>
#include <ostream>
#include <stdexcept>

namespace tflinspect
{

void DumpOperators::run(std::ostream &os, const tflite::Model *model)
{
  tflinspect::Reader reader(model);

  assert(reader.num_subgraph() == 1);
  reader.select_subgraph(0);

  auto ops = reader.operators();

  // dump operators
  for (uint32_t i = 0; i < ops->Length(); ++i)
  {
    const auto op = ops->Get(i);

    auto op_name = reader.opcode_name(op);

    os << op_name << std::endl;
  }
}

} // namespace tflinspect

namespace
{

const tflite::Operator *operator_match_output(tflinspect::Reader &reader, const int32_t tensor)
{
  auto ops = reader.operators();

  for (uint32_t i = 0; i < ops->Length(); ++i)
  {
    const auto op = ops->Get(i);

    const std::vector<int32_t> &outputs = tflinspect::as_index_vector(op->outputs());

    for (auto output : outputs)
    {
      if (output == tensor)
        return op;
    }
  }
  return nullptr;
}

size_t tensor_buffer_size(tflinspect::Reader &reader, const int32_t tensor_id)
{
  auto tensors = reader.tensors();

  if (tensor_id < 0 || tensor_id >= tensors->Length())
  {
    throw std::runtime_error("Invalid Tensor ID");
  }

  auto tensor = tensors->Get(tensor_id);
  auto buffer_id = tensor->buffer();

  size_t size = reader.buffer_info(buffer_id, nullptr);

  return size;
}

} // namespace

namespace tflinspect
{

void DumpConv2DWeight::run(std::ostream &os, const tflite::Model *model)
{
  tflinspect::Reader reader(model);

  assert(reader.num_subgraph() == 1);
  reader.select_subgraph(0);

  auto ops = reader.operators();

  // dump Conv2D, DepthwiseConv2D and its weight input operator
  for (uint32_t i = 0; i < ops->Length(); ++i)
  {
    const auto op = ops->Get(i);
    auto bc = reader.builtin_code(op);

    if (bc == tflite::BuiltinOperator_CONV_2D || bc == tflite::BuiltinOperator_DEPTHWISE_CONV_2D)
    {
      const std::vector<int32_t> &inputs = tflinspect::as_index_vector(op->inputs());
      if (inputs.size() < 2)
      {
        throw std::runtime_error("Operator has invalid input");
      }
      auto weight_input = inputs[1]; // Tensor ID of weight input

      const auto op_weight = operator_match_output(reader, weight_input);
      const auto buffer_size = tensor_buffer_size(reader, weight_input);

      std::string weight_op_name = "?";

      if (op_weight == nullptr && buffer_size > 0)
      {
        weight_op_name = "CONST";
      }
      else if (op_weight != nullptr)
      {
        weight_op_name = reader.opcode_name(op_weight);
      }

      auto op_name = reader.opcode_name(op);
      os << op_name << "," << weight_op_name << std::endl;
    }
  }
}

} // namespace tflinspect
