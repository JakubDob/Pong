#include "Pong.h"

#include <SDL2_Sandbox/Debug.h>
#include <SDL2_Sandbox/Engine2D.h>
#include <SDL2_Sandbox/FiniteStateMachine.h>
#include <SDL2_Sandbox/ecs/components/AnimatedSprite.h>
#include <SDL2_Sandbox/ecs/components/BoxCollider.h>
#include <SDL2_Sandbox/ecs/components/CircleCollider.h>
#include <SDL2_Sandbox/ecs/components/Collider2D.h>
#include <SDL2_Sandbox/ecs/components/Controller2D.h>
#include <SDL2_Sandbox/ecs/components/Physics2D.h>
#include <SDL2_Sandbox/ecs/components/StaticSprite.h>
#include <SDL2_Sandbox/ecs/components/Transform2D.h>

Pong::Pong() : container(Engine2D::getInstance().getEcs()) {
    auto &engine = Engine2D::getInstance();
    auto &ecs = engine.getEcs();
    auto &am = engine.getAssetsManager();
    auto &ps = engine.getPhysicsSystem();
    auto &gs = engine.getGuiSystem();
    auto &camera = engine.getCamera();
    auto &inputHandler = engine.getInputHandler();
    auto &aiSystem = engine.getAiSystem();

    ps.enableGravity(false);

    wallBottom = Pong::createWall(tbWidth, tbHeight);
    wallTop = Pong::createWall(tbWidth, tbHeight, {0, goalHeight + tbHeight});
    goalL = Pong::createGoal(
        goalWidth, goalHeight,
        {-tbWidth / 2 + goalWidth / 2, (tbHeight + goalHeight) / 2});
    goalR = Pong::createGoal(
        goalWidth, goalHeight,
        {+tbWidth / 2 - goalWidth / 2, (tbHeight + goalHeight) / 2});
    paddleL = Pong::createPaddle(
        paddleW, paddleH, PlayerType::LEFT,
        {-tbWidth / 2 + 1 + goalWidth / 2, (tbHeight + goalHeight) / 2});
    paddleR = Pong::createPaddle(
        paddleW, paddleH, PlayerType::RIGHT,
        {+tbWidth / 2 - 1 - goalWidth / 2, (tbHeight + goalHeight) / 2});
    ball = Pong::createBall(ballRadius, {0, goalHeight / 2 + tbHeight / 2});
    background = Pong::createBackground(
        tbWidth, goalHeight, {0, goalHeight / 2.f + tbHeight * 0.5f});

    auto &fontAtlas =
        am.getFontAtlas("default", EngineConstants::Font::defaultFont, 32,
                        EngineConstants::Color::white);
    auto &scoreAtlas =
        am.getFontAtlas("scoreAtlas", EngineConstants::Font::defaultFont, 64,
                        EngineConstants::Color::green);
    auto &window = engine.getWindow();
    auto const &text =
        am.saveDrawableText("default", fontAtlas, engine.getRenderer(),
                            engine.getShader(ShaderType::HUD));
    auto const &scoreText =
        am.saveDrawableText("scoreText", scoreAtlas, engine.getRenderer(),
                            engine.getShader(ShaderType::HUD));

    container.getComponent<Collider2D>(goalL)
        ->getColliders()[0]
        ->setHitboxCallback([&](ecs::EcsContainer &ecs, ecs::Entity e) {
            if (e == ball) {
                this->resetBallPosition();
                ++scoreB;
            }
        });

    container.getComponent<Collider2D>(goalR)
        ->getColliders()[0]
        ->setHitboxCallback([&](ecs::EcsContainer &ecs, ecs::Entity e) {
            if (e == ball) {
                this->resetBallPosition();
                ++scoreA;
            }
        });

    engine.setPause(true);
    auto windowSize = window.getSize();
    camera.setZoom(1);
    camera.getTransform().setPosition({0, goalHeight / 2 + tbHeight / 2});
    engine.setControlledEntity(paddleL);

    aiSystem.addBehavior([&](scalar_t dt) {
        auto const &ballPos =
            container.getComponent<Transform2D>(ball)->getPosition();
        auto const &paddlePos =
            container.getComponent<Transform2D>(paddleR)->getPosition();
        auto *paddleController = container.getComponent<Controller2D>(paddleR);
        if (ballPos[0] > 0) {
            if (ballPos[1] < paddlePos[1]) {
                paddleController->moveDown(ecs, dt);
            } else if (ballPos[1] > paddlePos[1]) {
                paddleController->moveUp(ecs, dt);
            }
        } else {
            auto midY = goalHeight / 2 + tbHeight / 2;
            auto speed = container.getComponent<Physics2D>(paddleR)
                             ->getLinearVelocity()[1];

            if (paddlePos[1] > midY) {
                if (speed > -1.0) {
                    paddleController->moveDown(ecs, dt);
                }
            } else if (paddlePos[1] < midY) {
                if (speed < 1.0) {
                    paddleController->moveUp(ecs, dt);
                }
            }
        }
    });

    engine.onUpdate([&](scalar_t dt) {
        auto *posL = container.getComponent<Transform2D>(paddleL);
        if (posL->getX() > -tbWidth / 2 + paddleW + goalWidth) {
            posL->setX(-tbWidth / 2 + paddleW + goalWidth);
            container.getComponent<Physics2D>(paddleL)->setLinearVelocityX(0);
        }
        auto windowSize = window.getSize();
        scoreText.render(std::to_string(scoreA),
                         {windowSize[0] * 0.2f, windowSize[1] * 0.2f});
        scoreText.render(std::to_string(scoreB),
                         {windowSize[0] * 0.8f, windowSize[1] * 0.2f});
        lastBallPos = container.getComponent<Transform2D>(ball)->getPosition();
        Engine2D::getInstance().getRenderer().setLightPosition(lastBallPos);

        // if ball escapes the walls (tunneling, currently no continous
        // collision detection), reset its position
        if (lastBallPos[0] < -tbWidth || lastBallPos[0] > tbWidth ||
            lastBallPos[1] < -goalHeight * 2 ||
            lastBallPos[1] > goalHeight * 2) {
            resetBallPosition();
        }
    });

    auto &menuWindow = uiManager.makeMenuWindow(0, 0, 500, 600);
    auto &menuButton = uiManager.makeMenuButton(0, 0, menuWindow);
    auto const &drawableText = am.getDrawableText("default");
    auto *resetAllBtn = new Gui::GuiButton("Reset all", drawableText, [&] {
        container.getComponent<Transform2D>(paddleL)->setPosition(
            {-tbWidth / 2 + 1 + goalWidth / 2, (tbHeight + goalHeight) / 2});
        container.getComponent<Transform2D>(paddleR)->setPosition(
            {+tbWidth / 2 - 1 - goalWidth / 2, (tbHeight + goalHeight) / 2});
        resetBallPosition();
    });
    menuWindow.addElement(resetAllBtn);
    // put 'reset all' at the top of the menu and keep 'quit' at the bottom
    menuWindow.setElementIndex(resetAllBtn, 0);
    auto totalElems = menuWindow.getContainer().getElements().size() - 1;
    menuWindow.setElementIndex(totalElems, totalElems - 1);

    auto &startButton = uiManager.makeStartButton(0, 0);

    engine.onInit([&] {
        auto &window = Engine2D::getInstance().getWindow();
        auto width = window.getWidthF();
        auto height = window.getHeightF();
        updateProjection(width, height);
        startButton.setPosition(width * 0.5f - startButton.getTotalWidth() / 2,
                                height * 0.5f);
        menuButton.setPosition(
            engine.getWindow().getWidthI() - menuButton.getTotalWidth(), 0);
        menuWindow.setPosition((width - menuWindow.getWidth()) * 0.5, 0);
        menuWindow.setVisible(false);
    });

    engine.onWindowSizeChange([&](scalar_t width, scalar_t height) {
        updateProjection(width, height);
        startButton.setPosition(width * 0.5f - startButton.getTotalWidth() / 2,
                                height * 0.5f);
        menuWindow.setPosition((width - menuWindow.getWidth()) * 0.5, 0);
        menuButton.setPosition(
            engine.getWindow().getWidthI() - menuButton.getTotalWidth(), 0);
    });
}

