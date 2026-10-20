#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(MyPlayLayer, PlayLayer) {
    struct Fields {
        int m_graceTicks = 0;
        int m_delayedClickTicks = 0;
        bool m_preventDeath = false;
    };

    void handleButton(bool down, PlayerButton button, bool isPlayer2) {
        // Ловим нажатие прыжка первого игрока
        if (down && button == PlayerButton::Jump && !isPlayer2) {
            m_fields->m_graceTicks = 4;
        }

        PlayLayer::handleButton(down, button, isPlayer2);
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        // Если смерть наступила в окне раннего нажатия
        if (m_fields->m_graceTicks > 0 && !m_fields->m_preventDeath) {
            log::info("Early Click Buffer: Смерть предотвращена, переносим клик.");

            m_fields->m_preventDeath = true;
            m_fields->m_graceTicks = 0;

            // Сдвигаем клик на 2 тика вперед
            m_fields->m_delayedClickTicks = 2;

            // Отпускаем кнопку, чтобы снять преждевременный импульс
            PlayLayer::handleButton(false, PlayerButton::Jump, false);
            return;
        }

        PlayLayer::destroyPlayer(player, object);
    }

    void update(float dt) {
        PlayLayer::update(dt);

        if (m_fields->m_graceTicks > 0) {
            m_fields->m_graceTicks--;
        }

        if (m_fields->m_delayedClickTicks > 0) {
            m_fields->m_delayedClickTicks--;
            if (m_fields->m_delayedClickTicks == 0) {
                log::info("Early Click Buffer: Выполняем сдвинутый прыжок.");
                this->handleButton(true, PlayerButton::Jump, false);
                m_fields->m_preventDeath = false;
            }
        }
    }
};

$on_mod(Loaded) {
    log::info("Early Click Buffer loaded successfully!");
}
