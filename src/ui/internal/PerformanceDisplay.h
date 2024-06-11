#ifndef PERFORMANCEDISPLAY_H
#define PERFORMANCEDISPLAY_H

struct PerformanceBlock final
{
    float width = 0;
    char text[32] = {};
};


struct PerformanceDisplay
{
    PerformanceBlock blocks[6]; // 5 blocks for FPS, CPU, GPU, DrawCalls, Upload, Download
    int tickCounter = 0;
    int updateDelayTicks = 10;

    void draw();
    void updateValues();
};


#endif //PERFORMANCEDISPLAY_H