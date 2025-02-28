// DebugDraw.cpp

#include "Box2DColors.h"
#include "DebugDraw.h"
#include "Car.h"
#include "WheelCollider.h"
#include <GL/glew.h>
#include <vector>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

DebugDraw::DebugDraw() {
}

DebugDraw::~DebugDraw() {
    if (m_vboId) glDeleteBuffers(1, &m_vboId);
    if (m_vaoId) glDeleteVertexArrays(1, &m_vaoId);
}

void DebugDraw::init() {
  // Update shader filenames
  m_program.compileShaders("Shaders/debug.vert", "Shaders/debug.frag");
  m_program.addAttribute("vertexPosition");
  m_program.addAttribute("vertexColor");
  m_program.linkShaders();

  // Create VAO and VBO
  glGenVertexArrays(1, &m_vaoId);
  glGenBuffers(1, &m_vboId);
}

// Implement the debug draw callbacks
void DebugDraw::drawSegment(b2Vec2 p1, b2Vec2 p2, b2HexColor color, void* context) {
    DebugDraw* debugDraw = static_cast<DebugDraw*>(context);

    // Convert hex color to float values (0-1 range)
    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = (color & 0xFF) / 255.0f;
    float a = debugDraw->m_alpha;

    // Setup vertex data
    float vertices[] = {
        p1.x, p1.y, r, g, b, a,
        p2.x, p2.y, r, g, b, a,
    };

    // Upload to GPU
    glBindVertexArray(debugDraw->m_vaoId);
    glBindBuffer(GL_ARRAY_BUFFER, debugDraw->m_vboId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    // Setup attributes
    setAttrib();

    // Draw
    glDrawArrays(GL_LINES, 0, 2);
}

void DebugDraw::drawPolygon(const b2Vec2* vertices, int32_t vertexCount, b2HexColor color, void* context) {
  DebugDraw* debugDraw = static_cast<DebugDraw*>(context);

  float r = ((color >> 16) & 0xFF) / 255.0f;
  float g = ((color >> 8) & 0xFF) / 255.0f;
  float b = (color & 0xFF) / 255.0f;
  float a = debugDraw->m_alpha;

  std::vector<float> vertexData;
  vertexData.reserve(vertexCount * 6); // pos + color per vertex

  for (int32_t i = 0; i < vertexCount; ++i) {
    vertexData.push_back(vertices[i].x);
    vertexData.push_back(vertices[i].y);
    vertexData.push_back(r);
    vertexData.push_back(g);
    vertexData.push_back(b);
    vertexData.push_back(a);
  }

  glBindVertexArray(debugDraw->m_vaoId);
  glBindBuffer(GL_ARRAY_BUFFER, debugDraw->m_vboId);
  glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_DYNAMIC_DRAW);

  debugDraw->setAttrib();

  // Draw outline
  glDrawArrays(GL_LINE_LOOP, 0, vertexCount);
}

void DebugDraw::drawSolidPolygon(b2Transform xf, const b2Vec2* vertices, int32_t vertexCount, float radius, b2HexColor color, void* context) {
  DebugDraw* debugDraw = static_cast<DebugDraw*>(context);

  float r = ((color >> 16) & 0xFF) / 255.0f;
  float g = ((color >> 8) & 0xFF) / 255.0f;
  float b = (color & 0xFF) / 255.0f;
  float a = debugDraw->m_alpha * 1.0f;  // Semi-transparent fill

  std::vector<float> vertexData;
  vertexData.reserve(vertexCount * 6);

  // Transform vertices using xf
  for (int32_t i = 0; i < vertexCount; ++i) {
    b2Vec2 v = b2TransformPoint(xf, vertices[i]);
    vertexData.push_back(v.x);
    vertexData.push_back(v.y);
    vertexData.push_back(r);
    vertexData.push_back(g);
    vertexData.push_back(b);
    vertexData.push_back(a);
  }

  glBindVertexArray(debugDraw->m_vaoId);
  glBindBuffer(GL_ARRAY_BUFFER, debugDraw->m_vboId);
  glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_DYNAMIC_DRAW);

  debugDraw->setAttrib();

  // Draw filled polygon
  glDrawArrays(GL_TRIANGLE_FAN, 0, vertexCount);

  // Draw outline with full alpha
  a = debugDraw->m_alpha;
  for (size_t i = 5; i < vertexData.size(); i += 6) {
    vertexData[i] = a;
  }
  glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_DYNAMIC_DRAW);
  glDrawArrays(GL_LINE_LOOP, 0, vertexCount);
}

