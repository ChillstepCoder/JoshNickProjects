//BallController.cpp

#include "BallController.h"
#include <iostream>
#include "Grid.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtc/matrix_transform.hpp>

void BallController::updateBalls(std::vector<Ball>& balls, Grid* grid, float deltaTime, int maxX, int maxY) {
  //glm::vec2 gravity = getGravityAccel();

  for (size_t i = 0; i < balls.size(); i++) {
    Ball& ball = balls[i];

    if (i != m_grabbedBall) {
      // Apply gravity
      ball.velocity += m_gravity * deltaTime;

      // Apply friction
      float frictionFactor = std::pow(1.0f - m_friction, deltaTime * 60.0f); // Adjust for 60 FPS
      ball.velocity *= frictionFactor;

      // Update position
      ball.position += ball.velocity * deltaTime * m_speedMultiplier;

      // Apply maximum speed limit
      float speed = glm::length(ball.velocity);
      if (speed > m_multipliedMaxSpeed) {
        ball.velocity = glm::normalize(ball.velocity) * m_multipliedMaxSpeed;
      }

      // Wall collisions
      handleWallCollision(ball, maxX, maxY);
    }

    // Update grid
    Cell* newCell = grid->getCell(ball.position);
    if (newCell != ball.ownerCell) {
      grid->removeBallFromCell(&balls[i]);
      grid->addBall(&balls[i], newCell);
    }
  }

  // Handle ball-to-ball collisions
  updateCollision(grid);
}

void BallController::updateMultipliedSpeeds() {
  m_multipliedMaxSpeed = m_maxSpeed * m_speedMultiplier;
}

void BallController::setSpeedMultiplier(float multiplier) {
  m_speedMultiplier = multiplier;
  updateMultipliedSpeeds();
}

void BallController::handleWallCollision(Ball& ball, int maxX, int maxY) {
  if (ball.position.x - ball.radius < 0) {
    ball.position.x = ball.radius;
    ball.velocity.x = -ball.velocity.x;
  }
  else if (ball.position.x + ball.radius > maxX) {
    ball.position.x = maxX - ball.radius;
    ball.velocity.x = -ball.velocity.x;
  }

  if (ball.position.y - ball.radius < 0) {
    ball.position.y = ball.radius;
    ball.velocity.y = -ball.velocity.y;
  }
  else if (ball.position.y + ball.radius > maxY) {
    ball.position.y = maxY - ball.radius;
    ball.velocity.y = -ball.velocity.y;
  }
}

void BallController::onMouseDown(std::vector<Ball>& balls, float mouseX, float mouseY) {
  for (size_t i = 0; i < balls.size(); i++) {
    if (isMouseOnBall(balls[i], mouseX, mouseY)) {
      m_grabbedBall = i;
      m_grabOffset = glm::vec2(mouseX, mouseY) - balls[i].position;
      m_prevPos = balls[i].position;
      balls[i].velocity = glm::vec2(0.0f);
      break;
    }
  }
}

void BallController::onMouseUp(std::vector<Ball>& balls) {
  if (m_grabbedBall != -1) {
    // Calculate velocity based on the difference in position and time
    // Assuming a frame time of 1/60 second
    balls[m_grabbedBall].velocity = (balls[m_grabbedBall].position - m_prevPos) * 60.0f;

    // Optionally, limit the release velocity
    float speed = glm::length(balls[m_grabbedBall].velocity);
    if (speed > m_maxSpeed * 2) {
      balls[m_grabbedBall].velocity = glm::normalize(balls[m_grabbedBall].velocity) * (m_maxSpeed * 2.0f);
    }

    m_grabbedBall = -1;
  }
}

void BallController::onMouseMove(std::vector<Ball>& balls, float mouseX, float mouseY) {
  if (m_grabbedBall != -1) {
    glm::vec2 newPosition(mouseX, mouseY);
    glm::vec2 oldPosition = balls[m_grabbedBall].position;
    balls[m_grabbedBall].position = newPosition;

    // Calculate velocity based on mouse movement
    balls[m_grabbedBall].velocity = (newPosition - oldPosition) * 60.0f; // Assuming 60 FPS

    // Optionally, limit the maximum velocity during dragging
    float speed = glm::length(balls[m_grabbedBall].velocity);
    if (speed > m_maxSpeed * 2) { // Allow dragged balls to move faster than the normal max speed
      balls[m_grabbedBall].velocity = glm::normalize(balls[m_grabbedBall].velocity) * (m_maxSpeed * 2.0f);
    }
  }
} 

