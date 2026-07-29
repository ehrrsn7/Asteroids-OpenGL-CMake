#pragma once
#include "window.hpp"

class Game
{
public:
   Game()
   {
      // Construct game objects here
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
   void update(float dt)
   {
      // Update object positions, check collisions, etc.
   }

   // 3. Output relies ONLY on the Draw class
   void output(Draw &draw)
   {
      draw.ship(0.0f, 0.0f, 0.0f, 1.0f);
      draw.asteroid(0.5f, 0.5f, 0.785f, 0.5f);
   }

private:
   // Students define their state variables here (x, y, velocities, etc.)
};