void DebugDraw::drawCircle(const b2Vec2 center, float radius, b2HexColor color, void* context) {
    DebugDraw* debugDraw = static_cast<DebugDraw*>(context);

    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = (color & 0xFF) / 255.0f;
    float a = debugDraw->m_alpha;

    const int segments = 16;
    std::vector<float> vertexData;
    vertexData.reserve(segments * 5);

    for (int i = 0; i < segments; ++i) {
        float angle = (float(i) / segments) * 2.0f * b2_pi;
        float x = center.x + radius * cosf(angle);
        float y = center.y + radius * sinf(angle);
        vertexData.push_back(x);
        vertexData.push_back(y);
        vertexData.push_back(r);
        vertexData.push_back(g);
        vertexData.push_back(b);
        vertexData.push_back(a);
    }

    glBindVertexArray(debugDraw->m_vaoId);
    glBindBuffer(GL_ARRAY_BUFFER, debugDraw->m_vboId);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_DYNAMIC_DRAW);

    setAttrib();

    glDrawArrays(GL_LINE_LOOP, 0, segments);
}

void DebugDraw::drawSolidCircle(b2Transform xf, float radius, b2HexColor color, void* context) {
  DebugDraw* debugDraw = static_cast<DebugDraw*>(context);

  float r = ((color >> 16) & 0xFF) / 255.0f;
  float g = ((color >> 8) & 0xFF) / 255.0f;
  float b = (color & 0xFF) / 255.0f;
  float a = debugDraw->m_alpha * 0.5f;

  // Get the center from xf.p
  b2Vec2 center = xf.p;

  const int segments = 16;
  std::vector<float> vertexData;
  vertexData.reserve((segments + 1) * 6);

  // Generate circle vertices
  for (int i = 0; i <= segments; ++i) {
    float angle = (float(i) / segments) * 2.0f * b2_pi;
    float x = center.x + radius * cosf(angle);
    float y = center.y + radius * sinf(angle);
    vertexData.push_back(x);
    vertexData.push_back(y);
    vertexData.push_back(r);
    vertexData.push_back(g);
    vertexData.push_back(b);
    vertexData.push_back(a);
  }

  glBindVertexArray(debugDraw->m_vaoId);
  glBindBuffer(GL_ARRAY_BUFFER, debugDraw->m_vboId);
  glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_DYNAMIC_DRAW);

  debugDraw->setAttrib();

  // Draw filled circle
  glDrawArrays(GL_TRIANGLE_FAN, 0, segments + 1);
}

void DebugDraw::drawTransform(b2Transform xf, void* context) {
  const float axisScale = 0.4f;

  // Draw X axis in red
  b2Vec2 p1 = b2TransformPoint(xf, { 0.0f, 0.0f });  // Create b2Vec2 with braces
  b2Vec2 p2 = b2TransformPoint(xf, { axisScale, 0.0f });
  drawSegment(p1, p2, (b2HexColor)(255, 0, 0), context);

  // Draw Y axis in green
  b2Vec2 p2Y = b2TransformPoint(xf, { 0.0f, axisScale });
  drawSegment(p1, p2Y, (b2HexColor)(0, 255, 0), context);
}

