#ifndef MAGIQUE_ACHIEVEMENTS_H
#define MAGIQUE_ACHIEVEMENTS_H

#include <string>
#include <functional>
#include <magique/core/Types.h>

//-----------------------------------------------
// Achievements Module
//-----------------------------------------------
// .....................................................................
// This helps to create dynamic and flexible achievements that are automatically managed

namespace magique
{
    using AchievementCallback = void (*)(const Achievement& achievement);

    // Defines a new achieveable achievement and adds it to the sysem
    // Note: If constraint evaluates to true the achievement is done - only evaluated if not finished
    // Failure: returns false if the name already exists
    bool AddAchievement(const std::string& name, const std::function<bool()>& constraint);

    // Returns true if the achievement exists and is finished
    // Failure: returns false if achievement doesnt exist or isnt finished
    bool IsAchievementDone(const std::string& name);

    // Sets this achievement to done and triggers the callback - skips evaluation the condition
    // Failure: returns false if achievement is already done or doesnt exist
    bool TriggerAchievement(const std::string& name);

    // Sets the callback for finished achievements
    // Called each time a achievement is done for the first time ONLY
    void SetAchievementCallback(AchievementCallback callback);

    // Checks for completion of any achievements
    // Note: This is called automatically if your using the game template
    void CheckAchievements();

    //----------------- PERSISTENCE -----------------//

    // Returns a data pointer and size to save the current achievements state
    // Useful for saving it in the game save
    // IMPORTANT: allocates memory on each call
    std::pair<const unsigned char*, int> GetAchievementsData();

    // Restores a previous achievements state from the given data
    bool LoadAchievements(const unsigned char* data, int size);


}; // namespace magique

#endif //MAGIQUE_ACHIEVEMENTS_H