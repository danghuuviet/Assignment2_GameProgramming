#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <iostream>
#include <sstream>
#include <cmath>

// Constants for screen dimensions and game elements
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int PADDLE_WIDTH = 20;
const int PADDLE_HEIGHT = 100;
const int BALL_RADIUS = 10;
const int BALL_SPEED_X = 8;
const int BALL_SPEED_Y = 8;

// Global variables for SDL window, renderer, font, and menu texture
SDL_Window* gWindow = nullptr;
SDL_Renderer* gRenderer = nullptr;
TTF_Font* gFont = nullptr;
SDL_Texture* menuTexture = nullptr;

// Structs to represent paddles and the ball
struct Paddle {
    int x, y;
    int w, h;
};

struct Ball {
    float x, y;
    int r;
    float dx, dy;
    float angle;
};

// Variables for paddles, ball, goal areas, and scores
Paddle leftPaddle, rightPaddle;
Ball ball;
SDL_Rect leftGoal = { 0, (SCREEN_HEIGHT - 300) / 2, 10, 300 };
SDL_Rect rightGoal = { SCREEN_WIDTH - 10, (SCREEN_HEIGHT - 300) / 2, 10, 300 };
int leftScore = 0;
int rightScore = 0;

// Enumeration for menu options
enum class MenuOption { START, QUIT };
MenuOption selectedOption = MenuOption::START;
// Global variable for the goal sound
Mix_Chunk* goalSound = nullptr;
// Global variable for the paddles sound
Mix_Chunk* paddleSound = nullptr;
// Global variable for the paddles sound
Mix_Chunk* wallSound = nullptr;

// Function to initialize SDL, SDL_ttf, and SDL_image
bool initialize() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Initialize SDL_ttf
    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return false;
    }

    // Initialize SDL_image
    if (IMG_Init(IMG_INIT_JPG) == -1) {
        std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
        return false;
    }

    // Create window
    gWindow = SDL_CreateWindow("Pong Goal", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (gWindow == nullptr) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Create renderer
    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
    if (gRenderer == nullptr) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Load font
    gFont = TTF_OpenFont("vtks chalk 79.ttf", 28);
    if (gFont == nullptr) {
        std::cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return false;
    }

    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return false;
    }

    // Load the goal sound
    goalSound = Mix_LoadWAV("goal.mp3");
    if (goalSound == nullptr) {
        std::cerr << "Failed to load goal sound! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return false;
    }

    // Load the paddle sound
    paddleSound = Mix_LoadWAV("paddle.mp3");
    if (paddleSound == nullptr) {
        std::cerr << "Failed to load paddle sound! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return false;
    }

    // Load the wall sound
    wallSound = Mix_LoadWAV("wallsound.mp3");
    if (paddleSound == nullptr) {
        std::cerr << "Failed to load wall sound! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return false;
    }

    return true;
}

// Function to free resources and close SDL
void close() {
    TTF_CloseFont(gFont);
    gFont = nullptr;

    SDL_DestroyTexture(menuTexture);
    menuTexture = nullptr;

    Mix_FreeChunk(goalSound);
    goalSound = nullptr;

    Mix_FreeChunk(paddleSound);
    paddleSound = nullptr;

    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    SDL_Quit();
    TTF_Quit();
    IMG_Quit();
    Mix_CloseAudio();
}

