// Car.h
#pragma once
#include <Box2D/box2d.h>
#include <glm/glm.hpp>
#include "InputState.h"
#include "JAGEngine/Vertex.h"

class Car {
public:
  struct CarProperties {
    // Movement properties
    float maxSpeed = 2000.0f;         
    float acceleration = 20000.0f;    
    float turnSpeed = 20.0f;          
    float lateralDamping = 0.8f;     // Reduced from 0.95f for smoother slides
    float dragFactor = 0.995f;       // Increased from 0.99f for less drag
    float brakingForce = 0.7f;       // Increased from 0.5f for better control
    float maxAngularVelocity = 4.0f;        
    float minSpeedForTurn = 1.0f;
    float turnResetRate = 5.0f;

    // Friction properties
    float wheelFriction = 1.0f;       
    float baseFriction = 0.5f;        
  };

  struct DebugInfo {
    glm::vec2 position;
    glm::vec2 velocity;
    float currentSpeed = 0.0f;
    float forwardSpeed = 0.0f;
    float angle = 0.0f;
    float angularVelocity = 0.0f;
    float effectiveFriction = 0.0f;
    b2BodyId bodyId;
  };

  Car(b2BodyId bodyId);
  ~Car() = default;

  void update(const InputState& input);
  void resetPosition(const b2Vec2& position = { -100.0f, -100.0f }, float angle = 0.0f);

  CarProperties& getProperties() { return m_properties; }
  const CarProperties& getProperties() const { return m_properties; }
  void setProperties(const CarProperties& props) { m_properties = props; }
  DebugInfo getDebugInfo() const;
  void setColor(const JAGEngine::ColorRGBA8& color) { m_color = color; }

  float getEffectiveFriction() const {
    return m_properties.wheelFriction * m_properties.baseFriction;
  }

  const JAGEngine::ColorRGBA8& getColor() const { return m_color; }

private:
  static b2Rot angleToRotation(float angle) {
    return b2MakeRot(angle);
  }

  b2BodyId m_bodyId;
  CarProperties m_properties;

  b2Vec2 getForwardVector() const;
  void updateMovement(const InputState& input);
  void handleTurning(const InputState& input, float forwardSpeed);
  void applyDrag(const b2Vec2& currentVel, float forwardSpeed);
  void applyFriction(const b2Vec2& currentVel);
  JAGEngine::ColorRGBA8 m_color{ 255, 0, 0, 255 };
};
