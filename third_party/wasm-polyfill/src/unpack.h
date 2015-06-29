#ifndef unpack_h_
#define unpack_h_

#include <cstdint>
#include <vector>

namespace asmjs {

#ifdef CHECKED_OUTPUT_SIZE

uint32_t calculate_unpacked_size(const uint8_t* packed);

#else

bool has_magic_number(const uint8_t* packed);
uint32_t unpacked_size(const uint8_t* packed, const char* callback_name);
void unpack(const uint8_t* packed, const char* cb_name, uint32_t unpacked_size, uint8_t* unpacked);

#endif

}  // namespace asmjs
#endif  // unpack_h_
