// LevelRenderer.cpp

#include "LevelRenderer.h"

LevelRenderer::LevelRenderer() {
}

LevelRenderer::~LevelRenderer() {
  destroy();
}

void LevelRenderer::init() {
  m_spriteBatch.init();
  initShaders();
  initBackgroundQuad();
}

void LevelRenderer::destroy() {
  m_roadMesh.cleanup();
  m_offroadMesh.leftSide.cleanup();
  m_offroadMesh.rightSide.cleanup();
  m_barrierMesh.leftSide.cleanup();
  m_barrierMesh.rightSide.cleanup();
  m_startLineMesh.cleanup();
  m_backgroundQuad.cleanup();
}

void LevelRenderer::initShaders() {
  // Initialize regular texture shader for sprites
  m_textureProgram.compileShaders("Shaders/textureShading.vert", "Shaders/textureShading.frag");
  m_textureProgram.addAttribute("vertexPosition");
  m_textureProgram.addAttribute("vertexColor");
  m_textureProgram.addAttribute("vertexUV");
  m_textureProgram.linkShaders();

  // Initialize road shader
  m_roadShader.compileShaders("Shaders/road.vert", "Shaders/road.frag");
  m_roadShader.addAttribute("vertexPosition");
  m_roadShader.addAttribute("vertexUV");
  m_roadShader.addAttribute("distanceAlong");
  m_roadShader.linkShaders();

  // Initialize offroad shader
  m_offroadShader.compileShaders("Shaders/offroad.vert", "Shaders/offroad.frag");
  m_offroadShader.addAttribute("vertexPosition");
  m_offroadShader.addAttribute("vertexUV");
  m_offroadShader.addAttribute("distanceAlong");
  m_offroadShader.linkShaders();

  // Initialize grass shader
  m_grassShader.compileShaders("Shaders/grass.vert", "Shaders/grass.frag");
  m_grassShader.addAttribute("vertexPosition");
  m_grassShader.addAttribute("vertexUV");
  m_grassShader.linkShaders();

  // Initialize barrier shader
  m_barrierShader.compileShaders("Shaders/barrier.vert", "Shaders/barrier.frag");
  m_barrierShader.addAttribute("vertexPosition");
  m_barrierShader.addAttribute("vertexUV");
  m_barrierShader.addAttribute("distanceAlong");
  m_barrierShader.addAttribute("depth");
  m_barrierShader.linkShaders();

  // Initialize start line shader
  m_startLineShader.compileShaders("Shaders/startLine.vert", "Shaders/startLine.frag");
  m_startLineShader.addAttribute("vertexPosition");
  m_startLineShader.addAttribute("vertexUV");
  m_startLineShader.addAttribute("distanceAlong");
  m_startLineShader.addAttribute("depth");
  m_startLineShader.linkShaders();
}

void LevelRenderer::render(const JAGEngine::Camera2D& camera,
  SplineTrack* track,
  ObjectManager* objectManager,
  bool showSplinePoints,
  RoadViewMode viewMode) {
  glm::mat4 cameraMatrix = camera.getCameraMatrix();

  if (viewMode == RoadViewMode::Shaded) {
    renderBackground(cameraMatrix);
    renderOffroad(cameraMatrix);
    renderRoad(cameraMatrix);
    renderBarriers(cameraMatrix);
    renderStartLine(cameraMatrix);
  }

  // Show spline points only in spline view mode
  if (viewMode == RoadViewMode::Spline) {
    renderSplinePoints(cameraMatrix, track);
  }

  // Show nodes whenever showSplinePoints is true, regardless of view mode
  if (showSplinePoints) {
    renderNodes(cameraMatrix, track);
  }

  // Always render objects regardless of view mode
  renderObjects(cameraMatrix, objectManager);

  if (viewMode == RoadViewMode::Wireframe) {
    renderWireframe(cameraMatrix);
  }
}

