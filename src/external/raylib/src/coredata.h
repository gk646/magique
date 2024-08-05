#ifndef RAYLIB_COREDATA_H
#define RAYLIB_COREDATA_H

// Added so we can easily access the core data internally
// Should change nothing in any setup - just made this piece include-able anywhere else

#include "config.h"
#include <raylib/rlgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern Texture2D texShapes;    // Texture used on shapes drawing (white pixel loaded by rlgl)
extern Rectangle texShapesRec; // Texture source rectangle used on shapes drawing

typedef struct
{
    int x;
    int y;
} Point;
typedef struct
{
    unsigned int width;
    unsigned int height;
} Size;

// Core global state context data
typedef struct CoreData
{
    struct
    {
        const char* title;     // Window text title const pointer
        unsigned int flags;    // Configuration flags (bit based), keeps window state
        bool ready;            // Check if window has been initialized successfully
        bool fullscreen;       // Check if fullscreen mode is enabled
        bool shouldClose;      // Check if window set for closing
        bool resizedLastFrame; // Check if window has been resized last frame
        bool eventWaiting;     // Wait for events before ending frame
        bool usingFbo;         // Using FBO (RenderTexture) for rendering instead of default framebuffer

        Point position;         // Window position (required on fullscreen toggle)
        Point previousPosition; // Window previous position (required on borderless windowed toggle)
        Size display;           // Display width and height (monitor, device-screen, LCD, ...)
        Size screen;            // Screen width and height (used render area)
        Size previousScreen;    // Screen previous width and height (required on borderless windowed toggle)
        Size currentFbo;        // Current render width and height (depends on active fbo)
        Size render;            // Framebuffer width and height (render area, including black bars if required)
        Point renderOffset;     // Offset from render area (must be divided by 2)
        Size screenMin;         // Screen minimum width and height (for resizable window)
        Size screenMax;         // Screen maximum width and height (for resizable window)
        Matrix screenScale;     // Matrix to scale screen (framebuffer rendering)

        char** dropFilepaths;       // Store dropped files paths pointers (provided by GLFW)
        unsigned int dropFileCount; // Count dropped files strings

    } Window;
    struct
    {
        const char* basePath; // Base path for data storage

    } Storage;
    struct
    {
        struct
        {
            int exitKey;                              // Default exit key
            char currentKeyState[MAX_KEYBOARD_KEYS];  // Registers current frame key state
            char previousKeyState[MAX_KEYBOARD_KEYS]; // Registers previous frame key state

            // NOTE: Since key press logic involves comparing prev vs cur key state, we need to handle key repeats specially
            char keyRepeatInFrame[MAX_KEYBOARD_KEYS]; // Registers key repeats for current frame

            int keyPressedQueue[MAX_KEY_PRESSED_QUEUE]; // Input keys queue
            int keyPressedQueueCount;                   // Input keys queue count

            int charPressedQueue[MAX_CHAR_PRESSED_QUEUE]; // Input characters queue (unicode)
            int charPressedQueueCount;                    // Input characters queue count

        } Keyboard;
        struct
        {
            Vector2 offset;           // Mouse offset
            Vector2 scale;            // Mouse scaling
            Vector2 currentPosition;  // Mouse position on screen
            Vector2 previousPosition; // Previous mouse position

            int cursor;          // Tracks current mouse cursor
            bool cursorHidden;   // Track if cursor is hidden
            bool cursorOnScreen; // Tracks if cursor is inside client area

            char currentButtonState[MAX_MOUSE_BUTTONS];  // Registers current mouse button state
            char previousButtonState[MAX_MOUSE_BUTTONS]; // Registers previous mouse button state
            Vector2 currentWheelMove;                    // Registers current mouse wheel variation
            Vector2 previousWheelMove;                   // Registers previous mouse wheel variation

        } Mouse;
        struct
        {
            int pointCount;                            // Number of touch points active
            int pointId[MAX_TOUCH_POINTS];             // Point identifiers
            Vector2 position[MAX_TOUCH_POINTS];        // Touch position on screen
            char currentTouchState[MAX_TOUCH_POINTS];  // Registers current touch state
            char previousTouchState[MAX_TOUCH_POINTS]; // Registers previous touch state

        } Touch;
        struct
        {
            int lastButtonPressed;                                       // Register last gamepad button pressed
            int axisCount[MAX_GAMEPADS];                                 // Register number of available gamepad axis
            bool ready[MAX_GAMEPADS];                                    // Flag to know if gamepad is ready
            char name[MAX_GAMEPADS][64];                                 // Gamepad name holder
            char currentButtonState[MAX_GAMEPADS][MAX_GAMEPAD_BUTTONS];  // Current gamepad buttons state
            char previousButtonState[MAX_GAMEPADS][MAX_GAMEPAD_BUTTONS]; // Previous gamepad buttons state
            float axisState[MAX_GAMEPADS][MAX_GAMEPAD_AXIS];             // Gamepad axis state

        } Gamepad;
    } Input;
} CoreData;

