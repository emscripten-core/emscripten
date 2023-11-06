/**
 * Copyright 2018 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */

#include <emscripten.h>
#include <stdint.h>
#include <string.h>

const unsigned kRenderQuantumFrames = 128;
const unsigned kBytesPerChannel = kRenderQuantumFrames * sizeof(float);

// The "kernel" is an object that processes a audio stream, which contains
// one or more channels. It is supposed to obtain the frame data from an
// |input|, process and fill an |output| of the AudioWorkletProcessor.
//
//       AudioWorkletProcessor Input(multi-channel, 128-frames)
//                                 |
//                                 V
//                               Kernel
//                                 |
//                                 V
//       AudioWorkletProcessor Output(multi-channel, 128-frames)
//
// In this implementation, the kernel operates based on 128-frames, which is
// the render quantum size of Web Audio API.
extern "C" void EMSCRIPTEN_KEEPALIVE SimpleKernel_Process(uintptr_t input_ptr, uintptr_t output_ptr,
               unsigned channel_count) {
  float* input_buffer = reinterpret_cast<float*>(input_ptr);
  float* output_buffer = reinterpret_cast<float*>(output_ptr);

  // Bypasses the data. By design, the channel count will always be the same
  // for |input_buffer| and |output_buffer|.
  for (unsigned channel = 0; channel < channel_count; ++channel) {
    float* destination = output_buffer + channel * kRenderQuantumFrames;
    float* source = input_buffer + channel * kRenderQuantumFrames;
    memcpy(destination, source, kBytesPerChannel);
  }
}