// Function to load menu background texture
bool loadMenuTexture() {
    SDL_Surface* loadedSurface = IMG_Load("menubg.jpg");
    if (loadedSurface == nullptr) {
        std::cerr << "Unable to load image! SDL_image Error: " << IMG_GetError() << std::endl;
        return false;
    }
    menuTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
    if (menuTexture == nullptr) {
        std::cerr << "Unable to create texture from image! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }
    SDL_FreeSurface(loadedSurface);
    return true;
}

// Function to render the menu with options
void renderMenu() {
    SDL_RenderClear(gRenderer);
    SDL_RenderCopy(gRenderer, menuTexture, NULL, NULL);

    SDL_Color textColor = { 255, 255, 255, 255 };
    TTF_SetFontStyle(gFont, TTF_STYLE_BOLD);

    TTF_Font* largeFont = TTF_OpenFont("vtks chalk 79.ttf", 48); // Larger font size for Quit
    SDL_Surface* quitSurface = TTF_RenderText_Solid(largeFont, "Quit", textColor);
    SDL_Texture* quitTexture = SDL_CreateTextureFromSurface(gRenderer, quitSurface);
    int quitWidth, quitHeight;
    SDL_QueryTexture(quitTexture, NULL, NULL, &quitWidth, &quitHeight);
    SDL_Rect quitRect = { SCREEN_WIDTH - quitWidth - 20, SCREEN_HEIGHT - quitHeight - 20, quitWidth, quitHeight }; // Bottom-right corner with padding
    if (selectedOption == MenuOption::QUIT) {
        SDL_SetRenderDrawColor(gRenderer, 255, 255, 0, 255); // Yellow highlight color
        SDL_RenderDrawRect(gRenderer, &quitRect);
    }
    SDL_RenderCopy(gRenderer, quitTexture, NULL, &quitRect);
    SDL_FreeSurface(quitSurface);
    SDL_DestroyTexture(quitTexture);

    SDL_Surface* startSurface = TTF_RenderText_Solid(largeFont, "Start", textColor);
    SDL_Texture* startTexture = SDL_CreateTextureFromSurface(gRenderer, startSurface);
    int startWidth, startHeight;
    SDL_QueryTexture(startTexture, NULL, NULL, &startWidth, &startHeight);
    SDL_Rect startRect = { SCREEN_WIDTH - startWidth - 20, SCREEN_HEIGHT - quitHeight - startHeight - 40, startWidth, startHeight }; // Position "Start" above "Quit" with padding
    if (selectedOption == MenuOption::START) {
        SDL_SetRenderDrawColor(gRenderer, 255, 255, 0, 255); // Yellow highlight color
        SDL_RenderDrawRect(gRenderer, &startRect);
    }
    SDL_RenderCopy(gRenderer, startTexture, NULL, &startRect);
    SDL_FreeSurface(startSurface);
    SDL_DestroyTexture(startTexture);

    SDL_RenderPresent(gRenderer);

    TTF_CloseFont(largeFont);
}

// Function to reset the ball to its initial state
void resetBall(bool leftPlayerServe) {
    ball.r = BALL_RADIUS;
    ball.angle = 0;

    // Position the ball at the middle of the screen
    ball.x = SCREEN_WIDTH / 2 - ball.r;
    ball.y = SCREEN_HEIGHT / 2;

    // Calculate the velocity of the ball towards the middle goal
    if (leftPlayerServe) {
        ball.dx = BALL_SPEED_X; // Ball moves towards the right
    }
    else {
        ball.dx = -BALL_SPEED_X; // Ball moves towards the left
    }
    ball.dy = 0; // Ball moves straight (no vertical component)
    // Play the goal sound and delay for one second
    SDL_Delay(1000);
}




// Function to handle menu input events
void handleMenuInput(SDL_Event& e, bool& inMenu, bool& leftPlayerServe) {
    if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
        case SDLK_UP:
            selectedOption = (selectedOption == MenuOption::START) ? MenuOption::QUIT : MenuOption::START;
            renderMenu();
            break;
        case SDLK_DOWN:
            selectedOption = (selectedOption == MenuOption::START) ? MenuOption::QUIT : MenuOption::START;
            renderMenu();
            break;
        case SDLK_RETURN:
            if (selectedOption == MenuOption::START) {
                inMenu = false; // Exit the menu loop
                // Initialize game variables here
                resetBall(leftPlayerServe);
                leftScore = 0;
                rightScore = 0;
            }
            else if (selectedOption == MenuOption::QUIT) {
                close();
                exit(0);
            }
            break;
        default:
            break;
        }
    }
}


// Function to check collision between two rectangles
bool checkCollision(const SDL_Rect& rectA, const SDL_Rect& rectB) {
    return (rectA.x + rectA.w >= rectB.x && rectB.x + rectB.w >= rectA.x && rectA.y + rectA.h >= rectB.y && rectB.y + rectB.h >= rectA.y);
}

