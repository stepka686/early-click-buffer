#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

class $modify(MyPlayerObject, PlayerObject) {
    struct Fields {
        // Сколько физических тиков клик считается "свежим" для спасения
        int m_graceTicks = 0;
        // Запланированный клик, который нужно нажать позже
        int m_delayedClickTicks = 0;
        // Защита от бесконечного цикла отмены смерти
        bool m_isSafeFromEarlyDeath = false;
    };

    void pushButton(PlayerButton btn) {
        if (btn == PlayerButton::Jump) {
            // Засекаем 3 физических тика окна "раннего" клика
            m_fields->m_graceTicks = 3;
        }
        PlayerObject::pushButton(btn);
    }

    void playerDestroyed(bool p0) {
        // Если игрок умер в течение 3 тиков после нажатия прыжка
        if (m_fields->m_graceTicks > 0 && !m_fields->m_isSafeFromEarlyDeath) {
            log::info("Спасли от ранней смерти! Сдвигаем клик дальше.");
            
            // Отменяем смерть
            m_fields->m_isSafeFromEarlyDeath = true;
            m_fields->m_graceTicks = 0;

            // Откладываем прыжок на 2 тика вперёд (то самое окно)
            m_fields->m_delayedClickTicks = 2;

            // Сбрасываем текущий зажатый прыжок, чтобы не залипал в потолке
            this->releaseButton(PlayerButton::Jump);
            return;
        }

        PlayerObject::playerDestroyed(p0);
    }

    void update(float dt) {
        PlayerObject::update(dt);

        // Уменьшаем окно проверки раннего клика
        if (m_fields->m_graceTicks > 0) {
            m_fields->m_graceTicks--;
        }

        // Если есть отложенный клик — ждём нужный тик и жмём
        if (m_fields->m_delayedClickTicks > 0) {
            m_fields->m_delayedClickTicks--;
            if (m_fields->m_delayedClickTicks == 0) {
                log::info("Применяем отложенный клик в допустимое окно.");
                this->pushButton(PlayerButton::Jump);
                m_fields->m_isSafeFromEarlyDeath = false;
            }
        }
    }
};

$on_mod(Loaded) {
    log::info("Early Click Buffer loaded!");
}
