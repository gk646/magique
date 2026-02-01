// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_ACHIEVEMENTS_H
#define MAGIQUE_ACHIEVEMENTS_H

#include <string>
#include <functional>

//===============================================
// Achievements Module
//===============================================
// .....................................................................
// This helps to create dynamic and flexible achievements that are automatically managed
// Note: Use the persistence function together with the GameSaveData to persist the state
// .....................................................................

namespace magique
{
    using AchievementCallback = void (*)(std::string& name);

    // Defines a new achievable achievement and adds it to the system
    // Note: If constraint evaluates to true the achievement is done - only evaluated if not finished
    // Failure: returns false if the name already exists
    bool AddAchievement(const std::string& name, const std::function<bool()>& constraint);

    // Returns true if the achievement exists and is finished
    // Failure: returns false otherwise
    bool IsAchievementDone(const std::string& name);

    // Manually finishes the achievement and triggers the callback - skips evaluating the condition
    // Failure: returns false if achievement is already done or doesn't exist
    bool TriggerAchievement(const std::string& name);

    // Sets the callback for finished achievements
    // Called each time an achievement is finished for the first time
    void SetAchievementCallback(AchievementCallback callback);

    // Checks for completion of any achievements
    // Note: This is called automatically every 30 ticks - but may want to be invoked manually at certain points
    void CheckAchievements();

    //================= PERSISTENCE =================//

    // Returns a json with the achievement state
    // Note: Only name and completion state is saved not the constraint (... how would you do that?)
    std::string ExportAchievementsState();

    // Restores a previous achievements state from the given data
    // Note: only sets the completion state of existing achievements only if the name matches with the saved data
    bool ImportAchievementsState(std::string_view json);

} // namespace magique

#endif //MAGIQUE_ACHIEVEMENTS_H
