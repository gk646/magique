#include <functional>
#include <raylib/raylib.h>

#include <magique/persistence/container/GameConfig.h>
#include <magique/core/Types.h>
#include <magique/internal/Macros.h>
#include <magique/util/Logging.h>

#include "internal/headers/Security.h"

inline constexpr auto HEADER = "CONFIG";

namespace magique
{
    GameConfig GameConfig::LoadFromFile(const char* fileName, const uint64_t encryptionKey)
    {
        const auto time = GetTime();
        FILE* file = fopen(fileName, "rb");
        if (!file)
        {
            LOG_WARNING("A GameConfig with the given name doesn't exist! Starting a new one: %s", fileName);
            return {};
        }

        fseek(file, 0, SEEK_END);
        const size_t fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);

        std::vector<unsigned char> fileData(fileSize);
        const int readSize = static_cast<int>(fread(fileData.data(), 1, fileSize, file));
        fclose(file);

        if (readSize != fileSize || fileSize < 8 || std::memcmp(fileData.data(), HEADER, 6) != 0)
        {
            LOG_ERROR("Given file is not a GameConfig save: %s", fileName);
            return {};
        }

        // Decrypt
        SymmetricEncrypt((char*)&fileData[6], fileSize - 6, encryptionKey);

        GameConfig config;
        int idx = 6;

        uint16_t entries;
        std::memcpy(&entries, &fileData[idx], 2);
        idx += 2;

        for (uint16_t i = 0; i < entries; ++i)
        {
            if (fileSize - idx < sizeof(GameConfigStorageCell))
            {
                LOG_ERROR("Failed to parse GameConfig file: %s", fileName);
                return {};
            }

            GameConfigStorageCell cell;
            std::memcpy(&cell, &fileData[idx], sizeof(GameConfigStorageCell));
            config.storage.push_back(cell);
            idx += sizeof(GameConfigStorageCell);
        }

        while (idx < fileSize)
        {
            if (fileSize - idx < 2)
            {
                LOG_ERROR("Failed to parse GameConfig file: %s", fileName);
                return {};
            }

            uint16_t stringSize;
            std::memcpy(&stringSize, &fileData[idx], 2);
            idx += 2;

            if (fileSize - idx < stringSize)
            {
                LOG_ERROR("Failed to parse GameConfig file: %s", fileName);
                return {};
            }

            config.stringStorage.emplace_back(reinterpret_cast<char*>(&fileData[idx]), stringSize);
            idx += stringSize;
        }
        const auto* format = "Successfully loaded config: %s | Took %d millis | Entries: %d";
        LOG_INFO(format, fileName, (int)std::round((GetTime() - time) * 1000), static_cast<int>(config.storage.size()));
        return config;
    }

    void GameConfig::SaveToFile(const GameConfig& config, const char* fileName, const uint64_t encryptionKey)
    {
        const auto time = GetTime();
        if (config.storage.empty() && config.stringStorage.empty())
        {
            FILE* file = fopen(fileName, "wb");
            if (file != nullptr)
            {
                fclose(file);
                LOG_INFO("Successfully saved game config. Total entries: 0");
            }
            else
            {
                LOG_ERROR("Failed to save GameConfig to: %s", fileName);
            }
            return;
        }

        int totalSize = 0;
        totalSize += 6 + 2; // Header + amount of entries
        for (const auto& s : config.stringStorage)
        {
            totalSize += 2 + static_cast<int>(s.size());
        }
        totalSize += static_cast<int>(config.storage.size()) * sizeof(GameConfigStorageCell);

        // Allocate memory for the data to be encrypted
        auto* data = new unsigned char[totalSize - 6];
        int idx = 0;

        const auto entries = static_cast<uint16_t>(config.storage.size());
        std::memcpy(&data[idx], &entries, 2);
        idx += 2;

        for (const auto& cell : config.storage)
        {
            std::memcpy(&data[idx], &cell, sizeof(GameConfigStorageCell));
            idx += sizeof(GameConfigStorageCell);
        }

        for (const auto& s : config.stringStorage)
        {
            auto stringSize = static_cast<uint16_t>(s.size());
            std::memcpy(&data[idx], &stringSize, 2);
            idx += 2; // Size
            std::memcpy(&data[idx], s.data(), stringSize);
            idx += stringSize;
        }

        // Encrypt the data
        SymmetricEncrypt((char*)data, totalSize - 6, encryptionKey);

        FILE* file = fopen(fileName, "wb");
        if (!file)
        {
            LOG_ERROR("Failed to save GameConfig to: %s", fileName);
            delete[] data;
            return;
        }

        fwrite(HEADER, 1, 6, file);

        fwrite(data, 1, totalSize - 6, file);

        delete[] data;
        fclose(file);
        auto t = (GetTime() - time)*1000;

        const auto* format = "Successfully saved config: %s | Took %d millis | Entries: %d";
        LOG_INFO(format, fileName, (int)std::round((GetTime() - time) * 1000), static_cast<int>(config.storage.size()));
    }

    void GameConfig::initializeIfEmpty(const std::function<void(GameConfig& config)>& func)
    {
        if (storage.empty() && stringStorage.empty())
        {
            func(*this);
        }
    }

