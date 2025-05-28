// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_MINIMAP_H
#define MAGIQUE_MINIMAP_H

#include <initializer_list>
#include <magique/fwd.hpp>
#include <raylib/raylib.h>

//===============================================
// Minimap Module
//===============================================
// .....................................................................
// .....................................................................


namespace magique
{

    // Specifies a color
    struct MinimapColorInfo final
    {
        Color color;
        int tileClass;
    };

    struct MinimapOption final
    {
        enum class MinimapShape : uint8_t
        {
            RECT,
            CIRCLE
        } shape;
        
    };

    struct Minimap final
    {
        explicit Minimap(const TileSet& tileSheet);

        void setColors(const std::initializer_list<MinimapColorInfo>& colors);

        void draw(const TileMap& tileMap);

        // Centers the minimap on the given
        void centerOn(int x, int y);

    private:
        int centerX, centerY;
    };

} // namespace magique

#endif //MAGIQUE_MINIMAP_H