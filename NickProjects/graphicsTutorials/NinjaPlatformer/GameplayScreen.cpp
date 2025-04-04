#include "GameplayScreen.h"
#include <iostream>
#include <SDL/SDL.h>
#include <Bengine/IMainGame.h>
#include <Bengine/ResourceManager.h>

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
    worldDef.gravity = b2Vec2(0.0f, -70.0f);
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
    m_texture = Bengine::ResourceManager::getTexture("Textures/dirtBlock.png");

    Bengine::ColorRGBA8 textureColor;
    textureColor.r = 255;
    textureColor.g = 255;
    textureColor.b = 255;
    textureColor.a = 255;

    // Make a box
    Box newBox;
    newBox.init(&m_world, glm::vec2(0.0f, 14.0f), glm::vec2(3.0f, 3.0f), m_texture, textureColor, false);
    m_boxes.push_back(newBox);

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

    m_player.update(m_game->inputManager);
}

void GameplayScreen::draw() {
    std::cout << "Draw\n";
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
    for (auto& b : m_boxes) {
        b.draw(m_spriteBatch);
    }

    m_player.draw(m_spriteBatch);

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
        m_game->onSDLEvent(evnt);

        if (evnt.type == SDL_KEYDOWN) { // Add debug render toggle on F1 key
            if (evnt.key.keysym.sym == SDLK_F1) {
                m_debugRenderEnabled = !m_debugRenderEnabled;
            }
        }
    }
}