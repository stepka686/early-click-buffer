#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

// Окно поиска безопасного кадра (8 физических шагов)
constexpr int MAX_DYNAMIC_LOOKAHEAD = 8;

class $modify(DynamicPlayLayer, PlayLayer) {
    struct Fields {
        int m_scheduledTicksLeft = -1;
        bool m_isSimulating = false;
        bool m_simulatedDeath = false;
    };

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        // Если смерть была вызвана во время виртуальной проверки
        if (m_fields->m_isSimulating) {
            m_fields->m_simulatedDeath = true;
            return; // Глушим реальное уничтожение персонажа
        }

        PlayLayer::destroyPlayer(player, object);
    }

    // Проверяем: убьет ли прыжок прямо в текущем положении?
    bool wouldDieIfJumpNow() {
        if (!m_player1) return false;

        // Сохраняем физическое состояние куба
        auto savedPos = m_player1->getPosition();
        auto savedYVel = m_player1->m_yVelocity;
        auto savedRotation = m_player1->getRotation();

        m_fields->m_isSimulating = true;
        m_fields->m_simulatedDeath = false;

        // Пробуем нажать прыжок
        m_player1->pushButton(PlayerButton::Jump);

        // Проверяем коллизии (передаем 3 аргумента: игрок, dt, ignoreDamage = false)
        this->checkCollisions(m_player1, 0.0f, false);

        // Запоминаем результат проверки
        bool willDie = m_fields->m_simulatedDeath;

        // Откатываем параметры куба назад в исходное состояние
        m_player1->releaseButton(PlayerButton::Jump);
        m_player1->setPosition(savedPos);
        m_player1->m_yVelocity = savedYVel;
        m_player1->setRotation(savedRotation);
        m_player1->m_isDead = false;

        m_fields->m_isSimulating = false;
        m_fields->m_simulatedDeath = false;

        return willDie;
    }

    void handleButton(bool down, int button, bool isPlayer1) {
        // Ловим клик прыжка первого игрока
        if (down && button == 1 && isPlayer1) {
            // Если клик в текущем тике смертелен (слишком рано)
            if (this->wouldDieIfJumpNow()) {
                log::info("Dynamic: Ранний клик опасен! Откладываем до безопасного окна.");
                m_fields->m_scheduledTicksLeft = MAX_DYNAMIC_LOOKAHEAD;
                return; // Подавляем клик прямо сейчас
            }
        }

        PlayLayer::handleButton(down, button, isPlayer1);
    }

    void update(float dt) {
        // Если в очереди висит отложенный ранний клик
        if (m_fields->m_scheduledTicksLeft > 0) {
            m_fields->m_scheduledTicksLeft--;

            // Проверяем, безопасно ли прыгать в текущем тике
            if (!this->wouldDieIfJumpNow()) {
                log::info("Dynamic: Безопасное окно открылось! Выполняем идеальный прыжок.");
                m_fields->m_scheduledTicksLeft = -1;
                this->handleButton(true, 1, true);
            } else if (m_fields->m_scheduledTicksLeft == 0) {
                // Если за все 8 тиков безопасного положения не возникло
                log::info("Dynamic: Окно истекло, безопасная траектория не найдена.");
                m_fields->m_scheduledTicksLeft = -1;
            }
        }

        PlayLayer::update(dt);
    }
};

$on_mod(Loaded) {
    log::info("Dynamic Click Aligner (Lookahead) loaded!");
}
