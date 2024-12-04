// Vertex.h

#pragma once

#ifndef APIENTRY
#define APIENTRY
#endif
#include <GL/glew.h>
#include <ImGui/imgui.h>

namespace JAGEngine {
  struct Position {
    float x;
    float y;
  };

  struct ColorRGBA8 {
    ColorRGBA8() : r(0), g(0), b(0), a(0) { }
    ColorRGBA8(GLubyte R, GLubyte G, GLubyte B, GLubyte A) :
      r(R), g(G), b(B), a(A) { }

    GLubyte r;
    GLubyte g;
    GLubyte b;
    GLubyte a;

    ImVec4 toImVec4() const {
      return ImVec4(
        static_cast<float>(r) / 255.0f,
        static_cast<float>(g) / 255.0f,
        static_cast<float>(b) / 255.0f,
        static_cast<float>(a) / 255.0f
      );
    }
  };

  struct UV {
    float u;
    float v;
  };

  struct Vertex {
    Position position;
    ColorRGBA8 color;
    //uv texture coordinates
    UV uv;
    void setPosition(float x, float y) {
      position.x = x;
      position.y = y;
    }
    void setColor(GLubyte r, GLubyte b, GLubyte g, GLubyte a) {
      color.r = r;
      color.g = g;
      color.b = b;
      color.a = a;
    }
    void setUV(float u, float v) {
      uv.u = u;
      uv.v = v;
    }
  };
}
