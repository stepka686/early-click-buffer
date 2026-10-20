
#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

using namespace geode::prelude;

class $modify(EarlyBufferLayer, GJBaseGameLayer) {
    void handleButton(bool down, int button, bool isPlayer1) {
        // Пока передаём весь ввод игре сразу.
        GJBaseGameLayer::handleButton(down, button, isPlayer1);
    }
};

$on_mod(Loaded) {
    log::info("Early Click Buffer: instant input mode loaded!");
}
