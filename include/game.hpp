#pragma once

#include <algorithm>
#include <vector>
#include <format>
#include <cmath>
#include <cstdlib>
#include <stdexcept>

#define M_PI 3.14159265358979323846
#define _USE_MATH_DEFINES
#include "window.hpp"

class Game
{
public:
   static constexpr double SHIP_ROTATION_SPEED = -M_PI * 2.5;      // radians per second
   static constexpr double SHIP_RADIUS = 0.05;                     // units
   static constexpr double ASTEROID_ROTATION_SPEED = -M_PI * 0.75; // radians per second
   static constexpr double LASER_SPEED = 1.0;                      // units per second
   static constexpr double FIRE_RATE = 5.0;                        // shots per second
   static constexpr double ROCK_SPEED = 0.3;                       // units per second
   static constexpr double SCREEN_BUFFER = 0.035;
   static constexpr double HITBOX_TOGGLE_RATE = 5.0; // toggles per second

   Game()
   {
      rockAngle = M_PI * 1.5; // down
      shipAngle = M_PI * 0.5; // up

      // Initialize normal rock
      rockPositions.push_back({0.5, 0.5});
      rockVelocities.push_back({(std::rand() / (double)RAND_MAX - 0.5) * ROCK_SPEED,
                                (std::rand() / (double)RAND_MAX - 0.5) * ROCK_SPEED}); // random v
      rockTypes.push_back(Draw::NORMAL_ROCK);
      rockRadii.push_back(0.1);
      rockHitboxColors.push_back(Draw::green);

      // Initialize small rock
      rockPositions.push_back({-0.5, -0.5});
      rockVelocities.push_back({(std::rand() / (double)RAND_MAX - 0.5) * ROCK_SPEED,
                                (std::rand() / (double)RAND_MAX - 0.5) * ROCK_SPEED});
      rockTypes.push_back(Draw::SMALL_ROCK);
      rockRadii.push_back(0.05);
      rockHitboxColors.push_back(Draw::green);

      // Initialize large rock
      rockPositions.push_back({0.0, -0.5});
      rockVelocities.push_back({(std::rand() / (double)RAND_MAX - 0.5) * ROCK_SPEED,
                                (std::rand() / (double)RAND_MAX - 0.5) * ROCK_SPEED});
      rockTypes.push_back(Draw::LARGE_ROCK);
      rockRadii.push_back(0.2);
      rockHitboxColors.push_back(Draw::green);

      // Check fire rate
      if (FIRE_RATE <= 0.0)
         throw std::runtime_error("Invalid fire rate.");
   }

   ~Game() = default;

   // 1. Input completely encapsulated by the UI class
   void input(UI &ui)
   {
      // Close window
      if (ui.isKeyDown(GLFW_KEY_ESCAPE) || (ui.isKeyDown(GLFW_KEY_C) && ui.isKeyDown(GLFW_KEY_LEFT_CONTROL)))
      { // Escape or Ctrl+C
         ui.closeWindow();
      }

      // Set ship rotation to sum of left/right inputs
      shipRotation = 0.0;
      if (ui.isKeyDown(GLFW_KEY_LEFT) || ui.isKeyDown(GLFW_KEY_S))
      { // Left or 'S' key
         shipRotation -= SHIP_ROTATION_SPEED;
      }
      if (ui.isKeyDown(GLFW_KEY_RIGHT) || ui.isKeyDown(GLFW_KEY_F))
      { // Right or 'F' key
         shipRotation += SHIP_ROTATION_SPEED;
      }

      // Set ship thrust based on up input
      if (ui.isKeyDown(GLFW_KEY_UP) || ui.isKeyDown(GLFW_KEY_E))
      { // Up or 'E' key
         shipThrust = 1.0;
      }
      else
         shipThrust = 0.0;

      // Fire Laser
      if (ui.isKeyDown(GLFW_KEY_SPACE) && laserCooldown <= 0.0)
      { // Space Key and laser cooldown check
         laserPositions.push_back(std::pair<double, double>{
             // ship position + offset along the ship's facing direction
             shipPosition.first + SHIP_RADIUS * std::cos(shipAngle),
             shipPosition.second + SHIP_RADIUS * std::sin(shipAngle)});
         laserVelocities.push_back(std::pair<double, double>{
             // velocity along the ship's facing direction + ship's current velocity
             std::cos(shipAngle) * LASER_SPEED + shipVelocity.first,
             std::sin(shipAngle) * LASER_SPEED + shipVelocity.second});
         // Reset laser cooldown
         laserHitboxColors.push_back(Draw::green);
         laserCooldown = 1 / FIRE_RATE;

         if (laserPositions.size() != laserVelocities.size() ||
             laserPositions.size() != laserHitboxColors.size())
         {
            throw std::runtime_error("Laser state vectors out of sync.");
         }
      }

      if (ui.isKeyDown(GLFW_KEY_H) && hitboxCooldown <= 0.0)
      {
         showHud = !showHud;
         // reset hitbox cooldown
         hitboxCooldown = 1.0 / HITBOX_TOGGLE_RATE;
      }
   }

