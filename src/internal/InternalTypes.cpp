#include <cstring>
#include <cmath>

#include <magique/internal/InternalTypes.h>

namespace magique
{
    void GameSaveStorageCell::grow(const int newSize)
    {
        if (newSize > allocatedSize)
        {
            const int newAllocatedSize = newSize;
            auto* newData = new char[newAllocatedSize];
            if (data)
            {
                std::memcpy(newData, data, size);
                delete[] data;
            }
            data = newData;
            allocatedSize = newAllocatedSize;
        }
    }

    void GameSaveStorageCell::append(const char* ptr, const int bytes)
    {
        grow(size + bytes);
        std::memcpy(data + size, ptr, bytes);
        size += bytes;
    }

    void GameSaveStorageCell::assign(const char* ptr, int bytes)
    {
        grow(bytes);
        std::memcpy(data, ptr, bytes);
        size = bytes;
    }

    bool EmitterData::operator==(const EmitterData& other) const
    {
        return resolutionScaling == other.resolutionScaling && r == other.r && g == other.g && b == other.b &&
            a == other.a && shape == other.shape && emissionShape == other.emissionShape && amount == other.amount &&
            lifeTime == other.lifeTime && width == other.width && height == other.height &&
            emissionX == other.emissionX && emissionY == other.emissionY && std::fabs(p1 - other.p1) < 1e-6 &&
            std::fabs(p2 - other.p2) < 1e-6 && std::fabs(p3 - other.p3) < 1e-6 && std::fabs(p4 - other.p4) < 1e-6 &&
            std::fabs(minScale - other.minScale) < 1e-6 && std::fabs(maxScale - other.maxScale) < 1e-6 &&
            std::fabs(minInitVeloc - other.minInitVeloc) < 1e-6 && std::fabs(maxInitVeloc - other.maxInitVeloc) < 1e-6 &&
            std::fabs(dirX - other.dirX) < 1e-6 && std::fabs(dirY - other.dirY) < 1e-6 &&
            std::fabs(spreadAngle - other.spreadAngle) < 1e-6 && scaleFunc == other.scaleFunc &&
            colorFunc == other.colorFunc;
    }

} // namespace magique