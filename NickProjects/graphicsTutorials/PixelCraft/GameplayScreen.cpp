#include "GameplayScreen.h"
#include <iostream>
#include <SDL/SDL.h>
#include <Bengine/IMainGame.h>
#include <Bengine/ResourceManager.h>
#include "Bengine/ImGuiManager.h"

GameplayScreen::GameplayScreen(Bengine::Window* window) : m_window(window) {

}
GameplayScreen::~GameplayScreen() {
    b2DestroyWorld(m_world);
}

int GameplayScreen::getNextScreenIndex() const {
    return SCREEN_INDEX_NO_SCREEN;
}

int GameplayScreen::getPreviousScreenIndex() const {
    return SCREEN_INDEX_NO_SCREEN;
}

void GameplayScreen::build() {

}

void GameplayScreen::destroy() {

}

void GameplayScreen::onEntry() {
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = b2Vec2(0.0f, m_gravity);
    m_world = b2CreateWorld(&worldDef);


    b2BodyDef groundBodyDef = b2DefaultBodyDef();
    groundBodyDef.position = b2Vec2(0.0f, -60.0f);
    m_ground = b2CreateBody(m_world, &groundBodyDef);

    b2Polygon const groundBox = b2MakeBox(50.0f, 10.0f);
    b2ShapeDef groundShapeDef = b2DefaultShapeDef();
    groundShapeDef.density = 1.0f;
    groundShapeDef.friction = 0.2f;
    // Enable contact events for the ground shape
    groundShapeDef.enableContactEvents = true;
    b2ShapeId groundShapeId = b2CreatePolygonShape(m_ground, &groundShapeDef, &groundBox);

    // Pass the ground shape ID to the player
    m_player.setGroundShapeId(groundShapeId);

    // Load the texture
    m_texture = Bengine::ResourceManager::getTexture("Textures/connectedGrassBlock.png");

    Bengine::ColorRGBA8 textureColor;
    textureColor.r = 255;
    textureColor.g = 255;
    textureColor.b = 255;
    textureColor.a = 255;

    const int NUM_BLOCKS = 80; // Number of blocks
    const float BLOCK_WIDTH = 2.5f;
    const float BLOCK_HEIGHT = 2.5f;
    const float START_X = -100.0f; // Starting position
    const float START_Y = 14.0f; // Vertical position for blocks
    const float SPACING = 0.00f; // Space between blocks

    // Create blocks
    for (int i = 0; i < NUM_BLOCKS; ++i) {
        Block newBox;
        glm::vec2 position(START_X + i * (BLOCK_WIDTH + SPACING), START_Y); // Calculate position
        newBox.init(&m_world, position, glm::vec2(BLOCK_WIDTH, BLOCK_HEIGHT), m_texture, textureColor, false);
        m_blocks.push_back(newBox);
    }
    m_texture = Bengine::ResourceManager::getTexture("Textures/connectedDirtBlock.png");
    for (int i = 0; i < NUM_BLOCKS - 2; ++i) {
        Block newBox;
        glm::vec2 position(START_X + 2.5f + i * (BLOCK_WIDTH + SPACING), START_Y - 2.5f); // Calculate position
        newBox.init(&m_world, position, glm::vec2(BLOCK_WIDTH, BLOCK_HEIGHT), m_texture, textureColor, false);
        m_blocks.push_back(newBox);
    }
    m_texture = Bengine::ResourceManager::getTexture("Textures/connectedDirtBlockTR.png");
    Block newBox;
    glm::vec2 position(START_X, START_Y - 2.5f); // Calculate position
    newBox.init(&m_world, position, glm::vec2(BLOCK_WIDTH, BLOCK_HEIGHT), m_texture, textureColor, false);
    m_blocks.push_back(newBox);

    Block newBox2;
    glm::vec2 position2(START_X + 2.5f, START_Y - 5.0f); // Calculate position
    newBox2.init(&m_world, position2, glm::vec2(BLOCK_WIDTH, BLOCK_HEIGHT), m_texture, textureColor, false);
    m_blocks.push_back(newBox2);

    m_texture = Bengine::ResourceManager::getTexture("Textures/connectedDirtBlockTLR.png");
    for (int i = 0; i < NUM_BLOCKS - 4; ++i) {
        Block newBox;
        glm::vec2 position(START_X + 5.0f + i * (BLOCK_WIDTH + SPACING), START_Y - 5.0f); // Calculate position
        newBox.init(&m_world, position, glm::vec2(BLOCK_WIDTH, BLOCK_HEIGHT), m_texture, textureColor, false);
        m_blocks.push_back(newBox);
    }




    // Init Imgui
    Bengine::ImGuiManager::init(m_window);

    // Initialize spritebatch
    m_debugDraw.init();
    m_spriteBatch.init();

    // Shader.init
    // Compile our color shader
    m_textureProgram.compileShaders("Shaders/textureShadingVert.txt", "Shaders/textureShadingFrag.txt");
    m_textureProgram.addAttribute("vertexPosition");
    m_textureProgram.addAttribute("vertexColor");
    m_textureProgram.addAttribute("vertexUV");
    m_textureProgram.linkShaders();

    // Init camera
    m_camera.init(m_window->getScreenWidth(), m_window->getScreenHeight());
    m_camera.setScale(8.0f);

    // Init player
    m_player.init(&m_world, glm::vec2(0.0f, 30.0f), glm::vec2(3.5f, 8.0f), textureColor);
}

