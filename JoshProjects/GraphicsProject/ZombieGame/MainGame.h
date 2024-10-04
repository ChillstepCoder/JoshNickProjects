//MainGame.h

#pragma once

#include <SDL/SDL.h>
#include <windows.h>
#include <GL/glew.h> 
#include <stdio.h>

#include "JAGEngine/GLSLProgram.h"
#include "JAGEngine/Sprite.h"
#include "JAGEngine/GLTexture.h"
#include "JAGEngine/Window.h"
#include "JAGEngine/JAGEngine.h"
#include "JAGEngine/Camera2D.h"
#include "JAGEngine/SpriteBatch.h"
#include "JAGEngine/InputManager.h"
#include "JAGEngine/Timing.h"
#include "JAGEngine/ResourceManager.h"
#include "JAGEngine/SpriteFont.h"
#include <JAGEngine/AudioEngine.h>
#include "JAGEngine/ParticleEngine2D.h"
#include "JAGEngine/ParticleBatch2D.h"

#include "Player.h"
#include "Level.h"
#include "Bullet.h"
#include <vector>
#include <cmath>

class Zombie;

enum class GameState { PLAY, EXIT };

class MainGame
{
public:
  MainGame();
  ~MainGame();

  void run();
private:
  void initSystems();

  void initLevel();

  void initShaders();
  void gameLoop();

  void updateAgents(float deltaTime);
  void updateBullets(float deltaTime);

  void checkVictory();
  void loadNextLevel();
  bool doesNextLevelExist();

  void processInput();
  void drawGame();

  void drawHud();

  void addBlood(const glm::vec2& position, int numParticles);

  JAGEngine::Window m_window;

  int m_screenWidth = 1024;
  int m_screenHeight = 768;

  GameState m_gameState;

  JAGEngine::Sprite m_sprite;
  JAGEngine::GLSLProgram m_textureProgram;
  JAGEngine::GLSLProgram m_textRenderingProgram;
  JAGEngine::Camera2D m_camera;
  JAGEngine::Camera2D m_hudCamera;

  JAGEngine::SpriteBatch m_agentSpriteBatch; //draws all agents
  JAGEngine::SpriteBatch m_hudSpriteBatch;

  JAGEngine::SpriteBatch m_spriteBatch;

  JAGEngine::InputManager m_inputManager;
  JAGEngine::FpsLimiter m_fpsLimiter;

  std::vector<Level*> m_levels;

  Player* m_player;
  std::vector<Human*> m_humans;
  std::vector<Zombie*> m_zombies;
  std::vector<Bullet> m_bullets;

  JAGEngine::ResourceManager m_resourceManager;
  JAGEngine::SpriteFont* m_spriteFont;

  JAGEngine::ParticleEngine2D m_particleEngine;
  JAGEngine::ParticleBatch2D* m_bloodParticleBatch;

  JAGEngine::AudioEngine m_audioEngine;

  float m_fps;
  float m_maxFPS;
  float m_time;
  int m_currentLevel;

  int m_numHumansKilled; //humans killed by player
  int m_numZombiesKilled;


};

