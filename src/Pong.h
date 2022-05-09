#pragma once
#include <SDL2_Sandbox/DrawableText.h>
#include <SDL2_Sandbox/ecs/EcsContainer.h>
#include <SDL2_Sandbox/gui/GuiButton.h>

#include "UiManager.h"

enum class PlayerType { LEFT, RIGHT };

class Pong {
   public:
    Pong();
    static ecs::Entity createPaddle(scalar_t width, scalar_t height,
                                    PlayerType playerType,
                                    Vec2 const& position = {0, 0});
    static ecs::Entity createBall(scalar_t radius,
                                  Vec2 const& position = {0, 0});
    static ecs::Entity createWall(scalar_t width, scalar_t height,
                                  Vec2 const& position = {0, 0});
    static ecs::Entity createGoal(scalar_t width, scalar_t height,
                                  Vec2 const& position = {0, 0});
    static ecs::Entity createBackground(scalar_t width, scalar_t height,
                                        Vec2 const& position = {0, 0});
    static void loadAssets(std::vector<std::string> const& assetsPath);
    static void loadAssetsFromFilesystem(std::string const& assetsPath);
    void run();

   protected:
    void resetBallPosition();
    void updateProjection(scalar_t width, scalar_t height);

   private:
    ecs::EcsContainer& container;
    UiManager uiManager;

    ecs::Entity wallBottom, wallTop, goalL, goalR, paddleL, paddleR, ball,
        background;
    float tbWidth = 20, tbHeight = 1, goalWidth = 1, goalHeight = 10,
          paddleW = 1, paddleH = 4, ballRadius = 0.5;
    int scoreA = 0, scoreB = 0;
    Vec2 lastBallPos;
};