// RoadMeshGenerator.h

#pragma once
#include <vector>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include <GL/gl.h>
#include "SplineTrack.h"

class SplineTrack;

class RoadMeshGenerator {
public:
  struct RoadVertex {
    glm::vec2 position;     // Vertex position
    glm::vec2 uv;           // Texture coordinates
    float distanceAlong;    // Distance along road for texturing
    float depth;            // Depth component for z-ordering
  };

  // Base mesh data structure
  struct MeshData {
    std::vector<RoadVertex> vertices;
    std::vector<GLuint> indices;
    GLuint vao;
    GLuint vbo;
    GLuint ibo;

    MeshData();
    ~MeshData();
    MeshData(const MeshData& other);
    MeshData& operator=(const MeshData& other);
    void cleanup();
  };

  // Start line mesh data structure
  struct StartLineMeshData {
    std::vector<RoadVertex> vertices;
    std::vector<GLuint> indices;
    GLuint vao;
    GLuint vbo;
    GLuint ibo;

    StartLineMeshData();
    ~StartLineMeshData();
    StartLineMeshData(const StartLineMeshData& other);
    StartLineMeshData& operator=(const StartLineMeshData& other);
    void cleanup();
  };

  // Offroad mesh data structure
  struct OffroadMeshData {
    MeshData leftSide;
    MeshData rightSide;
  };

  // Barrier mesh data structure
  struct BarrierMeshData {
    MeshData leftSide;
    MeshData rightSide;

    BarrierMeshData() = default;
    BarrierMeshData(const BarrierMeshData& other);
    BarrierMeshData& operator=(const BarrierMeshData& other);
  };

  // Main mesh generation functions
  static MeshData generateRoadMesh(const SplineTrack& track, int baseLOD = 10);
  static OffroadMeshData generateOffroadMesh(const SplineTrack& track, int baseLOD = 10);
  static BarrierMeshData generateBarrierMesh(const SplineTrack& track, int baseLOD = 10);
  static StartLineMeshData generateStartLineMesh(const SplineTrack& track);

  // Buffer creation helpers
  static void createBuffers(MeshData& mesh);
  static void createStartLineBuffers(StartLineMeshData& mesh);

private:
  static glm::vec2 calculateSmoothPerpendicular(
    const glm::vec2& prev,
    const glm::vec2& curr,
    const glm::vec2& next
  );
};
