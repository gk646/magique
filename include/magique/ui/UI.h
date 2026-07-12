// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_UI_H
#define MAGIQUE_UI_H

#include <functional>
#include <vector>
#include <magique/core/Types.h>

//===============================================
// UI Module
//===============================================
// .....................................................................
// All UI modules in magique uses a fixed resolution of 1920x1080 (default). This means that you can define all dimensions,
// offsets and gaps in absolute numbers. These values are then automatically scaled to fit to the current resolution.
// To fit different screen ratios (21:9, 4:3) you can use anchor points.
// The intended workflow is to completely work in 1920x1080 and specify all measurements in that resolution.
// This makes it easy to follow designs and reason about distances while the engine handles scaling automatically.
//
// Note: Also contains the GamepadMapping functionality that allows to add gamepad navigation to Menu and other UIObjects
// .....................................................................

namespace magique
{
    // T = Top / M = Middle / B = Bottom
    // L = Left / C = Center / R = Right
    // =========================
    // |(TL)      (TC)      (TR)|
    // |                        |
    // |(ML)      (MC)      (MR)|
    // |                        |
    // |(BL)      (BC)      (BR)|
    // =========================
    // Returns the (top-left) coordinates of the specified anchor point in absolute coordinates (draw coordinates)
    // Passing a size aligns the point to be centered bound relative to the dimension
    // Inset applies an offset on the border points inwards
    // Passing a UIObject return the anchor point relative to the object bounds
    Point UIGetAnchor(Anchor anchor, const UIObject& relative, Point size = {}, Point inset = {});
    Point UIGetAnchor(Anchor anchor, const Rect& relative, Point size = {}, Point inset = {});
    // Uses the screen dimensions as base
    Point UIGetAnchor(Anchor anchor, Point size = {}, Point inset = {});

    // Accepts a value in the logical resolution and returns the value in the current resolution (scaled vertically)
    // Note: This is useful when needing static offsets that automatically scale
    float UIGetScaled(float val);
    Point UIGetScaled(Point p);

    // Returns the current horizontal and vertical scaling
    Point UIGetScaling();

    // Returns the position where the left mouse button was first pressed - resets when the button is released
    // Failure: returns {-1,-1} if the mouse was not yet pressed or was released
    Point UIGetDragStart();

    // Sets the logical ui resolution in which all numbers are interpreted
    // IMPORTANT: this likely completely changes all your UI - should only be done when using pixel art with fixed resolutions
    void UISetSourceResolution(Point resolution);

    // Sets the resolution the UI scales TO - if set to (0,0) display resolution is used
    // Note: This is useful if your using different render targets
    void UISetTargetResolution(Point resolution);
    Point UIGetTargetResolution();

    // Needs to be called MANUALLY to draw the popup - should be called as the last thing thats rendered (on top of the rest)
    void UIDrawPopups();

    // Adds a new popup on top - doesn't allow duplicates - handled in the order they are added
    // Calling this function for an existing one
    void UIAddPopup(Popup& popup);
    bool UIRemovePopup(Popup& popup);
    const std::vector<Popup*>& UIGetPopups();

    // Return the world mouse pos using CameraGet()
    Point GetWorldMousePos();

    // If enabled, shows the hitboxes off all ui elements in BLUE
    void UIShowHitboxes(bool value);

    // Returns true a gamepad is the currently used input method
    // This is smart - e.g. if keyboard input is detected, switches to keyboard until (any) gamepad input is detected
    // Note: This is useful for deciding which UI hints to draw or what input device to pick
    // Note: If true, mouse cursor is disabled (hidden)
    bool UIUsingGamepad();

    // Returns a rectangle that adjusted to be on the screen (target resolution) by picking which corner the mouse is
    // Default its bottom right corner is the mouse position - can be offset manually
    // Note: Useful for tooltips
    Rectangle UIGetRectOnScreen(const Point& offset, float width, float height, Point base = GetMousePosition());

