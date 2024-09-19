#pragma once

#include "Agent.h"

class Zombie : public Agent
{
public:
    Zombie();
    ~Zombie();

    void init(float speed, glm::vec2 pos);

    virtual void update(const std::vector<std::string>& levelData,
                        std::vector<Human*>& humans,
                        std::vector<Zombie*>& zombies);
    void draw(Bengine::SpriteBatch& _spriteBatch);
private:

    Human* getNearestHuman(std::vector<Human*>& humans);

    float _rotation; // Rotation in degrees
};

