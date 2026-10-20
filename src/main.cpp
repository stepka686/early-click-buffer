#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(MyPlayLayer, PlayLayer) {
    struct Fields {
        int m_delayedTicks = 0;
        bool m_isSimulated = false;
        CCPoint m_savedTouchPos;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        // Визуальная плашка: если мод загружен, ты УВИДИШЬ этот текст при старте уровня
        auto label = CCLabelBMFont::create("BUFFER MOD ACTIVE", "bigFont.fnt");
        label->setPosition({ CCDirector::sharedDirector()->getWinSize().width / 2, 280.f });
        label->setScale(0.6f);
        label->setColor({ 0, 255, 0 });
        label->setTag(9999);
        this->addChild(label, 100);

        return true;
    }

    bool ccTouchBegan(CCTouch* touch, CCEvent* event) {
        // Если это наш собственный программный клик — даём игре его обработать
        if (m_fields->m_isSimulated) {
            return PlayLayer::ccTouchBegan(touch, event);
        }

        bool isEnabled = Mod::get()->getSettingValue<bool>("enabled");
        if (!isEnabled) {
            return PlayLayer::ccTouchBegan(touch, event);
        }

        int64_t delay = Mod::get()->getSettingValue<int64_t>("delay");
        
        // Запоминаем точку тача и взводим задержку
        m_fields->m_savedTouchPos = touch->getLocation();
        m_fields->m_delayedTicks = static_cast<int>(delay);

        // Мигаем надписью красным при перехвате тача
        if (auto label = this->getChildByTag(9999)) {
            static_cast<CCLabelBMFont*>(label)->setColor({ 255, 0, 0 });
        }

        // ВОЗВРАЩАЕМ FALSE: игра ДУМАЕТ, ЧТО ТАЧА НЕ БЫЛО ВООБЩЕ!
        return false;
    }

    void update(float dt) {
        PlayLayer::update(dt);

        if (m_fields->m_delayedTicks > 0) {
            m_fields->m_delayedTicks--;

            if (m_fields->m_delayedTicks == 0) {
                // Возвращаем надпись в зеленый
                if (auto label = this->getChildByTag(9999)) {
                    static_cast<CCLabelBMFont*>(label)->setColor({ 0, 255, 0 });
                }

                // Симулируем нажатие прыжка напрямую первому игроку
                if (m_player1) {
                    m_fields->m_isSimulated = true;
                    // В GD 2.2 базовый прыжок куба/волны из слоя вызывается так:
                    this->pushButton(1, true);
                    m_fields->m_isSimulated = false;
                }
            }
        }
    }
};

$on_mod(Loaded) {
    log::info("Touch-level Input Buffer Hook Loaded!");
}