   // 2. Update logic relies ONLY on delta time
   void update(double dt)
   {
      // Update angles using rotation (angular velocity)
      shipAngle += shipRotation * dt;
      rockAngle += ASTEROID_ROTATION_SPEED * dt;

      // Update ship velocity based on thrust and angle
      shipVelocity.first += shipThrust * std::cos(shipAngle) * dt;
      shipVelocity.second += shipThrust * std::sin(shipAngle) * dt;

      // Update ship position based on velocity
      shipPosition.first += shipVelocity.first * dt;
      shipPosition.second += shipVelocity.second * dt;

      // Clamp rock angle between 0 and 2*PI
      rockAngle = std::fmod(rockAngle, 2.0 * M_PI);
      if (rockAngle < 0.0)
         rockAngle += 2.0 * M_PI;
      if (rockAngle >= 2.0 * M_PI)
         rockAngle -= 2.0 * M_PI;

      // Clamp ship angle between 0 and 2*PI
      shipAngle = std::fmod(shipAngle, 2.0 * M_PI);
      if (shipAngle < 0.0)
         shipAngle += 2.0 * M_PI;
      if (shipAngle >= 2.0 * M_PI)
         shipAngle -= 2.0 * M_PI;

      // Update all laser positions based on their velocities
      for (size_t i = 0; i < laserPositions.size(); ++i)
      {
         laserPositions[i].first += laserVelocities[i].first * dt;
         laserPositions[i].second += laserVelocities[i].second * dt;
      }

      // Update rock positions based on their velocities
      for (size_t i = 0; i < rockPositions.size(); ++i)
      {
         rockPositions[i].first += rockVelocities[i].first * dt;
         rockPositions[i].second += rockVelocities[i].second * dt;
      }

      // Flag velocities of lasers that have moved off-screen preemptively (so they can be removed later)
      for (size_t i = 0; i < laserVelocities.size(); ++i)
      {
         // remove laser vector positions at iterator.begin + i if off screen
         if (std::abs(laserPositions[i].first) > 1.0 ||
             std::abs(laserPositions[i].second) > 1.0)
         {
            laserPositions.erase(laserPositions.begin() + i);
            laserVelocities.erase(laserVelocities.begin() + i);
            laserHitboxColors.erase(laserHitboxColors.begin() + i);
            --i;
         }
      }

      // Update laser cooldown
      laserCooldown -= dt;
      if (laserCooldown < 0.0)
         laserCooldown = 0.0;

      hitboxCooldown -= dt;
      if (hitboxCooldown < 0.0)
         hitboxCooldown = 0.0;

      // update fps "FPS: ###.#" (6 space buffer for number)
      hudText = "FPS:" + std::format("{:6.1f}\n", 1.0 / dt);
      hudText += "Game State: " + std::string(shipHitboxColor == Draw::red ? "Game Over" : "Playing") + '\n';

      wrap();
      checkCollisions();
      destroyZombies();
   }

   // 3. Output relies ONLY on the Draw class
   void output(Draw &draw)
   {
      draw.ship(shipPosition, shipAngle, shipRadius, draw.white, true);
      if (showHud)
         draw.circle(shipPosition, shipRadius, shipHitboxColor);

      for (size_t i = 0; i < laserPositions.size(); ++i)
      {
         draw.dot(laserPositions[i], 0.5, draw.white);
         if (showHud)
            draw.circle(laserPositions[i], 0.02, laserHitboxColors[i]);
      }
      // draw all rocks
      for (size_t i = 0; i < rockPositions.size(); ++i)
      {
         draw.rock(rockPositions[i], rockAngle, rockRadii[i], draw.gray, rockTypes[i]);
         if (showHud)
            draw.circle(rockPositions[i], rockRadii[i], rockHitboxColors[i]);
      }

      if (showHud)
         draw.text({-0.95, 0.95}, hudText, 0.12, draw.white);
   }

private:
   // Students define their state variables here (x, y, velocities, etc.)

   // rocks
   double rockAngle;
   std::vector<std::pair<double, double>> rockPositions;
   std::vector<std::pair<double, double>> rockVelocities;
   std::vector<Draw::rocks> rockTypes;
   std::vector<double> rockRadii;
   std::vector<std::tuple<double, double, double, double>> rockHitboxColors;

   // ship
   double shipAngle;
   double shipRotation;
   double shipRadius{0.1};
   std::pair<double, double> shipPosition{0.0, 0.0};
   std::pair<double, double> shipVelocity{0.0, 0.0};
   std::tuple<double, double, double, double> shipHitboxColor{Draw::green};
   double shipThrust{0.0};

   // laser
   std::vector<std::pair<double, double>> laserPositions;
   std::vector<std::pair<double, double>> laserVelocities;
   std::string hudText;
   double laserCooldown{0.0};
   std::vector<std::tuple<double, double, double, double>> laserHitboxColors;