// Function to update game state
void update() {
    // Update ball position
    ball.x += ball.dx;
    ball.y += ball.dy;

    // Define ballRect for collision detection
    SDL_Rect ballRect = { static_cast<int>(ball.x), static_cast<int>(ball.y), 2 * ball.r, 2 * ball.r };

    // Handle ball collisions with top and bottom borders
    if (ball.y <= 0 || ball.y + ball.r * 2 >= SCREEN_HEIGHT) {
        Mix_PlayChannel(-1, wallSound, 0);
        ball.dy = -ball.dy;
    }
    // Handle ball collisions with left and right walls (sides)
    if (ball.x <= 0 || ball.x + ball.r * 2 >= SCREEN_WIDTH) {
        Mix_PlayChannel(-1, wallSound, 0);
        ball.dx = -ball.dx;
    }

    // Handle ball collisions with left and right walls (goals)
    if (ball.x <= leftGoal.x + leftGoal.w) {
        if (ball.y + ball.r >= leftGoal.y && ball.y <= leftGoal.y + leftGoal.h) {
            Mix_PlayChannel(-1, goalSound, 0);
            ++rightScore;
            resetBall(false); // Pass false to indicate right player serves next
        }
    }
    else if (ball.x + ball.r * 2 >= rightGoal.x) {
        if (ball.y + ball.r >= rightGoal.y && ball.y <= rightGoal.y + rightGoal.h) {
            Mix_PlayChannel(-1, goalSound, 0);
            ++leftScore;
            resetBall(true); // Pass true to indicate left player serves next
        }
    }

    // Define paddle areas for collision detection
    SDL_Rect leftPaddleTop = { leftPaddle.x, leftPaddle.y, leftPaddle.w, leftPaddle.h / 3 };
    SDL_Rect leftPaddleMiddle = { leftPaddle.x, leftPaddle.y + leftPaddle.h / 3, leftPaddle.w, leftPaddle.h / 3 };
    SDL_Rect leftPaddleBottom = { leftPaddle.x, leftPaddle.y + 2 * leftPaddle.h / 3, leftPaddle.w, leftPaddle.h / 3 };
    SDL_Rect rightPaddleTop = { rightPaddle.x, rightPaddle.y, rightPaddle.w, rightPaddle.h / 3 };
    SDL_Rect rightPaddleMiddle = { rightPaddle.x, rightPaddle.y + rightPaddle.h / 3, rightPaddle.w, rightPaddle.h / 3 };
    SDL_Rect rightPaddleBottom = { rightPaddle.x, rightPaddle.y + 2 * rightPaddle.h / 3, rightPaddle.w, rightPaddle.h / 3 };

    // Check for collision with left paddle
    if (checkCollision(ballRect, leftPaddleTop)) {
        Mix_PlayChannel(-1, paddleSound, 0);
        ball.dx = -ball.dx;
        ball.dy = -BALL_SPEED_Y; // Ball moves upward
    }
    else if (checkCollision(ballRect, leftPaddleMiddle)) {
        Mix_PlayChannel(-1, paddleSound, 0);
        ball.dx = -ball.dx;
        ball.dy = 0; // Ball moves straight
    }
    else if (checkCollision(ballRect, leftPaddleBottom)) {
        Mix_PlayChannel(-1, paddleSound, 0);
        ball.dx = -ball.dx;
        ball.dy = BALL_SPEED_Y; // Ball moves downward
    }

    // Check for collision with right paddle
    if (checkCollision(ballRect, rightPaddleTop)) {
        Mix_PlayChannel(-1, paddleSound, 0);
        ball.dx = -ball.dx;
        ball.dy = -BALL_SPEED_Y; // Ball moves upward
    }
    else if (checkCollision(ballRect, rightPaddleMiddle)) {
        Mix_PlayChannel(-1, paddleSound, 0);
        ball.dx = -ball.dx;
        ball.dy = 0; // Ball moves straight
    }
    else if (checkCollision(ballRect, rightPaddleBottom)) {
        Mix_PlayChannel(-1, paddleSound, 0);
        ball.dx = -ball.dx;
        ball.dy = BALL_SPEED_Y; // Ball moves downward
    }

    // Increment ball angle for rotation effect
    ball.angle += 0.1;
}




