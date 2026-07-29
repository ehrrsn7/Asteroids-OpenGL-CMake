/**********************************************************
 * Program:
 *
 **********************************************************/

#pragma once

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>

// Forward declaration of the Game class to avoid circular dependency
// Do not #include "game.hpp"
class Game;

class Window
{
public:
   Window(int width, int height, const std::string &title);
   ~Window();

   bool shouldClose() const;
   void swapBuffersAndPollEvents();

   // Time delta management
   float updateDeltaTime();

   GLFWwindow *getNativeWindow() const { return m_window; }

private:
   GLFWwindow *m_window{nullptr};
   float m_lastFrameTime{0.0f};
   float m_deltaTime{0.0f};
};

class UI
{ // user input interface
public:
   UI(Window &window) : window(window) {}
   // UI(int width, int height, const std::string &title);

   // Simple wrapper for student input checks
   bool isKeyDown(int key) const;

   // Pass throughs
   Window getWindow() { return window; }
   GLFWwindow *getNativeWindow() { return window.getNativeWindow(); }
   float getDeltaTime() { return window.updateDeltaTime(); }
   float updateDeltaTime() { return window.updateDeltaTime(); }
   void swapBuffersAndPollEvents() { window.swapBuffersAndPollEvents(); }
   bool shouldClose() const { return window.shouldClose(); }

   // Allows the student to easily quit without needing GLFWwindow pointers
   void closeWindow();

private:
   Window &window;
};

class Shader
{
public:
   GLuint ID{0};

   Shader(const char *vertexPath, const char *fragmentPath);
   ~Shader();

   void use() const;

   // Simple helpers for our 2D math
   void setFloat(const std::string &name, float value) const;
   void setVec2(const std::string &name, float x, float y) const;

private:
   void checkCompileErrors(GLuint shader, const std::string &type);
};

class Draw
{ // graphical user interface
public:
   // Pass a reference to the shader so Draw knows where to send the uniforms
   Draw(Shader *shader);
   Draw();
   ~Draw();

   // High-level drawing functions
   void asteroid(float x, float y, float angle, float scale);
   void ship(float x, float y, float angle, float scale);

   Shader* getShader() { return m_shader; }

private:
   Shader *m_shader;

   // OpenGL IDs for the Asteroid
   GLuint m_asteroidVAO, m_asteroidVBO, m_asteroidEBO;

   // OpenGL IDs for the Ship
   GLuint m_shipVAO, m_shipVBO, m_shipEBO;

   // init functions to set up the vertex data
   void setupAsteroidVerticies();
   void setupShipVerticies();
};

int run(UI &ui, Game &game, Draw &gui); // Forward declaration of the run function
