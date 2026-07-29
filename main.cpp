/**********************************************************
 * Program:
 *
 **********************************************************/

#include "window.hpp" // instructor defined
#include "game.hpp" // student defined

int main()
{
   // Initialize the interfaces
   Window window(800, 600, "Asteroids");
   UI ui(window);
   Draw gui; //#test
   Game game;

   // --- MAIN ENGINE LOOP ---
   run(ui, game, gui);

   return 0;
}
