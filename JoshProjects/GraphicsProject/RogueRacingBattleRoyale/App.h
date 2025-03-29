// App.h
#pragma once
#include <JAGEngine/IMainGame.h>
#include "AudioEngine.h"
#include "GameplayScreen.h"
#include "LevelEditorScreen.h"

// Option 1: Keep using original AudioEngine
class App : public JAGEngine::IMainGame {
public:
    App() : IMainGame() {
        std::cout << "App constructor start\n";
    }

    ~App() {
        std::cout << "App destructor\n";
        signalCleanup();
    }

    virtual void onInit() override;
    virtual void addScreens() override;
    virtual void onExit() override;

    virtual void updateAudio() override {
        if (m_audioEngine) {
            m_audioEngine->update();
        }
    }

    AudioEngine& getAudioEngine() { return *m_audioEngine; }

private:
    std::unique_ptr<GameplayScreen> m_gameplayScreen = nullptr;
    std::unique_ptr<LevelEditorScreen> m_levelEditorScreen = nullptr;
    std::unique_ptr<AudioEngine> m_audioEngine = nullptr;
};

/*
// Option 2: If you want to switch to WWiseAudioEngine later:
// Uncomment this section and comment out the above class

#include <JAGEngine/WWiseAudioEngine.h>  // Add this include

class App : public JAGEngine::IMainGame {
public:
  App() : IMainGame() {
    std::cout << "App constructor start\n";
  }

  ~App() {
    std::cout << "App destructor\n";
    signalCleanup();
  }

  virtual void onInit() override;
  virtual void addScreens() override;
  virtual void onExit() override;

  virtual void updateAudio() override {
    if (m_audioEngine) {
      m_audioEngine->update();
    }
  }

  JAGEngine::WWiseAudioEngine& getAudioEngine() { return *m_audioEngine; }

private:
  std::unique_ptr<GameplayScreen> m_gameplayScreen = nullptr;
  std::unique_ptr<LevelEditorScreen> m_levelEditorScreen = nullptr;
  std::unique_ptr<JAGEngine::WWiseAudioEngine> m_audioEngine = nullptr;
};
*/