void LevelRenderer::renderBackground(const glm::mat4& cameraMatrix) {
  m_grassShader.use();
  glUniformMatrix4fv(m_grassShader.getUniformLocation("P"), 1, GL_FALSE, &cameraMatrix[0][0]);
  glUniform3fv(m_grassShader.getUniformLocation("grassColor"), 1, &m_grassColor[0]);
  glUniform1f(m_grassShader.getUniformLocation("noiseScale"), m_grassNoiseScale);
  glUniform1f(m_grassShader.getUniformLocation("noiseIntensity"), m_grassNoiseIntensity);

  glBindVertexArray(m_backgroundQuad.vao);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
  glBindVertexArray(0);
  m_grassShader.unuse();
}

void LevelRenderer::renderRoad(const glm::mat4& cameraMatrix) {
  m_roadShader.use();
  glUniformMatrix4fv(m_roadShader.getUniformLocation("P"), 1, GL_FALSE, &cameraMatrix[0][0]);
  if (m_roadMesh.vao != 0 && !m_roadMesh.indices.empty()) {
    glBindVertexArray(m_roadMesh.vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_roadMesh.indices.size()), GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
  }
  m_roadShader.unuse();
}

void LevelRenderer::renderOffroad(const glm::mat4& cameraMatrix) {
  m_offroadShader.use();
  glUniformMatrix4fv(m_offroadShader.getUniformLocation("P"), 1, GL_FALSE, &cameraMatrix[0][0]);
  glUniform3fv(m_offroadShader.getUniformLocation("offroadColor"), 1, &m_offroadColor[0]);

  // Draw left and right offroad
  for (auto* mesh : { &m_offroadMesh.leftSide, &m_offroadMesh.rightSide }) {
    if (mesh->vao != 0) {
      glBindVertexArray(mesh->vao);
      glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh->indices.size()), GL_UNSIGNED_INT, nullptr);
    }
  }
  glBindVertexArray(0);
  m_offroadShader.unuse();
}

void LevelRenderer::renderBarriers(const glm::mat4& cameraMatrix) {
  m_barrierShader.use();
  glUniformMatrix4fv(m_barrierShader.getUniformLocation("P"), 1, GL_FALSE, &cameraMatrix[0][0]);
  glUniform3fv(m_barrierShader.getUniformLocation("primaryColor"), 1, &m_barrierPrimaryColor[0]);
  glUniform3fv(m_barrierShader.getUniformLocation("secondaryColor"), 1, &m_barrierSecondaryColor[0]);
  glUniform1f(m_barrierShader.getUniformLocation("patternScale"), m_barrierPatternScale);

  for (auto* mesh : { &m_barrierMesh.leftSide, &m_barrierMesh.rightSide }) {
    if (mesh->vao != 0) {
      glBindVertexArray(mesh->vao);
      glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh->indices.size()), GL_UNSIGNED_INT, nullptr);
    }
  }
  glBindVertexArray(0);
  m_barrierShader.unuse();
}

void LevelRenderer::renderStartLine(const glm::mat4& cameraMatrix) {
  if (m_startLineMesh.vao != 0) {
    m_startLineShader.use();
    glUniformMatrix4fv(m_startLineShader.getUniformLocation("P"), 1, GL_FALSE, &cameraMatrix[0][0]);

    glBindVertexArray(m_startLineMesh.vao);
    glDrawElements(GL_TRIANGLES,
      static_cast<GLsizei>(m_startLineMesh.indices.size()),
      GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);

    m_startLineShader.unuse();
  }
}

void LevelRenderer::renderSplinePoints(const glm::mat4& cameraMatrix, SplineTrack* track) {
  if (!track) return;

  m_textureProgram.use();
  glUniformMatrix4fv(m_textureProgram.getUniformLocation("P"), 1, GL_FALSE, &cameraMatrix[0][0]);
  m_spriteBatch.begin();

  drawRoadEdges(track);

  m_spriteBatch.end();
  m_spriteBatch.renderBatch();
  m_textureProgram.unuse();
}

