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
    debugDraw.DrawSolidCapsule = drawCapsule;


    // Draw the world
    b2World_Draw(*world, &debugDraw);

    // Draw all line loops
    // Build mesh of line loops
    // Draw mesh

    // Upload to GPU
    glBindVertexArray(m_vaoId);

    // Setup attributes
    setAttrib();

    // Lines
    glBindBuffer(GL_ARRAY_BUFFER, m_vboId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(DebugVertex), m_lineVertexData.data(), GL_DYNAMIC_DRAW);

    // Draw lines
    glDrawArrays(GL_LINES, 0, m_lineVertexData.size() / 2);

    m_lineVertexData.clear();

    // Line loops
    glBindVertexArray(m_vaoId);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(DebugVertex), m_lineLoopVertexData.data(), GL_DYNAMIC_DRAW);

    // Draw Line loops
    glDrawArrays(GL_LINE_LOOP, 0, m_lineLoopVertexData.size() / 2);

    m_lineLoopVertexData.clear();

    // Triangle Fans
    glBindVertexArray(m_vaoId);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(DebugVertex), m_triangleFanVertexData.data(), GL_DYNAMIC_DRAW);

    // Draw Triangle Fans
    glDrawArrays(GL_TRIANGLE_FAN, 0, m_triangleFanVertexData.size() / 2);

    m_triangleFanVertexData.clear();




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
    m_lineVertexData.push_back(DebugVertex(p1, r, g, b, a));
    m_lineVertexData.push_back(DebugVertex(p2, r, g, b, a));
}

void DebugDraw::drawPolygon(const b2Vec2* vertices, int vertexCount, b2HexColor color, void* context) {
    DebugDraw* debugDraw = static_cast<DebugDraw*>(context);

    // Convert hex color to float values (0-1 range)
    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = (color & 0xFF) / 255.0f;
    float a = debugDraw->m_alpha;

    // Create vertex buffer with positions and colors
    m_lineLoopVertexData.reserve(vertexCount * 5); // 2 for position, 3 for color

    for (int i = 0; i < vertexCount; ++i) {
        m_lineLoopVertexData.push_back(DebugVertex(vertices[i], r, g, b, a));
    }
}

void DebugDraw::drawSolidPolygon(b2Transform xf, const b2Vec2* vertices, int vertexCount, float radius, b2HexColor color, void* context) {
    DebugDraw* debugDraw = static_cast<DebugDraw*>(context);

    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = (color & 0xFF) / 255.0f;
    float a = debugDraw->m_alpha;

    // Transform vertices by xf
    m_triangleFanVertexData.reserve(vertexCount * 5);

    for (int i = 0; i < vertexCount; ++i) {
        b2Vec2 v = b2TransformPoint(xf, vertices[i]);
        m_triangleFanVertexData.push_back(DebugVertex(v, r, g, b, a));
    }
}

void DebugDraw::drawCircle(b2Vec2 center, float radius, b2HexColor color, void* context) {
    DebugDraw* debugDraw = static_cast<DebugDraw*>(context);

    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = (color & 0xFF) / 255.0f;
    float a = debugDraw->m_alpha;

    const int segments = 16;
    m_lineLoopVertexData.reserve(segments * 5);

    for (int i = 0; i < segments; ++i) {
        float angle = (float(i) / segments) * 2.0f * b2_pi;
        float x = center.x + radius * cosf(angle);
        float y = center.y + radius * sinf(angle);
        m_lineLoopVertexData.push_back(DebugVertex((b2Vec2)(x, y), r, g, b, a));
    }
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
    m_triangleFanVertexData.reserve((segments + 1) * 5); // +1 for center point

    // Center point
    m_triangleFanVertexData.push_back(DebugVertex((center), r, g, b, a));

    // Circle points
    for (int i = 0; i < segments; ++i) {
        float angle = (float(i) / segments) * 2.0f * b2_pi;
        float x = center.x + radius * cosf(angle);
        float y = center.y + radius * sinf(angle);
        m_triangleFanVertexData.push_back(DebugVertex((b2Vec2)(x, y), r, g, b, a));
    }
}

void DebugDraw::drawCapsule(b2Vec2 p1, b2Vec2 p2, float radius, b2HexColor color, void* context) {

    // Draw the end circles
    b2Transform circle1Transform = { p1 };
    b2Transform circle2Transform = { p2 };
    drawSolidCircle(circle1Transform, radius, color, context);
    drawSolidCircle(circle2Transform, radius, color, context);

    // Calculate the direction vector between centers
    b2Vec2 d = b2Sub(p2, p1);
    float angle = atan2f(d.y, d.x);

    // Calculate the corner points of the rectangle
    b2Vec2 p1l = {
        p1.x - radius * sinf(angle),
        p1.y + radius * cosf(angle)
    };
    b2Vec2 p1r = {
        p1.x + radius * sinf(angle),
        p1.y - radius * cosf(angle)
    };
    b2Vec2 p2l = {
        p2.x - radius * sinf(angle),
        p2.y + radius * cosf(angle)
    };
    b2Vec2 p2r = {
        p2.x + radius * sinf(angle),
        p2.y - radius * cosf(angle)
    };

    // Draw the connecting lines
    drawSegment(p1l, p2l, color, context);
    drawSegment(p1r, p2r, color, context);
}

void DebugDraw::drawTransform(b2Transform xf, void* context) {
    const float axisScale = 0.4f;

    // Draw X axis in red
    b2Vec2 p1 = b2TransformPoint(xf, b2Vec2_zero);
    b2Vec2 p2 = b2TransformPoint(xf, b2Vec2(axisScale, 0.0f));
    drawSegment(p1, p2, (b2HexColor)(255, 0, 0), context);

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

    m_triangleFanVertexData.push_back(DebugVertex(p, r, g, b, a));
}

void DebugDraw::setAttrib() {
    // Setup attributes
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)offsetof(DebugVertex, pos));
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(DebugVertex), (void*)offsetof(DebugVertex, color));
}