#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

// Задаём константу окна (8 тиков при 240Hz ≈ 33 мс, т.е. 2 кадра на 60Hz)
constexpr int BUFFER_WINDOW_TICKS = 8;

class $modify(MyPlayLayer, PlayLayer) {
    struct Fields {
        int m_graceTicks = 0;
        int m_delayedClickTicks = 0;
        bool m_preventDeath = false;
    };

    void handleButton(bool down, int button, bool isPlayer1) {
        if (down && button == 1 && isPlayer1) {
            // Открываем окно раннего ввода на 8 тиков
            m_fields->m_graceTicks = BUFFER_WINDOW_TICKS;
        }

        PlayLayer::handleButton(down, button, isPlayer1);
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        // Если смерть наступила во время нашего 8-тикового окна
        if (m_fields->m_graceTicks > 0 && !m_fields->m_preventDeath) {
            log::info("Early Click Buffer: Смерть перехвачена! Сдвигаем клик на 8 тиков.");

            m_fields->m_preventDeath = true;
            
            // Откладываем выполнение клика на остаток окна (до 8 тиков)
            m_fields->m_delayedClickTicks = m_fields->m_graceTicks;
            m_fields->m_graceTicks = 0;

            // Сбрасываем текущее нажатие, чтобы не ломать траекторию
            PlayLayer::handleButton(false, 1, true);
            return;
        }

        // Защита от смерти на всё время пролёта до безопасной точки
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
                log::info("Early Click Buffer: 8-тиковое окно подошло, прожимаем прыжок!");
                this->handleButton(true, 1, true);
                m_fields->m_preventDeath = false;
            }
        }
    }
};

$on_mod(Loaded) {
    log::info("Early Click Buffer (8 ticks) loaded!");
}
