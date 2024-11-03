// RoadMeshGenerator.h

#pragma once
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "SplineTrack.h"
#include <GL/glew.h>
#include <GL/gl.h>
#include <iostream>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

struct RoadVertex {
  glm::vec2 position;  // Vertex position
  glm::vec2 uv;        // Texture coordinates
  float distanceAlong; // Distance along road for texturing
  float depth;         // Depth component for z-ordering
};

class RoadMeshGenerator {
public:
  struct MeshData {
    std::vector<RoadVertex> vertices;
    std::vector<GLuint> indices;
    GLuint vao;
    GLuint vbo;
    GLuint ibo;

    MeshData() : vao(0), vbo(0), ibo(0) {}

    // Cleanup method
    void cleanup() {
      if (vao != 0) {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
      }
      if (vbo != 0) {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
      }
      if (ibo != 0) {
        glDeleteBuffers(1, &ibo);
        ibo = 0;
      }
    }

    // Destructor
    ~MeshData() {
      cleanup();
    }

    // Copy constructor
    MeshData(const MeshData& other) {
      vertices = other.vertices;
      indices = other.indices;
      vao = 0;
      vbo = 0;
      ibo = 0;
      if (!vertices.empty() && !indices.empty()) {
        createBuffers(*this);
      }
    }

    // Assignment operator
    MeshData& operator=(const MeshData& other) {
      if (this != &other) {
        cleanup();
        vertices = other.vertices;
        indices = other.indices;
        vao = 0;
        vbo = 0;
        ibo = 0;
        if (!vertices.empty() && !indices.empty()) {
          createBuffers(*this);
        }
      }
      return *this;
    }
  };

  static void createBuffers(MeshData& mesh) {
    // Cleanup any existing buffers
    mesh.cleanup();

    if (mesh.vertices.empty() || mesh.indices.empty()) {
      std::cout << "Cannot create buffers: No vertex or index data\n";
      return;
    }

    // Generate and bind VAO
    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    // Create and populate vertex buffer
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER,
      mesh.vertices.size() * sizeof(RoadVertex),
      mesh.vertices.data(),
      GL_STATIC_DRAW);

    // Create and populate index buffer
    glGenBuffers(1, &mesh.ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
      mesh.indices.size() * sizeof(GLuint),
      mesh.indices.data(),
      GL_STATIC_DRAW);

