// SPDX-License-Identifier: zlib-acknowledgement
#include <string_view>
#include <utility>
#include <cstring>
#include <random>

#include <magique/util/Data.h>
#include <magique/util/Logging.h>

#include "external/sdefl.h"
#include "external/sinfl.h"
#include "external/tinyaes/aes.h"
#include "external/blake3/blake3.h"

namespace magique
{

    bool DataReadFile(std::string_view file, std::string& data)
    {
        FILE* f = fopen(file.data(), "rb");
        setvbuf(f, nullptr, _IONBF, 0);
        if (f == nullptr)
        {
            LOG_INFO("Failed to open file for reading: %s", file.data());
            return false;
        }
        fseek(f, 0, SEEK_END);
        const size_t fileSize = ftell(f);
        data.resize(fileSize);
        fseek(f, 0, SEEK_SET);
        fread(data.data(), fileSize, 1, f);
        fclose(f);
        return true;
    }

    bool DataWriteFile(std::string_view file, std::string_view content)
    {
        FILE* f = fopen(file.data(), "w+b");
        setvbuf(f, nullptr, _IONBF, 0);
        if (f == nullptr)
        {
            LOG_ERROR("Failed to open file for writing: %s", file);
            return false;
        }
        fwrite(content.data(), content.size(), 1, f);
        fclose(f);
        return true;
    }

    std::string_view DataCompress(std::string_view data)
    {
        auto& res = internal::DataCompressImpl(data);
        return res;
    }

    std::optional<std::string_view> DataDecompress(std::string_view data)
    {
        const auto res = internal::DataDecompressImpl(data);
        if (res.has_value())
        {
            auto& value = res.value().get();
            return value;
        }
        return {};
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

    Hash DataHash(std::string_view data)
    {
        constexpr size_t BLAKE3_OUT_LEN = 32;
        thread_local std::string CACHE(BLAKE3_OUT_LEN, '\0');

        blake3 hasher;
        blake3_init(&hasher);
        blake3_update(&hasher, data.data(), data.size());
        blake3_out(&hasher, (uint8_t*)CACHE.data(), BLAKE3_OUT_LEN);

        return {CACHE};
    }

    namespace internal
    {
        std::string& DataCompressImpl(std::string_view data)
        {
            thread_local sdefl* compCtx = new sdefl();
            thread_local std::string COMP_BUFFER{};

            COMP_BUFFER.resize(sdefl_bound(data.size()));
            int compSize = sdeflate(compCtx, COMP_BUFFER.data(), data.data(), data.size(), 1);
            COMP_BUFFER.resize(compSize);

            return COMP_BUFFER;
        }

        std::optional<std::reference_wrapper<std::string>> DataDecompressImpl(std::string_view data)
        {
            thread_local std::string COMP_BUFFER;

            // Should usually be enough
            COMP_BUFFER.resize(std::max(data.size() * 3, 1024UL));

            int size = sinflate(COMP_BUFFER.data(), COMP_BUFFER.capacity(), data.data(), data.size());
            if (size <= 0)
                return {};

            // Try resizing a few times
            for (int i = 0; i < 10; i++)
            {
                if ((int)COMP_BUFFER.size() < size) [[unlikely]]
                {
                    COMP_BUFFER.resize(size);
                    size = sinflate(COMP_BUFFER.data(), COMP_BUFFER.capacity(), data.data(), data.size());
                }
                else
                    break;
            }

            if ((int)COMP_BUFFER.size() < size) [[unlikely]]
                return {};

            COMP_BUFFER.resize(size);
            return COMP_BUFFER;
        }
    } // namespace internal

} // namespace magique
