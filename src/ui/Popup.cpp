#include <magique/ui/controls/Popup.h>
#include <magique/ui/UI.h>
#include <magique/gamedev/TextDrawer.h>
#include <magique/util/RayUtils.h>
#include <magique/core/Engine.h>

namespace magique
{

    Popup::Popup(Rect size, Anchor anchor, Point inset, ScalingMode scaling) : UIObject(size, anchor, inset, scaling) {}

    void Popup::close() { UIRemovePopup(*this); }

    void Popup::action() const { actionFunc(); }

    void Popup::setOnAction(const ActionFunction& func) { actionFunc = func; }

    void Popup::onUpdate(const Rect& bounds, bool wasDrawn)
    {
        if (wasDrawn)
        {
            LayeredInput::ConsumeKey();
            LayeredInput::ConsumeMouse();
        }
    }

    ConfirmPopup::ConfirmPopup(std::string_view infoText) : Popup(GetScreenDims()), infoText(infoText)
    {
        confirm.setOnClick(
            [&](const Rect& bounds, int button)
            {
                action();
                close();
            });
        back.setOnClick([&](const Rect& bounds, int button) { close(); });
    }

    void ConfirmPopup::onDraw(const Rect& bounds)
    {
        const auto& theme = EngineGetTheme();
        const auto& fnt = EngineGetFont();

        DrawRectangleRec(Rect{UIGetTargetResolution()}, ColorAlpha(theme.backActive, 0.6F));

        const auto dims = MeasureTextEx(fnt, infoText.c_str(), fnt.baseSize, 1.0F);
        auto modalSize = GetScreenDims() * 0.15F;
        modalSize.x = std::min(modalSize.x, dims.x + 12);

        const Rect modal = Rect{UIGetAnchor(Anchor::MID_CENTER, modalSize), modalSize};
        DrawRectFrameFilled(modal, theme.background, theme.backOutline);

        TextDrawer drawer{fnt, modal};
        drawer.gapV().center(infoText, theme.textHighlight);

        confirm.align(Anchor::MID_LEFT, modal, {2, 0});
        confirm.draw();

        back.align(Anchor::MID_RIGHT, modal, {2, 0});
        back.draw();
    }


} // namespace magique
