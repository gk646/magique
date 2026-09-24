// SPDX-License-Identifier: zlib-acknowledgement
#include <string_view>
#include <utility>
#include <cstring>
#include <random>

#include <magique/util/Data.h>

#include "external/sdefl.h"
#include "external/sinfl.h"
#include "external/tinyaes/aes.h"
#include "external/blake3/blake3.h"

namespace magique
{

    std::pair<std::string_view, bool> DataCompress(std::string_view data, size_t minSize)
    {
        thread_local sdefl* compCtx = new sdefl();
        thread_local std::string COMP_BUFFER{};

        if (data.empty())
        {
            COMP_BUFFER.shrink_to_fit();
            return {data, false};
        }

        if (data.size() < minSize)
            return {data, false};

        COMP_BUFFER.resize(sdefl_bound(data.size()));
        int compSize = sdeflate(compCtx, COMP_BUFFER.data(), data.data(), data.size(), 1);
        COMP_BUFFER.resize(compSize);

        if (COMP_BUFFER.size() < data.size())
            return {{COMP_BUFFER.data(), COMP_BUFFER.size()}, true};

        return {data, false};
    }

    std::string_view DataDecompress(std::string_view data, size_t minOutBuffer)
    {
        thread_local std::string COMP_BUFFER;

        if (data.empty())
        {
            COMP_BUFFER.clear();
            COMP_BUFFER.shrink_to_fit();
            return {};
        }

        // Should usually be enough
        COMP_BUFFER.resize(std::max(data.size() * 3, minOutBuffer));

        int size = sinflate(COMP_BUFFER.data(), COMP_BUFFER.capacity(), data.data(), data.size());
        if ((int)COMP_BUFFER.size() < size)
            return {};

        COMP_BUFFER.resize(size);

        if (size <= 0)
            return {};
        return {COMP_BUFFER.data(), COMP_BUFFER.size()};
    }

    bool DataEncrypt(std::string& data, EncryptionKey key)
    {
        if (key.getIsNull())
            return true;
        std::random_device dev;
        std::array<uint8_t, 16> iv;
        for (auto& val : iv)
        {
            val = (uint8_t)dev();
        }

        AES_ctx ctx;
        AES_init_ctx_iv(&ctx, key.getKey(), iv.data());

        AES_CTR_xcrypt_buffer(&ctx, (uint8_t*)data.data(), data.size());
        data.append_range(std::string_view{(const char*)iv.data(), iv.size()});

        return true;
    }

    bool DataDecrypt(std::string& data, EncryptionKey key)
    {
        if (key.getIsNull())
            return true;

        if (data.size() < 16)
            return false;

        std::array<uint8_t, 16> iv;
        std::memcpy(iv.data(), data.data() + (data.size() - iv.size()), iv.size());

        AES_ctx ctx;
        AES_init_ctx_iv(&ctx, key.getKey(), iv.data());

        AES_CTR_xcrypt_buffer(&ctx, (uint8_t*)data.data(), data.size());

        for (size_t i = 0; i < iv.size(); i++)
            data.pop_back();

        return true;
    }

    std::string_view DataHash(std::string_view data)
    {
        constexpr size_t BLAKE3_OUT_LEN = 32;
        thread_local std::string CACHE(BLAKE3_OUT_LEN, '\0');
        std::memset(CACHE.data(), 0, CACHE.size());

        blake3 hasher;
        blake3_init(&hasher);
        blake3_update(&hasher, data.data(), data.size());
        blake3_out(&hasher, (uint8_t*)CACHE.data(), BLAKE3_OUT_LEN);

        return CACHE;
    }

} // namespace magique
