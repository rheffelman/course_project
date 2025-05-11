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
#include "nlohmann/json.hpp"

nlohmann::json gameConfig; // define it here

constexpr float ECB_WIDTH  = 40.0f;
constexpr float ECB_HEIGHT = 120.0f;

void Game::loadGameConfig(const string& filename) {
    ifstream f(filename);
    if (!f.is_open()) {
        cerr << "Failed to open game config\n";
        return;
    }
    f >> gameConfig;
}

bool Game::checkAABBCollision(const Entity* a, const Entity* b) {
    const auto& aT = a->get<CTransform>();
    const auto& aS = a->get<CShape>();
    const auto& bT = b->get<CTransform>();
    const auto& bS = b->get<CShape>();

    const auto& aSize = aS.rect.getSize();
    const auto& bSize = bS.rect.getSize();

    bool xOverlap = std::abs(aT.pos.x - bT.pos.x) <= (aSize.x / 2 + bSize.x / 2);
    bool yOverlap = std::abs(aT.pos.y - bT.pos.y) <= (aSize.y / 2 + bSize.y / 2);

    return xOverlap && yOverlap;
}
//--
bool Game::pointInDiamond(const sf::Vector2f& point, const sf::ConvexShape& diamond) {
    int crossings = 0;
    int count = static_cast<int>(diamond.getPointCount());

    for (int i = 0; i < count; ++i) {
        sf::Vector2f a = diamond.getPoint(i);
        sf::Vector2f b = diamond.getPoint((i + 1) % count);

        if (((a.y > point.y) != (b.y > point.y)) &&
            (point.x < (b.x - a.x) * (point.y - a.y) / ((b.y - a.y) + 1e-6f) + a.x)) {
            crossings++;
        }
    }

    return (crossings % 2) == 1;
}
//--
bool Game::diamondIntersectsAABB(const sf::ConvexShape& diamond, const sf::FloatRect& box) {
    // Check if any of the diamond's points are inside the AABB
    for (int i = 0; i < 4; ++i) {
        sf::Vector2f pt = diamond.getPoint(i);
        if (box.contains(pt)) return true;
    }

    // Check if any of the AABB corners are inside the diamond
    sf::Vector2f boxCorners[] = {
        {box.left, box.top},
        {box.left + box.width, box.top},
        {box.left, box.top + box.height},
        {box.left + box.width, box.top + box.height}
    };

    for (const auto& corner : boxCorners) {
        if (pointInDiamond(corner, diamond)) return true;
    }

    return false;
}
//--
bool Game::canChangeTo(PlayerState current, PlayerState target) {
    if (current == PlayerState::Dashing && target == PlayerState::Attacking) return false;
    return true; // allow all others by default
}
//--
bool Game::isStateLocked(Entity* e) {
    auto& state = e->get<CState>();
    return state.stateLockFrames > 0;
}
//--
void Game::handleLanding(Entity* e) {
    auto& jump = e->get<CJump>();
    auto& buffer = e->get<CBuffer>();
    auto& trans = e->get<CTransform>();
    auto& state = e->get<CState>();

    jump.jumpsLeft = 2;
    jump.jumpReleased = true;

    if (buffer.has("jump")) {
        trans.velocity.y = -10.0f;
        jump.jumpsLeft--;
        jump.jumpReleased = false;
        buffer.clear("jump");
        state.state = PlayerState::Jump1;
    }
}
//--
void Game::spawnTrail(const Vec2f& pos, const sf::Sprite& sourceSprite, const sf::Color& color) {
    auto* trail = entityManager.addEntity("trail");

    Animation snapshot;
    snapshot = Animation(); // dummy
    snapshot.getSprite() = sourceSprite;

    sf::Sprite& s = snapshot.getSprite();
    s.setPosition(sf::Vector2f(pos.x, pos.y));
    s.setColor(color); // tint it blue

    trail->add<CTransform>(pos, Vec2f(0, 0), 0);
    trail->add<CAnimation>(snapshot, "trail");
    trail->add<CLifespan>(10); // remove after 10 frames
}
//--
void Game::handleDash(Entity* e) {
    auto& input = e->get<CInput>();
    auto& dash = e->get<CDash>();
    auto& cooldowns = e->get<CCooldowns>();
    auto& state = e->get<CState>();
    auto& trans = e->get<CTransform>();
    auto& buffer = e->get<CBuffer>();

    if ((input.dash || buffer.has("dash")) && cooldowns.ready("dash") && !dash.active) {
        cooldowns.reset("dash");
        dash.start();
        buffer.clear("dash");
        state.state = PlayerState::Dashing;
        state.stateLockFrames = 12;
    }

    if (dash.active) {
        if (currentFrame % 2 == 0) {
            auto& anim = e->get<CAnimation>().anim;
            sf::Sprite currentSprite = anim.getSprite(); // capture current frame
            currentSprite.setScale(state.facing_right ? 4.0f : -4.0f, 4.0f); // apply flip
            currentSprite.setOrigin(
                currentSprite.getTextureRect().width / 2.0f,
                currentSprite.getTextureRect().height / 2.0f
            );
            spawnTrail(trans.pos, currentSprite, sf::Color(0, 100, 255, 255));
        }
        trans.velocity.y = 0;
        trans.velocity.x = state.facing_right ? 15.0f : -15.0f;
    }

}
//--
void Game::handlePlayerInput(Entity* e, bool onGroundNow) {
    auto& input = e->get<CInput>();
    auto& trans = e->get<CTransform>();
    auto& state = e->get<CState>();
    auto& jump = e->get<CJump>();
    auto& buffer = e->get<CBuffer>();

    const float moveSpeed = 5.0f;
    const float jumpVelocity = -10.0f;

    float prevVelX = trans.velocity.x;

    trans.velocity.x = 0;
    if (input.left) {
        trans.velocity.x = -moveSpeed;
        if (state.facing_right) {
            state.facing_right = false;
            if (onGroundNow && state.state != PlayerState::RunningTurn) {
                state.state = PlayerState::RunningTurn;
                state.stateLockFrames = 5;
                return;
            }
        }
    } else if (input.right) {
        trans.velocity.x = moveSpeed;
        if (!state.facing_right) {
            state.facing_right = true;
            if (onGroundNow && state.state != PlayerState::RunningTurn) {
                state.state = PlayerState::RunningTurn;
                state.stateLockFrames = 5;
                return;
            }
        }
    }

    // Jump logic (buffered)
    bool jumpRequested = (input.up || buffer.has("jump"));
    bool freshJump = jump.jumpReleased && input.up;

    if (jumpRequested && freshJump &&
        (jump.jumpsLeft > 0 || jump.coyoteTimer > 0)) {
        
        trans.velocity.y = jumpVelocity;

        if (jump.coyoteTimer > 0) {
            jump.coyoteTimer = 0;
        } else {
            jump.jumpsLeft--;
        }

        jump.jumpReleased = false;
        buffer.clear("jump");

        if (jump.jumpsLeft == 1 && state.state != PlayerState::Jump1) {
            state.state = PlayerState::Jump1;
        } else if (jump.jumpsLeft == 0 && state.state != PlayerState::Jump2) {
            state.state = PlayerState::Jump2;
        }

        return;
    }

    // Air states
    if (!onGroundNow) {
        if (trans.velocity.y < 0) {
            if (jump.jumpsLeft == 1 && state.state != PlayerState::Jump1) {
                state.state = PlayerState::Jump1;
            } else if (jump.jumpsLeft == 0 && state.state != PlayerState::Jump2) {
                state.state = PlayerState::Jump2;
            }
        } else {
            if (state.state != PlayerState::Falling) {
                state.state = PlayerState::Falling;
            }
        }
        return;
    }

    // Grounded movement states
    if (prevVelX == 0 && trans.velocity.x != 0) {
        // just started moving
        if (state.state != PlayerState::RunningStart) {
            state.state = PlayerState::RunningStart;
        }
    } else if (prevVelX != 0 && trans.velocity.x == 0) {
        // just stopped moving
        if (state.state != PlayerState::RunningStop) {
            state.state = PlayerState::RunningStop;
        }
    } else if (trans.velocity.x != 0) {
        if (state.state != PlayerState::Running) {
            state.state = PlayerState::Running;
        }
    } else {
        if (state.state != PlayerState::Idle) {
            state.state = PlayerState::Idle;
        }
    }
}
//--
string Game::getAnimationNameForState(PlayerState state, bool facingRight) {
    std::string base;
    switch (state) {
        case PlayerState::Idle:         base = "idle"; break;
        case PlayerState::RunningStart: base = "dashstart"; break;
        case PlayerState::Running:      base = "dash"; break;
        case PlayerState::RunningStop:  base = "dashstop"; break;
        case PlayerState::RunningTurn:  base = "dashturn"; break;
        case PlayerState::Jump1:        base = "jump"; break;
        case PlayerState::Jump2:        base = "doublejump"; break;
        case PlayerState::Falling:      base = "fall"; break;
        case PlayerState::Dashing:      base = "dattack"; break;
        case PlayerState::Attacking:    base = "ftilt"; break;
        default:                        base = "idle"; break;
    }

    if (!player_has_bone) base += "_boneless";
    return base;
}
//--
void Game::loadAllAnimations() {
    animationLoadMessages.clear();
    animations.clear();
    textures.clear();

    std::set<std::string> oneShotAnims = {
        "dattack", "ftilt", "jump", "doublejump", "uspecial", "freya_attack"
    };

    animationLoadMessages.push_back("Loading animations from config...");

    std::regex stripPattern(R"((.*)_strip(\d+)\.png)");

    auto tryLoad = [&](const std::string& name, const std::string& relativePath, const std::string& baseDir) {
        std::string fullPath = baseDir + relativePath;

        auto tex = std::make_shared<sf::Texture>();
        if (!tex->loadFromFile(fullPath)) {
            animationLoadMessages.push_back("❌ Failed to load: " + fullPath);
            std::cerr << "Failed to load texture: " << fullPath << "\n";
            return;
        }

        textures[name] = tex;

        size_t frameCount = 1;
        std::smatch match;
        if (std::regex_match(relativePath, match, stripPattern)) {
            frameCount = std::stoul(match[2].str());
        }

        Animation anim;
        anim.loadFromStrip(tex, frameCount, 8); // Default frame duration: 8
        anim.loop = (oneShotAnims.count(name) == 0);
        animations[name] = anim;

        animationLoadMessages.push_back("✅ Loaded: " + name + " (" + std::to_string(frameCount) + " frames)");
    };

    // Load player animations from ./sprites/
    if (gameConfig.contains("player_sprites")) {
        for (auto& [name, file] : gameConfig["player_sprites"].items()) {
            tryLoad(name, file.get<std::string>(), "./sprites/");
        }
    }

    // Load Freya animations from ./enemy_sprites/freya/
    if (gameConfig.contains("freya_sprites")) {
        for (auto& [name, file] : gameConfig["freya_sprites"].items()) {
            tryLoad(name, file.get<std::string>(), "./enemy_sprites/freya/");
        }
    }

    if (animations.empty()) {
        animationLoadMessages.push_back("⚠️ No animations found in config.");
    }
}
//--
Game::Game(const string& config) {
    init(config);
}
//--
void Game::init(const string& path) {
    std::ifstream file(path);
    assert(file);
    std::string line;
    while (getline(file, line)) {
        std::istringstream iss(line);
        std::vector<std::string> words;
        std::string word;
        while (iss >> word) {
            words.push_back(word);
        }
        if (words.empty()) continue;
        if (words[0] == "Window") {
            sf::VideoMode videoMode;
            sf::Uint32 style;
            if (stoi(words[4])) {
                videoMode = sf::VideoMode::getDesktopMode();
                style = sf::Style::Fullscreen;
            } else {
                videoMode = sf::VideoMode(stoi(words[1]), stoi(words[2]));
                style = sf::Style::Titlebar | sf::Style::Close;
            }
            window.create(videoMode, "Platformer", style);
            window.setFramerateLimit(stoi(words[3]));
        }
    }

    ImGui::SFML::Init(window);
    ImGui::GetStyle().ScaleAllSizes(2.0f);
    ImGui::GetIO().FontGlobalScale = 2.0f;
    srand(static_cast<unsigned>(time(nullptr)));

    loadGameConfig("config.json");
    loadAllAnimations();
    spawn_test_level();
}
//--
Entity* Game::player() {
    return entityManager.getEntities("player")[0];
}
//--
void Game::run() {
    while (running) {
        entityManager.update();
        ImGui::SFML::Update(window, deltaClock.restart());
        if (!paused) {
            if (m_lifespanSystem) sLifeSpan();
            if (m_movementSystem) sMovement();
            if (m_collisionSystem) sCollision();
            if (m_inputSystem) sUserInput();
            if (m_attackSystem) sAttack();
            if (m_boneThrow and player_has_bone) sBoneThrow();
        } else {
            sUserInput();
        }
        sGUI();
        if (m_animationSystem) sAnimation();
        if (m_drawSystem) sRender();
        currentFrame++;
    }
}
//--
void Game::spawn_player() {
    auto* p = entityManager.addEntity("player");

    // Core components
    Vec2f spawnPos = {100, 100};
    p->add<CTransform>(spawnPos, Vec2f(0, 0), 0);
    p->add<CInput>();
    p->add<CCooldowns>();
    p->add<CDash>(12);
    p->add<CState>(PlayerState::Idle);
    p->add<CGravity>(0.5f);
    p->add<CCollision>(0);
    p->add<CJump>();
    p->add<CBuffer>();

    // Animation setup
    auto& idleAnim = animations["idle"];
    idleAnim.setScale(4.0f);
    p->add<CAnimation>(idleAnim, "idle");

    // Size from animation sprite
    sf::Vector2f frameSize = {
        idleAnim.getSprite().getGlobalBounds().width,
        idleAnim.getSprite().getGlobalBounds().height
    };

    // Visible debug shape
    p->add<CShape>(frameSize, sf::Color::Transparent, sf::Color::White, 0);

    // CECB setup (diamond shape)
    float ecbWidth = 40.0f;
    float ecbHeight = 80.0f;

    CECB ecb;
    ecb.setDiamond(spawnPos, ecbWidth, ecbHeight);
    p->add<CECB>(ecb);

    // Cooldowns
    auto& cds = p->get<CCooldowns>();
    cds.addCooldown("dash", 60);
    cds.addCooldown("attack", 70);
    cds.addCooldown("bone_throw", 60); // 1s at 60fps
}
//--
void Game::spawn_enemy(Vec2f pos, Vec2f size, int health) {
    auto* enemy = entityManager.addEntity("enemy");
    enemy->add<CTransform>(pos, Vec2f(0, 0), 0);
    enemy->add<CShape>(size, sf::Color::Green, sf::Color::Black, 2);
    enemy->add<CHealth>(health);
    enemy->add<CGravity>(0.5f);
    enemy->add<CCollision>(0);
}
//--
void Game::spawn_freya(const Vec2f& pos, int health) {
    auto* freya = entityManager.addEntity("freya");

    freya->add<CHealth>(3);
    freya->add<CGravity>(0.5f);
    freya->add<CCollision>();
    freya->add<CState>(PlayerState::Idle);

    auto& idle_anim = animations["freya_idle"];
    idle_anim.setScale(2.0f);
    sf::Sprite& sprite = idle_anim.getSprite();

    sf::Vector2f texSize(
        static_cast<float>(sprite.getTextureRect().width),
        static_cast<float>(sprite.getTextureRect().height)
    );
    printf("%f, %f", texSize.x, texSize.y);

    freya->add<CTransform>(pos, Vec2f(0, 0), 0);

    freya->add<CAnimation>(idle_anim, "freya_idle");

    sf::Vector2f frameSize = {
        texSize.x * 1,
        texSize.y * 1
    };
    freya->add<CShape>(frameSize, sf::Color::Transparent, sf::Color::White, 0);

    CECB ecb;
    ecb.setDiamond(pos, ECB_WIDTH, ECB_HEIGHT);
    freya->add<CECB>(ecb);
}
//--
void Game::spawnPlatform(Vec2f pos, Vec2f size) {
    auto* platform = entityManager.addEntity("platform");
    platform->add<CTransform>(pos, Vec2f(0, 0), 0);
    platform->add<CShape>(size, sf::Color::Blue, sf::Color::White, 2);
    platform->add<CCollision>(0);
}
//--
void Game::spawn_test_level() {
    spawn_player();

    spawnPlatform({1920, 2100}, {3840, 60});
    spawn_enemy({1800, 2040}, {60, 60}, 5);

    spawnPlatform({300, 1900}, {200, 30});
    spawnPlatform({300, 1700}, {200, 30});
    spawnPlatform({300, 1500}, {200, 30});
    spawnPlatform({300, 1300}, {200, 30});
    spawn_enemy({300, 1250}, {60, 60}, 3);

    spawnPlatform({1920, 1800}, {300, 30});
    spawnPlatform({1920, 1600}, {250, 30});
    spawnPlatform({1920, 1400}, {200, 30});
    spawnPlatform({1920, 1200}, {150, 30});
    spawn_enemy({1920, 1160}, {60, 60}, 4);

    spawnPlatform({2800, 1600}, {200, 30});
    spawnPlatform({3000, 1450}, {200, 30});
    spawnPlatform({3200, 1300}, {200, 30});
    spawn_enemy({3200, 1260}, {60, 60}, 6);

    spawnPlatform({300, 600}, {150, 30});
    spawn_enemy({300, 540}, {60, 60}, 7);

    spawnPlatform({3500, 600}, {200, 30});
    spawn_freya({3500, 540}, 100);

    spawnPlatform({1400, 1000}, {250, 30});
    spawnPlatform({1800, 1000}, {250, 30});
    spawn_freya({1600, 940}, 100);

    spawnPlatform({800, 1600}, {100, 20});
    spawnPlatform({1000, 1500}, {100, 20});
    spawnPlatform({1200, 1400}, {100, 20});

    spawn_freya({1800, 1980}, 100);
}
//--
bool Game::onGround(Entity* playerEntity) {
    if (!playerEntity->has<CECB>()) return false;
    const auto& ecb = playerEntity->get<CECB>();
    sf::Vector2f bottom = ecb.shape.getPoint(2); // bottom vertex of the diamond
    
    if (playerEntity->tag() == "bone") {
        printf("bone on ground");
    }
    const float epsilon = 1.0f;

    for (auto* platform : entityManager.getEntities("platform")) {
        const auto& platTrans = platform->get<CTransform>();
        const auto& platShape = platform->get<CShape>();
        sf::Vector2f platSize = platShape.rect.getSize();

        sf::FloatRect platBounds(
            platTrans.pos.x - platSize.x / 2,
            platTrans.pos.y - platSize.y / 2,
            platSize.x,
            platSize.y
        );

        float platformTop = platBounds.top;

        // Check vertical proximity
        if (std::abs(bottom.y - platformTop) <= epsilon) {
            // And horizontal coverage
            if (bottom.x >= platBounds.left && bottom.x <= platBounds.left + platBounds.width) {
                return true;
            }
        }
    }

    return false;
}
//--