ecs::Entity Pong::createPaddle(scalar_t width, scalar_t height,
                               PlayerType playerType, Vec2 const &position) {
    auto &engine = Engine2D::getInstance();
    auto &am = engine.getAssetsManager();
    auto &ecsContainer = engine.getEcs();
    auto const &atlas = am.getAtlas("pong");
    auto entity = ecsContainer.createEntity();
    ecs::AnimatedSprite sprite(ecs::AnimatedSprite::FsmType(),
                               atlas.getTexture(), width, height);
    sprite.setFramesFor<animation::IDLE>(atlas.getFrame("paddle0"),
                                         atlas.getFrame("paddle1"),
                                         atlas.getFrame("paddle2"));
    if (playerType == PlayerType::LEFT) {
        sprite.rotate(-90);
    }
    if (playerType == PlayerType::RIGHT) {
        sprite.rotate(90);
    }
    sprite.setFps(10);
    Collider2D collider;
    Physics2D physics;
    Transform2D transform;
    transform.setPosition(position);
    transform.setNdcDepth(-0.99);
    physics.setPartiallyStatic(true);
    physics.setMass(1000);
    physics.setFriction(0);
    // 3 colliders: two round edges and a rectangle in the middle
    float roundEdgeRadius = width / 2;
    auto middlePartHeight = height - roundEdgeRadius * 2;
    collider.add<BoxCollider>(width, middlePartHeight, ColliderType::PHYSICS);
    collider.add<CircleCollider>(roundEdgeRadius, ColliderType::PHYSICS,
                                 Vec2({0, middlePartHeight / 2}));
    collider.add<CircleCollider>(roundEdgeRadius, ColliderType::PHYSICS,
                                 Vec2({0, -middlePartHeight / 2}));
    ecsContainer.addComponent<Collider2D>(entity, std::move(collider));
    ecsContainer.addComponent<ecs::AnimatedSprite>(entity, std::move(sprite));
    ecsContainer.addComponent<Transform2D>(entity, std::move(transform));
    ecsContainer.addComponent<Physics2D>(entity, std::move(physics));
    ecsContainer.addComponent<Controller2D>(entity);

    return entity;
}