// Function to render the game scene
void render() {
    SDL_SetRenderDrawColor(gRenderer, 0, 128, 0, 255);
    SDL_RenderClear(gRenderer);

    // Render borders
    SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
    SDL_Rect topBorder = { 0, 0, SCREEN_WIDTH, 10 };
    SDL_Rect bottomBorder = { 0, SCREEN_HEIGHT - 10, SCREEN_WIDTH, 10 };
    SDL_Rect leftBorder = { 0, 0, 10, SCREEN_HEIGHT };
    SDL_Rect rightBorder = { SCREEN_WIDTH - 10, 0, 10, SCREEN_HEIGHT };
    SDL_RenderFillRect(gRenderer, &topBorder);
    SDL_RenderFillRect(gRenderer, &bottomBorder);
    SDL_RenderFillRect(gRenderer, &leftBorder);
    SDL_RenderFillRect(gRenderer, &rightBorder);

    // Render goals
    SDL_SetRenderDrawColor(gRenderer, 255, 0, 0, 255);
    SDL_RenderFillRect(gRenderer, &leftGoal);
    SDL_RenderFillRect(gRenderer, &rightGoal);

    // Render center spot
    SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
    SDL_Rect centerSpot = { SCREEN_WIDTH / 2 - 5, SCREEN_HEIGHT / 2 - 5, 10, 10 };
    SDL_RenderFillRect(gRenderer, &centerSpot);

    // Render halfway line
    SDL_Rect halfwayLine = { SCREEN_WIDTH / 2 - 1, 0, 2, SCREEN_HEIGHT };
    SDL_RenderFillRect(gRenderer, &halfwayLine);

    // Render penalty areas
    SDL_Rect leftPenaltyArea = { 0, SCREEN_HEIGHT / 4, 150, SCREEN_HEIGHT / 2 };
    SDL_Rect rightPenaltyArea = { SCREEN_WIDTH - 150, SCREEN_HEIGHT / 4, 150, SCREEN_HEIGHT / 2 };
    SDL_RenderDrawRect(gRenderer, &leftPenaltyArea);
    SDL_RenderDrawRect(gRenderer, &rightPenaltyArea);

    // Render circle
    int circleCenterX = SCREEN_WIDTH / 2;
    int circleCenterY = SCREEN_HEIGHT / 2;
    int radius = 80;
    int density = 1;
    for (int i = 0; i < 360; i += density) {
        float angle = i * M_PI / 180.0;
        int x = circleCenterX + radius * std::cos(angle);
        int y = circleCenterY + radius * std::sin(angle);
        SDL_RenderDrawPoint(gRenderer, x, y);
    }

    // Render paddles
    SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
    SDL_Rect leftPaddleRect = { leftPaddle.x, leftPaddle.y, leftPaddle.w, leftPaddle.h };
    SDL_Rect rightPaddleRect = { rightPaddle.x, rightPaddle.y, rightPaddle.w, rightPaddle.h };
    SDL_RenderFillRect(gRenderer, &leftPaddleRect);
    SDL_RenderFillRect(gRenderer, &rightPaddleRect);

    // Render particles
    int numParticles = 20;
    for (int i = 0; i < numParticles; ++i) {
        int radius = rand() % 10 + 5;
        int offsetX = rand() % (2 * radius) - radius;
        int offsetY = rand() % (2 * radius) - radius;
        Uint8 red = 255;
        Uint8 green = rand() % 256;
        Uint8 blue = 0;
        Uint8 alpha = rand() % 256;
        SDL_SetRenderDrawColor(gRenderer, red, green, blue, alpha);
        SDL_Rect particleRect = { ball.x + offsetX, ball.y + offsetY, radius * 2, radius * 2 };
        SDL_RenderFillRect(gRenderer, &particleRect);
    }

    // Render ball
    SDL_SetRenderDrawColor(gRenderer, 255, 128, 0, 255);
    int centerX = ball.x + ball.r;
    int centerY = ball.y + ball.r;
    for (int i = 0; i < 360; i += 30) {
        float angle = (ball.angle + i) * M_PI / 180.0;
        int endX = centerX + ball.r * std::cos(angle);
        int endY = centerY + ball.r * std::sin(angle);
        SDL_RenderDrawLine(gRenderer, centerX, centerY, endX, endY);
    }

    // Render scores
    SDL_Color textColor = { 255, 255, 255, 255 };
    TTF_Font* largeFont = TTF_OpenFont("score.ttf", 48); // Larger font size for scores
    if (largeFont == nullptr) {
        std::cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return;
    }
    std::stringstream leftScoreStream;
    leftScoreStream << leftScore;
    std::string leftScoreString = leftScoreStream.str();
    std::stringstream rightScoreStream;
    rightScoreStream << rightScore;
    std::string rightScoreString = rightScoreStream.str();
    SDL_Surface* leftScoreSurface = TTF_RenderText_Solid(largeFont, leftScoreString.c_str(), textColor);
    SDL_Surface* rightScoreSurface = TTF_RenderText_Solid(largeFont, rightScoreString.c_str(), textColor);
    SDL_Texture* leftScoreTexture = SDL_CreateTextureFromSurface(gRenderer, leftScoreSurface);
    SDL_Texture* rightScoreTexture = SDL_CreateTextureFromSurface(gRenderer, rightScoreSurface);
    SDL_Rect leftScoreRect = { 50, 50, leftScoreSurface->w, leftScoreSurface->h };
    SDL_Rect rightScoreRect = { SCREEN_WIDTH - 50 - rightScoreSurface->w, 50, rightScoreSurface->w, rightScoreSurface->h };
    SDL_RenderCopy(gRenderer, leftScoreTexture, NULL, &leftScoreRect);
    SDL_RenderCopy(gRenderer, rightScoreTexture, NULL, &rightScoreRect);
    SDL_FreeSurface(leftScoreSurface);
    SDL_FreeSurface(rightScoreSurface);
    SDL_DestroyTexture(leftScoreTexture);
    SDL_DestroyTexture(rightScoreTexture);
    TTF_CloseFont(largeFont);

    SDL_RenderPresent(gRenderer);
}