    // Getters for input that allows for consumption - when consumed all methods return false
    // The consumed state is automatically reset at the beginning of each update tick
    // Note: This is very useful when dealing with layered UI to prevent unwanted input propagation
    // Note: You can still use the raylib inputs methods if you need exceptions to this rule!
    struct LayeredInput final
    {
        // Returns true only if input state AND not consumed
        static bool IsKeyPressed(int key);
        static bool IsKeyDown(int key);
        static bool IsKeyReleased(int key); // consumed OR Released
        static bool IsKeyPressedRepeat(int key);

        static bool IsMouseButtonPressed(int key);
        static bool IsMouseButtonDown(int key);
        static bool IsMouseButtonReleased(int key);

        static bool IsGamepadButtonPressed(int gamepad, int key);
        static bool IsGamepadButtonDown(int gamepad, int key);
        static bool IsGamepadButtonReleased(int gamepad, int key); // consumed OR Released

        // Consume the key and gamepad input or mouse input - all methods will return false
        //      - deferred: will be set after the current
        static void ConsumeKey();
        static void ConsumeMouse();
        static bool GetIsKeyConsumed();
        static bool GetIsMouseConsumed();
    };

    // Artificially creates a mouse press - optionally allows to set the mouse position before
    void UIEmitMousePress(MouseButton button = MOUSE_BUTTON_LEFT, Point mouse = {-1});

    // Receives current state - returns the point where the mouse should be next
    // -1 mouse positions will be ignored
    using GamepadMappingFunc = std::function<Point(GamepadMappingState& state, GamepadButton button)>;

    // Used to enable gamepad (and arrow keys) navigation of UI menus - without any logic changes
    // Works by positioning the cursor (even when not shown) to the correct position
    // However menus should still be designed with controller in mind to make it easier
    // Note: UIObject::Menu allows to store a mapping and automatically applies it if its activated
    struct GamepadMapping
    {
        GamepadMapping(UIObject& object, const GamepadMappingFunc& func = nullptr);
        virtual ~GamepadMapping() = default;

        // Resets the state
        void reset();

        // Sets the callback on event
        void setOnEvent(const GamepadMappingFunc& func);

        // Called on button presses other than the special functions
        // Note: Invalid parameter is -1
        // Default: -1 (ignored)
        virtual Point onButton(GamepadButton gamepad, KeyboardKey key) { return -1; }

        // Returns the object its attached to or state
        UIObject& getObject();
        const GamepadMappingState& getState() const;

        // Can be called manually to trigger the corresponding callback
        // Note: These are called AUTOMATICALLY when the appropriate buttons are pressed
        void triggerEvent(GamepadMappingEvent event, GamepadButton button = {});

    private:
        void setMouse(Point pos);
        GamepadMappingState state_{};
        UIObject* object = nullptr;
        GamepadMappingFunc func;
    };

    // Sets/gets the current input map
    //      - resetStack: clears the stack of mappings and sets this as the top one (default for menus)
    // Note: This is set automatically called when a menu with a mapping is activated
    // Stacking mappings allows to delegate logic - when a object is clicked its mapping can be activated and takes over
    // Once back is pressed (and the back action is not consumed) the previous mapping will be set active
    void UISetGamepadMap(GamepadMapping* map, bool resetStack = false);

    // Sets the previously active map active - automatically called if the back action is not consumed by the active mapping
    // Note: Only works if there is at least 1 previous mapping active - else stays on the current one
    void UISetPreviousGamepadMap();

    // Returns the current active mapping
    GamepadMapping* UIGetGamepadMap();

    // Sets the mouse position to the world pos - useful when using ui controls in worldspace not ui space
    // Destructor resets it back to original screen pos
    struct MouseToWorld
    {
        MouseToWorld();
        ~MouseToWorld();

    private:
        Point prev;
    };

} // namespace magique

#endif // MAGIQUE_UI_H
