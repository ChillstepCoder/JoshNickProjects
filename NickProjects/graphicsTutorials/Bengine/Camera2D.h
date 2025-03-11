#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Bengine {

    class Camera2D
    {

    public:
        Camera2D();
        ~Camera2D();

        void init(int screenWidth, int screenHeight);

        void update();

        glm::vec2 convertScreenToWorld(glm::vec2 screenCoords);

        bool isBoxInView(const glm::vec2& position, const glm::vec2& dimensions);

        //Setters
        void setMapBoundaries(const glm::vec2& mapMin, const glm::vec2& mapMax) {
            m_mapMinBounds = mapMin;
            m_mapMaxBounds = mapMax;
        }

        void setPosition(const glm::vec2& newPosition) {
            m_position = clampPositionToBoundary(newPosition);
            m_needsMatrixUpdate = true;
        }

        void setScale(float newScale) { m_scale = newScale; m_needsMatrixUpdate = true; }


        //Getters
        glm::vec2 getPosition() { return m_position; }
        float getScale() { return m_scale; }
        glm::mat4 getCameraMatrix() { return m_cameraMatrix; }


        glm::vec2 getCameraViewMin() const {
            glm::vec2 scaledHalfScreen = glm::vec2(m_screenWidth, m_screenHeight) / (2.0f * m_scale);
            return m_position - scaledHalfScreen;
        }

        glm::vec2 getCameraViewMax() const {
            glm::vec2 scaledHalfScreen = glm::vec2(m_screenWidth, m_screenHeight) / (2.0f * m_scale);
            return m_position + scaledHalfScreen;
        }

    private:

        glm::vec2 clampPositionToBoundary(const glm::vec2& position) {
            // Calculate camera view boundaries
            glm::vec2 halfScreenWorld = glm::vec2(m_screenWidth, m_screenHeight) / (2.0f * m_scale);

            // Calculate min/max allowed camera position
            glm::vec2 minAllowedPos = m_mapMinBounds + halfScreenWorld;
            glm::vec2 maxAllowedPos = m_mapMaxBounds - halfScreenWorld;

            // If the map is smaller than the view, center the camera on the map
            if (minAllowedPos.x > maxAllowedPos.x) {
                float center = (m_mapMinBounds.x + m_mapMaxBounds.x) / 2.0f;
                minAllowedPos.x = center;
                maxAllowedPos.x = center;
            }
            if (minAllowedPos.y > maxAllowedPos.y) {
                float center = (m_mapMinBounds.y + m_mapMaxBounds.y) / 2.0f;
                minAllowedPos.y = center;
                maxAllowedPos.y = center;
            }

            // Clamp the position
            glm::vec2 clampedPos;
            clampedPos.x = std::max(minAllowedPos.x, std::min(position.x, maxAllowedPos.x));
            clampedPos.y = std::max(minAllowedPos.y, std::min(position.y, maxAllowedPos.y));
            return clampedPos;
        }


        int m_screenWidth, m_screenHeight;
        bool m_needsMatrixUpdate;
        float m_scale;
        glm::vec2 m_position;
        glm::mat4 m_cameraMatrix;
        glm::mat4 m_orthoMatrix;

        glm::vec2 m_mapMinBounds = glm::vec2(0.0f, 0.0f);
        glm::vec2 m_mapMaxBounds = glm::vec2(100000.0f, 100000.0f); // Default to large values
    };

}
