#ifndef PERSISTENCEDEMO_H
#define PERSISTENCEDEMO_H

#include <string>
#include <utility>

#include <magique/core/Game.h>
#include <magique/persistence/TaskInterface.h>
#include <magique/persistence/GameSaveData.h>
#include <magique/ui/controls/TextField.h>
#include <magique/ui/controls/Button.h>
#include <magique/ui/UI.h>

using namespace magique;

// Initialize interfaces
TaskInterface GAME_SAVER{};
TaskInterface GAME_LOADER{};
const char* SAVE_PATH = "./mySave.save";

// Define storage ID's
enum class StorageID
{
    TEXT_FIELD_STRING,
};

// Button with text
struct TextButton final : Button
{
    std::string text;

    TextButton(std::string text, float x, float y) : Button(x, y, 150, 150), text(std::move(text)) {}

    void onDraw(const Rectangle& bounds) override
    {
        drawDefault(bounds);
        DrawText(text.c_str(), bounds.x, bounds.y, 25, BLACK);
    }
};

// Variables
TextField inputField{250, 250, Anchor::MID_CENTER};

struct SaveClass final : ITask<GameSaveData>
{
    void execute(GameSaveData& res) override
    {
        printf("Saving text:%s\n", inputField.getText().c_str());
        res.saveString(StorageID::TEXT_FIELD_STRING, inputField.getText());
    }
};

struct LoadClass final : ITask<GameSaveData>
{
    void execute(GameSaveData& res) override
    {
        const std::string savedText = res.getStringOrElse(StorageID::TEXT_FIELD_STRING, "Not found");
        printf("Loading text:%s\n", savedText.c_str());
        inputField.setText(savedText.c_str());
    }
};


struct PersistenceDemo final : Game
{
    TextButton saveButton{"Save", 550, 350};
    TextButton loadButton{"Load", 1225, 350};

    void onStartup(AssetLoader& loader) override
    {
        GAME_SAVER.registerTask(new SaveClass());
        GAME_LOADER.registerTask(new LoadClass());
    }

    void onLoadingFinished() override {}

    void updateGame(GameState gameState) override
    {
        if (saveButton.getIsClicked())
        {
            GameSaveData save; // Cleaned up automatically
            GAME_SAVER.invoke(save);
            SaveToDisk(save, SAVE_PATH);
        }

        if (loadButton.getIsClicked())
        {
            GameSaveData save; // Cleaned up automatically
            LoadFromDisk(save, SAVE_PATH);
            GAME_LOADER.invoke(save);
        }
    }

    void drawGame(GameState gameState, Camera2D& camera2D) override
    {
        DrawText("Input Text to be saved:", 50, 50, 25, BLACK);
        inputField.draw();
        saveButton.draw();
        loadButton.draw();
    }
};


#endif //PERSISTENCEDEMO_H