void BallController::updateCollision(Grid* grid) {
    for (size_t i = 0; i < grid->m_cells.size(); i++) {

        int x = i % grid->m_numXCells;
        int y = i / grid->m_numXCells;

        Cell& cell = grid->m_cells[i];

        // Loop through all balls in a cell
        for (size_t j = 0; j < cell.balls.size(); j++) {
            Ball* ball = cell.balls[j];
            /// Update with the residing cell
            checkCollision(ball, cell.balls, j + 1);

            /// Update collision with neighbor cells
            if (x > 0) {
                // Left
                checkCollision(ball, grid->getCell(x - 1, y)->balls, 0);
                if (y > 0) {
                    /// Top left
                    checkCollision(ball, grid->getCell(x - 1, y - 1)->balls, 0);
                }
                if (y < grid->m_numYCells - 1) {
                    // Bottom left
                    checkCollision(ball, grid->getCell(x - 1, y + 1)->balls, 0);
                }
            }
            // Up cell
            if (y > 0) {
                checkCollision(ball, grid->getCell(x, y - 1)->balls, 0);
            }
        }
    }
}

void BallController::checkCollision(Ball* ball, std::vector<Ball*>& ballsToCheck, int startingIndex) {
    for (size_t i = startingIndex; i < ballsToCheck.size(); i++) {
        checkCollision(*ball, *ballsToCheck[i]);
    }
}

void BallController::checkCollision(Ball& b1, Ball& b2) {
  glm::vec2 distVec = b2.position - b1.position;
  float dist = glm::length(distVec);
  float totalRadius = b1.radius + b2.radius;

  if (dist < totalRadius) {
    glm::vec2 collisionNormal = glm::normalize(distVec);

    // Move balls apart
    float overlap = totalRadius - dist;
    float totalMass = b1.mass + b2.mass;
    float b1Ratio = b1.mass / totalMass;
    float b2Ratio = b2.mass / totalMass;

    b1.position -= overlap * b2Ratio * collisionNormal;
    b2.position += overlap * b1Ratio * collisionNormal;

    // Calculate relative velocity
    glm::vec2 relativeVelocity = b2.velocity - b1.velocity;

    // Calculate impulse
    float impulseStrength = glm::dot(relativeVelocity, collisionNormal);

    // If balls are moving apart, don't apply impulse
    if (impulseStrength > 0) {
      return;
    }

    float e = 1.0f; // Perfect elasticity
    float j = -(1 + e) * impulseStrength;
    j /= 1 / b1.mass + 1 / b2.mass;

    glm::vec2 impulse = j * collisionNormal;

    // Apply impulse
    b1.velocity -= impulse / b1.mass;
    b2.velocity += impulse / b2.mass;

    // Apply maximum speed limit
    float speed1 = glm::length(b1.velocity);
    float speed2 = glm::length(b2.velocity);

    if (speed1 > m_multipliedMaxSpeed) {
      b1.velocity = glm::normalize(b1.velocity) * m_multipliedMaxSpeed;
    }
    if (speed2 > m_multipliedMaxSpeed) {
      b2.velocity = glm::normalize(b2.velocity) * m_multipliedMaxSpeed;
    }

    // Transfer color based on relative speed
    if (glm::length(b1.velocity - b2.velocity) > 0.5f) {
      bool b1Faster = glm::length(b1.velocity) > glm::length(b2.velocity);
      if (b1Faster) {
        b2.color = b1.color;
      }
      else {
        b1.color = b2.color;
      }
    }
  }
}


bool BallController::isMouseOnBall(const Ball& ball, float mouseX, float mouseY) {
  return glm::distance(glm::vec2(mouseX, mouseY), ball.position) < ball.radius;
}

