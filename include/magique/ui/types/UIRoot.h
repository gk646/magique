#ifndef MAGIQUE_UIROOT_H_
#define MAGIQUE_UIROOT_H_

#include <magique/core/Types.h>
#include <magique/ui/types/UIContainer.h>

//-----------------------------------------------
// UIStateRoot
//-----------------------------------------------
// .....................................................................
// This class is the root of all ui-elements for a given state. States are basically scenes in other engines.
// This helps to split up logic
// .....................................................................

namespace magique
{
    struct UIRoot final
    {
        //----------------- ACCESS -----------------//

        // Adds a new ui element to the root - elements within the same layer are drawn in the order they are added
        // Takes owner ship of the pointer - dont save or access it after passing it to this method - use new MyClass()
        // Note: name is copied and only has to be valid until this method returns
        void addObject(const char* name, UIObject* object, UILayer layer = UILayer::MEDIUM);

        // Returns a pointer to the ui-object with the given name - optionally pass a specific type
        // Failure: returns nullptr if the given element does not exist!
        template <typename T = UIObject>
        T* getObject(const char* name) const;

        // Removes and deletes the uiobject with the given name
        // Failure: returns false if the element wasnt found
        bool removeObject(const char* name) const;

        //----------------- DRAW -----------------//

        // Calls the draw method on all elements in the correct order
        void draw();

    private:
        UIObject* getObjectImpl(const char* name) const;
        int state = 0;
        friend struct UIData;
    };
} // namespace magique


//----------------- IMPLEMENTATION -----------------//

namespace magique
{
    template <typename T>
    T* UIRoot::getObject(const char* name) const
    {
        return static_cast<T>(getObjectImpl(name));
    }

} // namespace magique
#endif //MAGIQUE_UIROOT_H_