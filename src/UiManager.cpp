#include "UiManager.h"

#include <SDL2_Sandbox/Engine2D.h>

Gui::GuiWindow& UiManager::makeMenuWindow(int x, int y, int w, int h) {
    auto& assetsManager = engine.getAssetsManager();
    auto const& fontAtlas = assetsManager.getFontAtlas(
        "default", EngineConstants::Font::defaultFont, 32,
        EngineConstants::Color::white);
    auto const& drawableText = assetsManager.saveDrawableText(
        "default", fontAtlas, engine.getRenderer(),
        engine.getShader(ShaderType::HUD));
    auto window =
        std::make_unique<Gui::GuiWindow>(x, y, w, h, "Options", drawableText);

    auto toggleVSync = [vsyncEnabled = true]() mutable {
        vsyncEnabled = !vsyncEnabled;
        vsyncEnabled ? SDL_GL_SetSwapInterval(1) : SDL_GL_SetSwapInterval(0);
    };

    auto toggleGravity = [&] { engine.getPhysicsSystem().toggleGravity(); };

    auto togglePause = [&, paused = false]() mutable {
        paused = !paused;
        engine.setPause(paused);
    };

    auto toggleLights = [&] { engine.getRenderer().toggleLight(); };

    auto quitApp = [&] { engine.quit(); };

    window->addElement(
        new Gui::GuiButton("Toggle VSync", drawableText, toggleVSync));
    window->addElement(
        new Gui::GuiButton("Toggle gravity", drawableText, toggleGravity));
    window->addElement(
        new Gui::GuiButton("Toggle pause", drawableText, togglePause));
    window->addElement(
        new Gui::GuiButton("Toggle lights", drawableText, toggleLights));
    window->addElement(new Gui::GuiButton("Quit", drawableText, quitApp));
    engine.getInputHandler().registerInputTarget(*window);
    engine.getGuiSystem().add(*window);
    windows.emplace_back(std::move(window));
    return *windows.back();
}

Gui::GuiButton& UiManager::makeStartButton(int x, int y) {
    auto& assetsManager = engine.getAssetsManager();
    auto const& fontAtlas = assetsManager.getFontAtlas(
        "default", EngineConstants::Font::defaultFont, 32,
        EngineConstants::Color::white);
    auto const& drawableText = assetsManager.saveDrawableText(
        "default", fontAtlas, engine.getRenderer(),
        engine.getShader(ShaderType::HUD));
    auto btn = std::make_unique<Gui::GuiButton>("Start", drawableText);
    btn->setPosition(x, y);

    btn->setOnClick([&, &this_btn = *btn] {
        engine.setPause(false);
        this_btn.setVisible(false);
    });

    engine.getGuiSystem().add(*btn);
    engine.getInputHandler().registerInputTarget(*btn);
    buttons.emplace_back(std::move(btn));
    return *buttons.back();
}

Gui::GuiButton& UiManager::makeMenuButton(int x, int y,
                                          Gui::GuiWindow& window) {
    auto& assetsManager = engine.getAssetsManager();
    auto const& fontAtlas = assetsManager.getFontAtlas(
        "default", EngineConstants::Font::defaultFont, 32,
        EngineConstants::Color::white);
    auto const& drawableText = assetsManager.saveDrawableText(
        "default", fontAtlas, engine.getRenderer(),
        engine.getShader(ShaderType::HUD));

    auto menuButton =
        std::make_unique<Gui::GuiButton>("", drawableText, [&window] {
            bool visible = window.isVisible();
            window.setVisible(!visible);
        });

    auto const& pongAtlas = assetsManager.getAtlas("pong");
    menuButton->setContentWidth(60);
    menuButton->setContentHeight(60);
    menuButton->setTexture(pongAtlas.getTexture(), pongAtlas.getFrame("menu"));
    menuButton->setPosition(x, y);

    engine.getInputHandler().registerInputTarget(*menuButton);
    engine.getGuiSystem().add(*menuButton);
    buttons.emplace_back(std::move(menuButton));
    return *buttons.back();
}