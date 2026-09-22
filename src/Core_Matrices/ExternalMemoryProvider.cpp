#include "ExternalMemoryProvider.h"

namespace {

const NV3047MemoryProviderV1* registered_provider = nullptr;

bool providerValid(
    const NV3047MemoryProviderV1* provider) {

    return
        provider != nullptr &&
        provider->abi_version ==
            NV3047_MEMORY_PROVIDER_ABI_VERSION &&
        provider->begin != nullptr &&
        provider->end != nullptr &&
        provider->is_ready != nullptr &&
        provider->front_buffer != nullptr &&
        provider->draw_buffer != nullptr &&
        provider->swap_buffers != nullptr &&
        provider->buffer_count != nullptr &&
        provider->buffer_size_bytes != nullptr &&
        provider->total_allocated_bytes != nullptr &&
        provider->free_managed_bytes != nullptr &&
        provider->largest_free_managed_block_bytes != nullptr &&
        provider->begin_frame != nullptr &&
        provider->service != nullptr &&
        provider->acquire_dma != nullptr &&
        provider->release_dma != nullptr;
}

} // namespace

extern "C" bool nv3047_driver_register_memory_provider(
    const NV3047MemoryProviderV1* provider) {

    if (!providerValid(provider)) {
        return false;
    }

    // Registration is idempotent. Multiple translation units may include
    // NV3047_Memory.h and therefore run the automatic registrar.
    if (registered_provider &&
        registered_provider != provider) {
        return false;
    }

    registered_provider = provider;
    return true;
}

extern "C" const NV3047MemoryProviderV1*
nv3047_driver_get_memory_provider() {
    return registered_provider;
}

extern "C" bool nv3047_driver_has_memory_provider() {
    return registered_provider != nullptr;
}
