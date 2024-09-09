#include "MainGame.h"
#include "Errors.h"

#include <iostream>
#include <string>


MainGame::MainGame() : 
    _screenWidth(1540),
    _screenHeight(1024),
    _time(0.0f),
    _window(nullptr),
    _gameState(GameState::PLAY),
    _maxFPS(60.0f)
{

}

MainGame::~MainGame()
{
}

void MainGame::run() {
    initSystems();

    //Initialize our sprite. (temporary)
    _sprites.push_back(new Sprite());
    _sprites.back()->init(-1.0f, -1.0f, 1.0f, 1.0f, "Textures/jimmyJump_pack/PNG/CharacterRight_Standing.png");

    _sprites.push_back(new Sprite());
    _sprites.back()->init(0.0f, -1.0f, 1.0f, 1.0f, "Textures/jimmyJump_pack/PNG/CharacterRight_Standing.png");

    _sprites.push_back(new Sprite());
    _sprites.back()->init(-1.0f, 0.0f, 1.0f, 1.0f, "Textures/jimmyJump_pack/PNG/CharacterRight_Standing.png");
    //This only returns when the game ends
    gameLoop();
}

//Initialize SDL and OpenGL and whatever else we need
void MainGame::initSystems() {
    //Initialize SDL
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_HAPTIC);

    //Tell SDL that we want a double buffered window so we dont get any flickering
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    //Open an SDL window
    _window = SDL_CreateWindow("Game Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _screenWidth, _screenHeight, SDL_WINDOW_OPENGL);
    if (_window == nullptr) {
        fatalError("SDL Window could not be created!");
    }

    //Set up our OpenGL context
    SDL_GLContext glContext = SDL_GL_CreateContext(_window);
    if (glContext == nullptr) {
        fatalError("SDL_GL context could not be created!");
    }

    //Set up glew (optional but recommended)
    GLenum error = glewInit();
    if (error != GLEW_OK) {
        fatalError("Could not initialize glew!");
    }

    //Check the OpenGL version
    std::printf("***   OpenGL Version: %s  ***", glGetString(GL_VERSION));


    //Set the background color
    glClearColor(0.4f, 0.4f, 0.4f, 1.0f);

    //Set Vsync to on. Unneeded code tho since it was on already by default
    SDL_GL_SetSwapInterval(1);

    initShaders();
}

void MainGame::initShaders() {
    _colorProgram.compileShaders("Shaders/colorShadingVert.txt", "Shaders/colorShadingFrag.txt");
    _colorProgram.addAttribute("vertexPosition");
    _colorProgram.addAttribute("vertexColor");
    _colorProgram.addAttribute("vertexUV");
    _colorProgram.linkShaders();
}

void MainGame::gameLoop() {
    //Used for frame time measuring
    float startTicks = SDL_GetTicks();


    while (_gameState != GameState::EXIT) {
        processInput();
        _time += 0.05f;
        drawGame();
        calculateFPS();

        //print only once every 10 frames
        static int frameCounter = 0;
        frameCounter++;
        if (frameCounter == 10) {
            std::cout << _fps << std::endl;
            frameCounter = 0;
        }

        float frameTicks = SDL_GetTicks() - startTicks;
        //Limit the FPS to the max FPS
        if (1000.0f / _maxFPS > frameTicks) {
            SDL_Delay(1000.0f / _maxFPS - frameTicks);
        }
        
    }
}
void MainGame::processInput() {
    SDL_Event evnt;

    while (SDL_PollEvent(&evnt)) {
        switch (evnt.type) {
        case SDL_QUIT:
            _gameState = GameState::EXIT;
            break;
        case SDL_MOUSEMOTION:
            //std::cout << evnt.motion.x << " " << evnt.motion.y << std::endl;
            break;
        }
    }
}

//Draws the game using the almighty OpenGL
void MainGame::drawGame() {


    //Set the base depth to 1.0
    glClearDepth(1.0);
    //Clear the color and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _colorProgram.use();
    //We are using texture unit 0
    glActiveTexture(GL_TEXTURE0);
    //Get the uniform location
    GLint textureLocation = _colorProgram.getUniformLocation("mySampler");
    //Tell the shader that the texture is in texture unit 0
    glUniform1i(textureLocation, 0);

    //GLint timeLocation = _colorProgram.getUniformLocation("time");
    //glUniform1f(timeLocation, _time);

    //Draw our sprite!
    for (int i = 0; i < _sprites.size(); i++) {
        _sprites[i]->draw();
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    _colorProgram.unuse();

    //Swap our buffer and draw everything to the screen!
    SDL_GL_SwapWindow(_window);
}

void MainGame::calculateFPS() {
    static const int NUM_SAMPLES = 10;
    static float frameTimes[NUM_SAMPLES];
    static int currentFrame = 0;

    static float prevTicks = SDL_GetTicks();
    float currentTicks;
    currentTicks = SDL_GetTicks();

    _frameTime = currentTicks - prevTicks;
    frameTimes[currentFrame % NUM_SAMPLES] = _frameTime;

    prevTicks = currentTicks;

    int count;

    currentFrame++;
    if (currentFrame < NUM_SAMPLES) {
        count = currentFrame;
    } else {
        count = NUM_SAMPLES;
    }

    float frameTimeAverage = 0;
    for (int i = 0; i < count; i++) {
        frameTimeAverage += frameTimes[i];
    }
    frameTimeAverage /= count;

    if (frameTimeAverage > 0) {
        _fps = 1000.0f / frameTimeAverage;
    } else {
        _fps = 60.0f;
    }

}