ecs::Entity Pong::createBall(scalar_t radius, Vec2 const &position) {
    auto &engine = Engine2D::getInstance();
    auto &am = engine.getAssetsManager();
    auto &ecsContainer = engine.getEcs();
    auto const &atlas = am.getAtlas("pong");
    auto entity = ecsContainer.createEntity();
    ecs::AnimatedSprite sprite(ecs::AnimatedSprite::FsmType(),
                               atlas.getTexture(), radius * 2, radius * 2);
    sprite.setFramesFor<animation::IDLE>(
        atlas.getFrame("fireball0"), atlas.getFrame("fireball1"),
        atlas.getFrame("fireball2"), atlas.getFrame("fireball3"),
        atlas.getFrame("fireball4"), atlas.getFrame("fireball5"),
        atlas.getFrame("fireball6"), atlas.getFrame("fireball7"),
        atlas.getFrame("fireball8"));
    sprite.setFps(15);
    Collider2D collider;
    Physics2D physics;
    Transform2D transform;
    transform.setPosition(position);
    transform.setNdcDepth(-0.99);
    physics.setMass(1);
    physics.setFriction(0);
    physics.setLinearVelocity(
        utils::RandomMatrix<scalar_t>::instance().getVector<2>(-10, 10));
    collider.add<CircleCollider>(radius, ColliderType::PHYSICS);
    ecsContainer.addComponent<Collider2D>(entity, std::move(collider));
    ecsContainer.addComponent<ecs::AnimatedSprite>(entity, std::move(sprite));
    ecsContainer.addComponent<Transform2D>(entity, std::move(transform));
    ecsContainer.addComponent<Physics2D>(entity, std::move(physics));
    ecsContainer.addComponent<Controller2D>(entity);
    return entity;
}

