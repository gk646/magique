#ifndef MAGIQUE_ACHIEVEMENTS_H
#define MAGIQUE_ACHIEVEMENTS_H

#include <string>
#include <magique/core/Types.h>

//-----------------------------------------------
// Achievements Module
//-----------------------------------------------
// .....................................................................
// This helps to create dynamic and flexible achievements that are automatically managed
// .....................................................................

namespace magique
{
    using AchievementCallback = void (*)(std::string& name);

    // Defines a new achieveable achievement and adds it to the sysem
    // Note: If constraint evaluates to true the achievement is done - only evaluated if not finished
    // Failure: returns false if the name already exists
    bool AddAchievement(const std::string& name, const std::function<bool()>& constraint);

    // Returns true if the achievement exists and is finished
    // Failure: returns false otherwise
    bool IsAchievementDone(const std::string& name);

    // Manually finishes the achievement and triggers the callback - skips evaluating the condition
    // Failure: returns false if achievement is already done or doesnt exist
    bool TriggerAchievement(const std::string& name);

    // Sets the callback for finished achievements
    // Called each time a achievement is finished for the first time
    void SetAchievementCallback(AchievementCallback callback);

    // Checks for completion of any achievements
    // Note: This is called automatically every 30 ticks
    void CheckAchievements();

    //----------------- PERSISTENCE -----------------//

    // Returns an allocated pointer and size to save the current achievements state - use e.g. GameSave::saveData()
    // Note: Only name and completion state is saved not the constraint - achievements have to be added each time
    // IMPORTANT: allocates memory on each call
    [[nodiscard("Allocates")]] DataPointer<const unsigned char> GetAchievementsData();

    // Restores a previous achievements state from the given data
    // Note: only sets the completion state of existing achievements and only if the name matches with the saved data
    bool LoadAchievements(const unsigned char* data, int size);


}; // namespace magique

#endif //MAGIQUE_ACHIEVEMENTS_H