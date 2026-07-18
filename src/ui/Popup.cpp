#include <magique/ui/controls/Popup.h>
#include <magique/ui/UI.h>
#include <magique/gamedev/TextDrawer.h>
#include <magique/util/RayUtils.h>
#include <magique/core/Engine.h>

namespace magique
{
    Popup::Popup(Rect size, Anchor anchor, Point inset, ScalingMode scaling) : UIObject(size, anchor, inset, scaling) {}

    void Popup::triggerClose() { UIRemovePopup(*this); }

    void Popup::triggerAction()
    {
        actionFunc();
        triggerClose();
    }

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
        confirm.setOnClick([&](MouseButton button) { triggerAction(); });
        back.setOnClick([&](MouseButton button) { triggerClose(); });
    }

    void ConfirmPopup::setInfoText(std::string_view text) { infoText = text; }

    std::string_view ConfirmPopup::getInfoText() const { return infoText; }

    void ConfirmPopup::onDraw(const Rect& bounds)
    {
        const auto& theme = EngineGetTheme();
        const auto& fnt = EngineGetFont();

        DrawRectangleRec(Rect{UIGetTargetResolution()}, ColorAlpha(theme.backActive, 0.6F));

        const auto dims = MeasureTextEx(fnt, infoText.c_str(), fnt.baseSize, 1.0F);
        auto modalSize =
            Point{std::min(dims.x + 12, UIGetTargetResolution().x * 0.2F), UIGetTargetResolution().y * 0.15F};

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
