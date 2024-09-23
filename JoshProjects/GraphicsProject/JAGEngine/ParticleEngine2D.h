//ParticleEngine2D.h

#pragma once

#include <vector>

namespace JAGEngine {

  class ParticleBatch2D;
  class SpriteBatch;

  class ParticleEngine2D
  {
  public:
    ParticleEngine2D();
    ~ParticleEngine2D();

    //after adding particle batch, the particle engine 2d becomes
    //responsible for the deallocation.
    void addParticleBatch(ParticleBatch2D* particleBatch);

    void update(float deltaTime);

    void draw(SpriteBatch* spriteBatch);

  private:
    std::vector<ParticleBatch2D*> m_batches;
  };
}
