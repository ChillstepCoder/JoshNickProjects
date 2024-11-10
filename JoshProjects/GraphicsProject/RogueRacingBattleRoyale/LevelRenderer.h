// LevelRenderer.h

#pragma once
#include <JAGEngine/SpriteBatch.h>
#include <JAGEngine/GLSLProgram.h>
#include <JAGEngine/Camera2D.h>
#include "SplineTrack.h"
#include "ObjectManager.h"
#include "RoadMeshGenerator.h"
#include <glm/glm.hpp>

class LevelRenderer {
public:
  enum class RoadViewMode {
    Spline,      // Shows spline points and control nodes
    Wireframe,   // Shows triangle mesh wireframe
    Shaded       // Shows fully shaded road
  };

  LevelRenderer();
  ~LevelRenderer();

  void init();
  void destroy();

  // Main render function
  void render(const JAGEngine::Camera2D& camera,
    SplineTrack* track,
    ObjectManager* objectManager,
    bool showSplinePoints,
    RoadViewMode viewMode);

  // Mesh update function
  void updateRoadMesh(SplineTrack* track);

  // Getters and setters for appearance
  void setGrassColor(const glm::vec3& color) { m_grassColor = color; }
  void setOffroadColor(const glm::vec3& color) { m_offroadColor = color; }
  void setGrassNoiseParams(float scale, float intensity) {
    m_grassNoiseScale = scale;
    m_grassNoiseIntensity = intensity;
  }
  void setBarrierColors(const glm::vec3& primary, const glm::vec3& secondary) {
    m_barrierPrimaryColor = primary;
    m_barrierSecondaryColor = secondary;
  }

  void setObjectPlacementPreview(bool show, PlaceableObject* obj, const glm::vec2& pos) {
    m_showObjectPreview = show;
    m_previewObject = obj;
    m_previewPosition = pos;
  }

  void setBarrierPatternScale(float scale) { m_barrierPatternScale = scale; }
  void setRoadLOD(int lod) { m_roadLOD = glm::clamp(lod, MIN_LOD, MAX_LOD); }

  void setPreviewNode(const glm::vec2& position, bool show) {
    m_previewNodePosition = position;
    m_showPreviewNode = show;
  }

  void setObjectPlacementMode(bool mode, int templateIndex) {
    m_objectPlacementMode = mode;
    m_selectedTemplateIndex = templateIndex;
  }

  void setPreviewPosition(const glm::vec2& pos) {
    m_previewPosition = pos;
  }

private:
  // Helper rendering functions
  void renderBackground(const glm::mat4& cameraMatrix);
  void renderRoad(const glm::mat4& cameraMatrix);
  void renderOffroad(const glm::mat4& cameraMatrix);
  void renderBarriers(const glm::mat4& cameraMatrix);
  void renderStartLine(const glm::mat4& cameraMatrix);
  void renderSplinePoints(const glm::mat4& cameraMatrix, SplineTrack* track);
  void renderNodes(const glm::mat4& cameraMatrix, SplineTrack* track);
  void renderObjects(const glm::mat4& cameraMatrix, ObjectManager* objectManager);
  void renderWireframe(const glm::mat4& cameraMatrix);
  void drawRoadEdges(SplineTrack* track);
  void initBackgroundQuad();

  // Shader initialization
  void initShaders();

  // Rendering components
  JAGEngine::SpriteBatch m_spriteBatch;
  JAGEngine::GLSLProgram m_textureProgram;      // For sprites
  JAGEngine::GLSLProgram m_roadShader;
  JAGEngine::GLSLProgram m_offroadShader;
  JAGEngine::GLSLProgram m_grassShader;
  JAGEngine::GLSLProgram m_barrierShader;
  JAGEngine::GLSLProgram m_startLineShader;

  // Mesh data
  RoadMeshGenerator::MeshData m_roadMesh;
  RoadMeshGenerator::OffroadMeshData m_offroadMesh;
  RoadMeshGenerator::BarrierMeshData m_barrierMesh;
  RoadMeshGenerator::StartLineMeshData m_startLineMesh;
  RoadMeshGenerator::MeshData m_backgroundQuad;

  // Appearance settings
  glm::vec3 m_grassColor = glm::vec3(0.2f, 0.5f, 0.1f);
  glm::vec3 m_offroadColor = glm::vec3(0.45f, 0.32f, 0.15f);
  float m_grassNoiseScale = 100.0f;
  float m_grassNoiseIntensity = 1.0f;
  glm::vec3 m_barrierPrimaryColor = glm::vec3(1.0f, 1.0f, 1.0f);
  glm::vec3 m_barrierSecondaryColor = glm::vec3(0.0f, 0.0f, 0.0f);
  float m_barrierPatternScale = 5.0f;

  // LOD settings
  int m_roadLOD = 10;
  static const int MIN_LOD = 4;
  static const int MAX_LOD = 50;

  // Node preview state
  glm::vec2 m_previewNodePosition = glm::vec2(0.0f);
  bool m_showPreviewNode = false;

  bool m_objectPlacementMode = false;
  int m_selectedTemplateIndex = -1;
  glm::vec2 m_previewPosition = glm::vec2(0.0f);

  bool m_showObjectPreview = false;
  PlaceableObject* m_previewObject = nullptr;
};