void LevelRenderer::renderNodes(const glm::mat4& cameraMatrix, SplineTrack* track) {
  if (!track) return;

  m_textureProgram.use();
  glUniformMatrix4fv(m_textureProgram.getUniformLocation("P"), 1, GL_FALSE, &cameraMatrix[0][0]);
  m_spriteBatch.begin();

  // Draw nodes
  for (const auto& node : track->getNodes()) {
    glm::vec4 nodeRect(
      node.getPosition().x - 10.0f,
      node.getPosition().y - 10.0f,
      20.0f,
      20.0f
    );

    JAGEngine::ColorRGBA8 nodeColor;
    if (node.isSelected()) {
      nodeColor = JAGEngine::ColorRGBA8(255, 0, 0, 255);
    }
    else if (node.isHovered()) {
      nodeColor = JAGEngine::ColorRGBA8(0, 150, 255, 255);
    }
    else {
      nodeColor = JAGEngine::ColorRGBA8(0, 0, 0, 255);
    }

    m_spriteBatch.draw(nodeRect, glm::vec4(0, 0, 1, 1), 0, 0.0f, nodeColor);
  }

  // Draw preview node
  if (m_showPreviewNode) {
    glm::vec4 previewRect(
      m_previewNodePosition.x - 10.0f,
      m_previewNodePosition.y - 10.0f,
      20.0f,
      20.0f
    );
    m_spriteBatch.draw(previewRect, glm::vec4(0, 0, 1, 1), 0, 0.0f, JAGEngine::ColorRGBA8(255, 255, 0, 200));
  }

  m_spriteBatch.end();
  m_spriteBatch.renderBatch();
  m_textureProgram.unuse();
}

void LevelRenderer::renderObjects(const glm::mat4& cameraMatrix, ObjectManager* objectManager) {
  if (!objectManager) return;

  m_textureProgram.use();
  glUniformMatrix4fv(m_textureProgram.getUniformLocation("P"), 1, GL_FALSE, &cameraMatrix[0][0]);
  m_spriteBatch.begin();

  // Draw placed objects
  for (const auto& obj : objectManager->getPlacedObjects()) {
    const auto& texture = obj->getTexture();
    float baseWidth = texture.width * obj->getScale().x;
    float baseHeight = texture.height * obj->getScale().y;

    glm::vec4 destRect(
      obj->getPosition().x - baseWidth * 0.5f,
      obj->getPosition().y - baseHeight * 0.5f,
      baseWidth,
      baseHeight
    );

    JAGEngine::ColorRGBA8 color = obj->isSelected() ?
      JAGEngine::ColorRGBA8(255, 255, 255, 255) :
      JAGEngine::ColorRGBA8(200, 200, 200, 255);

    m_spriteBatch.draw(destRect,
      glm::vec4(0, 0, 1, 1),
      texture.id,
      obj->getRotation(),
      color);
  }

  // Add object placement preview
  if (m_objectPlacementMode && m_selectedTemplateIndex >= 0) {
    const auto& templates = objectManager->getObjectTemplates();
    if (m_selectedTemplateIndex < templates.size()) {
      const auto& template_obj = templates[m_selectedTemplateIndex];
      const auto& texture = template_obj->getTexture();
      float baseWidth = texture.width * template_obj->getScale().x;
      float baseHeight = texture.height * template_obj->getScale().y;

      glm::vec4 destRect(
        m_previewPosition.x - baseWidth * 0.5f,
        m_previewPosition.y - baseHeight * 0.5f,
        baseWidth,
        baseHeight
      );

      bool validPlacement = objectManager->isValidPlacement(template_obj.get(), m_previewPosition);
      JAGEngine::ColorRGBA8 previewColor = validPlacement ?
        JAGEngine::ColorRGBA8(0, 255, 0, 128) :  // Green with transparency
        JAGEngine::ColorRGBA8(255, 0, 0, 128);   // Red with transparency

      m_spriteBatch.draw(destRect,
        glm::vec4(0, 0, 1, 1),
        texture.id,
        template_obj->getRotation(),
        previewColor);
    }
  }

  m_spriteBatch.end();
  m_spriteBatch.renderBatch();
  m_textureProgram.unuse();
}

