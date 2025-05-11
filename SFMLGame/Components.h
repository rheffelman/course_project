#pragma once

#include "Vec2.h"
#include "Animation.h"
#include<SFML/Graphics.hpp>
#include<string>
#include<unordered_map>
using namespace std;
//#include "Animation.h"
//#include <variant>

class Component
{
public:
	bool exists = false;
};

class CTransform : public Component
{
public:
	CTransform() = default;
	CTransform(const Vec2f& p, const Vec2f& v, float a)
		:pos(p), velocity(v), angle(a) {}

	Vec2f pos = { 0,0 };
	Vec2f velocity = { 0,0 };
	float angle = 0;
};

class CShape : public Component
{
public:
	bool isRect = false;
	sf::CircleShape circle;
	sf::RectangleShape rect;

	CShape() = default;

	// Circle constructor
	CShape(float radius, int points, const sf::Color& fill,
		const sf::Color& outline, float thickness)
		: isRect(false)
	{
		circle = sf::CircleShape(radius, points);
		circle.setFillColor(fill);
		circle.setOutlineColor(outline);
		circle.setOutlineThickness(thickness);
		circle.setOrigin(radius, radius);
	}

	// Rectangle constructor
	CShape(sf::Vector2f size, const sf::Color& fill,
		const sf::Color& outline, float thickness)
		: isRect(true)
	{
		rect = sf::RectangleShape(size);
		rect.setOrigin(size.x / 2.0f, size.y / 2.0f);
		rect.setFillColor(fill);
		rect.setOutlineColor(outline);
		rect.setOutlineThickness(thickness);
		rect.setOrigin(size.x / 2.0f, size.y / 2.0f);
	}
};

class CBoundingBox : public Component
{
public:
	CBoundingBox() = default;
	CBoundingBox(const Vec2f& s)
		:size(s), halfSize(s.x / 2, s.y / 2) {}
	Vec2f size;
	Vec2f halfSize;
};

class CECB : public Component {
public:
    sf::ConvexShape shape;

    CECB() {
        shape.setPointCount(4);
        shape.setFillColor(sf::Color::Transparent);
        shape.setOutlineColor(sf::Color::Magenta);
        shape.setOutlineThickness(1);
    }

    void setDiamond(const Vec2f& center, float width, float height) {
        shape.setPoint(0, sf::Vector2f(center.x, center.y - height / 2)); // top
        shape.setPoint(1, sf::Vector2f(center.x + width / 2, center.y));  // right
         shape.setPoint(2, sf::Vector2f(center.x, center.y + height / 2)); // bottom
        shape.setPoint(3, sf::Vector2f(center.x - width / 2, center.y));  // left
    }
	void setTriangle(const Vec2f& center, float width, float height) {
        shape.setPoint(0, sf::Vector2f(center.x, center.y - height / 2)); // top
        shape.setPoint(1, sf::Vector2f(center.x + width / 2, center.y));  // right
         shape.setPoint(2, sf::Vector2f(center.x, center.y)); // bottom
        shape.setPoint(3, sf::Vector2f(center.x - width / 2, center.y));  // left
    }
};

class CHealth : public Component
{
public:
    CHealth() = default;
    CHealth(int hp) : max(hp), current(hp) {}
    int max = 1;
    int current = 1;
};

class CGravity : public Component
{
public:
	CGravity() = default;
	CGravity(float g) : gravity(g) {}
	float gravity = 0;
};

enum class PlayerState {
    Idle,
    Running,
    Jump1,
    Jump2,
    Falling,
    Attacking,
    Dashing,
    RunningStart,
    RunningStop,
    RunningTurn
};

class CState : public Component {
public:
    PlayerState state = PlayerState::Idle;
    bool facing_right = true;
    int stateLockFrames = 0;

    CState() = default;
    CState(PlayerState s) : state(s) {}

