#ifndef PERSISTENCEDEMO_H
#define PERSISTENCEDEMO_H

#include <string>
#include <utility>

#include <magique/core/Game.h>
#include <magique/persistence/GameLoader.h>
#include <magique/persistence/GameSaver.h>
#include <magique/persistence/container/GameSave.h>
#include <magique/ui/controls/TextField.h>
#include <magique/ui/controls/Button.h>
#include <magique/ui/UI.h>

using namespace magique;

// Initialize interfaces
GameSaver GAME_SAVER{};
GameLoader GAME_LOADER{};
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
TextField inputField{250, 250, Anchor::TOP_CENTER};

struct SaveClass final : ITask<GameSave>
{
    void execute(GameSave& res) override
    {
        printf("Saving text:%s\n",inputField.getText().c_str());
        res.saveString(StorageID::TEXT_FIELD_STRING, inputField.getText());
    }
};

struct LoadClass final : ITask<GameSave>
{
    void execute(GameSave& res) override
    {
        const std::string savedText = res.getStringOrElse(StorageID::TEXT_FIELD_STRING, "Not found");
        printf("Loading text:%s\n",savedText.c_str());
        inputField.setText(savedText.c_str());
    }
};


struct PersistenceDemo final : Game
{
    TextButton saveButton{"Save", 450, 350};
    TextButton loadButton{"Load", 1000, 350};

    void onStartup(AssetLoader& loader) override
    {
        GAME_SAVER.registerTask(new SaveClass());
        GAME_LOADER.registerTask(new LoadClass());
    }

    void onLoadingFinished() override {}

    void updateGame(GameState gameState) override {}

    void drawGame(GameState gameState, Camera2D& camera2D) override
    {
        DrawText("Input Text to be saved:", 50, 50, 25, BLACK);

        inputField.draw();

        saveButton.draw();
        if (saveButton.getIsClicked())
        {
            printf("Clicked Save\n");
            GameSave save;
            GAME_SAVER.save(save);
            SaveToDisk(save, SAVE_PATH);
        }

        loadButton.draw();
        if (loadButton.getIsClicked())
        {
            printf("Clicked Load\n");
            GameSave save;
            LoadFromDisk(save, SAVE_PATH);
            GAME_LOADER.load(save);
        }
    }
};


#endif //PERSISTENCEDEMO_H