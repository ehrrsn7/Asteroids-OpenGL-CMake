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
#include <utility>

// Forward declaration of the Game class to avoid circular dependency
// Do not #include "game.hpp"
class Game;

class Window
{
public:
   Window(std::pair<int, int> size, const std::string &title,
          std::tuple<double, double, double, double> bgColor = {0.0, 0.0, 0.0, 1.0});
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
   void setVec3(const std::string &name, float r, float g, float b) const;

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

   // High-level drawing functions with color
   enum rocks
   {
      SMALL_ROCK,
      NORMAL_ROCK,
      LARGE_ROCK
   }; // match rockFiles[] below
   void rock(std::pair<double, double> position, double angle, double scale,
             std::tuple<double, double, double, double> color = white, rocks which = NORMAL_ROCK);
   void ship(std::pair<double, double> position, double angle, double scale,
             std::tuple<double, double, double, double> color = white, bool thrusting = false);
   void dot(std::pair<double, double> position, double scale,
            std::tuple<double, double, double, double> color = white);
   void circle(std::pair<double, double> position, double radius,
               std::tuple<double, double, double, double> color = white);
   void rectangle(std::pair<double, double> position, std::pair<double, double> dimensions, double angle = 0.0,
                  std::tuple<double, double, double, double> color = white);
   enum align
   {
      LEFT,
      CENTER,
      RIGHT
   };
   enum style
   {
      NONE,
      UNDERLINE,
      BOLD,
      ITALIC
   };
   void text(std::pair<double, double> position, const std::string &text, double size,
             std::tuple<double, double, double, double> color = white,
             align which = LEFT, style textStyle = NONE);

   void setAspectRatio(float aspect);
   Shader *getShader() { return m_shader; }

   // colors
   static constexpr std::tuple<double, double, double, double> green{0.0, 1.0, 0.0, 1.0};
   static constexpr std::tuple<double, double, double, double> red{1.0, 0.0, 0.0, 1.0};
   static constexpr std::tuple<double, double, double, double> blue{0.0, 0.0, 1.0, 1.0};
   static constexpr std::tuple<double, double, double, double> white{1.0, 1.0, 1.0, 1.0};
   static constexpr std::tuple<double, double, double, double> black{0.0, 0.0, 0.0, 1.0};
   static constexpr std::tuple<double, double, double, double> yellow{1.0, 1.0, 0.0, 1.0};
   static constexpr std::tuple<double, double, double, double> cyan{0.0, 1.0, 1.0, 1.0};
   static constexpr std::tuple<double, double, double, double> magenta{1.0, 0.0, 1.0, 1.0};
   static constexpr std::tuple<double, double, double, double> orange{1.0, 0.5, 0.0, 1.0};
   static constexpr std::tuple<double, double, double, double> purple{0.5, 0.0, 0.5, 1.0};
   static constexpr std::tuple<double, double, double, double> brown{0.6, 0.3, 0.0, 1.0};
   static constexpr std::tuple<double, double, double, double> pink{1.0, 0.75, 0.8, 1.0};
   static constexpr std::tuple<double, double, double, double> gray{0.5, 0.5, 0.5, 1.0};
   static constexpr std::tuple<double, double, double, double> lightGray{0.75, 0.75, 0.75, 1.0};
   static constexpr std::tuple<double, double, double, double> darkGray{0.25, 0.25, 0.25, 1.0};
   static constexpr std::tuple<double, double, double, double> teal{0.0, 0.5, 0.5, 1.0};
   static constexpr std::tuple<double, double, double, double> navy{0.0, 0.0, 0.5, 1.0};
   std::tuple<double, double, double, double> randomColor()
   {
      return {static_cast<double>(rand()) / RAND_MAX,
              static_cast<double>(rand()) / RAND_MAX,
              static_cast<double>(rand()) / RAND_MAX,
              1.0};
   }

private:
   Shader *m_shader;
   float m_aspect{0.75f};

   // OpenGL IDs for the Ship
   void setupShipVertices();
   GLuint m_shipVAO, m_shipVBO, m_shipEBO;
   unsigned int m_shipLineCount{0}, m_shipTriCount{0};
   GLsizei m_shipVertexCount = 0;

   // OpenGL IDs for the Ship Thrust
   void setupShipThrustVertices();
   GLuint m_shipThrustVAO, m_shipThrustVBO, m_shipThrustEBO;
   unsigned int m_shipThrustTriCount{0};

   // OpenGL IDs for the Asteroid
   void setupRockVertices();
   GLuint m_rockVAO[3], m_rockVBO[3], m_rockEBO[3];
   unsigned int m_rockTriCount[3];

   std::string rockFiles[3] = {
       "assets/shapes/rockSmall.txt",
       "assets/shapes/rockNormal.txt",
       "assets/shapes/rockLarge.txt"};

   // OpenGL IDs for simple dot rendering
   void setupDotVertices();
   GLuint m_dotVAO, m_dotVBO, m_dotEBO;
   unsigned int m_dotTriCount{0};

   // OpenGL IDs for circle rendering
   void setupCircle();
   GLuint m_circleVAO, m_circleVBO, m_circleEBO;
   unsigned int m_circleTriCount{0};

   // OpenGL IDs for rectangle rendering
   void setupRectangle();
   GLuint m_rectangleVAO, m_rectangleVBO, m_rectangleEBO;
   unsigned int m_rectangleTriCount{0};

   // OpenGL IDs for text rendering
   void setupText();
   GLuint m_textVAO, m_textVBO, m_textEBO;
   unsigned int m_textTriCount{0};

   // init functions to set up the vertex data
   void loadShape(std::vector<double> &vertices, std::vector<unsigned int> &indices, std::string path);
   void pushVertex(std::vector<double> &verts, double x, double y);
   void centerVertices(std::vector<double> &vertices);
};

int run(UI &ui, Game &game, Draw &gui); // Forward declaration of the run function
