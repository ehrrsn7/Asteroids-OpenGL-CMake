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
   double updateDeltaTime();

   GLFWwindow *getNativeWindow() const { return m_window; }

private:
   GLFWwindow *m_window{nullptr};
   double m_lastFrameTime{0.0f};
   double m_deltaTime{0.0f};
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
   double updateDeltaTime() { return window.updateDeltaTime(); }
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
   enum rocks { SMALL_ROCK, NORMAL_ROCK, LARGE_ROCK }; // match rockFiles[] below
   void rock(double x, double y, double angle, double scale, rocks which = NORMAL_ROCK);
   void rock(std::pair<double, double> position, double angle, double scale, rocks which = NORMAL_ROCK) {
      rock(position.first, position.second, angle, scale, which);
   }

   void ship(double x, double y, double angle, double scale = 0.1);
   void ship(std::pair<double, double> position, double angle, double scale = 0.1) {
      ship(position.first, position.second, angle, scale);
   }

   void dot(double x, double y, double scale);
   void dot(std::pair<double, double> position, double scale) {
      dot(position.first, position.second, scale);
   }
   
   void setAspectRatio(float aspect);
   Shader* getShader() { return m_shader; }

private:
   Shader *m_shader;
   float m_aspect{0.75f};

   // OpenGL IDs for the Asteroid
   GLuint m_rockVAO[3], m_rockVBO[3], m_rockEBO[3];
   unsigned int m_rockTriCount[3];
   static constexpr double rockFallbackVerts[] = {
      0.0, 0.2, 0.0, 0.6, 0.6, 0.6,
      0.17, 0.1, 0.0, 0.6, 0.6, 0.6,
      0.17, -0.1, 0.0, 0.6, 0.6, 0.6,
      0.0, -0.2, 0.0, 0.6, 0.6, 0.6,
      -0.17, -0.1, 0.0, 0.6, 0.6, 0.6,
      -0.17, 0.1, 0.0, 0.6, 0.6, 0.6};
   static constexpr unsigned int rockFallbackInds[] = {0, 1, 5, 1, 2, 5, 2, 3, 4, 2, 4, 5};

   std::string rockFiles[3] = {
      "assets/shapes/rockSmall.txt", 
      "assets/shapes/rockNormal.txt", 
      "assets/shapes/rockLarge.txt"
   };

   // OpenGL IDs for simple dot rendering
   GLuint m_dotVAO, m_dotVBO, m_dotEBO;
   unsigned int m_dotTriCount{0};

   // OpenGL IDs for the Ship
   GLuint m_shipVAO, m_shipVBO, m_shipEBO;
   unsigned int m_shipLineCount{0}, m_shipTriCount{0};
   // Draw class
   GLsizei m_shipVertexCount = 0;
   static constexpr double shipFallbackVerts[] = {
      // Line A
      -1.0f/25.0f, 22.0f/25.0f, 0.0f, 1.0f, 1.0f, 1.0f,
      1.0f/25.0f, 22.0f/25.0f, 0.0f, 1.0f, 1.0f, 1.0f,
      // Line A2
      0.0f/25.0f, 16.0f/25.0f, 0.0f, 1.0f, 1.0f, 1.0f,
      0.0f/25.0f, 22.0f/25.0f, 0.0f, 1.0f, 1.0f, 1.0f,
      // Base Polygon (Triangulated)
      -15.0f/25.0f,   0.0f/25.0f, 0.0f, 0.5f, 0.5f, 0.5f,
      15.0f/25.0f,   0.0f/25.0f, 0.0f, 0.5f, 0.5f, 0.5f,
      15.0f/25.0f, -10.0f/25.0f, 0.0f, 0.5f, 0.5f, 0.5f,
      -15.0f/25.0f, -10.0f/25.0f, 0.0f, 0.5f, 0.5f, 0.5f 
   };
   static constexpr unsigned int shipFallbackInds[] = {
      0, 1, // Line 1
      2, 3, // Line 2
      4, 5, 6, 4, 6, 7 // Poly
   };

   // init functions to set up the vertex data
   void setupRockVertices();
   void setupShipVertices();
   void setupDotVertices();
   void loadShape(std::vector<double>& vertices, std::vector<unsigned int>& indices, std::string path);
   void pushVertex(std::vector<double>& verts, double x, double y);
   void centerVertices(std::vector<double>& vertices);
};

int run(UI &ui, Game &game, Draw &gui); // Forward declaration of the run function
