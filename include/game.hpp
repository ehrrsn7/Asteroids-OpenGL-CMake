#pragma once

#include <algorithm>
#include <cmath> // Make sure this is included for fmod if needed, or just use standard math
#include <vector>
#include "window.hpp"

class Game
{
public:
   static constexpr double SHIP_ROTATION_SPEED = -2.5;
   static constexpr double SHIP_RADIUS = 0.1;

   Game()
   {
      // Construct game objects here
      rockAngle = (double)M_PI * 1.5;
      shipAngle = (double)M_PI * 0;
   }

   ~Game()
   {
   }

   // 1. Input completely encapsulated by the UI class
   void input(UI &ui)
   {
      if (ui.isKeyDown(GLFW_KEY_ESCAPE) ||
         (ui.isKeyDown(GLFW_KEY_C) && ui.isKeyDown(GLFW_KEY_LEFT_CONTROL))
      ) {
         ui.closeWindow();
      }

      shipRotation = 0.0;
      if (ui.isKeyDown(GLFW_KEY_LEFT) || ui.isKeyDown(GLFW_KEY_S))
      {
         shipRotation -= SHIP_ROTATION_SPEED;
      }
      if (ui.isKeyDown(GLFW_KEY_RIGHT) || ui.isKeyDown(GLFW_KEY_F))
      {
         shipRotation += SHIP_ROTATION_SPEED;
      }

      if (ui.isKeyDown(GLFW_KEY_UP) || ui.isKeyDown(GLFW_KEY_E))
      {
         shipThrust = 1.0;
      } else shipThrust = 0.0;

      if (ui.isKeyDown(GLFW_KEY_SPACE) && laserCooldown <= 0.0)
      {
         std::pair<double, double> offset{
            SHIP_RADIUS * std::cos(shipAngle + (M_PI * 0.5)),
            SHIP_RADIUS * std::sin(shipAngle + (M_PI * 0.5))
         };
         std::pair<double, double> velocity{
            std::cos(shipAngle + (M_PI * 0.5)),
            std::sin(shipAngle + (M_PI * 0.5))
         };
         std::pair<double, double> position{ 
            shipPosition.first + offset.first,
            shipPosition.second + offset.second
         };
         laserPositions.push_back(position);
         laserVelocities.push_back(velocity);
         
         // handle cooldown
         laserCooldown = 0.2; // 5 shots per second
      }
   }

   // 2. Update logic relies ONLY on delta time
   void update(double dt)
   {
      constexpr double ASTEROID_ROTATION_SPEED = M_PI * 1.25;

      shipAngle += shipRotation * dt;
      rockAngle += ASTEROID_ROTATION_SPEED * dt;

      shipVelocity.first += shipThrust * std::cos(shipAngle + (M_PI * 0.5)) * dt;
      shipVelocity.second += shipThrust * std::sin(shipAngle + (M_PI * 0.5)) * dt;

      shipPosition.first += shipVelocity.first * dt;
      shipPosition.second += shipVelocity.second * dt;

      // wrap angles to keep the rotation stable across long-running sessions
      rockAngle = std::fmod(rockAngle, 2.0 * M_PI);
      if (rockAngle < 0.0) {
         rockAngle += 2.0 * M_PI;
      }

      for (size_t i = 0; i < laserPositions.size(); ++i)
      {
         laserPositions[i].first += laserVelocities[i].first * dt;
         laserPositions[i].second += laserVelocities[i].second * dt;
      }

      // Flag velocities of lasers that have moved off-screen preemptively
      for (size_t i = 0; i < laserVelocities.size(); ++i)
         if (std::abs(laserPositions[i].first) > 1.0 || std::abs(laserPositions[i].second) > 1.0)
            laserVelocities[i] = {0.0, 0.0};

      // Remove lasers that have moved off-screen (simple bounds check)
      laserPositions.erase(
         std::remove_if(
            laserPositions.begin(),
            laserPositions.end(),
            [](const std::pair<double, double> &laser) {
               return std::abs(laser.first) > 1.0 || std::abs(laser.second) > 1.0;
            }),
         laserPositions.end()
      );

      // Remove flagged velocities corresponding to lasers that have been removed
      laserVelocities.erase(
         std::remove_if(
            laserVelocities.begin(),
            laserVelocities.end(),
            [](const std::pair<double, double> &vel) {
               return vel.first == 0.0 && vel.second == 0.0;
            }),
         laserVelocities.end()
      );

      // Laser cooldown: maximum 5 per second
      laserCooldown -= dt;
      if (laserCooldown < 0.0) laserCooldown = 0.0;
      std::cout << "Laser cooldown: " << laserCooldown << std::endl;

      // wrap
      wrap();
   }

   // 3. Output relies ONLY on the Draw class
   void output(Draw &draw)
   {
      // std::cout << shipAngle << std::endl;
      draw.ship(shipPosition.first, shipPosition.second, shipAngle, 0.1);
      draw.rock(0.5, 0.5, rockAngle, 0.1);
      draw.rock(-0.5, 0.5, rockAngle, 0.1, Draw::SMALL_ROCK);
      draw.rock(0.5, -0.5, rockAngle, 0.1, Draw::LARGE_ROCK);
      for (const std::pair<double, double> &p : laserPositions) draw.dot(p, 0.5);
   }

private:
   // Students define their state variables here (x, y, velocities, etc.)
   double rockAngle;
   double shipAngle;
   double shipRotation;
   std::pair<double, double> shipPosition{0.0, 0.0};
   std::pair<double, double> shipVelocity{0.0, 0.0};
   std::vector<std::pair<double, double>> laserPositions;
   std::vector<std::pair<double, double>> laserVelocities;
   double laserCooldown{0.0};
   double shipThrust{0.0};

   void wrap() {
      // ship
      if (shipPosition.first > 1.0) shipPosition.first = -1.0;
      else if (shipPosition.first < -1.0) shipPosition.first = 1.0;
      if (shipPosition.second > 1.0) shipPosition.second = -1.0;
      else if (shipPosition.second < -1.0) shipPosition.second = 1.0;

      // rocks
      // (assuming rocks are static for now, otherwise implement similar wrapping)
   }
};
