#include <magique/persistence/container/GameConfig.h>
#include <magique/core/Types.h>
#include <magique/internal/Macros.h>

namespace magique
{
    void GameConfig::SaveKeybind(ConfigID id, Keybind keybind)
    {
        if (storage.size() < (int)id + 1)
        {
            storage.resize((int)id + 1);
        }
        storage[(int)id].keybind = keybind;
    }

    void GameConfig::SaveSetting(ConfigID id, Setting setting)
    {
        if (storage.size() < (int)id + 1)
        {
            storage.resize((int)id + 1);
        }
        storage[(int)id].setting = setting;
    }

    void GameConfig::SaveString(ConfigID id, const std::string& string)
    {
        if (storage.size() < (int)id + 1)
        {
            storage.resize((int)id + 1);
        }
        if (storage[(int)id].string != nullptr)
        {
            *static_cast<std::string*>(storage[(int)id].string) = string;
        }
        else
        {
            storage[(int)id].string = new std::string(string);
        }
    }

    template <typename T>
    void GameConfig::SaveValue(ConfigID id, T val)
    {
        if (storage.size() < (int)id + 1)
        {
            storage.resize((int)id + 1);
        }
        auto& buff = storage[(int)id].buffer;
        std::memcpy(buff, &val, sizeof(T));
    }


    Keybind& GameConfig::GetKeybind(ConfigID id)
    {
        M_ASSERT(storage.size() > (int)id, "Setting was never assigned!");
        return storage[(int)id].keybind;
    }

    Setting& GameConfig::GetSetting(ConfigID id)
    {
        M_ASSERT(storage.size() > (int)id, "Setting was never assigned!");
        return storage[(int)id].setting;
    }

    std::string& GameConfig::GetString(ConfigID id)
    {
        M_ASSERT(storage.size() > (int)id, "Setting was never assigned!");
        M_ASSERT(storage[(int)id].string != nullptr, "No string saved to this slot");
        return *static_cast<std::string*>(storage[(int)id].string);
    }

    template <typename T>
    T& GameConfig::GetValue(ConfigID id)
    {
        M_ASSERT(storage.size() > (int)id, "Setting was never assigned!");
        auto& buff = storage[(int)id].buffer;
        return *reinterpret_cast<T*>(buff);
    }


    template void GameConfig::SaveValue<float>(ConfigID id, float val);
    template float& GameConfig::GetValue<float>(ConfigID id);

    template void GameConfig::SaveValue<double>(ConfigID id, double val);
    template double& GameConfig::GetValue<double>(ConfigID id);

    template void GameConfig::SaveValue<int32_t>(ConfigID id, int32_t val);
    template int32_t& GameConfig::GetValue<int32_t>(ConfigID id);

    template void GameConfig::SaveValue<uint64_t>(ConfigID id, uint64_t val);
    template uint64_t& GameConfig::GetValue<uint64_t>(ConfigID id);

    template void GameConfig::SaveValue<int64_t>(ConfigID id, int64_t val);
    template int64_t& GameConfig::GetValue<int64_t>(ConfigID id);

    template void GameConfig::SaveValue<int8_t>(ConfigID id, int8_t val);
    template int8_t& GameConfig::GetValue<int8_t>(ConfigID id);

    template void GameConfig::SaveValue<uint8_t>(ConfigID id, uint8_t val);
    template uint8_t& GameConfig::GetValue<uint8_t>(ConfigID id);

} // namespace magique