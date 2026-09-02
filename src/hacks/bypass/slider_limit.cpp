#include <Geode/Geode.hpp>
#include <Geode/modify/SliderTouchLogic.hpp>
#include <Geode/modify/GJScaleControl.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Bypass", "Slider Limit", "Removes the limit on sliding beyond set limits", false);

#ifdef GEODE_IS_WINDOWS
$execute {
    auto& gui = GDH::Gui::get();
    auto& hack = gui.getWindow("Bypass").findHackByName("Slider Limit");   
    hack.setHandler([](bool enabled) {
        static auto patches = []() {
            auto* mod = geode::Mod::get();
            auto base = geode::base::get();
            
            return std::array{
                mod->patch((void*)(base + 0x71657),  {0xEB}).unwrapOr(nullptr),
                mod->patch((void*)(base + 0x71671),  {0xEB}).unwrapOr(nullptr),
                mod->patch((void*)(base + 0x12A9C4), {0x90, 0x90}).unwrapOr(nullptr),
                mod->patch((void*)(base + 0x12A9D8), {0x90, 0x90}).unwrapOr(nullptr),
                mod->patch((void*)(base + 0x1214D4), {0x0F, 0x57, 0xFF, 0x90, 0x90, 0x90, 0x90, 0x90}).unwrapOr(nullptr),
                mod->patch((void*)(base + 0x12159E), {0x0F, 0x84}).unwrapOr(nullptr),
                mod->patch((void*)(base + 0x1215A7), {0x90, 0x90, 0x90, 0x90, 0x90, 0x90}).unwrapOr(nullptr),
                mod->patch((void*)(base + 0x121606), {0x0F, 0x84}).unwrapOr(nullptr),
                mod->patch((void*)(base + 0x12160F), {0x90, 0x90, 0x90, 0x90, 0x90, 0x90}).unwrapOr(nullptr)
            };
        }();

        for (auto* patch : patches) {
            if (patch) (void)patch->toggle(enabled);
        }
    });
}
#endif

#ifdef GEODE_IS_ANDROID
class $modify(SliderLimitSliderTouchLogic, SliderTouchLogic) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Bypass").findHackByName("Slider Limit");        
        
        hack.addHookPtr(self.getHook("SliderTouchLogic::ccTouchMoved").unwrap());
    }

    void ccTouchMoved(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override {
        auto touchPos = this->convertTouchToNodeSpace(touch);
        auto position = touchPos - this->m_position;
        if (this->m_rotated) {
            this->m_thumb->setPosition({0.f, position.y});
        } else {
            this->m_thumb->setPosition({position.x, 0.f});
        }

        if (this->m_activateThumb)
            this->m_thumb->activate();

        if (this->m_slider)
            this->m_slider->updateBar();
    }
};

class $modify(SliderLimitGJScaleControl, GJScaleControl) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Bypass").findHackByName("Slider Limit");        
        
        hack.addHookPtr(self.getHook("GJScaleControl::ccTouchMoved").unwrap());
    }

    void ccTouchMoved(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) {
        GJScaleControl::ccTouchMoved(touch, event);

        if (m_sliderXY && m_sliderXY->m_touchLogic->m_activateThumb) {
            m_sliderXY->getThumb()->setPositionX(
                this->convertToNodeSpace(touch->getLocation()).x
            );
            m_sliderXY->updateBar();

            float value = m_sliderXY->getThumb()->getValue();
            this->updateLabelXY(value);
            this->sliderChanged(m_sliderXY->getThumb());

            if (auto editorUI = EditorUI::get()) 
                editorUI->scaleXYChanged(value, value, m_scaleLocked);
        }
    }
};
#endif