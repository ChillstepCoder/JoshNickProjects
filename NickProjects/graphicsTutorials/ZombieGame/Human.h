#pragma once

#include "Agent.h"

class Human : public Agent
{
public:
    Human();
    virtual ~Human();

    void init(float speed, glm::vec2 pos);

    virtual void update(const std::vector<std::string>& levelData,
                        std::vector<Human*>& humans,
                        std::vector<Zombie*>& zombies, 
                        float deltaTime) override;

    virtual void draw(Bengine::SpriteBatch& _spriteBatch);

private:
    glm::vec2 _direction;
    int _frames;

    float _rotation;

};

