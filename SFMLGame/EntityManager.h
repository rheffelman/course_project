#pragma once

#include "Entity.h"
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <cassert>

using EntityVec = std::vector<Entity*>;
using EntityMap = std::map<std::string, EntityVec>;

class EntityManager
{
public:
    EntityManager() = default;

    ~EntityManager()
    {
        // Clean up all dynamically allocated entities
        for (auto* e : m_entities) {
            delete e;
        }
        m_entities.clear();
        m_entityMap.clear();
    }

    Entity* addEntity(const std::string& tag)
    {
        std::string safeTag = tag.empty() ? "default" : tag;
        Entity* entity = new Entity(m_totalEntities++, safeTag);

        // std::cout << "[Create] Entity ID " << entity->id() << " (tag: " << safeTag << ")\n";

        m_entitiesToAdd.push_back(entity);
        m_entityMap[safeTag].push_back(entity);
        return entity;
    }

    void update()
    {
        // Add new entities
        for (Entity* e : m_entitiesToAdd) {
            m_entities.push_back(e);
        }
        m_entitiesToAdd.clear();

        // Remove dead entities from all storage
        removeDeadEntities(m_entities);

        for (auto it = m_entityMap.begin(); it != m_entityMap.end(); ) {
            removeDeadEntities(it->second);
            if (it->second.empty()) {
                it = m_entityMap.erase(it);
            } else {
                ++it;
            }
        }
    }

    EntityVec& getEntities() { return m_entities; }
    EntityVec& getEntities(const std::string& tag) { return m_entityMap[tag]; }

private:
    EntityVec m_entities;
    EntityVec m_entitiesToAdd;
    EntityMap m_entityMap;
    size_t m_totalEntities = 0;

    void removeDeadEntities(EntityVec& vec)
    {
        EntityVec survivors;
        for (auto* e : vec) {
            if (!e) {
                std::cerr << "[Warning] Null entity pointer found in vector!\n";
                continue;
            }

            if (!e->isActive()) {
    			if (e->isDeleted()) {
     				// std::cerr << "[Error] Entity ID " << e->id() << " was already deleted! Skipping.\n";
        			continue;
    			}

				if (e->id() > 1000000) {
					std::cerr << "[Error] Entity has corrupted ID: " << e->id() << ". Skipping delete.\n";
					continue;
				}

				// std::cout << "[Delete] Entity ID " << e->id() << " (tag: " << e->tag() << ")\n";
				e->destroy(); // ensure m_deleted = true
				delete e;
			}
			else {
				survivors.push_back(e);
			}
		}
        vec = std::move(survivors);
    }
};
