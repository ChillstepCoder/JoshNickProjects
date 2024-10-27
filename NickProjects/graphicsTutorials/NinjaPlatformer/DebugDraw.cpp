

#include "DebugDraw.h"
#include <GL/glew.h>
#include <vector>

DebugDraw::DebugDraw() {
}

DebugDraw::~DebugDraw() {
    if (m_vboId) glDeleteBuffers(1, &m_vboId);
    if (m_vaoId) glDeleteVertexArrays(1, &m_vaoId);
}

void DebugDraw::init() {
    // Compile shaders
    m_program.compileShaders("Shaders/debugVert.txt", "Shaders/debugFrag.txt");
    m_program.addAttribute("vertexPosition");
    m_program.addAttribute("vertexColor");
    m_program.linkShaders();

    // Create VAO and VBO
    glGenVertexArrays(1, &m_vaoId);
    glGenBuffers(1, &m_vboId);
}

void DebugDraw::drawWorld(b2WorldId* world, const glm::mat4& projectionMatrix) {
    m_program.use();

    // Upload projection matrix
    GLint pUniform = m_program.getUniformLocation("P");
    glUniformMatrix4fv(pUniform, 1, GL_FALSE, &projectionMatrix[0][0]);

    // Setup debug draw struct
    b2DebugDraw debugDraw = {};
    debugDraw.drawShapes = true;
    debugDraw.drawJoints = true;
    debugDraw.drawAABBs = true;
    debugDraw.drawContacts = true;
    debugDraw.drawContactNormals = true;
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
    b2World_Draw(*world, &debugDraw);

    m_program.unuse();
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
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));

    // Draw
    glDrawArrays(GL_LINES, 0, 2);
}

void DebugDraw::drawPolygon(const b2Vec2* vertices, int vertexCount, b2HexColor color, void* context) {
    DebugDraw* debugDraw = static_cast<DebugDraw*>(context);

    // Convert hex color to float values (0-1 range)
    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = (color & 0xFF) / 255.0f;
    float a = debugDraw->m_alpha;

    // Create vertex buffer with positions and colors
    std::vector<float> vertexData;
    vertexData.reserve(vertexCount * 5); // 2 for position, 3 for color

    for (int i = 0; i < vertexCount; ++i) {
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

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));

    glDrawArrays(GL_LINE_LOOP, 0, vertexCount);
}

void DebugDraw::drawSolidPolygon(b2Transform xf, const b2Vec2* vertices, int vertexCount, float radius, b2HexColor color, void* context) {
    DebugDraw* debugDraw = static_cast<DebugDraw*>(context);

    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = (color & 0xFF) / 255.0f;
    float a = debugDraw->m_alpha;

    // Transform vertices by xf
    std::vector<float> vertexData;
    vertexData.reserve(vertexCount * 5);

    for (int i = 0; i < vertexCount; ++i) {
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

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));

    glDrawArrays(GL_TRIANGLE_FAN, 0, vertexCount);
}

void DebugDraw::drawCircle(b2Vec2 center, float radius, b2HexColor color, void* context) {
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

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));

    glDrawArrays(GL_LINE_LOOP, 0, segments);
}

void DebugDraw::drawSolidCircle(b2Transform xf, float radius, b2HexColor color, void* context) {
    DebugDraw* debugDraw = static_cast<DebugDraw*>(context);

    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = (color & 0xFF) / 255.0f;
    float a = debugDraw->m_alpha;

    b2Vec2 center = b2TransformPoint(xf, b2Vec2_zero);

    const int segments = 16;
    std::vector<float> vertexData;
    vertexData.reserve((segments + 1) * 5); // +1 for center point

    // Center point
    vertexData.push_back(center.x);
    vertexData.push_back(center.y);
    vertexData.push_back(r);
    vertexData.push_back(g);
    vertexData.push_back(b);
    vertexData.push_back(a);

    // Circle points
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

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));

    glDrawArrays(GL_TRIANGLE_FAN, 0, segments + 1);
}

void DebugDraw::drawTransform(b2Transform xf, void* context) {
    const float axisScale = 0.4f;

    // Draw X axis in red
    b2Vec2 p1 = b2TransformPoint(xf, b2Vec2_zero);
    b2Vec2 p2 = b2TransformPoint(xf, b2Vec2(axisScale, 0.0f));
    drawSegment(p1, p2, (b2HexColor)(255,0,0), context);

    // Draw Y axis in green
    p2 = b2TransformPoint(xf, b2Vec2(0.0f, axisScale));
    drawSegment(p1, p2, (b2HexColor)(0, 255, 0), context);
}

void DebugDraw::drawPoint(b2Vec2 p, float size, b2HexColor color, void* context) {
    DebugDraw* debugDraw = static_cast<DebugDraw*>(context);

    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = (color & 0xFF) / 255.0f;
    float a = debugDraw->m_alpha;

    float halfSize = size * 0.5f;

    // Create a small square
    float vertices[] = {
        p.x - halfSize, p.y - halfSize, r, g, b, a,
        p.x + halfSize, p.y - halfSize, r, g, b, a,
        p.x + halfSize, p.y + halfSize, r, g, b, a,
        p.x - halfSize, p.y + halfSize, r, g, b, a,
    };

    glBindVertexArray(debugDraw->m_vaoId);
    glBindBuffer(GL_ARRAY_BUFFER, debugDraw->m_vboId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void DebugDraw::drawString(b2Vec2 p, const char* s, void* context) {
    // Note: This is left empty as text rendering requires additional setup
    // If you want to implement text rendering, you'll need to add a text rendering system
    // This could use FreeType, SDL_ttf, or another text rendering library
}

