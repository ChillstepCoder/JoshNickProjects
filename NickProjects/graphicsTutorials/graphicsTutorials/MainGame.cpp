#include "MainGame.h"
#include <Bengine/BengineErrors.h>
#include <Bengine/ResourceManager.h>

#include <iostream>
#include <string>


MainGame::MainGame() : 
    m_screenWidth(1540),
    m_screenHeight(1024),
    m_time(0.0f),
    m_gameState(GameState::PLAY),
    m_maxFPS(60.0f)
{
    m_camera.init(m_screenWidth, m_screenHeight);
}

MainGame::~MainGame()
{
}

void MainGame::run() {
    initSystems();


    //This only returns when the game ends
    gameLoop();
}

//Initialize SDL and OpenGL and whatever else we need
void MainGame::initSystems() {
    
    Bengine::init();

    m_window.create("Game Engine", m_screenWidth, m_screenHeight, 0);

    initShaders();

    m_spriteBatch.init();
    m_fpsLimiter.init(m_maxFPS);

}

void MainGame::initShaders() {
    m_colorProgram.compileShaders("Shaders/colorShadingVert.txt", "Shaders/colorShadingFrag.txt");
    m_colorProgram.addAttribute("vertexPosition");
    m_colorProgram.addAttribute("vertexColor");
    m_colorProgram.addAttribute("vertexUV");
    m_colorProgram.linkShaders();
}

void MainGame::gameLoop() {

    while (m_gameState != GameState::EXIT) {
        m_fpsLimiter.begin();

        processInput();
        m_time += 0.05f;

        m_camera.update();

        //Update all bullets
        for (int i = 0; i < m_bullets.size();) {
            if (m_bullets[i].update() == true) {
                m_bullets[i] = m_bullets.back();
                m_bullets.pop_back();
            } else {
                i++;
            }
        }

        drawGame();

        m_fps = m_fpsLimiter.end(); 

        //print only once every 10000 frames
        static int frameCounter = 0;
        frameCounter++;
        if (frameCounter == 10000) {
            std::cout << m_fps << std::endl;
            frameCounter = 0;
        }
    }
}
void MainGame::processInput() {
    SDL_Event evnt;

    const float CAMERA_SPEED = 2.0f;
    const float SCALE_SPEED = 0.1f;

    while (SDL_PollEvent(&evnt)) {
        switch (evnt.type) {
        case SDL_QUIT:
            m_gameState = GameState::EXIT;
            break;
        case SDL_MOUSEMOTION:
            m_inputManager.setMouseCoords(evnt.motion.x, evnt.motion.y);
            break;
        case SDL_KEYDOWN:
            m_inputManager.pressKey(evnt.key.keysym.sym);
            break;
        case SDL_KEYUP:
            m_inputManager.releaseKey(evnt.key.keysym.sym);
            break;
        case SDL_MOUSEBUTTONDOWN:
            m_inputManager.pressKey(evnt.button.button);
            break;
        case SDL_MOUSEBUTTONUP:
            m_inputManager.releaseKey(evnt.button.button);
            break;
        }
    }

    if (m_inputManager.isKeyDown(SDLK_w)) {
        m_camera.setPosition(m_camera.getPosition() + glm::vec2(0.0f, -CAMERA_SPEED));
    }
    if (m_inputManager.isKeyDown(SDLK_s)) {
        m_camera.setPosition(m_camera.getPosition() + glm::vec2(0.0f, CAMERA_SPEED));
    }
    if (m_inputManager.isKeyDown(SDLK_a)) {
        m_camera.setPosition(m_camera.getPosition() + glm::vec2(CAMERA_SPEED, 0.0f));
    }
    if (m_inputManager.isKeyDown(SDLK_d)) {
        m_camera.setPosition(m_camera.getPosition() + glm::vec2(-CAMERA_SPEED, 0.0f));
    }
    if (m_inputManager.isKeyDown(SDLK_q)) {
        m_camera.setScale(m_camera.getScale() + SCALE_SPEED);
    }
    if (m_inputManager.isKeyDown(SDLK_e)) {
        m_camera.setScale(m_camera.getScale() - SCALE_SPEED);
    }

    if (m_inputManager.isKeyDown(SDL_BUTTON_LEFT)) {
        glm::vec2 mouseCoords = m_inputManager.getMouseCoords();
        mouseCoords = m_camera.convertScreenToWorld(mouseCoords);

        glm::vec2 playerPosition(0.0f);
        glm::vec2 direction = mouseCoords - playerPosition;
        direction = glm::normalize(direction);

        m_bullets.emplace_back(playerPosition, direction, 5.00f, 100);
    }
    
}

//Draws the game using the almighty OpenGL
void MainGame::drawGame() {


    //Set the base depth to 1.0
    glClearDepth(1.0);
    //Clear the color and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_colorProgram.use();
    //We are using texture unit 0
    glActiveTexture(GL_TEXTURE0);
    //Get the uniform location
    GLint textureLocation = m_colorProgram.getUniformLocation("mySampler");
    //Tell the shader that the texture is in texture unit 0
    glUniform1i(textureLocation, 0);

    //Set the camera matrix
    GLint pLocation = m_colorProgram.getUniformLocation("P");
    glm::mat4 cameraMatrix = m_camera.getCameraMatrix();

    glUniformMatrix4fv(pLocation, 1, GL_FALSE, &(cameraMatrix[0][0]));

    m_spriteBatch.begin();

    glm::vec4 pos(-20.0f, -20.0f, 50.0f, 50.0f);
    glm::vec4 uv(0.0f, 0.0f, 1.0f, 1.0f);
    static Bengine::GLTexture texture = Bengine::ResourceManager::getTexture("Textures/jimmyJump_pack/PNG/CharacterRight_Standing.png");
    Bengine::ColorRGBA8 color;
    color.r = 255;
    color.g = 255;
    color.b = 255;
    color.a = 255;

    m_spriteBatch.draw(pos, uv, texture.id, 0.0f, color);
    
    for (int i = 0; i < m_bullets.size(); i++) {
        m_bullets[i].draw(m_spriteBatch);
    }

    m_spriteBatch.end();

    m_spriteBatch.renderBatch();

    //unbind the texture
    glBindTexture(GL_TEXTURE_2D, 0);

    //disable the shader
    m_colorProgram.unuse();

    //Swap our buffer and draw everything to the screen!
    m_window.swapBuffer();
}
