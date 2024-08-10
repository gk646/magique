#ifndef MAGIQUE_UI_H
#define MAGIQUE_UI_H

#include <magique/core/Types.h>

//-----------------------------------------------
// UI Module
//-----------------------------------------------
// .....................................................................
// The UI module in magique uses a fixed logical resolution of 1920x1080. This means that you can define all dimensions,
// offsets and gaps in absolute numbers. These values are then automatically scaled to fit to the current resolution.
// To fit all screen ratios (16:9, 4:3) you can use anchor points.
// The intended workflow is to completely work in 1920x1080 and specify all measurements in that resolution.
// This makes it easy to follow designs and reason about distances while the engine handles scaling automatically.
//
// States: Additionally, the UI is divided into game states (e.g. MAIN_MENU, GAME_OVER...)
// This helps to organize what to draw and when
// .....................................................................

namespace magique
{
    //----------------- UI OBJECTS -----------------//

    // Adds a new ui element to the given state - elements within the same layer are in the order they are added
    // Takes owner ship of the pointer - dont save or access it after passing it to this method
    // Note: the name has to be unique across all states - else will be overwritten silently
    // Note: The same object can be added to multiple gamestates (with the same name)!
    void AddUIObject(const char* name, GameState gameState, UIObject* object, UILayer layer = UILayer::MEDIUM);

    // Returns a pointer to the ui-object with the given name - optionally pass a specific type
    // Failure: returns nullptr if the given element does not exist!
    template <typename T = UIObject>
    T* GetUIObject(const char* name);

    // Removes the object with the given name from all states - optionally limit to a given gamestate
    // Failure: returns false if the element wasnt found
    bool RemoveUIObject(const char* name, GameState gameState = GameState(INT32_MAX));
    bool RemoveUIObject(UIObject* object, GameState gameState = GameState(INT32_MAX));

    //----------------- SETTERS -----------------//

    // Sets the loading screen instance to handle different loading scenarios - see ui/LoadingScreen.h for more info
    // Note: Takes ownership of the pointer - dont save or access it after passing it to this method
    void SetLoadingScreen(LoadingScreen* loadingScreen);

    //----------------- UTIL -----------------//

    // T = Top / M = Middle / B = Bottom
    // L = Left / C = Center / R = Right
    // -------------------------
    // |(TL)      (TC)      (TR)|
    // |                        |
    // |(ML)      (MC)      (MR)|
    // |                        |
    // |(BL)      (BC)      (BR)|
    // -------------------------
    // Returns the (top-left) coordinates of the specified anchor point
    // Passing width and height aligns the anchor to fit them - inset applies a inset for the border points inwards
    Point GetUIAnchor(AnchorPosition anchor, float width = 0, float height = 0, float inset = 0);

    // Accepts a value in the logical resolution and returns the value in the current resolution (scaled horizontally)
    float GetScaled(float val);

    // Returns the current scale horizontal and vertical scaling
    Point GetUIScaling();

} // namespace magique


//----------------- IMPLEMENTATION -----------------//
namespace magique
{
    template <typename T>
    T* GetUIObject(const char* name)
    {
        return static_cast<T*>(GetUIObject<void>(name));
    }

} // namespace magique

#endif //MAGIQUE_UI_H