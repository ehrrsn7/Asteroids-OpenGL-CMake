#pragma once

#include <cmath> // Make sure this is included for fmod if needed, or just use standard math
#include "window.hpp"

class Game
{
public:
   Game()
   {
      // Construct game objects here
      // shipAngle = 2.785;
      rockAngle = (double)M_PI * 1.5;
   }

   ~Game()
   {
   }

   // 1. Input completely encapsulated by the UI class
   void input(UI &ui)
   {
      if (ui.isKeyDown(GLFW_KEY_ESCAPE))
      {
         ui.closeWindow();
      }
   }

   // 2. Update logic relies ONLY on delta time
   void update(double dt)
   {
      // constexpr double SHIP_ROTATION_SPEED = .5;
      constexpr double ASTEROID_ROTATION_SPEED = M_PI * 1.25;

      // // shipAngle += SHIP_ROTATION_SPEED * dt;
      rockAngle += ASTEROID_ROTATION_SPEED * dt;

      constexpr double TWO_PI = 2.0 * static_cast<double>(M_PI);

      // wrap angles to keep the rotation stable across long-running sessions
      rockAngle = std::fmod(rockAngle, TWO_PI);
      if (rockAngle < 0.0) {
         rockAngle += TWO_PI;
      }
   }

   // 3. Output relies ONLY on the Draw class
   void output(Draw &draw)
   {
      // std::cout << shipAngle << std::endl;
      // // draw.ship(0.0, 0.0, shipAngle, 1.0);
      draw.rock(0.5, 0.5, rockAngle, 0.1);
      draw.rock(-0.5, 0.5, rockAngle, 0.1, Draw::SMALL_ROCK);
      draw.rock(0.5, -0.5, rockAngle, 0.1, Draw::LARGE_ROCK);
   }

private:
   // Students define their state variables here (x, y, velocities, etc.)
   double rockAngle;
   // double shipAngle;
};
