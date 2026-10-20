#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(MyPlayLayer, PlayLayer) {
    struct Fields {
        int m_graceTicks = 0;
        int m_delayedClickTicks = 0;
        bool m_preventDeath = false;
    };

    void handleButton(bool down, int button, bool isPlayer1) {
        // Кнопка 1 — это прыжок
        if (down && button == 1 && isPlayer1) {
            m_fields->m_graceTicks = 4;
        }

        PlayLayer::handleButton(down, button, isPlayer1);
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        // Если смерть наступила сразу после нажатия или пока ждём отложенный клик
        if ((m_fields->m_graceTicks > 0 || m_fields->m_delayedClickTicks > 0) && !m_fields->m_preventDeath) {
            log::info("Early Click Buffer: Перехватили раннюю смерть! Сдвигаем ввод.");

            m_fields->m_preventDeath = true;
            m_fields->m_graceTicks = 0;

            // Задержка в 2 физических тика до целевого окна
            m_fields->m_delayedClickTicks = 2;

            // Сбрасываем нажатие, чтобы не удерживать фатальный прыжок
            PlayLayer::handleButton(false, 1, true);
            return;
        }

        // Если активен защитный период сдвига — не даём шипам убить куб
        if (m_fields->m_delayedClickTicks > 0) {
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
                log::info("Early Click Buffer: Прожимаем клик в безопасном окне.");
                // Имитируем нажатие кнопки 1
                this->handleButton(true, 1, true);
                m_fields->m_preventDeath = false;
            }
        }
    }
};

$on_mod(Loaded) {
    log::info("Early Click Buffer loaded successfully!");
}
