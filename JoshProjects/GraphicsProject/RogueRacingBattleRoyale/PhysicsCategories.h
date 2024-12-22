// PhysicsCategories.h

#pragma once
#include <memory>

static const uint16_t CATEGORY_CAR = 0x0001;         // Moving vehicles
static const uint16_t CATEGORY_BARRIER = 0x0002;     // Immovable world barriers
static const uint16_t CATEGORY_SOLID = 0x0004;       // Solid obstacles (trees, poles, etc)
static const uint16_t CATEGORY_HAZARD = 0x0008;      // Drivable hazards (potholes, oil slicks, etc)
static const uint16_t CATEGORY_PUSHABLE = 0x0010;    // Light objects cars can push (cones, boxes, etc)
static const uint16_t CATEGORY_POWERUP = 0x0020;

static const uint16_t MASK_CAR = CATEGORY_BARRIER | CATEGORY_SOLID | CATEGORY_PUSHABLE | CATEGORY_CAR | CATEGORY_POWERUP;