void GameplayScreen::onExit() {

}

void GameplayScreen::update() {
    m_camera.update();
    checkInput();

    //Update the physics simulation
    float timeStep = 1.0f / 60.0f;
    int subStepCount = 4;
    b2World_Step(m_world, timeStep, subStepCount);

    m_player.update(m_game->inputManager, m_blocks);

    const glm::vec2 playerPos = glm::vec2(b2Body_GetPosition(m_player.getID()).x, b2Body_GetPosition(m_player.getID()).y);
    m_camera.setPosition(playerPos); // Set camera position to player's position
}

void GameplayScreen::draw() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.4f, 0.4f, 0.8f, 1.0f);

    m_textureProgram.use();

    // Upload texture uniform
    GLint textureUniform = m_textureProgram.getUniformLocation("mySampler");
    glUniform1i(textureUniform, 0);
    glActiveTexture(GL_TEXTURE0);

    // Camera matrix
    glm::mat4 projectionMatrix = m_camera.getCameraMatrix();
    GLint pUniform = m_textureProgram.getUniformLocation("P");
    glUniformMatrix4fv(pUniform, 1, GL_FALSE, &projectionMatrix[0][0]);

    m_spriteBatch.begin();

    // Draw all the boxes
    for (auto& b : m_blocks) {
        b.draw(m_spriteBatch);
    }

    m_player.draw(m_spriteBatch);

    Bengine::ImGuiManager::newFrame();

    drawImgui();

    Bengine::ImGuiManager::renderFrame();

    m_spriteBatch.end();
    m_spriteBatch.renderBatch();

    m_textureProgram.unuse();

    if (m_debugRenderEnabled) {
        // Enable blending for transparency
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Set debug drawing alpha
        m_debugDraw.setAlpha(m_debugAlpha);

        // Draw debug info
        m_debugDraw.drawWorld(&m_world, m_camera.getCameraMatrix());

    }

}

void GameplayScreen::checkInput() {
    SDL_Event evnt;
    while (SDL_PollEvent(&evnt)) {
        Bengine::ImGuiManager::processEvent(evnt);
        m_game->onSDLEvent(evnt);
    }
}

void GameplayScreen::drawImgui() {
    ImGui::Begin("Settings");

    static bool check = false;
    if (ImGui::Checkbox("Debug Renderer", &check)) {
        m_debugRenderEnabled = !m_debugRenderEnabled;
    }

    if (ImGui::InputFloat("Gravity", &m_gravity, 1.0f, 1.0f, "%.3f")) {
        updateGravity();
    }

    float jumpForce = m_player.getJumpForce(); // Get the current jump force
    if (ImGui::InputFloat("Jump Force", &jumpForce, 10.0f, 100.0f, "%.1f")) {
        m_player.setJumpForce(jumpForce); // Update the player's jump force
    }

    ImGui::End();
}

void GameplayScreen::updateGravity() {
    b2Vec2 newGravity(0.0f, m_gravity);
    b2World_SetGravity(m_world, newGravity);
}