#ifndef MAGIQUE_CORE_UI_H
#define MAGIQUE_CORE_UI_H




namespace magique
{

    inline void DrawUI()
    {
        if (global::CONFIGURATION.showPerformanceOverlay)
        {
            global::PERF_DATA.perfOverlay.draw();
        }
    }
} // namespace magique
#endif //MAGIQUE_CORE_UI_H