typedef struct rlglData
{
    rlRenderBatch* currentBatch; // Current render batch
    rlRenderBatch defaultBatch;  // Default internal render batch

    struct
    {
        int drawCalls;                   // Total count rlRenderActiveBatch() has been called this frame
        int prevDrawCalls;               // Total count rlRenderActiveBatch() has been called this frame
        int vertexCounter;               // Current active render batch vertex counter (generic, used for all batches)
        float texcoordx, texcoordy;      // Current active texture coordinate (added on glVertex*())
        float normalx, normaly, normalz; // Current active normal (added on glVertex*())
        unsigned char colorr, colorg, colorb, colora; // Current active color (added on glVertex*())

        int currentMatrixMode;  // Current matrix mode
        Matrix* currentMatrix;  // Current matrix pointer
        Matrix modelview;       // Default modelview matrix
        Matrix projection;      // Default projection matrix
        Matrix transform;       // Transform matrix to be used with rlTranslate, rlRotate, rlScale
        bool transformRequired; // Require transform matrix application to current draw-call vertex (if required)
        Matrix stack[RL_MAX_MATRIX_STACK_SIZE]; // Matrix stack for push/pop
        int stackCounter;                       // Matrix stack counter

        unsigned int defaultTextureId; // Default texture used on shapes/poly drawing (required by shader)
        unsigned int activeTextureId
            [RL_DEFAULT_BATCH_MAX_TEXTURE_UNITS]; // Active texture ids to be enabled on batch drawing (0 active by default)
        unsigned int defaultVShaderId;            // Default vertex shader id (used by default shader program)
        unsigned int defaultFShaderId;            // Default fragment shader id (used by default shader program)
        unsigned int defaultShaderId;             // Default shader program id, supports vertex color and diffuse texture
        int* defaultShaderLocs;                   // Default shader locations pointer to be used on rendering
        unsigned int currentShaderId; // Current shader id to be used on rendering (by default, defaultShaderId)
        int*
            currentShaderLocs; // Current shader locations pointer to be used on rendering (by default, defaultShaderLocs)

        bool stereoRender;          // Stereo rendering flag
        Matrix projectionStereo[2]; // VR stereo rendering eyes projection matrices
        Matrix viewOffsetStereo[2]; // VR stereo rendering eyes view offset matrices

        // Blending variables
        int currentBlendMode;           // Blending mode active
        int glBlendSrcFactor;           // Blending source factor
        int glBlendDstFactor;           // Blending destination factor
        int glBlendEquation;            // Blending equation
        int glBlendSrcFactorRGB;        // Blending source RGB factor
        int glBlendDestFactorRGB;       // Blending destination RGB factor
        int glBlendSrcFactorAlpha;      // Blending source alpha factor
        int glBlendDestFactorAlpha;     // Blending destination alpha factor
        int glBlendEquationRGB;         // Blending equation for RGB
        int glBlendEquationAlpha;       // Blending equation for alpha
        bool glCustomBlendModeModified; // Custom blending factor and equation modification status

        int framebufferWidth;  // Current framebuffer width
        int framebufferHeight; // Current framebuffer height

    } State; // Renderer state
    struct
    {
        bool vao; // VAO support (OpenGL ES2 could not support VAO extension) (GL_ARB_vertex_array_object)
        bool
            instancing; // Instancing supported (GL_ANGLE_instanced_arrays, GL_EXT_draw_instanced + GL_EXT_instanced_arrays)
        bool texNPOT;       // NPOT textures full support (GL_ARB_texture_non_power_of_two, GL_OES_texture_npot)
        bool texDepth;      // Depth textures supported (GL_ARB_depth_texture, GL_OES_depth_texture)
        bool texDepthWebGL; // Depth textures supported WebGL specific (GL_WEBGL_depth_texture)
        bool texFloat32;    // float textures support (32 bit per channel) (GL_OES_texture_float)
        bool texFloat16;    // half float textures support (16 bit per channel) (GL_OES_texture_half_float)
        bool
            texCompDXT; // DDS texture compression support (GL_EXT_texture_compression_s3tc, GL_WEBGL_compressed_texture_s3tc, GL_WEBKIT_WEBGL_compressed_texture_s3tc)
        bool
            texCompETC1; // ETC1 texture compression support (GL_OES_compressed_ETC1_RGB8_texture, GL_WEBGL_compressed_texture_etc1)
        bool texCompETC2; // ETC2/EAC texture compression support (GL_ARB_ES3_compatibility)
        bool texCompPVRT; // PVR texture compression support (GL_IMG_texture_compression_pvrtc)
        bool
            texCompASTC; // ASTC texture compression support (GL_KHR_texture_compression_astc_hdr, GL_KHR_texture_compression_astc_ldr)
        bool texMirrorClamp; // Clamp mirror wrap mode supported (GL_EXT_texture_mirror_clamp)
        bool texAnisoFilter; // Anisotropic texture filtering support (GL_EXT_texture_filter_anisotropic)
        bool computeShader;  // Compute shaders support (GL_ARB_compute_shader)
        bool ssbo;           // Shader storage buffer object support (GL_ARB_shader_storage_buffer_object)

        float maxAnisotropyLevel; // Maximum anisotropy level supported (minimum is 2.0f)
        int maxDepthBits;         // Maximum bits for depth component

    } ExtSupported; // Extensions supported flags
} rlglData;


extern CoreData CORE; // Global CORE state context
extern rlglData RLGL;
#ifdef __cplusplus
}
#endif

#endif //RAYLIB_COREDATA_H