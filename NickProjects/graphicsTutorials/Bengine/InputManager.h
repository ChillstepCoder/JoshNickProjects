#pragma once

#include <unordered_map>
#include <glm/glm.hpp>

namespace Bengine {

    class InputManager
    {
    public:
        InputManager();
        ~InputManager();

        void update();

        void pressKey(unsigned int keyID);
        void releaseKey(unsigned int keyID);

        void setMouseCoords(float x, float y);

        // Returns true if the key is held down
        bool isKeyDown(unsigned int keyID);

        // Returns true if the key was just pressed
        bool isKeyPressed(unsigned int keyID);

        //getters
        glm::vec2 getMouseCoords() const { return m_mouseCoords; }
    private:
        // Returns true if the key is held down
        bool wasKeyDown(unsigned int keyID);

        std::unordered_map<unsigned int, bool> m_keyMap;
        std::unordered_map<unsigned int, bool> m_previousKeyMap;
        glm::vec2 m_mouseCoords;
    };

}

