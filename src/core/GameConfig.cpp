// SPDX-License-Identifier: zlib-acknowledgement
#define _CRT_SECURE_NO_WARNINGS

#include <magique/core/GameConfig.h>

#include "internal/globals/EngineData.h"
#include "internal/globals/EngineConfig.h"

namespace magique
{
    GameConfig& GameConfigGet()
    {
        MAGIQUE_ASSERT(global::ENGINE_DATA.gameConfig.isLoaded,
                       "Config has not been loaded yet! Accessible earliest inside Game::onStartup()");
        return global::ENGINE_DATA.gameConfig;
    }

    void GameConfigEnable(bool value) { global::ENGINE_CONFIG.useGameConfig = value; }

    void GameConfig::saveKeybind(std::string_view slot, Keybind keybind)
    {
        auto& cell = getCellOrNew(slot, StorageType::KEY_BIND);
        JSONExport(keybind, cell.data);
    }

    void GameConfig::saveString(std::string_view slot, const std::string_view& string)
    {
        auto& cell = getCellOrNew(slot, StorageType::STRING);
        cell.data = string;
    }

    Keybind GameConfig::getKeybindOrElse(std::string_view slot, Keybind elseVal)
    {
        const auto* cell = getCell(slot);
        M_GAMESAVE_SLOT_MISSING(elseVal);
        M_GAMESAVE_TYPE_MISMATCH(KEY_BIND, elseVal);
        JSONImport(cell->data, elseVal);
        return elseVal;
    }

    std::string GameConfig::getStringOrElse(std::string_view slot, const std::string& elseVal)
    {
        const auto* cell = getCell(slot);
        M_GAMESAVE_SLOT_MISSING(elseVal);
        M_GAMESAVE_TYPE_MISMATCH(STRING, elseVal);
        return cell->data;
    }

    void GameConfig::clear() { clearImpl(); }
    void GameConfig::erase(std::string_view slot) { eraseImpl(slot); }
    StorageType GameConfig::getSlotType(std::string_view slot) { return getSlotTypeImpl(slot); }


} // namespace magique