    // Position attribute (vec2)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(RoadVertex),
      (void*)offsetof(RoadVertex, position));

    // UV attribute (vec2)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(RoadVertex),
      (void*)offsetof(RoadVertex, uv));

    // Distance attribute (float)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(RoadVertex),
      (void*)offsetof(RoadVertex, distanceAlong));

    // Depth attribute (float)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(RoadVertex),
      (void*)offsetof(RoadVertex, depth));

    glBindVertexArray(0);

    // Debug output
    std::cout << "Created buffers - VAO: " << mesh.vao
      << ", VBO: " << mesh.vbo
      << ", IBO: " << mesh.ibo
      << ", Vertices: " << mesh.vertices.size()
      << ", Indices: " << mesh.indices.size() << "\n";

    // Verify buffer creation
    if (mesh.vao == 0 || mesh.vbo == 0 || mesh.ibo == 0) {
      std::cout << "Failed to create one or more buffers!\n";
    }
  }

  static glm::vec2 calculateSmoothPerpendicular(const glm::vec2& prev, const glm::vec2& curr, const glm::vec2& next) {
    // Calculate the incoming and outgoing directions
    glm::vec2 inDir = glm::normalize(curr - prev);
    glm::vec2 outDir = glm::normalize(next - curr);

    // Calculate the angle between the directions
    float dot = glm::dot(inDir, outDir);
    dot = glm::clamp(dot, -1.0f, 1.0f);
    float angle = std::acos(dot);

    // Calculate weighted average direction based on the angle
    glm::vec2 avgDir;
    if (angle > 0.01f) { // If there's a significant turn
      // Use a weighted average that favors the sharper turning direction
      float inWeight = 1.0f + angle;
      float outWeight = 1.0f + angle;
      avgDir = glm::normalize((inDir * inWeight + outDir * outWeight) * 0.5f);
    }
    else {
      // For nearly straight sections, use simple average
      avgDir = glm::normalize(inDir + outDir);
    }

    // Calculate perpendicular vector
    glm::vec2 perp(-avgDir.y, avgDir.x);

    // Scale the perpendicular vector based on the angle to prevent overlap
    float scaleFactor = 1.0f / (std::cos(angle * 0.5f) + 0.01f);
    scaleFactor = glm::clamp(scaleFactor, 1.0f, 1.5f);

    return perp * scaleFactor;
  }

  // Generates mesh data for the road surface
  static MeshData generateRoadMesh(const SplineTrack& track, int baseLOD = 10) {
    MeshData mesh;
    auto splinePoints = track.getSplinePoints(baseLOD);

    if (splinePoints.size() < 2) {
      std::cout << "Not enough spline points to generate mesh\n";
      return mesh;
    }

    std::cout << "Generating road mesh with " << splinePoints.size() << " points\n";

    float totalDistance = 0.0f;
    std::vector<RoadVertex> vertices;
    std::vector<GLuint> indices;

    // Reserve space
    vertices.reserve((splinePoints.size() + 1) * 3);
    indices.reserve(splinePoints.size() * 6);

    // Generate Vertices
    for (size_t i = 0; i < splinePoints.size(); ++i) {
      const auto& current = splinePoints[i];
      const auto& next = splinePoints[(i + 1) % splinePoints.size()];

      // Calculate road direction and perpendicular
      glm::vec2 direction = glm::normalize(next.position - current.position);
      glm::vec2 perp(-direction.y, direction.x);

      // Create three vertices (left edge, center, right edge)
      RoadVertex leftEdge, center, rightEdge;

      // Set positions
      leftEdge.position = current.position + perp * current.roadWidth;
      center.position = current.position;
      rightEdge.position = current.position - perp * current.roadWidth;

      // Set UV coordinates
      leftEdge.uv = glm::vec2(0.0f, totalDistance * 0.01f);
      center.uv = glm::vec2(0.5f, totalDistance * 0.01f);
      rightEdge.uv = glm::vec2(1.0f, totalDistance * 0.01f);

      // Set distance along road
      leftEdge.distanceAlong = totalDistance;
      center.distanceAlong = totalDistance;
      rightEdge.distanceAlong = totalDistance;

      // Set depth for z-ordering: center line always on top, edges deeper
      float baseDepth = 0.01f * static_cast<float>(i) / splinePoints.size(); // Now positive
      center.depth = baseDepth + 0.01f;    // Closest to camera
      leftEdge.depth = baseDepth;          // Further back
      rightEdge.depth = baseDepth;         // Further back

      // Add vertices
      vertices.push_back(leftEdge);
      vertices.push_back(center);
      vertices.push_back(rightEdge);

      // Create triangles (except for last point)
      if (i < splinePoints.size() - 1) {
        GLuint baseIndex = static_cast<GLuint>(i * 3);

        // Left triangles
        indices.push_back(baseIndex);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 3);

        indices.push_back(baseIndex + 3);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 4);

        // Right triangles
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 4);

        indices.push_back(baseIndex + 4);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 5);
      }

      totalDistance += glm::length(next.position - current.position);
    }

    // Connect last segment to first to close the loop
    if (splinePoints.size() > 2) {
      GLuint lastIndex = static_cast<GLuint>((splinePoints.size() - 1) * 3);
      GLuint firstIndex = 0;

      // Left side
      indices.push_back(lastIndex);
      indices.push_back(lastIndex + 1);
      indices.push_back(firstIndex);

      indices.push_back(firstIndex);
      indices.push_back(lastIndex + 1);
      indices.push_back(firstIndex + 1);

      // Right side
      indices.push_back(lastIndex + 1);
      indices.push_back(lastIndex + 2);
      indices.push_back(firstIndex + 1);

      indices.push_back(firstIndex + 1);
      indices.push_back(lastIndex + 2);
      indices.push_back(firstIndex + 2);
    }

    mesh.vertices = std::move(vertices);
    mesh.indices = std::move(indices);

    createBuffers(mesh);
    return mesh;
  }
};
