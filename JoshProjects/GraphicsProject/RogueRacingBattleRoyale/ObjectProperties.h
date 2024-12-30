// ObjectProperties.h

#pragma once

enum class CollisionType {
  DEFAULT,
  POWERUP,
  PUSHABLE,
  HAZARD
};

struct BoosterProperties {
  float maxBoostSpeed = 1500.0f;
  float boostAccelRate = 100.0f;
  float boostDecayRate = 0.95f;
  float directionFactor = 1.0f;
};

struct XPProperties {
  int xpAmount = 1;
  float respawnTime = 3.0f;
  bool isActive = true;
  float respawnTimer = 0.0f;
};

enum class ObjectType {
  Default,
  Booster,
  XPPickup,
  Tree,
  TrafficCone,
  Pothole
};
