#ifndef PERSISTENCEDEMO_H
#define PERSISTENCEDEMO_H

#include <magique/magique.hpp>

struct PersistenceDemo final : Game
{
    TextField inputField{{400, 25}, Anchor::MID_CENTER};
    TextButton saveButton{"Save", Anchor::MID_CENTER, {-50, -50}};
    TextButton loadButton{"Load", Anchor::MID_CENTER, {50, -50}};
    std::string_view path = "./save";
    EncryptionKey key = 0; // Null key = no encrpytion

    void onLoadingFinished() override
    {
        // If a filled storage is not saved (and it wasnt loaded) a warning is shown
        {
            GameStorage storage2;
            storage2.saveString("1", "hey");
        }
    }

    void onUpdateGame(GameState gameState) override
    {
        if (saveButton.getIsClicked())
        {
            GameStorage storage;
            storage.saveString("textField", inputField.getText());
            // Save the storage to file
            if (!GameStorageToFile(storage, path))
                LOG_WARNING("Failed to save storage");
        }

        if (loadButton.getIsClicked())
        {
            GameStorage storage;

            // Load the storage from disk
            if (!GameStorageFromFile(storage, path))
                LOG_WARNING("Failed to load storage");

            // std::optional allows to react if value is missing
            auto savedValue = storage.getString("textField");
            inputField.setText(savedValue.value_or("Default"));
        }
    }

    void onDrawGame(GameState state, Camera2D& camera) override
    {
        DrawText("Input Text to be saved:", 50, 50, 25, BLACK);
        inputField.draw();
        saveButton.draw();
        loadButton.draw();
    }
};


#endif //PERSISTENCEDEMO_H
