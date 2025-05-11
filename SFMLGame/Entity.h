#pragma once

#include "Components.h"
#include <tuple>
#include <string>
#include <iostream>
#include <cassert>

using ComponentTuple = std::tuple<
    CTransform, 
    CShape, 
    CCollision,
    CInput,
    CLifespan,
    CScore,
    CState,
    CCooldowns,
    CDash,
	CHealth,
	CGravity,
	CJump,
	CBuffer,
	CAnimation,
	CECB,
    CStuck
>;

class EntityManager;

class Entity {
    friend class EntityManager;

    size_t m_id = 0;
    std::string m_tag = "default";
    bool m_active = true;
    bool m_deleted = false; // Protects against double-delete
    ComponentTuple m_components;

    // Private constructor ensures only EntityManager can create
    Entity(size_t id, const std::string& tag)
        : m_id(id), m_tag(tag) {
        assert(!tag.empty() && "Entity tag must not be empty!");
    }

public:
    size_t id() const { return m_id; }
    const std::string& tag() const { return m_tag; }

    bool isActive() const { return m_active; }
    bool isDeleted() const { return m_deleted; }

    void destroy() {
        if (m_deleted) {
            std::cerr << "[Error] Attempted to destroy entity ID " << m_id << " twice!\n";
            return;
        }
        m_active = false;
        m_deleted = true;
    }

    template <typename T>
    bool has() const {
        return std::get<T>(m_components).exists;
    }

    template <typename T, typename... TArgs>
    T& add(TArgs&&... args) {
        T& comp = std::get<T>(m_components);
        comp = T(std::forward<TArgs>(args)...);
        comp.exists = true;
        return comp;
    }

    template <typename T>
    T& get() {
        return std::get<T>(m_components);
    }

    template <typename T>
    const T& get() const {
        return std::get<T>(m_components);
    }

    template <typename T>
    void remove() {
        std::get<T>(m_components) = T();
    }
};
