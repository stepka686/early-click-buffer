#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(DynamicPlayLayer, PlayLayer) {
    struct Fields {
        int m_scheduledTicksLeft = -1;
        bool m_isSimulating = false;
        bool m_simulatedDeath = false;
    };

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        if (m_fields->m_isSimulating) {
            m_fields->m_simulatedDeath = true;
            return;
        }
        PlayLayer::destroyPlayer(player, object);
    }

    // Универсальная симуляция столкновений для всех режимов игры
    bool wouldDieIfJumpNow() {
        if (!m_player1) return false;

        auto savedPos = m_player1->getPosition();
        auto savedYVel = m_player1->m_yVelocity;
        auto savedRotation = m_player1->getRotation();

        m_fields->m_isSimulating = true;
        m_fields->m_simulatedDeath = false;

        float gravitySign = m_player1->m_isUpsideDown ? -1.0f : 1.0f;

        // 1. Волна (Dart)
        if (m_player1->m_isDart) {
            float forwardStep = 10.0f;
            float verticalStep = 10.0f * gravitySign;
            m_player1->setPosition({ savedPos.x + forwardStep, savedPos.y + verticalStep });
        }
        // 2. Корабль / НЛО (Ship / Bird)
        else if (m_player1->m_isShip || m_player1->m_isBird) {
            float lift = 8.0f * gravitySign;
            m_player1->setPosition({ savedPos.x + 5.0f, savedPos.y + lift });
        }
        // 3. Шар / Паук (Ball / Spider)
        else if (m_player1->m_isBall || m_player1->m_isSpider) {
            float flipStep = 12.0f * (-gravitySign);
            m_player1->setPosition({ savedPos.x + 4.0f, savedPos.y + flipStep });
        }
        // 4. Обычный куб (Cube)
        else {
            m_player1->pushButton(PlayerButton::Jump);
        }

        // Проверяем коллизии в виртуальной точке
        this->checkCollisions(m_player1, 0.0f, false);

        bool willDie = m_fields->m_simulatedDeath;

        // Откатываем состояние игрока
        if (!m_player1->m_isDart && !m_player1->m_isShip && !m_player1->m_isBird && !m_player1->m_isBall && !m_player1->m_isSpider) {
            m_player1->releaseButton(PlayerButton::Jump);
        }
        m_player1->setPosition(savedPos);
        m_player1->m_yVelocity = savedYVel;
        m_player1->setRotation(savedRotation);
        m_player1->m_isDead = false;

        m_fields->m_isSimulating = false;
        m_fields->m_simulatedDeath = false;

        return willDie;
    }

    void handleButton(bool down, int button, bool isPlayer1) {
        bool isEnabled = Mod::get()->getSettingValue<bool>("enabled");

        if (isEnabled && down && button == 1 && isPlayer1) {
            if (this->wouldDieIfJumpNow()) {
                int64_t delayFrames = Mod::get()->getSettingValue<int64_t>("delay");
                m_fields->m_scheduledTicksLeft = static_cast<int>(delayFrames);
                return;
            }
        }

        PlayLayer::handleButton(down, button, isPlayer1);
    }

    void update(float dt) {
        if (m_fields->m_scheduledTicksLeft > 0) {
            m_fields->m_scheduledTicksLeft--;

            if (!this->wouldDieIfJumpNow()) {
                m_fields->m_scheduledTicksLeft = -1;
                this->handleButton(true, 1, true);
            } else if (m_fields->m_scheduledTicksLeft == 0) {
                m_fields->m_scheduledTicksLeft = -1;
            }
        }

        PlayLayer::update(dt);
    }
};

$on_mod(Loaded) {
    log::info("Early Click Buffer loaded with custom settings!");
}
