
#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <deque>

using namespace geode::prelude;

struct PendingInput {
    bool down;
    int button;
    bool player1;
    int remaining;
};

class $modify(EarlyBufferLayer, GJBaseGameLayer) {
    struct Fields {
        std::deque<PendingInput> queue;
        bool applying = false;
    };

    void handleButton(bool down, int button, bool isPlayer1) {
        auto* fields = m_fields.self();

        if (fields->applying ||
            !Mod::get()->getSettingValue<bool>("enabled")) {
            GJBaseGameLayer::handleButton(down, button, isPlayer1);
            return;
        }

        int frames = Mod::get()->getSettingValue<int>("delay");
        int ticks = frames * 4;

        fields->queue.push_back({
            down, button, isPlayer1, ticks
        });
    }

    void processCommands(float dt, bool isHalfTick, bool isLastTick) {
        auto* fields = m_fields.self();

        if (!Mod::get()->getSettingValue<bool>("enabled")) {
            fields->queue.clear();
        } else {
            for (auto it = fields->queue.begin();
                 it != fields->queue.end();) {
                --it->remaining;

                if (it->remaining <= 0) {
                    fields->applying = true;
                    GJBaseGameLayer::handleButton(
                        it->down, it->button, it->player1
                    );
                    fields->applying = false;
                    it = fields->queue.erase(it);
                } else {
                    ++it;
                }
            }
        }

        GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);
    }
};

class $modify(EarlyBufferPlayLayer, PlayLayer) {
    void resetLevel() {
        auto* base = static_cast<EarlyBufferLayer*>(
            static_cast<GJBaseGameLayer*>(this)
        );
        base->m_fields->queue.clear();

        PlayLayer::resetLevel();
    }
};

$on_mod(Loaded) {
    log::info("Early Click Buffer prototype loaded!");
}
