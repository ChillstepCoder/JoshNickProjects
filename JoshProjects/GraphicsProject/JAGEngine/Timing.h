#pragma once

namespace JAGEngine {

  class FpsLimiter {
  public:
    FpsLimiter();
    void init(float maxFPS);
    void setMaxFPS(float maxFPS);
    void begin();

    float end(); //returns current fps
  private:

    void calculateFPS();

    float m_fps;
    float m_maxFPS;
    float _frameTime;

    unsigned int _startTicks;
  };
}
