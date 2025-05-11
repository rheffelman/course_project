#include "Game.h"
#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cmath>
#include <string>
#include <set>


constexpr float ECB_WIDTH  = 40.0f;
constexpr float ECB_HEIGHT = 120.0f;

void Game::sRender() {
    window.clear();

    // --- PASS 1: TRAILS ---
    for (auto* e : entityManager.getEntities("trail")) {
        if (!e->isActive() || !e->has<CTransform>() || !e->has<CAnimation>()) continue;

        auto& transform = e->get<CTransform>();
        auto& anim = e->get<CAnimation>().anim;

        anim.setPosition(sf::Vector2f(transform.pos.x, transform.pos.y));
        anim.setRotation(transform.angle);
        anim.draw(window);
    }

    // --- PASS 2: PLAYER ---
    for (auto* e : entityManager.getEntities("player")) {
        if (!e->isActive() || !e->has<CTransform>()) continue;

        const auto& transform = e->get<CTransform>();

        if (e->has<CAnimation>()) {
            auto& anim = e->get<CAnimation>().anim;
            anim.setPosition(sf::Vector2f(transform.pos.x, transform.pos.y));
            anim.setRotation(transform.angle);
            anim.update();
            anim.draw(window);
        }

        // Health bar
        if (e->has<CHealth>()) {
            const auto& health = e->get<CHealth>();
            float barWidth = 50.0f;
            float barHeight = 6.0f;
            float percent = static_cast<float>(health.current) / health.max;

            sf::RectangleShape back(sf::Vector2f(barWidth, barHeight));
            back.setFillColor(sf::Color::Black);
            back.setPosition(transform.pos.x - barWidth / 2, transform.pos.y - 40);

            sf::RectangleShape front(sf::Vector2f(barWidth * percent, barHeight));
            front.setFillColor(sf::Color::Red);
            front.setPosition(transform.pos.x - barWidth / 2, transform.pos.y - 40);

            window.draw(back);
            window.draw(front);
        }
    }

    // --- PASS 3: ENEMIES ---
    for (auto* e : entityManager.getEntities("freya")) {
        if (!e->isActive() || !e->has<CTransform>()) continue;

        const auto& transform = e->get<CTransform>();

        if (e->has<CAnimation>()) {
            auto& anim = e->get<CAnimation>().anim;
            anim.setPosition(sf::Vector2f(transform.pos.x, transform.pos.y));
            anim.setRotation(transform.angle);
            anim.update();
            anim.draw(window);
        }

        if (e->has<CHealth>()) {
            const auto& health = e->get<CHealth>();
            float barWidth = 50.0f;
            float barHeight = 6.0f;
            float percent = static_cast<float>(health.current) / health.max;

            sf::RectangleShape back(sf::Vector2f(barWidth, barHeight));
            back.setFillColor(sf::Color::Black);
            back.setPosition(transform.pos.x - barWidth / 2, transform.pos.y - 40);

            sf::RectangleShape front(sf::Vector2f(barWidth * percent, barHeight));
            front.setFillColor(sf::Color::Red);
            front.setPosition(transform.pos.x - barWidth / 2, transform.pos.y - 40);

            window.draw(back);
            window.draw(front);
        }
    }

    // --- PASS 4: BONES ---
    for (auto* e : entityManager.getEntities("bone")) {
        if (!e->isActive() || !e->has<CTransform>()) continue;

        const auto& transform = e->get<CTransform>();
        if (e->has<CAnimation>()) {
            auto& anim = e->get<CAnimation>().anim;
            anim.setPosition(sf::Vector2f(transform.pos.x, transform.pos.y));
            anim.setRotation(transform.angle);
            anim.update();
            anim.draw(window);
        } else if (e->has<CShape>()) {
            auto& shape = e->get<CShape>();
            if (shape.isRect) {
                shape.rect.setPosition(sf::Vector2f(transform.pos.x, transform.pos.y));
                shape.rect.setRotation(transform.angle);
                window.draw(shape.rect);
            } else {
                shape.circle.setPosition(sf::Vector2f(transform.pos.x, transform.pos.y));
                shape.circle.setRotation(transform.angle);
                window.draw(shape.circle);
            }
        }
    }

    // --- PASS 5: ATTACKS ---
    for (auto* e : entityManager.getEntities("attack")) {
        if (!e->isActive() || !e->has<CTransform>() || !e->has<CShape>()) continue;

        auto& transform = e->get<CTransform>();
        auto& shape = e->get<CShape>();
        if (shape.isRect) {
            shape.rect.setPosition(sf::Vector2f(transform.pos.x, transform.pos.y));
            shape.rect.setRotation(transform.angle);
            if (!showHitboxes) {
                window.draw(shape.rect);
            }
        } else {
            shape.circle.setPosition(sf::Vector2f(transform.pos.x, transform.pos.y));
            shape.circle.setRotation(transform.angle);
            if (!showHitboxes) {
                window.draw(shape.circle);
            }
        }
    }

    // --- PASS 6: OTHER ENTITIES ---
    for (auto* e : entityManager.getEntities()) {
        if (!e->isActive() || e->tag() == "trail" || e->tag() == "player" || e->tag() == "enemy" || e->tag() == "bone" || e->tag() == "attack") continue;
        if (!e->has<CTransform>() || !e->has<CShape>()) continue;

        auto& transform = e->get<CTransform>();
        auto& shape = e->get<CShape>();

        if (shape.isRect) {
            shape.rect.setPosition(sf::Vector2f(transform.pos.x, transform.pos.y));
            shape.rect.setRotation(transform.angle);
            window.draw(shape.rect);
        } else {
            shape.circle.setPosition(sf::Vector2f(transform.pos.x, transform.pos.y));
            shape.circle.setRotation(transform.angle);
            window.draw(shape.circle);
        }
    }

    // --- PASS 7: CECB Wireframes ---
    if (showCECB) {
        for (auto* e : entityManager.getEntities()) {
            if (!e->isActive() || !e->has<CECB>()) continue;
            const auto& ecb = e->get<CECB>();
            window.draw(ecb.shape);
        }
    }

    // --- PASS 8: Hitbox Wireframes ---
    if (showHitboxes) {
        for (auto* e : entityManager.getEntities()) {
            if (!e->isActive() || !e->has<CShape>() || e->tag() == "trail") continue;

            auto& shape = e->get<CShape>();
            if (shape.isRect) {
                sf::RectangleShape outline = shape.rect;
                outline.setFillColor(sf::Color::Transparent);
                outline.setOutlineColor(sf::Color::Magenta);
                outline.setOutlineThickness(2);
                window.draw(outline);
            } else {
                sf::CircleShape outline = shape.circle;
                outline.setFillColor(sf::Color::Transparent);
                outline.setOutlineColor(sf::Color::Magenta);
                outline.setOutlineThickness(2);
                window.draw(outline);
            }
        }
    }

    // --- PASS 9: IMGUI & DISPLAY ---
    ImGui::SFML::Render(window);
    window.display();
}


