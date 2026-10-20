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
            return; // Не даём игре убить персонажа по-настоящему
        }

        PlayLayer::destroyPlayer(player, object);
    }

    // Проверяем наперёд: приведёт ли прыжок прямо сейчас к гибели?
    bool wouldDieIfJumpNow() {
        if (!m_player1) return false;

        // Сохраняем физическое состояние куба
        auto savedPos = m_player1->getPosition();
        auto savedYVel = m_player1->m_yVelocity;
        auto savedRotation = m_player1->getRotation();

        m_fields->m_isSimulating = true;
        m_fields->m_simulatedDeath = false;

        // Пробуем совершить прыжок
        m_player1->pushButton(PlayerButton::Jump);

        // Принудительно запускаем проверку коллизий для первого игрока
        this->checkCollisions(m_player1, 0.0f);

        // Фиксируем результат
        bool willDie = m_fields->m_simulatedDeath;

        // Моментально восстанавливаем исходное состояние
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
        // Ловим нажатие прыжка первого игрока
        if (down && button == 1 && isPlayer1) {
            // Если прыжок в текущем тике смертелен (слишком рано)
            if (this->wouldDieIfJumpNow()) {
                log::info("Dynamic: Ранний клик опасен! Откладываем до безопасного окна.");
                m_fields->m_scheduledTicksLeft = MAX_DYNAMIC_LOOKAHEAD;
                return; // Подавляем клик: куб продолжает безопасный полёт
            }
        }

        PlayLayer::handleButton(down, button, isPlayer1);
    }

    void update(float dt) {
        // Если в очереди висит подавленный ранний клик — проверяем каждый тик
        if (m_fields->m_scheduledTicksLeft > 0) {
            m_fields->m_scheduledTicksLeft--;

            // Если прямо в текущем тике прыгать уже безопасно
            if (!this->wouldDieIfJumpNow()) {
                log::info("Dynamic: Безопасное окно найдено! Выполняем идеальный клик.");
                m_fields->m_scheduledTicksLeft = -1;
                this->handleButton(true, 1, true);
            } else if (m_fields->m_scheduledTicksLeft == 0) {
                // Если за все 8 тиков безопасного окна так и не появилось
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