void LevelRenderer::renderWireframe(const glm::mat4& cameraMatrix) {
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  // Draw road wireframe
  renderRoad(cameraMatrix);
  renderOffroad(cameraMatrix);
  renderBarriers(cameraMatrix);

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void LevelRenderer::drawRoadEdges(SplineTrack* track) {
  if (!track) return;

  const auto& nodes = track->getNodes();
  if (nodes.empty()) return;

  auto splinePoints = track->getSplinePoints(200);
  std::vector<glm::vec2> leftEdgePoints, rightEdgePoints;
  std::vector<glm::vec2> leftOffroadPoints, rightOffroadPoints;

  // Calculate points
  for (size_t i = 0; i < splinePoints.size() - 1; ++i) {
    const auto& current = splinePoints[i];
    const auto& next = splinePoints[i + 1];

    glm::vec2 direction = next.position - current.position;
    glm::vec2 perpendicular = glm::normalize(glm::vec2(-direction.y, direction.x));

    leftEdgePoints.push_back(current.position + perpendicular * current.roadWidth);
    rightEdgePoints.push_back(current.position - perpendicular * current.roadWidth);
    leftOffroadPoints.push_back(current.position + perpendicular * (current.roadWidth + current.offroadWidth.x));
    rightOffroadPoints.push_back(current.position - perpendicular * (current.roadWidth + current.offroadWidth.y));
  }

  // Draw points
  JAGEngine::ColorRGBA8 roadEdgeColor(255, 255, 255, 255);
  JAGEngine::ColorRGBA8 offroadEdgeColor(200, 150, 50, 255);

  for (size_t i = 0; i < leftEdgePoints.size(); i += 2) {
    // Draw road edges
    glm::vec4 leftRect(leftEdgePoints[i].x - 1.0f, leftEdgePoints[i].y - 1.0f, 2.0f, 2.0f);
    glm::vec4 rightRect(rightEdgePoints[i].x - 1.0f, rightEdgePoints[i].y - 1.0f, 2.0f, 2.0f);
    m_spriteBatch.draw(leftRect, glm::vec4(0, 0, 1, 1), 0, 0.0f, roadEdgeColor);
    m_spriteBatch.draw(rightRect, glm::vec4(0, 0, 1, 1), 0, 0.0f, roadEdgeColor);

    // Draw offroad edges
    glm::vec4 leftOffroadRect(leftOffroadPoints[i].x - 1.0f, leftOffroadPoints[i].y - 1.0f, 2.0f, 2.0f);
    glm::vec4 rightOffroadRect(rightOffroadPoints[i].x - 1.0f, rightOffroadPoints[i].y - 1.0f, 2.0f, 2.0f);
    m_spriteBatch.draw(leftOffroadRect, glm::vec4(0, 0, 1, 1), 0, 0.0f, offroadEdgeColor);
    m_spriteBatch.draw(rightOffroadRect, glm::vec4(0, 0, 1, 1), 0, 0.0f, offroadEdgeColor);
  }
}

void LevelRenderer::initBackgroundQuad() {
  RoadVertex v1, v2, v3, v4;
  // Use even larger size to ensure coverage
  float size = 20000.0f;  // Increased size
  v1.position = glm::vec2(-size, -size);
  v2.position = glm::vec2(size, -size);
  v3.position = glm::vec2(size, size);
  v4.position = glm::vec2(-size, size);

  // UV coordinates for tiling
  v1.uv = glm::vec2(0.0f, 0.0f);
  v2.uv = glm::vec2(1.0f, 0.0f);
  v3.uv = glm::vec2(1.0f, 1.0f);
  v4.uv = glm::vec2(0.0f, 1.0f);

  m_backgroundQuad.vertices = { v1, v2, v3, v4 };
  m_backgroundQuad.indices = { 0, 1, 2, 2, 3, 0 };

  RoadMeshGenerator::createBuffers(m_backgroundQuad);
}

void LevelRenderer::updateRoadMesh(SplineTrack* track) {
  if (!track) return;

  m_roadMesh.cleanup();
  m_offroadMesh.leftSide.cleanup();
  m_offroadMesh.rightSide.cleanup();
  m_barrierMesh.leftSide.cleanup();
  m_barrierMesh.rightSide.cleanup();
  m_startLineMesh.cleanup();

  m_roadMesh = RoadMeshGenerator::generateRoadMesh(*track, m_roadLOD);
  m_offroadMesh = RoadMeshGenerator::generateOffroadMesh(*track, m_roadLOD);
  m_barrierMesh = RoadMeshGenerator::generateBarrierMesh(*track, m_roadLOD);
  m_startLineMesh = RoadMeshGenerator::generateStartLineMesh(*track);
}