void DebugDraw::drawPoint(b2Vec2 p, float size, b2HexColor color, void* context) {
  DebugDraw* debugDraw = static_cast<DebugDraw*>(context);

  float r = ((color >> 16) & 0xFF) / 255.0f;
  float g = ((color >> 8) & 0xFF) / 255.0f;
  float b = (color & 0xFF) / 255.0f;
  float a = debugDraw->m_alpha;

  float halfSize = size;  // Increase point size for better visibility

  // Create a cross shape for better visibility
  float vertices[] = {
    // Horizontal line
    p.x - halfSize, p.y, r, g, b, a,
    p.x + halfSize, p.y, r, g, b, a,
    // Vertical line
    p.x, p.y - halfSize, r, g, b, a,
    p.x, p.y + halfSize, r, g, b, a
  };

  glBindVertexArray(debugDraw->m_vaoId);
  glBindBuffer(GL_ARRAY_BUFFER, debugDraw->m_vboId);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

  setAttrib();

  glDrawArrays(GL_LINES, 0, 4);
}

void DebugDraw::drawString(b2Vec2 p, const char* s, void* context) {
    // Note: This is left empty as text rendering requires additional setup
    // If you want to implement text rendering, you'll need to add a text rendering system
    // This could use FreeType, SDL_ttf, or another text rendering library
}

void DebugDraw::setAttrib() {
    // Setup attributes
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
}

void DebugDraw::drawWorld(b2WorldId worldId, const glm::mat4& projectionMatrix) {
  m_program.use();

  // Upload projection matrix
  GLint pUniform = m_program.getUniformLocation("P");
  glUniformMatrix4fv(pUniform, 1, GL_FALSE, &projectionMatrix[0][0]);

  // Setup debug draw struct
  b2DebugDraw debugDraw = {};
  debugDraw.drawShapes = true;
  debugDraw.drawJoints = true;
  debugDraw.drawAABBs = false;
  debugDraw.drawContacts = false;
  debugDraw.drawContactNormals = false;
  debugDraw.context = this;

  // Set the callbacks
  debugDraw.DrawSegment = drawSegment;
  debugDraw.DrawPolygon = drawPolygon;
  debugDraw.DrawCircle = drawCircle;
  debugDraw.DrawSolidCircle = drawSolidCircle;
  debugDraw.DrawSolidPolygon = drawSolidPolygon;
  debugDraw.DrawTransform = drawTransform;
  debugDraw.DrawPoint = drawPoint;
  debugDraw.DrawString = drawString;

  // Draw the world
  b2World_Draw(worldId, &debugDraw);

  m_program.unuse();
}

void DebugDraw::drawWheelColliders(const Car& car, const glm::mat4& projectionMatrix) {
  if (!m_program.isValid()) return;

  m_program.use();
  GLint pUniform = m_program.getUniformLocation("P");
  glUniformMatrix4fv(pUniform, 1, GL_FALSE, &projectionMatrix[0][0]);

  const auto& wheelStates = car.getWheelStates();
  float carAngle = car.getDebugInfo().angle;

  for (const auto& state : wheelStates) {
    glm::vec2 pos = state.position;

    // Note: width and height are swapped in the visualization to match the rotated collider
    float height = 6.0f; // This will be the length of the wheel
    float width = 3.0f;  // This will be the width of the wheel

    // Create transform that includes car rotation plus 90 degrees for wheel orientation
    b2Transform wheelTransform;
    wheelTransform.p = { pos.x, pos.y };
    wheelTransform.q = b2MakeRot(carAngle + b2_pi * 0.5f); // Add 90 degrees rotation

    // Create vertices for wheel in local space
    b2Vec2 vertices[4] = {
        {-width / 2, -height / 2},  // Bottom left
        {width / 2, -height / 2},   // Bottom right
        {width / 2, height / 2},    // Top right
        {-width / 2, height / 2}    // Top left
    };

    // Color based on surface type
    b2HexColor color;
    switch (state.surface) {
    case WheelCollider::Surface::Road:
      color = Colors::Green();
      break;
    case WheelCollider::Surface::RoadOffroad:
      color = Colors::YellowGreen();
      break;
    case WheelCollider::Surface::Offroad:
      color = Colors::Yellow();
      break;
    case WheelCollider::Surface::OffroadGrass:
      color = Colors::Orange();
      break;
    case WheelCollider::Surface::Grass:
      color = Colors::Red();
      break;
    default:
      color = Colors::White();
      break;
    }

    // Draw filled polygon with rotation
    drawSolidPolygon(wheelTransform, vertices, 4, 0.0f, color, this);
  }

  m_program.unuse();
}
