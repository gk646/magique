#include <magique/util/RayUtils.h>


namespace  magique
{
    float MeasureTextUpTo(char* text, const int index, const Font& f, const float fontSize, const float spacing)
    {
        const auto temp = text[index];
        text[index] = '\0';
        const float ret = MeasureTextEx(f, text, fontSize, spacing).x;
        text[index] = temp;
        return ret;
    }

} // namespace magique