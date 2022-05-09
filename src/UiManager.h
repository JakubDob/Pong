#pragma once
#include <SDL2_Sandbox/Engine2D.h>
#include <SDL2_Sandbox/gui/GuiButton.h>
#include <SDL2_Sandbox/gui/GuiWindow.h>

#include <memory>
#include <vector>

class UiManager {
   public:
    Gui::GuiWindow& makeMenuWindow(int x, int y, int w, int h);
    Gui::GuiButton& makeStartButton(int x, int y);
    Gui::GuiButton& makeMenuButton(int x, int y, Gui::GuiWindow& window);

   private:
    Engine2D& engine = Engine2D::getInstance();

    std::vector<std::unique_ptr<Gui::GuiWindow>> windows;
    std::vector<std::unique_ptr<Gui::GuiButton>> buttons;
};