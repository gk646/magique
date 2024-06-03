#ifndef MAGIQUE_CORE_UI_H
#define MAGIQUE_CORE_UI_H




namespace magique
{

    inline void DrawUI()
    {
        if (CONFIGURATION.showPerformanceOverlay)
        {
            PERF_DATA.perfOverlay.draw(GetFontDefault());
        }
    }
} // namespace magique
#endif //MAGIQUE_CORE_UI_H