   // hitboxes
   bool showHud{false};
   double hitboxCooldown{0.0};

   // private methods
   void wrap()
   {
      // ship
      if (shipPosition.first > 1.0 + SCREEN_BUFFER)
         shipPosition.first = -1.0 - SCREEN_BUFFER;
      else if (shipPosition.first < -1.0 - SCREEN_BUFFER)
         shipPosition.first = 1.0 + SCREEN_BUFFER;
      if (shipPosition.second > 1.0 + SCREEN_BUFFER)
         shipPosition.second = -1.0 - SCREEN_BUFFER;
      else if (shipPosition.second < -1.0 - SCREEN_BUFFER)
         shipPosition.second = 1.0 + SCREEN_BUFFER;

      // wrap all rocks
      for (auto &pos : rockPositions)
      {
         if (pos.first > 1.0 + SCREEN_BUFFER)
            pos.first = -1.0 - SCREEN_BUFFER;
         else if (pos.first < -1.0 - SCREEN_BUFFER)
            pos.first = 1.0 + SCREEN_BUFFER;
         if (pos.second > 1.0 + SCREEN_BUFFER)
            pos.second = -1.0 - SCREEN_BUFFER;
         else if (pos.second < -1.0 - SCREEN_BUFFER)
            pos.second = 1.0 + SCREEN_BUFFER;
      }

      // Don't wrap lasers
   }

   void checkCollisions()
   {
      // Between rocks and lasers (lasers win)
      for (size_t i = 0; i < rockPositions.size(); ++i)
      {
         for (size_t j = 0; j < laserPositions.size(); ++j)
         {
            if (collision(rockPositions[i], rockRadii[i], laserPositions[j], 0.02))
            {
               // Flag rock and laser for removal (lasers win)
               rockHitboxColors[i] = Draw::red;
               laserHitboxColors[j] = Draw::red;
               splitRock(i);
            }
         }
      }

      // Between rocks and the ship (rocks win)
      for (size_t i = 0; i < rockPositions.size(); ++i)
      {
         if (collision(rockPositions[i], rockRadii[i], shipPosition, shipRadius))
         {
            // Flag rock and ship for removal (rocks win)
            shipHitboxColor = Draw::red;
            rockHitboxColors[i] = Draw::red;
            splitRock(i);
         }
      }

      // Between rocks and ship thruster (thruster wins)
   }

   bool collision(const std::pair<double, double> &aPos, double aRadius,
                  const std::pair<double, double> &bPos, double bRadius) const
   {
      const double dx = aPos.first - bPos.first;
      const double dy = aPos.second - bPos.second;
      const double distanceSquared = dx * dx + dy * dy;
      const double radiusSum = aRadius + bRadius;
      const bool collided = distanceSquared <= radiusSum * radiusSum;
      return collided;
   }

   void destroyZombies()
   {
      // Remove rocks that have been flagged for removal (red hitbox)
      {
         size_t initialRockCount = rockPositions.size();
         size_t i = 0;
         while (i < rockPositions.size())
         {
            if (rockHitboxColors[i] == Draw::red)
            {
               rockPositions.erase(rockPositions.begin() + i);
               rockVelocities.erase(rockVelocities.begin() + i);
               rockTypes.erase(rockTypes.begin() + i);
               rockRadii.erase(rockRadii.begin() + i);
               rockHitboxColors.erase(rockHitboxColors.begin() + i);
            }
            else
            {
               ++i;
            }
         }
      }

      // Remove lasers that have been flagged for removal (red hitbox)
      {
         size_t i = 0;
         while (i < laserPositions.size())
         {
            if (laserHitboxColors[i] == Draw::red)
            {
               laserPositions.erase(laserPositions.begin() + i);
               laserVelocities.erase(laserVelocities.begin() + i);
               laserHitboxColors.erase(laserHitboxColors.begin() + i);
            }
            else
            {
               ++i;
            }
         }
      }
   }

   void splitRock(size_t index)
   {
      // Split the rock into smaller rocks if possible
      if (rockTypes[index] != Draw::SMALL_ROCK)
      {
         // Create two smaller rocks
         // Transform first rock
         rockTypes[index] = static_cast<Draw::rocks>(rockTypes[index] - 1);
         rockRadii[index] *= 0.5;
         rockVelocities[index].first = -rockVelocities[index].first;
         rockVelocities[index].second = -rockVelocities[index].second;
         rockHitboxColors[index] = Draw::green;
         
         // Create identical second rock with negative velocity
         rockPositions.push_back(rockPositions[index]);
         rockVelocities.push_back({-rockVelocities[index].first, -rockVelocities[index].second});
         rockTypes.push_back(rockTypes[index]);
         rockRadii.push_back(rockRadii[index]);
         rockHitboxColors.push_back(Draw::green);
      }
      else
      {
         // Small rocks are destroyed completely
      }
   }
};