#define RESERVE_VEC()                                                                                                   \
    const auto idx = static_cast<int>(id);                                                                              \
    if (storage.size() < idx + 1)                                                                                       \
    {                                                                                                                   \
        storage.resize(idx + 1);                                                                                        \
    }

    void GameConfig::saveKeybind(ConfigID id, const Keybind keybind)
    {
        RESERVE_VEC();
        storage[idx].keybind = keybind;
    }

    void GameConfig::saveString(ConfigID id, std::string string)
    {
        RESERVE_VEC();
        const auto size = static_cast<int>(stringStorage.size());
        storage[idx].stringIndex = size;
        stringStorage.emplace_back(std::move(string));
    }

    template <typename T>
    void GameConfig::saveValue(ConfigID id, T val)
    {
        RESERVE_VEC();
        auto& buff = storage[(int)id].buffer;
        std::memcpy(buff, &val, sizeof(T));
    }


    Keybind& GameConfig::getKeybind(ConfigID id)
    {
        M_ASSERT(storage.size() > (int)id, "Setting was never assigned!");
        return storage[(int)id].keybind;
    }

    std::string& GameConfig::getString(ConfigID id)
    {
        M_ASSERT(storage.size() > (int)id, "Setting was never assigned!");
        const auto strIdx = storage[static_cast<int>(id)].stringIndex;
        M_ASSERT(strIdx < stringStorage.size() && strIdx >= 0,
                 "This slot doesnt belong to a string! You likely called the wrong method or used the wrong id!");
        return stringStorage[strIdx];
    }

    template <typename T>
    T& GameConfig::getValue(ConfigID id)
    {
        M_ASSERT(storage.size() > (int)id, "Setting was never assigned!");
        auto& buff = storage[(int)id].buffer;
        return *reinterpret_cast<T*>(buff);
    }


    template void GameConfig::saveValue<float>(ConfigID id, float val);
    template float& GameConfig::getValue<float>(ConfigID id);

    template void GameConfig::saveValue<double>(ConfigID id, double val);
    template double& GameConfig::getValue<double>(ConfigID id);

    template void GameConfig::saveValue<int>(ConfigID id, int val);
    template int& GameConfig::getValue<int>(ConfigID id);

    template void GameConfig::saveValue<uint64_t>(ConfigID id, uint64_t val);
    template uint64_t& GameConfig::getValue<uint64_t>(ConfigID id);

    template void GameConfig::saveValue<int64_t>(ConfigID id, int64_t val);
    template int64_t& GameConfig::getValue<int64_t>(ConfigID id);

    template void GameConfig::saveValue<int8_t>(ConfigID id, int8_t val);
    template int8_t& GameConfig::getValue<int8_t>(ConfigID id);

    template void GameConfig::saveValue<uint8_t>(ConfigID id, uint8_t val);
    template uint8_t& GameConfig::getValue<uint8_t>(ConfigID id);

    template void GameConfig::saveValue<Point>(ConfigID id, Point val);
    template Point& GameConfig::getValue<Point>(ConfigID id);

} // namespace magique

// Old description
/*
* // Returns a built config struct as loaded from the given file
        // Note: This is automatically called in the Game::run() method for the default path
        // Failure: returns an empty but valid struct
        static GameConfig LoadFromFile(const char* fileName, uint64_t encryptionKey = 0);

        // Returns an allocated data pointer with the data needed to persist this config instance
        // Note: the global config is automatically persisted
        // Failure: returns {nullptr,0} when the config is empty
        DataPointer<unsigned char> getSaveData(uint64_t encryptionKey = 0);

 */