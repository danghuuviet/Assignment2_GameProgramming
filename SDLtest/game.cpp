#include <iostream>
#include <string> // Add this line to include the <string> header
#include <SDL.h>
#include "game.h"
#include "resources.h"

// Initialize game variables
SDL_Rect leftPaddle = { PADDLE_PADDING, SCREEN_HEIGHT / 2 - PADDLE_HEIGHT / 2, PADDLE_WIDTH, PADDLE_HEIGHT };
SDL_Rect rightPaddle = { SCREEN_WIDTH - PADDLE_PADDING - PADDLE_WIDTH, SCREEN_HEIGHT / 2 - PADDLE_HEIGHT / 2, PADDLE_WIDTH, PADDLE_HEIGHT };
SDL_Rect ball = { SCREEN_WIDTH / 2 - BALL_SIZE / 2, SCREEN_HEIGHT / 2 - BALL_SIZE / 2, BALL_SIZE, BALL_SIZE };
int ballVelX = BALL_SPEED;
int ballVelY = BALL_SPEED;
int leftScore = 0;
int rightScore = 0;

// Function to update game logic
void updateGame() {
    // Update ball position
    ball.x += ballVelX;
    ball.y += ballVelY;

    // Check collision with paddles
    if (SDL_HasIntersection(&ball, &leftPaddle) || SDL_HasIntersection(&ball, &rightPaddle)) {
        ballVelX = -ballVelX;
        Mix_PlayChannel(-1, paddleSound, 0);
    }

    // Check collision with top or bottom walls
    if (ball.y <= 0 || ball.y >= SCREEN_HEIGHT - BALL_SIZE) {
        ballVelY = -ballVelY;
        Mix_PlayChannel(-1, wallSound, 0);
    }

    // Check for goal
    if (ball.x <= 0) {
        rightScore++;
        Mix_PlayChannel(-1, goalSound, 0);
        resetBall(true);
    }
    else if (ball.x >= SCREEN_WIDTH - BALL_SIZE) {
        leftScore++;
        Mix_PlayChannel(-1, goalSound, 0);
        resetBall(false);
    }
}

// Function to render game objects
void renderGame() {
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
    SDL_RenderClear(gRenderer);

    // Render paddles
    SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
    SDL_RenderFillRect(gRenderer, &leftPaddle);
    SDL_RenderFillRect(gRenderer, &rightPaddle);

    // Render ball
    SDL_RenderFillRect(gRenderer, &ball);

    // Render scores
    renderScore();

    SDL_RenderPresent(gRenderer);
}

// Function to reset the ball position
void resetBall(bool leftPlayerServe) {
    ball.x = SCREEN_WIDTH / 2 - BALL_SIZE / 2;
    ball.y = SCREEN_HEIGHT / 2 - BALL_SIZE / 2;

    if (leftPlayerServe) {
        ballVelX = BALL_SPEED;
    }
    else {
        ballVelX = -BALL_SPEED;
    }

    // Randomize ball's vertical velocity
    ballVelY = (rand() % (2 * BALL_SPEED + 1)) - BALL_SPEED;
}

// Function to render the scores on the screen
void renderScore() {
    SDL_Color textColor = { 255, 255, 255, 255 };
    std::string leftScoreText = "Left: " + std::to_string(leftScore);
    std::string rightScoreText = "Right: " + std::to_string(rightScore);

    SDL_Surface* leftSurface = TTF_RenderText_Solid(gFont, leftScoreText.c_str(), textColor);
    SDL_Surface* rightSurface = TTF_RenderText_Solid(gFont, rightScoreText.c_str(), textColor);

    SDL_Texture* leftTexture = SDL_CreateTextureFromSurface(gRenderer, leftSurface);
    SDL_Texture* rightTexture = SDL_CreateTextureFromSurface(gRenderer, rightSurface);

    int leftWidth, leftHeight, rightWidth, rightHeight;
    SDL_QueryTexture(leftTexture, NULL, NULL, &leftWidth, &leftHeight);
    SDL_QueryTexture(rightTexture, NULL, NULL, &rightWidth, &rightHeight);

    SDL_Rect leftRect = { 10, 10, leftWidth, leftHeight };
    SDL_Rect rightRect = { SCREEN_WIDTH - rightWidth - 10, 10, rightWidth, rightHeight };

    SDL_RenderCopy(gRenderer, leftTexture, NULL, &leftRect);
    SDL_RenderCopy(gRenderer, rightTexture, NULL, &rightRect);

    SDL_FreeSurface(leftSurface);
    SDL_FreeSurface(rightSurface);
    SDL_DestroyTexture(leftTexture);
    SDL_DestroyTexture(rightTexture);
}
// Function to handle game input events
void handleGameInput(SDL_Event& e, SDL_Rect& leftPaddle, SDL_Rect& rightPaddle) {
    // Handle paddle movement
    if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
        case SDLK_w:
            if (leftPaddle.y > 0) {
                leftPaddle.y -= PADDLE_SPEED;
            }
            break;
        case SDLK_s:
            if (leftPaddle.y < SCREEN_HEIGHT - PADDLE_HEIGHT) {
                leftPaddle.y += PADDLE_SPEED;
            }
            break;
        case SDLK_UP:
            if (rightPaddle.y > 0) {
                rightPaddle.y -= PADDLE_SPEED;
            }
            break;
        case SDLK_DOWN:
            if (rightPaddle.y < SCREEN_HEIGHT - PADDLE_HEIGHT) {
                rightPaddle.y += PADDLE_SPEED;
            }
            break;
        default:
            break;
        }
    }
}
