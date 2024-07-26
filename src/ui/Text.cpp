#include <vector>

#include <magique/ui/Text.h>
#include <magique/internal/DataStructures.h>

// Which type the value has
enum ValueType : uint8_t
{
    FLOAT,
    STRING,
    INT
};

// Which index and which type -> unique
struct ValueInfo final
{
    ValueType type = INT;
    int index = 0;
};

std::vector<std::string> STRINGS;
std::vector<float> FLOATS;
std::vector<int> INTS;


void eraseValue(ValueInfo info) {}

namespace magique
{

    HashMap<std::string, ValueInfo> VALUES;


    void SetFormatMask(const char* prefix, const char* suffix) {}

    void SetFormatValue(const char* format, const char* val)
    {
        auto it = VALUES.find(format);
        if (it != VALUES.end())
        {
            eraseValue(it->second);
        }
        auto size = static_cast<int>(STRINGS.size());
    }

    void DrawTextFmt(const char* text) {}
} // namespace magique