//--
void Game::sMovement() {

    for (auto* e : entityManager.getEntities()) {
        if (!e->has<CTransform>() || e->tag() == "platform") continue;
        auto& trans = e->get<CTransform>();

        // Player input, dash, trail, etc.
        if (e->tag() == "player" && e->has<CInput>()) {
            auto& cooldowns = e->get<CCooldowns>();
            auto& state     = e->get<CState>();
            auto& dash      = e->get<CDash>();
            auto& buffer    = e->get<CBuffer>();
            auto& jump      = e->get<CJump>();

            buffer.update();
            cooldowns.update();
            dash.update();

            // spawn trail every 3 frames
            if (currentFrame % 3 == 0 && e->has<CAnimation>()) {
                auto& anim = e->get<CAnimation>().anim;
                sf::Sprite currentSprite = anim.getSprite();
                currentSprite.setScale(
                    state.facing_right ? 4.f : -4.f,
                    4.f
                );
                currentSprite.setOrigin(
                    currentSprite.getTextureRect().width  / 2.f,
                    currentSprite.getTextureRect().height / 2.f
                );
                spawnTrail(trans.pos, currentSprite, sf::Color(0,100,255,128));
            }

            // state‐lock handling
            if (isStateLocked(e)) {
                state.stateLockFrames--;
                trans.pos += trans.velocity;
                continue;
            }

            // coyote & landing
            bool onGroundNow = onGround(e);
            if (onGroundNow) {
                handleLanding(e);
                jump.coyoteTimer = 6;
            } else if (jump.coyoteTimer > 0) {
                jump.coyoteTimer--;
            }

            handleDash(e);

            if (!dash.active) {
                handlePlayerInput(e, onGroundNow);
            }
        }

        if (e->has<CGravity>()) {
            auto& gravity = e->get<CGravity>();
            trans.velocity.y += gravity.gravity;
            if (trans.velocity.y > 10.f) trans.velocity.y = 10.f;
        }

        if (e->has<CStuck>()) {
            trans.velocity = Vec2f(0, 0);
            continue;
        }

        trans.pos += trans.velocity;
        trans.pos += trans.velocity;

        if (e->has<CECB>()) {
            auto& ecb = e->get<CECB>();
            if (e->tag() == "freya") {
                ecb.setDiamond(trans.pos, ECB_WIDTH, ECB_HEIGHT);
            } else {
                ecb.setDiamond(trans.pos, ECB_WIDTH, ECB_HEIGHT);
            }
        }
    }
}
//--
void Game::sCollision() {
    auto platforms = entityManager.getEntities("platform");

    for (auto* e : entityManager.getEntities()) {
        if (!e->isActive() || !e->has<CECB>() || !e->has<CTransform>()) continue;
        if (e->tag() == "platform") continue;
        if (e->has<CStuck>()) continue; // <<< prevent stuck entities from processing
        if (e->tag() == "bone") {
            printf("hit a bone\n");
        }
        auto& trans = e->get<CTransform>();
        auto& vel = trans.velocity;
        Vec2f nextPos = trans.pos + vel;

        auto& ecb = e->get<CECB>();
        // ecb.setDiamond(nextPos, ECB_WIDTH, ECB_HEIGHT); // simulate ECB at next position

        const float epsilon = 1.0f; // leniency for "almost touching"
        bool collisionResolved = false;

        for (auto* plat : platforms) {
            const auto& platTrans = plat->get<CTransform>();
            const auto& platShape = plat->get<CShape>();
            const sf::Vector2f& platSize = platShape.rect.getSize();

            sf::FloatRect platBounds(
                platTrans.pos.x - platSize.x / 2,
                platTrans.pos.y - platSize.y / 2,
                platSize.x,
                platSize.y
            );

            if (!diamondIntersectsAABB(ecb.shape, platBounds)) continue;
            if (e->tag() == "bone") {
                e->add<CStuck>();
            }
            sf::Vector2f bottomPoint = ecb.shape.getPoint(2); // index 2 is bottom of diamond
            float playerBottom = bottomPoint.y;

            float platformTop = platBounds.top;

            if (vel.y > 0 && playerBottom >= platformTop && std::abs(playerBottom - platformTop) <= epsilon + ECB_HEIGHT / 2.0f) {
                // Coming down onto platform
                vel.y = 0;

                // Snap player so ECB bottom aligns with platform top
                nextPos.y = platformTop - ECB_HEIGHT / 2.0f;

                collisionResolved = true;
                break;
            } else {
                // Optional: cancel horizontal motion into platform
                vel.x = 0;
                nextPos.x = trans.pos.x;
            }
        }

        // Apply resolved position
        trans.pos = nextPos;

        // Sync ECB after move
        // ecb.setDiamond(trans.pos, ECB_WIDTH, ECB_HEIGHT);
    }
}
//--
void Game::sAnimation() {
    for (auto* e : entityManager.getEntities()) {
        if (!e->isActive() || !e->has<CAnimation>() || !e->has<CTransform>())
            continue;

        auto& animComp = e->get<CAnimation>();
        auto& trans    = e->get<CTransform>();

        // --- pick & switch animation based on state ---
        if (e->has<CState>()) {
            auto& state = e->get<CState>();
            std::string desired;

            if (e->tag() == "freya") {
                switch (state.state) {
                    case PlayerState::Idle:      desired = "freya_idle";  break;
                    case PlayerState::Running:   desired = "freya_walk";  break;
                    case PlayerState::Attacking: desired = "freya_attack";break;
                    default:                     desired = "freya_idle";  break;
                }
            } else {
                desired = getAnimationNameForState(state.state, state.facing_right);
            }

            if (desired != animComp.currentName && animations.count(desired)) {
                animComp.anim       = animations[desired];
                animComp.anim.loop  = animations[desired].loop;
                animComp.anim.restart();
                animComp.currentName = desired;
            }

            // --- origin & scale ---
            sf::Sprite& sprite = animComp.anim.getSprite();
            auto texRect = sprite.getTextureRect();

            if (e->tag() == "freya") {
                // center‐orig and scale so height == 80px
                sprite.setOrigin(texRect.width/2.f, texRect.height/2.f);
                float scaleFactor = 80.f / float(texRect.height);
                sprite.setScale(
                    state.facing_right ?  scaleFactor : -scaleFactor,
                                             scaleFactor
                );
            } else {
                // existing 4× scale for player/others
                sprite.setOrigin(texRect.width/2.f, texRect.height/2.f);
                sprite.setScale(
                    state.facing_right ? 4.f : -4.f,
                                         4.f
                );
            }
        }

        // --- bone override ---
        if (e->tag() == "bone") {
            auto& anim   = animComp.anim;
            sf::Sprite& s = anim.getSprite();
            s.setScale(4.f, 4.f);
            s.setOrigin(
                s.getTextureRect().width  / 2.f,
                s.getTextureRect().height / 2.f
            );
        }

        // update, position & rotation
        if (e->tag() != "trail") {
            animComp.anim.update();
        }
        animComp.anim.setPosition(trans.pos);
        animComp.anim.setRotation(trans.angle);

        // unlock after one‐shot finishes
        if (e->has<CState>()) {
            auto& state = e->get<CState>();
            if (!animComp.anim.loop && animComp.anim.finished) {
                state.stateLockFrames = 0;
                auto& vel = trans.velocity;
                bool onGroundNow = onGround(e);

                if (!onGroundNow && vel.y > 0) {
                    state.state = PlayerState::Falling;
                } else if (onGroundNow && vel.x == 0) {
                    state.state = PlayerState::Idle;
                } else if (onGroundNow && vel.x != 0) {
                    state.state = PlayerState::Running;
                }

                animComp.anim.finished = false;
            }
        }
    }
}
//--
void Game::sUserInput() {
    sf::Event event;
    while (window.pollEvent(event)) {
        ImGui::SFML::ProcessEvent(event);
        if (event.type == sf::Event::Closed) running = false;

        auto* p = player();
        if (!p) continue;

        auto& input = p->get<CInput>();
        auto& state = p->get<CState>();
        auto& buffer = p->get<CBuffer>();

        if (event.type == sf::Event::KeyPressed) {
            switch (event.key.code) {
                case sf::Keyboard::Space:
                case sf::Keyboard::Up:
                    input.up = true;
                    buffer.add("jump", 15);
                    break;

                case sf::Keyboard::A:
                case sf::Keyboard::Left:
                    input.left = true;
                    state.facing_right = false;
                    break;

                case sf::Keyboard::D:
                case sf::Keyboard::Right:
                    input.right = true;
                    state.facing_right = true;
                    break;

                case sf::Keyboard::F:
                    input.attack = true;
                    buffer.add("attack");
                    break;

                case sf::Keyboard::E:
                    input.dash = true;
                    buffer.add("dash");
                    break;

                case sf::Keyboard::P:
                    paused = !paused;
                    break;

                case sf::Keyboard::Escape:
                    exit(1);
                
                case sf::Keyboard::R:
                    input.bone_throw = true;
                    buffer.add("bone_throw");
                default: break;
            }
        } else if (event.type == sf::Event::KeyReleased) {
            switch (event.key.code) {
                case sf::Keyboard::Space:
                case sf::Keyboard::Up:
                    input.up = false;
                    if (p->has<CJump>()) {
                        p->get<CJump>().jumpReleased = true;
                    }
                    break;

                case sf::Keyboard::A:
                case sf::Keyboard::Left:
                    input.left = false;
                    break;

                case sf::Keyboard::D:
                case sf::Keyboard::Right:
                    input.right = false;
                    break;

                case sf::Keyboard::F:
                    input.attack = false;
                    break;

                case sf::Keyboard::E:
                    input.dash = false;
                    break;

                default: break;
            }
        }
    }
}
//--
void Game::sGUI() {
    ImGui::Begin("Debug");


    if (ImGui::BeginTabBar("MainTabBar")) {

    if (ImGui::BeginTabItem("Debug View")) {
        ImGui::Checkbox("Show CECB Wireframes", &showCECB);
        ImGui::Checkbox("Show Hitbox Wireframes", &showHitboxes);  // ← NEW
        ImGui::EndTabItem();
    }

        // --- ENTITY LIST TAB ---
        if (ImGui::BeginTabItem("Entities")) {
            int count = 0;
            for (auto* e : entityManager.getEntities()) {
                if (!e->isActive()) continue;

                std::string label = "Entity " + std::to_string(count++) + " (" + e->tag() + ")";

                if (e->has<CTransform>()) {
                    auto& t = e->get<CTransform>();
                    ImGui::Text("%s - Pos: (%.1f, %.1f)", label.c_str(), t.pos.x, t.pos.y);
                } else {
                    ImGui::Text("%s - No position", label.c_str());
                }
            }

            if (count == 0) {
                ImGui::Text("No active entities.");
            }

            ImGui::EndTabItem();
        }

        // --- PLAYER STATE TAB ---
        if (ImGui::BeginTabItem("Player State")) {
            auto* p = player();
            if (p->has<CState>()) {
                auto& s = p->get<CState>();
                ImGui::Text("State: %s", s.stateString().c_str());
                ImGui::Text("Facing: %s", s.facing_right ? "Right" : "Left");
            }
            if (p->has<CJump>()) {
                auto& j = p->get<CJump>();
                ImGui::Text("Jumps Left: %d", j.jumpsLeft);
                ImGui::Text("Jump Released: %s", j.jumpReleased ? "true" : "false");
            }
            if (p->has<CCooldowns>()) {
                auto& cds = p->get<CCooldowns>();
                ImGui::Text("Cooldowns:");
                for (const auto& [name, cd] : cds.cds) {
                    ImGui::BulletText("%s: %d", name.c_str(), cd.currentCooldown);
                }
            }
            if (p->has<CBuffer>()) {
                auto& buffer = p->get<CBuffer>();
                ImGui::Text("Buffered Inputs:");
                for (const auto& input : buffer.inputs) {
                    ImGui::BulletText("%s (%d frames left)", input.action.c_str(), input.framesRemaining);
                }
                if (buffer.inputs.empty()) {
                    ImGui::Text("(none)");
                }
            }
            ImGui::EndTabItem();
        }

        // --- ANIMATION PREVIEW TAB ---
        if (ImGui::BeginTabItem("Animations")) {
            static std::string selectedAnimation;

            ImGui::Text("Available Animations:");
            for (auto& [name, anim] : animations) {
                if (ImGui::Selectable(name.c_str(), selectedAnimation == name)) {
                    selectedAnimation = name;
                }
            }

            if (!selectedAnimation.empty()) {
                auto& anim = animations[selectedAnimation];
                anim.update();

                sf::Sprite& sprite = anim.getSprite();
                sf::Texture* tex = const_cast<sf::Texture*>(sprite.getTexture());

                if (tex && tex->getSize().x > 0 && tex->getSize().y > 0) {
                    sf::IntRect rect = sprite.getTextureRect();

                    float scale = 3.0f;

                    ImVec2 uv0(
                        rect.left / static_cast<float>(tex->getSize().x),
                        rect.top / static_cast<float>(tex->getSize().y)
                    );

                    ImVec2 uv1(
                        (rect.left + rect.width) / static_cast<float>(tex->getSize().x),
                        (rect.top + rect.height) / static_cast<float>(tex->getSize().y)
                    );

                    ImVec2 displaySize(rect.width * scale, rect.height * scale);

                    ImTextureID texID = reinterpret_cast<ImTextureID>(
                        static_cast<uintptr_t>(tex->getNativeHandle())
                    );

                    ImGui::Text("Preview: %s", selectedAnimation.c_str());
                    ImGui::Image(texID, displaySize, uv0, uv1);
                } else {
                    ImGui::Text("Invalid texture for %s", selectedAnimation.c_str());
                }
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}
//--
void Game::sAttack() {
    auto* p = player();
    if (!p) return;

    auto& input = p->get<CInput>();
    auto& state = p->get<CState>();
    auto& trans = p->get<CTransform>();
    auto& cooldowns = p->get<CCooldowns>();
    auto& buffer = p->get<CBuffer>();

    cooldowns.update();

    if (!canChangeTo(state.state, PlayerState::Attacking)) return;

    if ((input.attack || buffer.has("attack")) && cooldowns.ready("attack")) {
        cooldowns.reset("attack");
        buffer.clear("attack");

        Vec2f offset = state.facing_right ? Vec2f(40, 0) : Vec2f(-40, 0);
        Vec2f pos = trans.pos + offset;

        auto* slash = entityManager.addEntity("attack");
        slash->add<CTransform>(pos, Vec2f(0, 0), 0);
        slash->add<CShape>(sf::Vector2f(60, 120), sf::Color::Red, sf::Color::White, 1);
        slash->add<CLifespan>(7);

        state.state = PlayerState::Attacking;
        state.stateLockFrames = 20;

    }

    for (auto* attack : entityManager.getEntities("attack")) {
        for (auto* enemy : entityManager.getEntities("enemy")) {
            if (!enemy->has<CHealth>()) continue;

            if (checkAABBCollision(attack, enemy)) {
                auto& health = enemy->get<CHealth>();
                health.current--;

                if (health.current <= 0) {
                    enemy->destroy();
                }

                attack->destroy();
                break;
            }
        }
    }
}
//--
void Game::sBoneThrow() {
    auto* p = player();
    if (!p || !player_has_bone) return;

    auto& input = p->get<CInput>();
    auto& state = p->get<CState>();
    auto& trans = p->get<CTransform>();
    auto& cooldowns = p->get<CCooldowns>();
    auto& buffer = p->get<CBuffer>();

    cooldowns.update();

    if (!Game::canChangeTo(state.state, PlayerState::Attacking)) return;

    if ((input.bone_throw || buffer.has("bone_throw")) && cooldowns.ready("bone_throw")) {
        input.bone_throw = false;
        buffer.clear("bone_throw");
        cooldowns.reset("bone_throw");

        const auto& cfg = gameConfig["abilities"]["bone_throw"];
        std::string playerAnim = cfg["player_animation"];
        std::string projAnim   = cfg["projectile_animation"];
        Vec2f velocity = {
            cfg["projectile_velocity"][0].get<float>(),
            cfg["projectile_velocity"][1].get<float>()
        };
        float ecbW = cfg["ecb"][0].get<float>();
        float ecbH = cfg["ecb"][1].get<float>();
        int lifespan = cfg["lifespan"].get<int>();

        // Lock into throw
        state.state = PlayerState::Attacking;
        state.stateLockFrames = 20;
        p->get<CAnimation>().currentName = playerAnim;

        Vec2f offset = state.facing_right ? Vec2f(40, 0) : Vec2f(-40, 0);
        Vec2f pos = trans.pos + offset;
        if (!state.facing_right) velocity.x *= -1;

        // --- SPAWN BONE ---
        auto* bone = entityManager.addEntity("bone");
        bone->add<CTransform>(pos, velocity, 0);
        bone->add<CLifespan>(lifespan);
        bone->add<CCollision>();
        bone->add<CGravity>(0.5f);

        if (animations.count(projAnim)) {
            Animation anim = animations[projAnim];
            anim.setScale(4.0f);
            bone->add<CAnimation>(anim, projAnim);
        } else {
            bone->add<CShape>(sf::Vector2f(60, 60), sf::Color::White, sf::Color::White, 1);
        }

        CECB ecb;
        ecb.setTriangle(pos, ecbW, ecbH);
        bone->add<CECB>(ecb);

        // --- Update player state ---
        player_has_bone = false;
    }

    // --- FALLING BONE HANDLING ---
    for (auto* b : entityManager.getEntities("bone")) {
        if (!b->has<CTransform>() || !b->has<CGravity>()) continue;
        if (b->has<CStuck>()) continue;
            auto& trans = b->get<CTransform>();
        auto& vel = trans.velocity;

        if (!onGround(b)) {
            vel.y += b->get<CGravity>().gravity;
            if (vel.y > 10.0f) vel.y = 10.0f;

            // Move
            trans.pos += vel;
        }


        // Check for landing
        if (onGround(b)) {
            vel = Vec2f(0, 0);
            b->add<CStuck>();           // mark it as stuck
            b->remove<CGravity>();      // stop gravity
            b->remove<CCollision>();    // optional: prevent other checks

            // Set animation to 'bone' if needed
            if (animations.count("bone")) {
                Animation stick;
                stick.loadFromStrip(textures["bone"], 1, 1);
                stick.setScale(4.0f);
                b->get<CAnimation>().anim = stick;
            }
        }
    }
}
//--
void Game::sLifeSpan() {
    for (auto* e : entityManager.getEntities()) {
        if (!e->has<CLifespan>()) continue;

        auto& life = e->get<CLifespan>();
        life.remaining--;

        if (life.remaining <= 0) {
            if (e->tag() == "bone") {
                player_has_bone = true; // bone returns
            }
            e->destroy();
        }
    }
}