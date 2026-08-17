/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#ifndef EMCC_NATIVE_DIAGNOSTICS_H
#define EMCC_NATIVE_DIAGNOSTICS_H

#include <string>
#include <string_view>
#include <vector>

namespace emscripten {

void parse_warning_flags(const std::vector<std::string>& user_args);
void emit_unused_warning(std::string_view msg);
bool is_emscripten_only_warning(std::string_view arg);

} // namespace emscripten

#endif // EMCC_NATIVE_DIAGNOSTICS_H
