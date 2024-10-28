// InputState.h

#pragma once

struct InputState {
  bool accelerating = false;   // W or Up
  bool braking = false;        // S or Down
  bool turningLeft = false;    // A or Left
  bool turningRight = false;   // D or Right
  bool handbrake = false;      // Space
};
