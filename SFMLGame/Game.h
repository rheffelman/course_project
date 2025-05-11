#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include "EntityManager.h"
#include "Vec2.h"
#include "Animation.h"
#include <unordered_map>
#include <filesystem>
#include <regex>
#include "nlohmann/json.hpp"
namespace fs = std::filesystem;
extern nlohmann::json gameConfig;

class Game
{
public:
    Game(const std::string& config);
    void run();

private:

	struct BufferedInput {
		bool active = false;
		float timer = 1.0f;
		float maxTime = 0.2f; // 200 ms buffer window
	};
    // Initialization
    void init(const std::string& config);

    // Systems
    void sMovement();
    void sUserInput();
    void sLifeSpan();
    void sRender();
    void sGUI();
    void sCollision();
    void sAttack();
    void sBoneThrow();
    void sAnimation();

    // Helpers
    Entity* player();
    bool onGround(Entity* playerEntity);
    void loadAllAnimations();
    string getAnimationNameForState(PlayerState state, bool facingRight);
    
    // Utils
    bool checkAABBCollision(const Entity* a, const Entity* b);
    bool pointInDiamond(const sf::Vector2f& point, const sf::ConvexShape& diamond);
    bool diamondIntersectsAABB(const sf::ConvexShape& diamond, const sf::FloatRect& box);
    bool canChangeTo(PlayerState current, PlayerState target);
    bool isStateLocked(Entity* e);
    void handleLanding(Entity* e);
    void handleDash(Entity* e);
    void loadECBConfig(const string& filename);
    void loadGameConfig(const string& filename);
    void handlePlayerInput(Entity* e, bool onGroundNow);
    // spawning
    void spawn_test_level();
    void spawnTrail(const Vec2f& pos, const sf::Sprite& sourceSprite, const sf::Color& color);
    void spawn_player();
    void spawnPlatform(Vec2f pos, Vec2f size);
    void spawn_enemy(Vec2f pos, Vec2f size, int health);
    void spawn_freya(const Vec2f& pos, int health);

    // Game state
    sf::RenderWindow window;
    sf::Clock deltaClock;
    EntityManager entityManager;

    bool paused = false;
    bool running = true;
    int currentFrame = 0;
	BufferedInput jumpBuffer;

    unordered_map<string, shared_ptr<sf::Texture>> textures;
    unordered_map<string, Animation> animations;
    vector<string> animationLoadMessages;
    unordered_map<string, pair<float, float>> ecbConfigs;

    bool m_drawSystem = true;
    bool m_movementSystem = true;
    bool m_inputSystem = true;
    bool m_lifespanSystem = true;
    bool m_collisionSystem = true;
    bool m_attackSystem = true;
    bool m_animationSystem = true;
    bool m_boneThrow = true;

    bool player_has_bone = true;
    bool showCECB = false;
    bool showHitboxes = false;


};
