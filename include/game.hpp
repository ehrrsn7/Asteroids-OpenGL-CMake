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
   static constexpr double SHIP_ROTATION_SPEED = -M_PI * 2.0;      // radians per second
   static constexpr double SHIP_RADIUS = 0.05;                     // units
   static constexpr double ASTEROID_ROTATION_SPEED = -M_PI * 0.75; // radians per second
   static constexpr double LASER_SPEED = 1.5;                      // units per second
   static constexpr double FIRE_RATE = 6.0;                        // shots per second
   static constexpr double ROCK_SPEED = 0.3;                       // units per second
   static constexpr double SCREEN_BUFFER = 0.035;
   static constexpr double UI_TOGGLE_RATE = 5.0; // toggles per second
   static constexpr double LASER_SIZE = 0.065; // units long

   Game()
   {
      resetAllObjects();
      if (FIRE_RATE <= 0.0)
         throw std::runtime_error("Invalid fire rate.");
   }

   ~Game() = default;

   void resetAllObjects()
   {
      // clear all vectors
      rockPositions.clear();
      rockVelocities.clear();
      rockTypes.clear();
      rockRadii.clear();
      rockHitboxColors.clear();
      laserPositions.clear();
      laserAngles.clear();
      laserVelocities.clear();
      laserHitboxColors.clear();

      shipPosition.first = 0.0;
      shipPosition.second = 0.0;
      shipVelocity.first = 0.0;
      shipVelocity.second = 0.0;
      shipHitboxColor = Draw::green;

      // initialize all objects
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
   } 

   // 1. Input completely encapsulated by the UI class
   void input(UI &ui)
   {
      // Close window
      if (ui.isKeyDown(GLFW_KEY_C) && ui.isKeyDown(GLFW_KEY_LEFT_CONTROL))
      { // Ctrl+C
         ui.closeWindow();
      }

      // Pause Menu
      if (ui.isKeyDown(GLFW_KEY_COMMA) && ui.isKeyDown(GLFW_KEY_LEFT_CONTROL))
      { // Escape
         if (menuIndex >= 0 && uiCooldown <= 0.0)
         { // menu is open
            uiCooldown = 1.0 / UI_TOGGLE_RATE;
            // menu action
         }
         else if (shipHitboxColor != Draw::red)
         { // above if and ship is alive
            // pause game
            menuIndex = 0; // highlight 'resume'
         }
      }

      // Set ship rotation to sum of left/right inputs
      shipRotation = 0.0;
      if (ui.isKeyDown(GLFW_KEY_LEFT) || ui.isKeyDown(GLFW_KEY_S))
      { // Left or 'S' key
         if (menuIndex >= 0 && uiCooldown <= 0.0)
         { // menu is open
            uiCooldown = 1.0 / UI_TOGGLE_RATE;
            // menu action

         }
         else if (shipHitboxColor != Draw::red)
         { // above if and ship is alive
            shipRotation -= SHIP_ROTATION_SPEED;
         }
      }
      if (ui.isKeyDown(GLFW_KEY_RIGHT) || ui.isKeyDown(GLFW_KEY_F))
      { // Right or 'F' key
         if (menuIndex >= 0 && uiCooldown <= 0.0)
         { // menu is open
            uiCooldown = 1.0 / UI_TOGGLE_RATE;
            // menu action
            
         }
         else if (shipHitboxColor != Draw::red)
         { // above if and ship is alive
            shipRotation += SHIP_ROTATION_SPEED;
         }
      }

      // Set ship thrust based on up input
      if (ui.isKeyDown(GLFW_KEY_UP) || ui.isKeyDown(GLFW_KEY_E))
      { // Up or 'E' key
         if (menuIndex >= 0 && uiCooldown <= 0.0)
         { // menu is open
            uiCooldown = 1.0 / UI_TOGGLE_RATE;
            // menu action
            menuIndex = (menuIndex - 1 + menuOptions.size()) % menuOptions.size();
         }
         else if (shipHitboxColor != Draw::red)
         { // above if and ship is alive
            shipThrust = 1.0;
         }
      }
      else
         shipThrust = 0.0;

      if (ui.isKeyDown(GLFW_KEY_DOWN) || ui.isKeyDown(GLFW_KEY_D))
      { // Down or 'D' key
         if (menuIndex >= 0 && uiCooldown <= 0.0)
         { // menu is open
            uiCooldown = 1.0 / UI_TOGGLE_RATE;
            // menu action
            menuIndex = (menuIndex + 1) % menuOptions.size();
         }
         else if (shipHitboxColor != Draw::red)
         { // above if and ship is alive
            // no game action for down key
         }
      }

      if (ui.isKeyDown(GLFW_KEY_SPACE) || ui.isKeyDown(GLFW_KEY_ENTER))
      { // Space Key and laser cooldown check
         if (menuIndex >= 0 && uiCooldown <= 0.0)
         {
            // menu actions
            int unpaused = -1; // Represents "inactive" menu

            // Find iterators safely
            auto resumeIt  = std::find(menuOptions.begin(), menuOptions.end(), "Resume");
            auto playIt    = std::find(menuOptions.begin(), menuOptions.end(), "Play");
            auto restartIt = std::find(menuOptions.begin(), menuOptions.end(), "Restart");
            auto quitIt    = std::find(menuOptions.begin(), menuOptions.end(), "Quit");
            auto currentIt = menuOptions.begin() + menuIndex;

            if (menuIndex == unpaused)
            {
               throw std::runtime_error("Menu context action when menu not active");               
            }

            // 1. Check "Resume" or "Play" (Verify iterator is valid BEFORE checking match)
            if ((resumeIt != menuOptions.end() && currentIt == resumeIt) || 
               (playIt != menuOptions.end() && currentIt == playIt))
            {
               if (rockPositions.size() > 0  && shipHitboxColor == Draw::green) 
               { // The game is ready to play
                  menuIndex = unpaused;
               }
               else if (uiCooldown <= 0.0)
               { // the game isn't even ready - where are the rocks? has the ship been hit?
                  // reset ui cooldown
                  uiCooldown = 1.0 / UI_TOGGLE_RATE;
                  std::cout << "Please press restart to reset the asteroids and continue playing.\n";
               }
               return;
            }

            // High Scores
            // Options

            // 2. Check "Restart"
            bool restartEnabled = rockPositions.size() <= 0 || shipHitboxColor != Draw::green;
            if (restartIt != menuOptions.end() && currentIt == restartIt && restartEnabled)
            {
               menuIndex = unpaused;
               resetAllObjects();
               return;
            }
            else if (uiCooldown <= 0.0)
            { // the game isn't even ready - where are the rocks? has the ship been hit?
               // reset ui cooldown
               uiCooldown = 1.0 / UI_TOGGLE_RATE;
               std::cout << "The game is already reset and ready to start playing.\n";
            }

            // 3. Check "Quit"
            if (quitIt != menuOptions.end() && currentIt == quitIt)
            {
               ui.closeWindow();
               return;
            }
         }
         // Fire Laser
         else if (shipHitboxColor != Draw::red && laserCooldown <= 0.0)
         { // ship is alive and laser cooldown expired
            laserPositions.push_back(std::pair<double, double>{
                // ship position + offset along the ship's facing direction
                shipPosition.first + SHIP_RADIUS * std::cos(shipAngle),
                shipPosition.second + SHIP_RADIUS * std::sin(shipAngle)});
            laserVelocities.push_back(std::pair<double, double>{
                // velocity along the ship's facing direction + ship's current velocity
                std::cos(shipAngle) * LASER_SPEED + shipVelocity.first,
                std::sin(shipAngle) * LASER_SPEED + shipVelocity.second});
            laserAngles.push_back(shipAngle);
            // Reset laser cooldown
            laserHitboxColors.push_back(Draw::green);
            laserCooldown = 1 / FIRE_RATE;
   
            if (laserPositions.size() != laserVelocities.size() ||
               laserPositions.size() != laserHitboxColors.size() || 
               laserPositions.size() != laserAngles.size())
            {
               throw std::runtime_error("Laser state vectors out of sync.");
            }
         }
      }

      if (ui.isKeyDown(GLFW_KEY_H) && uiCooldown <= 0.0)
      {
         showHud = !showHud;
         // reset ui cooldown
         uiCooldown = 1.0 / UI_TOGGLE_RATE;
      }
   }

   // 2. Update logic relies ONLY on delta time
   void update(double dt)
   {
      // Update angles using rotation (angular velocity)
      rockAngle += ASTEROID_ROTATION_SPEED * dt;

      // update ship stuff
      if (menuIndex < 0)
      { // game is un paused
         shipAngle += shipRotation * dt;
         
         // Update ship velocity based on thrust and angle
         shipVelocity.first += shipThrust * std::cos(shipAngle) * dt;
         shipVelocity.second += shipThrust * std::sin(shipAngle) * dt;
   
         // Update ship position based on velocity
         shipPosition.first += shipVelocity.first * dt;
         shipPosition.second += shipVelocity.second * dt;
      }

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
            laserAngles.erase(laserAngles.begin() + i);
            --i;
         }
      }

      // Update laser cooldown
      laserCooldown -= dt;
      if (laserCooldown < 0.0)
         laserCooldown = 0.0;

      uiCooldown -= dt;
      if (uiCooldown < 0.0)
         uiCooldown = 0.0;

      // update fps "FPS: ###.#" (6 space buffer for number)
      hudText = "FPS:" + std::format("{:6.1f}\n", 1.0 / dt);
      hudText += std::format("Game State: {}\n", getGameState());

      wrap();
      checkCollisions();
      destroyZombies();
   }

   // 3. Output relies ONLY on the Draw class
   void output(Draw &draw)
   {
      draw.ship(shipPosition, shipAngle, shipRadius, shipHitboxColor == draw.red ? draw.red : draw.white, true);
      if (showHud)
         draw.circle(shipPosition, shipRadius, shipHitboxColor);

      for (size_t i = 0; i < laserPositions.size(); ++i)
      {
         draw.rectangle(laserPositions[i], {LASER_SIZE, LASER_SIZE * 0.2}, laserAngles[i], draw.white);
         if (showHud)
            draw.circle(laserPositions[i], LASER_SIZE * 0.5, laserHitboxColors[i]);
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

      // show menu
      if (menuIndex >= 0)
      {
         draw.text({0.0, 0.3}, getGameState(), 0.12, draw.white, draw.CENTER);

         // play button
         bool playEnabled = rockPositions.size() > 0 && shipHitboxColor == Draw::green;
         draw.text({0.0, -0.3}, 
            true ? "Resume" : "Play", 
            0.12, 
            playEnabled ? draw.white : draw.gray, 
            draw.CENTER, 
            true ? draw.UNDERLINE : draw.ITALIC);
         if (menuIndex == 0) draw.rectangle({0.0, -0.35}, {0.5, 0.15}, 0.0, {1.0, 1.0, 1.0, 0.1});

         // restart button
         bool restartEnabled = rockPositions.size() <= 0 || shipHitboxColor != Draw::green;
         draw.text({0.0, -0.5}, "Restart",
            0.12, 
            restartEnabled ? draw.white : draw.gray, 
            draw.CENTER, 
            true ? draw.UNDERLINE : draw.ITALIC);
         if (menuIndex == 1) draw.rectangle({0.0, -0.55}, {0.5, 0.15}, 0.0, {1.0, 1.0, 1.0, 0.1});

         // quit button
         bool quitEnabled = true;
         draw.text({0.0, -0.7}, "Quit",
            0.12, 
            quitEnabled ? draw.white : draw.gray, 
            draw.CENTER, 
            true ? draw.UNDERLINE : draw.ITALIC);
         if (menuIndex == 2) draw.rectangle({0.0, -0.75}, {0.5, 0.15}, 0.0, {1.0, 1.0, 1.0, 0.1});
      }
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
   std::vector<double> laserAngles;
   std::string hudText;
   double laserCooldown{0.0};
   std::vector<std::tuple<double, double, double, double>> laserHitboxColors;

   // game state ui
   std::vector<std::string> menuOptions{
      "Play",
      "Restart", 
      // "High Scores",
      // "Options",
      "Quit"
   };
   int menuIndex{0}; // selected menu option (-1 means resume)
   bool showHud{false};
   double uiCooldown{0.0};

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
               std::cout << rockPositions.size() << std::endl;

               if (rockPositions.size() <= 1)
               { // game won
                  menuIndex = 0; // pause the game
               }
            }
         }
      }

      // Between rocks and the ship (rocks win)
      for (size_t i = 0; i < rockPositions.size(); ++i)
      {
         if (collision(rockPositions[i], rockRadii[i], shipPosition, shipRadius))
         {
            if (shipHitboxColor == Draw::red || menuIndex >= 0) 
            {
               // Ship is already destroyed and/or pause menu is toggled on
               std::pair<double, double> rockV0(rockVelocities[i]);
               rockVelocities[i].first += shipVelocity.first;
               rockVelocities[i].second += shipVelocity.second;
               shipVelocity.first *= 0.75;
               shipVelocity.second *= 0.75;
               shipVelocity.first -= rockV0.first * 0.35; // needs to total > 1 to prevent 'strong/weak' force
               shipVelocity.second -= rockV0.second * 0.35;
               menuIndex = 0; // pause the game
            }
            else
            {
               // Flag rock and ship for removal (rocks win)
               shipHitboxColor = Draw::red;
               menuIndex = 0; // pause the game even if rocks are gone
               rockHitboxColors[i] = Draw::red;
               splitRock(i);
            }
         }
      }

      // Between rocks and ship thruster (thruster wins)
   }

   bool collision(const std::pair<double, double> &aPos, double aRadius,
                  const std::pair<double, double> &bPos, double bRadius) const
   {
      const double buffer = -0.5; // % radius
      const double dx = aPos.first - bPos.first;
      const double dy = aPos.second - bPos.second;
      const double distanceSquared = dx * dx + dy * dy;
      const double radiusSum = aRadius + bRadius;
      const bool collided = distanceSquared * (1 - buffer) <= radiusSum * radiusSum;
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
               laserAngles.erase(laserAngles.begin() + i);
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

   std::string getGameState()
   {
      std::string gameState;
      switch ((int)menuIndex)
      {
         case -1: // unpaused, none selected
            gameState += "Playing";
            break;
         case 0: // paused, resume selected
         case 1: // restart
         case 2: // quit
            gameState += "Paused";
            break;
      }
      if (menuIndex >= 0 && shipHitboxColor == Draw::red) gameState = "Game Over";
      if (menuIndex >= 0 && rockPositions.size() <= 0 && shipHitboxColor != Draw::red) gameState = "Game Won";

      return gameState;
   } 
};
