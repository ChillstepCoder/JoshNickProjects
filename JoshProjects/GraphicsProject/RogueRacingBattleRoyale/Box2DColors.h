//Box2DColors.h

#pragma once
#include <box2d/box2d.h>

// Helper function to create b2HexColor from RGB components
inline b2HexColor makeHexColor(uint8_t r, uint8_t g, uint8_t b) {
  // Cast to b2HexColor explicitly
  return static_cast<b2HexColor>((r << 16) | (g << 8) | b);
}

// Helper function to create b2HexColor from float RGB components (0-1)
inline b2HexColor makeHexColorFloat(float r, float g, float b) {
  return makeHexColor(
    static_cast<uint8_t>(r * 255.0f),
    static_cast<uint8_t>(g * 255.0f),
    static_cast<uint8_t>(b * 255.0f)
  );
}

// Common colors
namespace Colors {
  inline b2HexColor Green() { return makeHexColor(0, 255, 0); }
  inline b2HexColor YellowGreen() { return makeHexColor(179, 255, 0); }
  inline b2HexColor Yellow() { return makeHexColor(255, 255, 0); }
  inline b2HexColor Orange() { return makeHexColor(255, 128, 0); }
  inline b2HexColor Red() { return makeHexColor(255, 0, 0); }
  inline b2HexColor White() { return makeHexColor(255, 255, 255); }
}
