#ifndef WEBGPU_CPP_CHAINED_STRUCT_H_
#define WEBGPU_CPP_CHAINED_STRUCT_H_

#include <cstddef>
#include <cstdint>

// This header file declares the ChainedStruct structures separately from the WebGPU
// headers so that dependencies can directly extend structures without including the larger header
// which exposes capabilities that may require correctly set proc tables.
namespace wgpu {

    enum class SType : uint32_t;

    struct ChainedStruct {
        ChainedStruct const * nextInChain = nullptr;
        SType sType = SType(0u);
    };

    struct ChainedStructOut {
        ChainedStructOut * nextInChain = nullptr;
        SType sType = SType(0u);
    };

}  // namespace wgpu}

#endif // WEBGPU_CPP_CHAINED_STRUCT_H_
