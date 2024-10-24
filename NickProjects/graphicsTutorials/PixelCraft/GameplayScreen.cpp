#include "GameplayScreen.h"
#include <iostream>
#include <SDL/SDL.h>
#include <Bengine/IMainGame.h>
#include <Bengine/ResourceManager.h>

GameplayScreen::GameplayScreen(Bengine::Window* window) : m_window(window) {

}
GameplayScreen::~GameplayScreen() {

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
    worldDef.gravity = (b2Vec2)(0.0f, -9.81f);
    b2WorldId worldId = b2CreateWorld(&worldDef);

    // Make the ground
    b2BodyDef groundBodyDef = b2DefaultBodyDef();
    groundBodyDef.position = (b2Vec2)(0.0f, -20.0f);
    b2BodyId groundId = b2CreateBody(worldId, &groundBodyDef);

    // Make the ground fixture
    b2Polygon groundBox = b2MakeBox(50.0f, 10.0f);
    b2ShapeDef groundShapeDef = b2DefaultShapeDef();
    b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);

    Box newBox;
    newBox.init(&worldId, glm::vec2(0.0f, 14.0f), glm::vec2(2.0f, 2.0f));
    m_boxes.push_back(newBox);

    // Initialize spritebatch
    m_spriteBatch.init();

    // Shader.init
    // Compile our color shader
    m_textureProgram.compileShaders("Shaders/textureShadingVert.txt", "Shaders/textureShadingFrag.txt");
    m_textureProgram.addAttribute("vertexPosition");
    m_textureProgram.addAttribute("vertexColor");
    m_textureProgram.addAttribute("vertexUV");
    m_textureProgram.linkShaders();

    // Load the texture
    m_texture = Bengine::ResourceManager::getTexture("Textures/dirtBlock.png");

    // Init camera
    m_camera.init(m_window->getScreenWidth(), m_window->getScreenHeight());
    m_camera.setScale(32.0f);
}

void GameplayScreen::onExit() {

}

void GameplayScreen::update(b2WorldId* world) {
    m_camera.update();
    checkInput();

    //Update the physics simulation
    float timeStep = 1.0f / 60.0f;
    int subStepCount = 4;
    for (int i = 0; i < 90; ++i)
    {
        b2World_Step(*world, timeStep, subStepCount);
        b2Vec2 position = b2Body_GetPosition(Box.getBody());
        b2Rot rotation = b2Body_GetRotation(newBox.getBody());
        printf("%4.2f %4.2f %4.2f\n", position.x, position.y, b2Rot_GetAngle(rotation));
    }
}

void GameplayScreen::draw() {
    std::cout << "Draw\n";
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

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
        glm::vec4 destRect;
        destRect.x = b.getBody()->GetPosition().x;
        destRect.y = b.getBody()->GetPosition().y;
        destRect.z = b.getDimensions().x;
        destRect.w = b.getDimensions().y;
        m_spriteBatch.draw(destRect, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), m_texture.id, 0.0f, Bengine::ColorRGBA8(255, 255, 255, 255), 0.0f);
    }

    m_spriteBatch.end();
    m_spriteBatch.renderBatch();
    m_textureProgram.unuse();
}

void GameplayScreen::checkInput() {
    SDL_Event evnt;
    while (SDL_PollEvent(&evnt)) {
        m_game->onSDLEvent(evnt);
    }
}