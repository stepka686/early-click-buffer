#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

class $modify(BufferPlayer, PlayerObject) {
    struct Fields {
        int m_bufferTicks = 0;
        bool m_isSimulatingClick = false;
    };

    void pushButton(PlayerButton button) {
        bool isEnabled = Mod::get()->getSettingValue<bool>("enabled");

        // Если это наш программный клик из буфера — просто выполняем его
        if (m_fields->m_isSimulatingClick || !isEnabled) {
            PlayerObject::pushButton(button);
            return;
        }

        // Если игрок физически нажал кнопку прыжка
        if (button == PlayerButton::Jump) {
            int64_t delay = Mod::get()->getSettingValue<int64_t>("delay");
            
            // Если задержка выставлена в 0 или 1 тик — отдаем сразу
            if (delay <= 1) {
                PlayerObject::pushButton(button);
                return;
            }

            // Иначе задерживаем выполнение клика на заданное число тиков
            m_fields->m_bufferTicks = static_cast<int>(delay);
            
            // Визуальный маркер для проверки: красим куб на мгновение
            // (если куб моргает — мод 100% работает и перехватывает клики!)
            this->setColor({ 255, 0, 0 }); 
            return; // Глушим раннее нажатие прямо сейчас!
        }

        PlayerObject::pushButton(button);
    }

    void update(float dt) {
        PlayerObject::update(dt);

        // Тикаем буфер задержки
        if (m_fields->m_bufferTicks > 0) {
            m_fields->m_bufferTicks--;

            if (m_fields->m_bufferTicks == 0) {
                // Время вышло: применяем сдвинутый клик в игру
                m_fields->m_isSimulatingClick = true;
                this->pushButton(PlayerButton::Jump);
                m_fields->m_isSimulatingClick = false;

                // Возвращаем цвет обратно в белый
                this->setColor({ 255, 255, 255 });
            }
        }
    }
};

$on_mod(Loaded) {
    log::info("Direct PlayerObject Input Buffer loaded!");
}