ecs::Entity Pong::createWall(scalar_t width, scalar_t height,
                             Vec2 const &position) {
    auto &engine = Engine2D::getInstance();
    auto &am = engine.getAssetsManager();
    auto &ecsContainer = engine.getEcs();
    auto const &atlas = am.getAtlas("pong");
    auto entity = ecsContainer.createEntity();
    ecs::StaticSprite sprite(atlas.getTexture(), atlas.getFrame("border"),
                             width, height, false);
    sprite.setOpaque(true);
    Collider2D collider;
    Physics2D physics;
    physics.setStatic(true);
    physics.setMass(1000);
    physics.setFriction(0);
    collider.add<BoxCollider>(width, height, ColliderType::PHYSICS);
    ecsContainer.addComponent<Collider2D>(entity, std::move(collider));
    ecsContainer.addComponent<ecs::StaticSprite>(entity, std::move(sprite));
    ecsContainer.addComponent<Transform2D>(entity)->setPosition(position);
    ecsContainer.addComponent<Physics2D>(entity, std::move(physics));
    ecsContainer.addComponent<Controller2D>(entity);

    return entity;
}
ecs::Entity Pong::createGoal(scalar_t width, scalar_t height,
                             Vec2 const &position) {
    auto &engine = Engine2D::getInstance();
    auto &am = engine.getAssetsManager();
    auto &ecsContainer = engine.getEcs();
    auto const &atlas = am.getAtlas("pong");
    auto entity = ecsContainer.createEntity();
    ecs::StaticSprite sprite(atlas.getTexture(), atlas.getFrame("goal"), width,
                             height, false);
    Collider2D collider;
    collider.add<BoxCollider>(width, height, ColliderType::PHYSICS);
    ecsContainer.addComponent<Collider2D>(entity, std::move(collider));
    ecsContainer.addComponent<ecs::StaticSprite>(entity, std::move(sprite));
    ecsContainer.addComponent<Transform2D>(entity)->setPosition(position);
    ecsContainer.addComponent<Physics2D>(entity)->setStatic(true);
    ecsContainer.addComponent<Controller2D>(entity);

    return entity;
}
ecs::Entity Pong::createBackground(scalar_t width, scalar_t height,
                                   Vec2 const &position) {
    auto &engine = Engine2D::getInstance();
    auto &am = engine.getAssetsManager();
    auto &ecsContainer = engine.getEcs();
    auto const &atlas = am.getAtlas("pong");
    auto entity = ecsContainer.createEntity();
    ecs::StaticSprite sprite(atlas.getTexture(), atlas.getFrame("background"),
                             width, height, false);
    sprite.setOpaque(true);
    sprite.setNormalMap(atlas.getFrame("normal_background"));
    Transform2D transform;
    transform.setNdcDepth(1.00);
    transform.setPosition(position);
    ecsContainer.addComponent<ecs::StaticSprite>(entity, std::move(sprite));
    ecsContainer.addComponent<Transform2D>(entity, std::move(transform));
    return entity;
}

void Pong::resetBallPosition() {
    container.getComponent<Transform2D>(ball)->setPosition(
        {0, goalHeight / 2 + tbHeight / 2});
    Vec2 randomDir;
    auto neg = utils::RandomMatrix<scalar_t>::instance().getScalar(0, 100);
    if (neg > 50) {
        randomDir =
            utils::RandomMatrix<scalar_t>::instance().getVector<2>(5, 10);
    } else {
        randomDir =
            utils::RandomMatrix<scalar_t>::instance().getVector<2>(-10, -5);
    }
    container.getComponent<Physics2D>(ball)->setLinearVelocity(randomDir);
}
void Pong::updateProjection(scalar_t width, scalar_t height) {
    auto windowAr = width / height;
    float left = (-tbWidth / 2 - goalWidth);
    float right = (tbWidth / 2 + goalWidth);
    float bottom = -goalHeight / 2 - tbHeight;
    float top = goalHeight / 2 + tbHeight;
    float currAr = (right - left) / (top - bottom);
    float mult = windowAr / currAr;
    if (windowAr >= currAr) {
        Engine2D::getInstance().setOrto(left * mult, right * mult, bottom, top);
    } else {
        Engine2D::getInstance().setOrto(left, right, bottom * (1 / mult),
                                        top * (1 / mult));
    }
}
void Pong::loadAssets(std::vector<std::string> const &assetsPath) {
    auto &engine = Engine2D::getInstance();
    engine.initializeAssets(assetsPath);
}
void Pong::loadAssetsFromFilesystem(std::string const &assetsPath) {
    auto &engine = Engine2D::getInstance();
    engine.initializeAssetsFromFileSystem(assetsPath);
}
void Pong::run() { Engine2D::getInstance().run(); }