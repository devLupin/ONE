/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd. All Rights Reserved
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

#include "kernels/L2Normalize.h"
#include "kernels/Utils.h"

#include <tensorflow/lite/kernels/internal/optimized/optimized_ops.h>

#include <stdexcept>

namespace luci_interpreter
{

namespace kernels
{

L2Normalize::L2Normalize(const Tensor *input, Tensor *output, const L2NormParams &params)
    : KernelWithParams<L2NormParams>(params), _input(input), _output(output)
{
}

void L2Normalize::configure()
{
  assert(_input->shape().num_dims() <= 4);
  assert(_output->element_type() == DataType::FLOAT32 || _output->element_type() == DataType::U8);
  assert(_input->element_type() == _output->element_type());
  if (_output->element_type() == DataType::U8)
  {
    assert(_output->scale() == (1. / 128.));
    assert(_output->zero_point() == 128);
  }
  assert(params().activation == Activation::NONE);
  _output->resize(_input->shape());
}

void L2Normalize::execute() const
{
  switch (_output->element_type())
  {
    case DataType::FLOAT32:
      eval<float>(0);
      break;
    case DataType::U8:
      eval<uint8_t>(_input->zero_point());
      break;
    default:
      throw std::runtime_error("Unsupported type.");
  }
}

template <typename T> void L2Normalize::eval(int32_t zero_point) const
{
  tflite::L2NormalizationParams op_params{};
  op_params.input_zero_point = zero_point;
  tflite::optimized_ops::L2Normalization(op_params, getTensorShape(_input),
                                         getTensorData<T>(_input), getTensorShape(_output),
                                         getTensorData<T>(_output));
}

} // namespace kernels
} // namespace luci_interpreter