// Function to move the paddle based on input
void movePaddle(Paddle& paddle, bool up, bool down) {
    if (up && paddle.y > 0) {
        paddle.y -= 5;
    }
    if (down && paddle.y < SCREEN_HEIGHT - paddle.h) {
        paddle.y += 5;
    }
}

// Function to handle game input
void handleGameInput(const Uint8* currentKeyStates) {
    movePaddle(leftPaddle, currentKeyStates[SDL_SCANCODE_W], currentKeyStates[SDL_SCANCODE_S]);
    movePaddle(rightPaddle, currentKeyStates[SDL_SCANCODE_UP], currentKeyStates[SDL_SCANCODE_DOWN]);
}

// Main function
int main(int argc, char* args[]) {
    bool inMenu = true;
    bool leftPlayerServe = true; // Variable to track which player serves

    if (!initialize()) {
        std::cerr << "Failed to initialize!" << std::endl;
        return -1;
    }

    if (!loadMenuTexture()) {
        std::cerr << "Failed to load menu texture!" << std::endl;
        close();
        return -1;
    }

    leftPaddle = { 20, SCREEN_HEIGHT / 2 - PADDLE_HEIGHT / 2, PADDLE_WIDTH, PADDLE_HEIGHT };
    rightPaddle = { SCREEN_WIDTH - 20 - PADDLE_WIDTH, SCREEN_HEIGHT / 2 - PADDLE_HEIGHT / 2, PADDLE_WIDTH, PADDLE_HEIGHT };

    // Start the game with the ball positioned at the left player's goal
    resetBall(leftPlayerServe);

    SDL_Event e;
    bool quit = false;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            if (inMenu) {
                handleMenuInput(e, inMenu, leftPlayerServe); // Pass leftPlayerServe to handleMenuInput
            }
        }

        if (inMenu) {
            renderMenu();
        }
        else {
            const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
            handleGameInput(currentKeyStates);
            update();
            render();

            // Check for goal
            if (ball.x <= leftGoal.x + leftGoal.w) {
                if (ball.y + ball.r >= leftGoal.y && ball.y <= leftGoal.y + leftGoal.h) {
                    ++rightScore;
                    leftPlayerServe = false; // Right player serves next
                    resetBall(leftPlayerServe);
                }
            }
            else if (ball.x + ball.r * 2 >= rightGoal.x) {
                if (ball.y + ball.r >= rightGoal.y && ball.y <= rightGoal.y + rightGoal.h) {
                    ++leftScore;
                    leftPlayerServe = true; // Left player serves next
                    resetBall(leftPlayerServe);
                }
            }
        }

        SDL_Delay(10);
    }

    close();
    return 0;
}