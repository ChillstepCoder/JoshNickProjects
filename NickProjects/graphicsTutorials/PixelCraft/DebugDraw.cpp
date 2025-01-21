#include "DebugDraw.h"
#include <GL/glew.h>
#include <vector>


DebugDraw::~DebugDraw() {
    glDeleteBuffers(1, &m_linesvboId);
    glDeleteVertexArrays(1, &m_linesvaoId);
    glDeleteBuffers(1, &m_trianglesvboId);
    glDeleteVertexArrays(1, &m_trianglesvaoId);
}

void DebugDraw::init() {
    // Compile shaders
    m_program.compileShaders("Shaders/debugVert.txt", "Shaders/debugFrag.txt");
    m_program.addAttribute("vertexPosition");
    m_program.addAttribute("vertexColor");
    m_program.linkShaders();

    // Create VAO and VBO
    glGenVertexArrays(1, &m_linesvaoId);
    glGenBuffers(1, &m_linesvboId);
    glGenVertexArrays(1, &m_trianglesvaoId);
    glGenBuffers(1, &m_trianglesvboId);

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
    debugDraw.DrawSolidCapsule = drawCapsule;

    if (m_vertexDataChanged) {
        m_lineVertexData.clear();
        m_triangleVertexData.clear();
        // Draw the world
        b2World_Draw(*world, &debugDraw);




        // Upload to GPU
        glBindVertexArray(m_linesvaoId);
        glBindBuffer(GL_ARRAY_BUFFER, m_linesvboId);
        setAttrib();

        // Lines
        glBufferData(GL_ARRAY_BUFFER, sizeof(DebugVertex) * m_lineVertexData.size(), m_lineVertexData.data(), GL_DYNAMIC_DRAW);


        //Triangles
        glBindVertexArray(m_trianglesvaoId);
        glBindBuffer(GL_ARRAY_BUFFER, m_trianglesvboId);
        setAttrib();

        glBufferData(GL_ARRAY_BUFFER, sizeof(DebugVertex) * m_triangleVertexData.size(), m_triangleVertexData.data(), GL_DYNAMIC_DRAW);

        m_vertexDataChanged = false;
    }

    // Draw lines
    glBindVertexArray(m_linesvaoId);
    glBindBuffer(GL_ARRAY_BUFFER, m_linesvboId);
    glDrawArrays(GL_LINES, 0, m_lineVertexData.size() / 2);
    // Draw Triangles
    glBindVertexArray(m_trianglesvaoId);
    glBindBuffer(GL_ARRAY_BUFFER, m_trianglesvboId);
    glDrawArrays(GL_TRIANGLES, 0, m_triangleVertexData.size() / 3);

    m_program.unuse();
}

// Implement the debug draw callbacks
void DebugDraw::drawSegment(b2Vec2 p1, b2Vec2 p2, b2HexColor color, void* context) {
    DebugDraw* debugDraw = static_cast<DebugDraw*>(context);

    // Convert hex color to float values (0-1 range)
    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = (color & 0xFF) / 255.0f;
    float a = 0.5f;

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

    // Iterate through the vertices and draw the edges
    for (int i = 0; i < vertexCount; ++i) {
        b2Vec2 p1 = vertices[i];
        b2Vec2 p2 = vertices[(i + 1) % vertexCount]; // Loops back to the first vertex

        // drawSegment draws the edge from p1 to p2
        drawSegment(p1, p2, color, context);
    }
}

void DebugDraw::drawSolidPolygon(b2Transform xf, const b2Vec2* vertices, int vertexCount, float radius, b2HexColor color, void* context) {
    assert(vertexCount == 4);
    DebugDraw* debugDraw = static_cast<DebugDraw*>(context);

    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = (color & 0xFF) / 255.0f;
    float a = debugDraw->m_alpha;

    // Transform vertices by xf
    m_triangleVertexData.reserve(vertexCount * 3); // 3 vertices per triangle

    b2Vec2 transformedVertices[4];
    for (int i = 0; i < vertexCount; ++i) {
        transformedVertices[i] = b2TransformPoint(xf, vertices[i]);
    }

    // Triangle 1: A, B, C
    m_triangleVertexData.push_back(DebugVertex(transformedVertices[0], r, g, b, a)); // A
    m_triangleVertexData.push_back(DebugVertex(transformedVertices[1], r, g, b, a)); // B
    m_triangleVertexData.push_back(DebugVertex(transformedVertices[2], r, g, b, a)); // C

    // Triangle 2: C, D, A
    m_triangleVertexData.push_back(DebugVertex(transformedVertices[2], r, g, b, a)); // C
    m_triangleVertexData.push_back(DebugVertex(transformedVertices[3], r, g, b, a)); // D
    m_triangleVertexData.push_back(DebugVertex(transformedVertices[0], r, g, b, a)); // A
}

