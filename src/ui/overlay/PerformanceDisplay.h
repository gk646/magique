#ifndef PERFORMANCEDISPLAY_H
#define PERFORMANCEDISPLAY_H

#include <magique/fwd.hpp>


struct PerformanceBlock final
{
    float width = 0;
    char text[32] = {};
};


struct PerformanceDisplay
{
    PerformanceBlock blocks[5]; // 5 blocks for FPS, CPU, GPU, Upload, Download
    int tickCounter = 0;
    int updateDelayTicks = 20;

    void draw(const Font& font);

private:
    void updateValues(const Font& font);
};


#endif //PERFORMANCEDISPLAY_H