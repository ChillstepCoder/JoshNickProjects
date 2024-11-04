#pragma once
#include <box2d/box2d.h>
#include <Bengine/GLSLProgram.h>
#include <glm/glm.hpp>
#include <Bengine/vertex.h>
#include <vector>

struct DebugVertex {
    DebugVertex(b2Vec2 pos, GLubyte R, GLubyte G, GLubyte B, GLubyte A) :
        pos(pos), color(R, G, B, A) {
    }
    DebugVertex(b2Vec2 pos, b2Vec2 vert, int vertCount, GLubyte R, GLubyte G, GLubyte B, GLubyte A) :
        pos(pos), vert(vert), vertCount(vertCount), color(R, G, B, A) {
    }


    b2Vec2 pos;
    b2Vec2 vert;
    int vertCount;
    Bengine::ColorRGBA8 color;
};

class DebugDraw {
public:
    DebugDraw();
    ~DebugDraw();

    void init();
    void drawWorld(b2WorldId* world, const glm::mat4& projectionMatrix);
    void setAlpha(float alpha) { m_alpha = alpha; }

private:
    // Updated callback signatures to match Box2D 3.0
    static void drawSegment(b2Vec2 p1, b2Vec2 p2, b2HexColor color, void* context);
    static void drawPolygon(const b2Vec2* vertices, int vertexCount, b2HexColor color, void* context);
    static void drawCircle(b2Vec2 center, float radius, b2HexColor color, void* context);
    static void drawSolidCircle(b2Transform xf, float radius, b2HexColor color, void* context);
    static void drawCapsule(b2Vec2 p1, b2Vec2 p2, float radius, b2HexColor color, void* context);
    static void drawSolidPolygon(b2Transform xf, const b2Vec2* vertices, int vertexCount, float radius, b2HexColor color, void* context);
    static void drawTransform(b2Transform xf, void* context);
    static void drawPoint(b2Vec2 p, float size, b2HexColor color, void* context);
    static void setAttrib();

    float m_alpha = 0.1f;
    Bengine::GLSLProgram m_program;
    GLuint m_vboId = 0;
    GLuint m_vaoId = 0;
    inline static std::vector<DebugVertex> m_lineVertexData;
    inline static std::vector<DebugVertex> m_lineLoopVertexData;
    inline static std::vector<DebugVertex> m_triangleFanVertexData;
};