void DebugDraw::drawCircle(b2Vec2 center, float radius, b2HexColor color, void* context) {
    DebugDraw* debugDraw = static_cast<DebugDraw*>(context);

    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = (color & 0xFF) / 255.0f;
    float a = debugDraw->m_alpha;

    const int numSegments = 30;
    float angleIncrement = 2.0f * b2_pi / numSegments; // Angle between each vertex

    // Iterate through the circle's vertices and draw line segments between them
    for (int i = 0; i < numSegments; ++i) {
        // Calculate the angle of the current vertex
        float angle = i * angleIncrement;

        // Calculate the position of the vertex based on the angle and radius
        b2Vec2 p1 = center + radius * b2Vec2(cos(angle), sin(angle));

        // Calculate the position of the next vertex (loop back to the first after the last)
        float nextAngle = (i + 1) * angleIncrement;
        b2Vec2 p2 = center + radius * b2Vec2(cos(nextAngle), sin(nextAngle));

        // Draw a segment from p1 to p2
        drawSegment(p1, p2, color, context);
    }
}

void DebugDraw::drawSolidCircle(b2Transform xf, float radius, b2HexColor color, void* context) {
    DebugDraw* debugDraw = static_cast<DebugDraw*>(context);

    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = (color & 0xFF) / 255.0f;
    float a = debugDraw->m_alpha;

    // Transform the center point to world space
    b2Vec2 center = b2TransformPoint(xf, b2Vec2_zero);

    const int segments = 30;
    m_triangleVertexData.reserve((segments + 1) * 3); // +1 for center point

    // Calc + store the circle's boundary points
    b2Vec2 prevPoint = center + radius * b2Vec2(cosf(0.0f), sinf(0.0f)); // starting point on the circle
    for (int i = 1; i <= segments; ++i) {

        // Calc the angle for current vertex
        float angle = (float(i) / segments) * 2.0f * b2_pi;
        b2Vec2 currentPoint = center + radius * b2Vec2(cosf(angle), sinf(angle));

        // Add the triangles for the current segment
        m_triangleVertexData.push_back(DebugVertex(prevPoint, r, g, b, a));
        m_triangleVertexData.push_back(DebugVertex(currentPoint, r, g, b, a));
        m_triangleVertexData.push_back(DebugVertex(center, r, g, b, a));

        // Move to next point
        prevPoint = currentPoint;
    }
}

void DebugDraw::drawCapsule(b2Vec2 p1, b2Vec2 p2, float radius, b2HexColor color, void* context) {
    return;
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

void DebugDraw::drawString(b2Vec2 p, const char* s, void* context) {
    // TODO: Implement line rendering
}

void DebugDraw::drawPoint(b2Vec2 p, float size, b2HexColor color, void* context) {
    DebugDraw* debugDraw = static_cast<DebugDraw*>(context);

    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = (color & 0xFF) / 255.0f;
    float a = debugDraw->m_alpha;

    float halfSize = size * 0.5f;

    // Define the four vertices of the square (forming two triangles)
    b2Vec2 topLeft = b2Vec2(p.x - halfSize, p.y - halfSize);
    b2Vec2 topRight = b2Vec2(p.x + halfSize, p.y - halfSize);
    b2Vec2 bottomRight = b2Vec2(p.x + halfSize, p.y + halfSize);
    b2Vec2 bottomLeft = b2Vec2(p.x - halfSize, p.y + halfSize);

    // Add the first triangle (top-left, top-right, bottom-right)
    m_triangleVertexData.push_back(DebugVertex(topLeft, r, g, b, a));
    m_triangleVertexData.push_back(DebugVertex(topRight, r, g, b, a));
    m_triangleVertexData.push_back(DebugVertex(bottomRight, r, g, b, a));

    // Add the second triangle (top-left, bottom-right, bottom-left)
    m_triangleVertexData.push_back(DebugVertex(topLeft, r, g, b, a));
    m_triangleVertexData.push_back(DebugVertex(bottomRight, r, g, b, a));
    m_triangleVertexData.push_back(DebugVertex(bottomLeft, r, g, b, a));
}

void DebugDraw::drawLineBetweenPoints(b2Vec2 p1, b2Vec2 p2, b2HexColor color, void* context) {
    DebugDraw* debugDraw = static_cast<DebugDraw*>(context);

    // Convert hex color to float values (0-1 range)
    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = (color & 0xFF) / 255.0f;
    float a = 0.5f;

    // Push the line data into the vertex buffer
    m_lineVertexData.push_back(DebugVertex(p1, r, g, b, a));
    m_lineVertexData.push_back(DebugVertex(p2, r, g, b, a));
}

void DebugDraw::drawBoxAtPoint(b2Vec2 position, b2Vec2 size, b2HexColor color, void* context) {
    DebugDraw* debugDraw = static_cast<DebugDraw*>(context);

    // Convert hex color to float values (0-1 range)
    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = (color & 0xFF) / 255.0f;
    float a = 0.5f;

    // Define the four corners of the box
    b2Vec2 topLeft = position;
    b2Vec2 topRight = position + b2Vec2(size.x, 0);
    b2Vec2 bottomRight = position + size;
    b2Vec2 bottomLeft = position + b2Vec2(0, size.y);

    // Push the four segments of the box (rectangular polygon)
    drawSegment(topLeft, topRight, color, context);
    drawSegment(topRight, bottomRight, color, context);
    drawSegment(bottomRight, bottomLeft, color, context);
    drawSegment(bottomLeft, topLeft, color, context);
}

void DebugDraw::setAttrib() {
    // Setup attributes
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)offsetof(DebugVertex, pos));
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(DebugVertex), (void*)offsetof(DebugVertex, color));
}