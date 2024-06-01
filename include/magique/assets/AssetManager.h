#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H


//-----------------------------------------------
// Asset Management Module
//-----------------------------------------------
// .....................................................................
// Per default all methods are made to load from memory to work with the asset image
// .....................................................................

namespace magique
{

    //-----------------Loading-----------------//

    // Loads a .png file and
    bool RegisterSpritesheet(const char* path, int size, const char* name);



} // namespace magique

#endif //ASSETMANAGER_H