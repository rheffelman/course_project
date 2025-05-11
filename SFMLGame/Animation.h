#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <memory>
#include "Vec2.h"

class Animation {
public:
    Animation() = default;

    Animation(const std::string& name, std::shared_ptr<sf::Texture> texture, size_t frameCount, size_t frameDuration)
        : name(name), frameCount(frameCount), frameDuration(frameDuration), currentFrame(0), currentAnimationFrame(0), finished(false) {
        loadFromStrip(texture, frameCount, frameDuration);
    }

    void loadFromStrip(std::shared_ptr<sf::Texture> tex, size_t frames, size_t duration) {
        texture = tex;
        frameCount = frames;
        frameDuration = duration;
        currentFrame = 0;
        currentAnimationFrame = 0;
        finished = false;

        frameSize = Vec2f(
            static_cast<float>(tex->getSize().x) / frameCount,
            static_cast<float>(tex->getSize().y)
        );

        sprite.setTexture(*texture);
        sprite.setOrigin(frameSize.x / 2.0f, frameSize.y / 2.0f);
        sprite.setTextureRect(sf::IntRect(0, 0, static_cast<int>(frameSize.x), static_cast<int>(frameSize.y)));
    }

    void update() {
        if (finished) return;

        currentFrame++;
        if (currentFrame >= frameDuration) {
            currentFrame = 0;
            currentAnimationFrame++;

            if (currentAnimationFrame >= frameCount) {
                if (loop) {
                    currentAnimationFrame = 0;
                } else {
                    currentAnimationFrame = frameCount - 1; // hold on final frame
                    finished = true;
                }
            }

            sprite.setTextureRect(sf::IntRect(
                static_cast<int>(currentAnimationFrame * frameSize.x),
                0,
                static_cast<int>(frameSize.x),
                static_cast<int>(frameSize.y)
            ));
        }
    }

    void restart() {
        currentFrame = 0;
        currentAnimationFrame = 0;
        finished = false;
    }

    void setPosition(const Vec2f& pos) {
        sprite.setPosition(sf::Vector2f(pos.x, pos.y));
    }

    void setRotation(float angle) {
        sprite.setRotation(angle);
    }

    void setScale(float scale) {
        sprite.setScale(scale, scale);
    }

    void draw(sf::RenderWindow& window) const {
        window.draw(sprite);
    }

    sf::Sprite& getSprite() {
        return sprite;
    }

    bool loop = true;
    bool finished = false;

private:
    std::string name;
    std::shared_ptr<sf::Texture> texture;
    sf::Sprite sprite;
    Vec2f frameSize;
    size_t frameCount = 1;
    size_t frameDuration = 1;
    size_t currentFrame = 0;
    size_t currentAnimationFrame = 0;
};
