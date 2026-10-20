#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

using namespace geode::prelude;

class $modify(MyBaseGameLayer, GJBaseGameLayer) {
    struct Fields {
        int m_graceTicks = 0;
        int m_delayedClickTicks = 0;
        bool m_preventDeath = false;
    };

    void handleButton(bool down, int button, bool isPlayer2) {
        // Ловим нажатие прыжка первого игрока (кнопка 1)
        if (down && button == 1 && !isPlayer2) {
            // Даем окно в 4 физических тика на проверку ранней смерти
            m_fields->m_graceTicks = 4;
        }

        GJBaseGameLayer::handleButton(down, button, isPlayer2);
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        // Если смерть произошла сразу после нажатия кнопки
        if (m_fields->m_graceTicks > 0 && !m_fields->m_preventDeath) {
            log::info("Early Click Buffer: Смерть перехвачена! Сдвигаем ввод.");

            m_fields->m_preventDeath = true;
            m_fields->m_graceTicks = 0;

            // Откладываем выполнение клика на 2 тика вперед
            m_fields->m_delayedClickTicks = 2;

            // Принудительно отпускаем кнопку, чтобы сбросить неудачный прыжок
            GJBaseGameLayer::handleButton(false, 1, false);
            return;
        }

        GJBaseGameLayer::destroyPlayer(player, object);
    }

    void update(float dt) {
        GJBaseGameLayer::update(dt);

        if (m_fields->m_graceTicks > 0) {
            m_fields->m_graceTicks--;
        }

        if (m_fields->m_delayedClickTicks > 0) {
            m_fields->m_delayedClickTicks--;
            if (m_fields->m_delayedClickTicks == 0) {
                log::info("Early Click Buffer: Прожимаем сдвинутый клик.");
                // Полноценно симулируем нажатие кнопки через движок игры
                this->handleButton(true, 1, false);
                m_fields->m_preventDeath = false;
            }
        }
    }
};

$on_mod(Loaded) {
    log::info("Early Click Buffer loaded successfully!");
}