    std::string stateString() const {
        switch (state) {
            case PlayerState::Idle: return "Idle";
            case PlayerState::Running: return "Running";
            case PlayerState::Jump1: return "Jump1";
            case PlayerState::Jump2: return "Jump2";
            case PlayerState::Falling: return "Falling";
            case PlayerState::Attacking: return "Attacking";
            case PlayerState::Dashing: return "Dashing";
        }
        return "Unknown";
    }
};

struct BufferedInput {
    std::string action;
    int framesRemaining;

    BufferedInput(const std::string& a, int f) : action(a), framesRemaining(f) {}
};

class CBuffer : public Component {
public:
    std::vector<BufferedInput> inputs;

	void add(const std::string& action, int duration = 15) { // ← was 10
    	inputs.emplace_back(action, duration);
	}


    void update() {
        for (auto& input : inputs) {
            input.framesRemaining--;
        }
        inputs.erase(std::remove_if(inputs.begin(), inputs.end(),
            [](const BufferedInput& i) { return i.framesRemaining <= 0; }),
            inputs.end());
    }

    bool has(const std::string& action) const {
        for (const auto& i : inputs) {
            if (i.action == action) return true;
        }
        return false;
    }

    void clear(const std::string& action) {
        inputs.erase(std::remove_if(inputs.begin(), inputs.end(),
            [&](const BufferedInput& i) { return i.action == action; }),
            inputs.end());
    }
};

class CAnimation : public Component {
public:
    Animation anim;
    std::string currentName;

    CAnimation() = default;
    CAnimation(const Animation& animation, const std::string& name)
        : anim(animation), currentName(name) {}
};

class CCollision : public Component
{
public:
	CCollision() = default;
	CCollision(float r)
		:radius(r) {}

	float radius = 0;
};

class CScore : public Component
{
public:
	CScore() = default;
	CScore(int s)
		:score(s) {
	}

	int score = 0;
};

class CLifespan : public Component
{
public:
	CLifespan() = default;
	CLifespan(int totalLifespan)
		:remaining(totalLifespan), lifespan(totalLifespan) {}

	int remaining = 0;
	int lifespan = 0;
};

class CCooldowns : public Component
{
public:
    struct Cooldown {
        int cooldownDuration = 0;
        int currentCooldown = 0;

        Cooldown() = default;
        Cooldown(int duration) : cooldownDuration(duration), currentCooldown(0) {}

        bool ready() const { return currentCooldown <= 0; }
        void reset() { currentCooldown = cooldownDuration; }
        void update() { if (currentCooldown > 0) currentCooldown--; }
    };

    std::unordered_map<std::string, Cooldown> cds;

    void addCooldown(const std::string& name, int duration) {
        cds[name] = Cooldown(duration);
    }

    bool ready(const std::string& name) const {
        auto it = cds.find(name);
        return it != cds.end() && it->second.ready();
    }

    void reset(const std::string& name) {
        auto it = cds.find(name);
        if (it != cds.end()) {
            it->second.reset();
        }
    }

    void update() {
        for (auto& [_, cd] : cds) {
            cd.update();
        }
    }
};

class CInput : public Component
{
public:
	CInput() = default;

	bool up = false;
	bool left = false;
	bool right = false;
	bool down = false;
	bool attack = false;
	bool dash = false;
	bool bone_throw = false;
};

// movement related
class CDash : public Component
{
public:
    CDash() = default;
    CDash(int duration) : duration(duration), timer(0), active(false) {}

    int duration = 0;    // total dash duration (in frames)
    int timer = 0;       // countdown timer
    bool active = false; // is dash currently active

    void start() {
        timer = duration;
        active = true;
    }

    void update() {
        if (active && timer > 0) {
            timer--;
            if (timer == 0) active = false;
        }
    }
};

class CJump : public Component {
public:
    int jumpsLeft = 2;
    bool jumpReleased = true;
    int coyoteTimer = 0;
};
class CStuck : public Component {};