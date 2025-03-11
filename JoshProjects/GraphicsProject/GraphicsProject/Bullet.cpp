#include "Bullet.h"
#include <JAGEngine/ResourceManager.h>

Bullet::Bullet(glm::vec2 pos, glm::vec2 dir, float speed, int lifeTime) {
  _position = pos;
  _direction = dir;
  _speed = speed;
  _lifeTime = lifeTime;
}

Bullet::~Bullet() {

}

void Bullet::draw(JAGEngine::SpriteBatch& spriteBatch) {

  glm::vec4 uv(0.0f, 0.0f, 1.0f, 1.0f);

  JAGEngine::GLTexture texture = JAGEngine::ResourceManager::getTexture("Textures/jimmyjump_pack/PNG/Lightning1.png");
   
  JAGEngine::ColorRGBA8 color;

  color.r = 255;
  color.g = 255;
  color.b = 255;
  color.a = 255;

  glm::vec4 posAndSize = glm::vec4(_position.x, _position.y, 300, 300);

  spriteBatch.draw(posAndSize, uv, texture.id, 0.1f, color);
}

bool Bullet::update() {
  _position += _direction * _speed;
  _lifeTime--;
  if (_lifeTime == 0) {
    return true;
  }
  return false;
}
