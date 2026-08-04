#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <filesystem>
#include <array>
#include <cctype>

#include "window.hpp"
#include "game.hpp" // Required so run() can call game.input, game.update, and game.output

Window::Window(std::pair<int, int> size, const std::string &title,
               std::tuple<double, double, double, double> bgColor)
{
   if (!glfwInit())
   {
      std::cerr << "Failed to initialize GLFW\n";
      return;
   }

   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

   glfwWindowHint(GLFW_SAMPLES, 4);

   m_window = glfwCreateWindow(size.first, size.second, title.c_str(), nullptr, nullptr);
   if (!m_window)
   {
      std::cerr << "Failed to create GLFW window\n";
      glfwTerminate();
      throw std::runtime_error("Failed to create GLFW window."); // Halts the program
   }

   // glEnable(GL_MULTISAMPLE); // !Fix halts the program

   glfwMakeContextCurrent(m_window);
   glfwSwapInterval(1); // Lock to vsync so frame timing (dt) is stable
   glfwSetWindowShouldClose(m_window, GLFW_FALSE);

   if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
   {
      std::cerr << "Failed to initialize GLAD\n";
      return;
   }

   int bufferWidth, bufferHeight;
   glfwGetFramebufferSize(m_window, &bufferWidth, &bufferHeight);
   glViewport(0, 0, bufferWidth, bufferHeight);

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   glClearColor(
       std::get<0>(bgColor),
       std::get<1>(bgColor),
       std::get<2>(bgColor),
       std::get<3>(bgColor));

   // 3. This stays exactly the same!
   glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow *, int w, int h)
                                  { glViewport(0, 0, w, h); });
}

Window::~Window()
{
   if (m_window)
   {
      glfwDestroyWindow(m_window);
   }
   glfwTerminate();
}

bool Window::shouldClose() const
{
   return glfwWindowShouldClose(m_window);
}

void Window::swapBuffersAndPollEvents()
{
   glfwSwapBuffers(m_window);
   glfwPollEvents();
}

double Window::updateDeltaTime()
{
   double currentFrame = glfwGetTime();
   m_deltaTime = currentFrame - m_lastFrameTime;
   m_lastFrameTime = currentFrame;
   return m_deltaTime;
}

// --- UI IMPLEMENTATION ---
bool UI::isKeyDown(int key) const
{
   return glfwGetKey(window.getNativeWindow(), key) == GLFW_PRESS;
}

void UI::closeWindow()
{
   glfwSetWindowShouldClose(window.getNativeWindow(), true);
}

// --- Shader IMPLEMENTATION ---
namespace
{
   std::string resolveAssetPath(const std::string &path)
   {
      namespace fs = std::filesystem;
      if (fs::exists(path))
         return path;

      const fs::path executablePath = fs::current_path();
      const fs::path candidate = executablePath / path;
      if (fs::exists(candidate))
         return candidate.string();

      const fs::path buildDir = executablePath.parent_path();
      const fs::path buildCandidate = buildDir / path;
      if (fs::exists(buildCandidate))
         return buildCandidate.string();

      return path;
   }

   struct GlyphCell
   {
      std::array<std::array<bool, 5>, 7> pixels{};
   };

   GlyphCell makeGlyph(char ch)
   {
      GlyphCell glyph{};
      const char upper = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));

      auto fillRows = [&](const std::array<const char *, 7> &rows)
      {
         for (int row = 0; row < 7; ++row)
         {
            for (int col = 0; col < 5; ++col)
            {
               glyph.pixels[row][col] = (rows[row][col] == '1');
            }
         }
      };

      switch (upper)
      {
      case 'A':
         fillRows({"01110", "10001", "10001", "11111", "10001", "10001", "10001"});
         break;
      case 'B':
         fillRows({"11110", "10001", "10001", "11110", "10001", "10001", "11110"});
         break;
      case 'C':
         fillRows({"01110", "10001", "10000", "10000", "10000", "10001", "01110"});
         break;
      case 'D':
         fillRows({"11110", "10001", "10001", "10001", "10001", "10001", "11110"});
         break;
      case 'E':
         fillRows({"11111", "10000", "10000", "11110", "10000", "10000", "11111"});
         break;
      case 'F':
         fillRows({"11111", "10000", "10000", "11110", "10000", "10000", "10000"});
         break;
      case 'G':
         fillRows({"01110", "10001", "10000", "10111", "10001", "10001", "01110"});
         break;
      case 'H':
         fillRows({"10001", "10001", "10001", "11111", "10001", "10001", "10001"});
         break;
      case 'I':
         fillRows({"01110", "00100", "00100", "00100", "00100", "00100", "01110"});
         break;
      case 'J':
         fillRows({"00111", "00010", "00010", "00010", "00010", "10010", "01100"});
         break;
      case 'K':
         fillRows({"10001", "10010", "10100", "11000", "10100", "10010", "10001"});
         break;
      case 'L':
         fillRows({"10000", "10000", "10000", "10000", "10000", "10000", "11111"});
         break;
      case 'M':
         fillRows({"10001", "11011", "10101", "10101", "10001", "10001", "10001"});
         break;
      case 'N':
         fillRows({"10001", "10001", "11001", "10101", "10011", "10001", "10001"});
         break;
      case 'O':
         fillRows({"01110", "10001", "10001", "10001", "10001", "10001", "01110"});
         break;
      case 'P':
         fillRows({"11110", "10001", "10001", "11110", "10000", "10000", "10000"});
         break;
      case 'Q':
         fillRows({"01110", "10001", "10001", "10001", "10101", "10010", "01101"});
         break;
      case 'R':
         fillRows({"11110", "10001", "10001", "11110", "10100", "10010", "10001"});
         break;
      case 'S':
         fillRows({"01111", "10000", "10000", "01110", "00001", "00001", "11110"});
         break;
      case 'T':
         fillRows({"11111", "00100", "00100", "00100", "00100", "00100", "00100"});
         break;
      case 'U':
         fillRows({"10001", "10001", "10001", "10001", "10001", "10001", "01110"});
         break;
      case 'V':
         fillRows({"10001", "10001", "10001", "10001", "10001", "01010", "00100"});
         break;
      case 'W':
         fillRows({"10001", "10001", "10001", "10101", "10101", "11011", "10001"});
         break;
      case 'X':
         fillRows({"10001", "10001", "01010", "00100", "01010", "10001", "10001"});
         break;
      case 'Y':
         fillRows({"10001", "10001", "01010", "00100", "00100", "00100", "00100"});
         break;
      case 'Z':
         fillRows({"11111", "00001", "00010", "00100", "01000", "10000", "11111"});
         break;
      case '0':
         fillRows({"01110", "10001", "10011", "10101", "11001", "10001", "01110"});
         break;
      case '1':
         fillRows({"00100", "01100", "00100", "00100", "00100", "00100", "01110"});
         break;
      case '2':
         fillRows({"01110", "10001", "00001", "00010", "00100", "01000", "11111"});
         break;
      case '3':
         fillRows({"11110", "00001", "00001", "01110", "00001", "00001", "11110"});
         break;
      case '4':
         fillRows({"00010", "00110", "01010", "10010", "11111", "00010", "00010"});
         break;
      case '5':
         fillRows({"11111", "10000", "10000", "11110", "00001", "00001", "11110"});
         break;
      case '6':
         fillRows({"01110", "10000", "10000", "11110", "10001", "10001", "01110"});
         break;
      case '7':
         fillRows({"11111", "00001", "00010", "00100", "01000", "01000", "01000"});
         break;
      case '8':
         fillRows({"01110", "10001", "10001", "01110", "10001", "10001", "01110"});
         break;
      case '9':
         fillRows({"01110", "10001", "10001", "01111", "00001", "00010", "01100"});
         break;
      case ':':
         fillRows({"00000", "00100", "00100", "00000", "00100", "00100", "00000"});
         break;
      case '.':
         fillRows({"00000", "00000", "00000", "00000", "00000", "00100", "00100"});
         break;
      case '-':
         fillRows({"00000", "00000", "00000", "11111", "00000", "00000", "00000"});
         break;
      case '+':
         fillRows({"00000", "00100", "00100", "11111", "00100", "00100", "00000"});
         break;
      case ' ':
         break;
      default:
         fillRows({"01110", "10001", "00110", "00000", "00100", "00000", "00100"});
         break;
      }

      return glyph;
   }

   void appendPixelQuad(std::vector<float> &vertices, std::vector<unsigned int> &indices, double x, double y, double size)
   {
      const size_t base = vertices.size() / 6;

      const std::array<std::pair<double, double>, 4> corners = {
          std::make_pair(x, y),
          std::make_pair(x + size, y),
          std::make_pair(x + size, y + size),
          std::make_pair(x, y + size)};

      for (const auto &corner : corners)
      {
         vertices.push_back(static_cast<float>(corner.first));
         vertices.push_back(static_cast<float>(corner.second));
         vertices.push_back(0.0f);
         vertices.push_back(1.0f);
         vertices.push_back(1.0f);
         vertices.push_back(1.0f);
      }

      indices.push_back(static_cast<unsigned int>(base));
      indices.push_back(static_cast<unsigned int>(base + 1));
      indices.push_back(static_cast<unsigned int>(base + 2));
      indices.push_back(static_cast<unsigned int>(base));
      indices.push_back(static_cast<unsigned int>(base + 2));
      indices.push_back(static_cast<unsigned int>(base + 3));
   }

   double appendTextGeometry(std::vector<float> &vertices, std::vector<unsigned int> &indices, const std::string &text, double cellSize)
   {
      const double charAdvance = cellSize * 6.0;
      double cursorX = 0.0;
      double cursorY = 0.0;
      const double lineAdvance = cellSize * 8.0;
      for (char ch : text)
      {
         if (ch == '\n')
         {
            cursorX = 0.0;          // Carriage return (reset horizontal position)
            cursorY -= lineAdvance; // Carriage return (move down to next line)
            continue;
         }

         const GlyphCell glyph = makeGlyph(ch);
         for (int row = 0; row < 7; ++row)
         {
            for (int col = 0; col < 5; ++col)
            {
               if (glyph.pixels[row][col])
               {
                  appendPixelQuad(vertices, indices, cursorX + col * cellSize, cursorY - row * cellSize, cellSize);
               }
            }
         }

         cursorX += charAdvance;
      }

      return cursorX * 0.72;
   }
} // namespace

Shader::Shader(const char *vertexPath, const char *fragmentPath)
{
   std::string vertexCode, fragmentCode;
   std::ifstream vShaderFile, fShaderFile;

   // Ensure ifstream objects can throw exceptions
   vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
   fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

   const std::string resolvedVertexPath = resolveAssetPath(vertexPath);
   const std::string resolvedFragmentPath = resolveAssetPath(fragmentPath);

   try
   {
      vShaderFile.open(resolvedVertexPath);
      fShaderFile.open(resolvedFragmentPath);

      std::stringstream vShaderStream, fShaderStream;
      vShaderStream << vShaderFile.rdbuf();
      fShaderStream << fShaderFile.rdbuf();

      vShaderFile.close();
      fShaderFile.close();

      vertexCode = vShaderStream.str();
      fragmentCode = fShaderStream.str();
   }
   catch (const std::ifstream::failure &e)
   {
      std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: "
                << resolvedVertexPath << " or " << resolvedFragmentPath << "\n";
   }

   const char *vShaderCode = vertexCode.c_str();
   const char *fShaderCode = fragmentCode.c_str();

   // 1. Compile Shaders
   GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
   glShaderSource(vertex, 1, &vShaderCode, NULL);
   glCompileShader(vertex);
   checkCompileErrors(vertex, "VERTEX");

   GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
   glShaderSource(fragment, 1, &fShaderCode, NULL);
   glCompileShader(fragment);
   checkCompileErrors(fragment, "FRAGMENT");

   // 2. Shader Program
   ID = glCreateProgram();
   glAttachShader(ID, vertex);
   glAttachShader(ID, fragment);
   glLinkProgram(ID);
   checkCompileErrors(ID, "PROGRAM");

   // 3. Delete shaders as they're linked into program now
   glDeleteShader(vertex);
   glDeleteShader(fragment);
}

Shader::~Shader()
{
   if (ID != 0)
   {
      glDeleteProgram(ID);
   }
}

void Shader::use() const
{
   glUseProgram(ID);
}

void Shader::setFloat(const std::string &name, float value) const
{
   glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setVec2(const std::string &name, float x, float y) const
{
   glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
}

void Shader::setVec3(const std::string &name, float x, float y, float z) const
{
   glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}

void Shader::checkCompileErrors(GLuint shader, const std::string &type)
{
   GLint success;
   GLchar infoLog[1024];
   if (type != "PROGRAM")
   {
      glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
      if (!success)
      {
         glGetShaderInfoLog(shader, 1024, NULL, infoLog);
         std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
                   << infoLog << "\n---------------------------------------------------\n";
      }
   }
   else
   {
      glGetProgramiv(shader, GL_LINK_STATUS, &success);
      if (!success)
      {
         glGetProgramInfoLog(shader, 1024, NULL, infoLog);
         std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
                   << infoLog << "\n---------------------------------------------------\n";
      }
   }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
   glViewport(0, 0, width, height);
}

Draw::Draw(Shader *shader) : m_shader(shader)
{
   setupRockVertices();
   setupShipVertices();
   setupDotVertices();
   setupCircle();
   setupRectangle();
   setupText();
}

Draw::Draw()
{
   // Use 'new' to dynamically allocate the shader so it stays alive
   m_shader = new Shader("assets/shaders/default.vert", "assets/shaders/default.frag");

   setupRockVertices();
   setupShipVertices();
   setupDotVertices();
   setupCircle();
   setupRectangle();
   setupText();
}

Draw::~Draw()
{
   delete m_shader;
   glDeleteVertexArrays(3, m_rockVAO);
   glDeleteBuffers(3, m_rockVBO);
   glDeleteBuffers(3, m_rockEBO);
   glDeleteVertexArrays(1, &m_dotVAO);
   glDeleteBuffers(1, &m_dotVBO);
   glDeleteBuffers(1, &m_dotEBO);
   glDeleteVertexArrays(1, &m_shipVAO);
   glDeleteBuffers(1, &m_shipVBO);
   glDeleteBuffers(1, &m_shipEBO);
   glDeleteVertexArrays(1, &m_circleVAO);
   glDeleteBuffers(1, &m_circleVBO);
   glDeleteBuffers(1, &m_circleEBO);
   glDeleteVertexArrays(1, &m_rectangleVAO);
   glDeleteBuffers(1, &m_rectangleVBO);
   glDeleteBuffers(1, &m_rectangleEBO);
   glDeleteVertexArrays(1, &m_textVAO);
   glDeleteBuffers(1, &m_textVBO);
   glDeleteBuffers(1, &m_textEBO);
}

void Draw::rock(std::pair<double, double> position, double angle, double scale,
                std::tuple<double, double, double, double> color, rocks which)
{
   m_shader->setVec2("u_offset", position.first, position.second);
   m_shader->setFloat("u_scale", static_cast<float>(scale));
   m_shader->setFloat("u_aspect", m_aspect);
   m_shader->setFloat("u_sinAngle", static_cast<float>(std::sin(angle)));
   m_shader->setFloat("u_cosAngle", static_cast<float>(std::cos(angle)));
   m_shader->setVec3("u_color", std::get<0>(color), std::get<1>(color), std::get<2>(color));
   m_shader->setFloat("u_alpha", std::get<3>(color));

   glBindVertexArray(m_rockVAO[which]);
   glDrawElements(GL_LINES, m_rockTriCount[which], GL_UNSIGNED_INT, 0); // 12 indices for hexagon
   glBindVertexArray(0);
}

void Draw::ship(std::pair<double, double> position, double angle, double scale,
                std::tuple<double, double, double, double> color, bool thrusting)
{
   m_shader->setVec2("u_offset", position.first, position.second);
   m_shader->setFloat("u_scale", static_cast<float>(scale));
   m_shader->setFloat("u_aspect", m_aspect);
   m_shader->setFloat("u_sinAngle", static_cast<float>(std::sin(angle)));
   m_shader->setFloat("u_cosAngle", static_cast<float>(std::cos(angle)));
   m_shader->setVec3("u_color", std::get<0>(color), std::get<1>(color), std::get<2>(color));
   m_shader->setFloat("u_alpha", std::get<3>(color));

   glBindVertexArray(m_shipVAO);
   glDrawElements(GL_LINES, m_shipTriCount, GL_UNSIGNED_INT, 0);
   glBindVertexArray(0);

   if (thrusting)
   {
      // red
      m_shader->setVec3("u_color", std::get<0>(red), std::get<1>(red), std::get<2>(red));
      m_shader->setFloat("u_alpha", std::get<3>(red));
      glBindVertexArray(m_shipThrustVAO);
      glDrawElements(GL_LINES, m_shipThrustTriCount, GL_UNSIGNED_INT, 0);
      glBindVertexArray(0);

      // orange
      m_shader->setVec3("u_color", std::get<0>(orange), std::get<1>(orange), std::get<2>(orange));
      m_shader->setFloat("u_alpha", std::get<3>(orange));
      glBindVertexArray(m_shipThrustVAO);
      glDrawElements(GL_LINES, m_shipThrustTriCount, GL_UNSIGNED_INT, 0);
      glBindVertexArray(0);

      // yellow
      m_shader->setVec3("u_color", std::get<0>(yellow), std::get<1>(yellow), std::get<2>(yellow));
      m_shader->setFloat("u_alpha", std::get<3>(yellow));
      glBindVertexArray(m_shipThrustVAO);
      glDrawElements(GL_LINES, m_shipThrustTriCount, GL_UNSIGNED_INT, 0);
      glBindVertexArray(0);
   }
}

void Draw::dot(std::pair<double, double> position, double scale,
               std::tuple<double, double, double, double> color)
{
   m_shader->setVec2("u_offset", position.first, position.second);
   m_shader->setFloat("u_scale", static_cast<float>(scale));
   m_shader->setFloat("u_aspect", m_aspect);
   m_shader->setFloat("u_sinAngle", 0.0f);
   m_shader->setFloat("u_cosAngle", 1.0f);
   m_shader->setVec3("u_color", std::get<0>(color), std::get<1>(color), std::get<2>(color));
   m_shader->setFloat("u_alpha", std::get<3>(color));

   glPointSize(static_cast<float>(10.0 * scale));
   glBindVertexArray(m_dotVAO);
   glDrawElements(GL_POINTS, m_dotTriCount, GL_UNSIGNED_INT, 0);
   glBindVertexArray(0);
}

void Draw::setupDotVertices()
{
   std::vector<float> vertices = {
       0.0f, 0.0f, 0.0f,
       1.0f, 1.0f, 1.0f};
   std::vector<unsigned int> indices = {0};

   m_dotTriCount = static_cast<unsigned int>(indices.size());

   glGenVertexArrays(1, &m_dotVAO);
   glGenBuffers(1, &m_dotVBO);
   glGenBuffers(1, &m_dotEBO);

   glBindVertexArray(m_dotVAO);
   glBindBuffer(GL_ARRAY_BUFFER, m_dotVBO);
   glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_dotEBO);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
   glEnableVertexAttribArray(0);
   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
   glEnableVertexAttribArray(1);
   glBindVertexArray(0);
}

void Draw::rectangle(std::pair<double, double> position,
                     std::pair<double, double> dimensions, double angle,
                     std::tuple<double, double, double, double> color)
{
   float w = static_cast<float>(dimensions.first);
   float h = static_cast<float>(dimensions.second);

   // Assuming position is the center.
   // (If position is bottom-left, change to 0,0 to w,h)
   float hw = w * 0.5f;
   float hh = h * 0.5f;

   // Define the 4 corners of the quad (Bottom-Left, Bottom-Right, Top-Right, Top-Left)
   float vertices[] = {
       -hw, -hh,
       hw, -hh,
       hw, hh,
       -hw, hh};

   // Bind and update the vertex buffer
   glBindVertexArray(m_rectangleVAO);
   glBindBuffer(GL_ARRAY_BUFFER, m_rectangleVBO);
   glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

   // Apply shader uniforms exactly like your text rendering pipeline
   m_shader->setVec2("u_offset", static_cast<float>(position.first), static_cast<float>(position.second));
   m_shader->setFloat("u_scale", 1.0f);
   m_shader->setFloat("u_aspect", m_aspect);
   m_shader->setFloat("u_sinAngle", static_cast<float>(std::sin(angle)));
   m_shader->setFloat("u_cosAngle", static_cast<float>(std::cos(angle)));
   m_shader->setVec3("u_color", std::get<0>(color), std::get<1>(color), std::get<2>(color));
   m_shader->setFloat("u_alpha", std::get<3>(color));

   // Draw the quad
   glDrawElements(GL_TRIANGLES, m_rectangleTriCount, GL_UNSIGNED_INT, 0);
   glBindVertexArray(0);
}

void Draw::circle(std::pair<double, double> position, double radius,
                  std::tuple<double, double, double, double> color)
{
   m_shader->setVec2("u_offset", position.first, position.second);
   m_shader->setFloat("u_scale", static_cast<float>(radius));
   m_shader->setFloat("u_aspect", m_aspect);
   m_shader->setFloat("u_sinAngle", 0.0f);
   m_shader->setFloat("u_cosAngle", 1.0f);
   m_shader->setVec3("u_color", std::get<0>(color), std::get<1>(color), std::get<2>(color));
   m_shader->setFloat("u_alpha", std::get<3>(color));

   glBindVertexArray(m_circleVAO);
   glDrawElements(GL_LINE_LOOP, m_circleTriCount, GL_UNSIGNED_INT, 0);
   glBindVertexArray(0);
}

void Draw::setupCircle()
{
   std::vector<float> vertices;
   std::vector<unsigned int> indices;

   const int numSegments = 64;
   for (int i = 0; i < numSegments; ++i)
   {
      float theta = 2.0f * M_PI * float(i) / float(numSegments);
      float x = std::cos(theta);
      float y = std::sin(theta);
      vertices.push_back(x);
      vertices.push_back(y);
      vertices.push_back(0.0f);
      vertices.push_back(1.0f);
      vertices.push_back(1.0f);
      vertices.push_back(1.0f);
      indices.push_back(i);
   }

   m_circleTriCount = static_cast<unsigned int>(indices.size());

   glGenVertexArrays(1, &m_circleVAO);
   glGenBuffers(1, &m_circleVBO);
   glGenBuffers(1, &m_circleEBO);

   glBindVertexArray(m_circleVAO);
   glBindBuffer(GL_ARRAY_BUFFER, m_circleVBO);
   glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_circleEBO);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
   glEnableVertexAttribArray(0);
   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
   glEnableVertexAttribArray(1);
   glBindVertexArray(0);
}

void Draw::setupRectangle()
{
   // Generate OpenGL objects
   glGenVertexArrays(1, &m_rectangleVAO);
   glGenBuffers(1, &m_rectangleVBO);
   glGenBuffers(1, &m_rectangleEBO);

   glBindVertexArray(m_rectangleVAO);

   // Set up the VBO for 4 vertices (2 floats each for x, y)
   // We use GL_DYNAMIC_DRAW because we will update the geometry in rectangle()
   glBindBuffer(GL_ARRAY_BUFFER, m_rectangleVBO);
   glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

   // Configure the position attribute (assuming layout location 0)
   glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
   glEnableVertexAttribArray(0);

   // Set up the EBO with static indices for a quad (two triangles)
   unsigned int indices[] = {
       0, 1, 2, // First triangle
       0, 2, 3  // Second triangle
   };
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rectangleEBO);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

   glBindVertexArray(0);

   // 2 triangles = 6 indices
   m_rectangleTriCount = 6;
}

void Draw::text(std::pair<double, double> position, const std::string &text, double scale,
                std::tuple<double, double, double, double> color, align which, style textStyle)
{
   std::vector<float> vertices;
   std::vector<unsigned int> indices;
   double effectiveCellSize = scale * 0.08;
   double textWidth = appendTextGeometry(vertices, indices, text, scale * 0.08);

   glLineWidth(1.5f);

   // --- STYLE IMPLEMENTATION (Geometry Modification) ---
   float &x = vertices[0];
   double y, boldOffset, yTop, yBottom, xStart, xEnd;
   size_t originalVertSize, originalIdxSize;
   unsigned int startIdx, vertexCount;
   switch (textStyle)
   {
   case ITALIC:
      // Skew X coordinates based on Y to create a slant.
      for (size_t i = 0; i < vertices.size(); i += 2)
      {
         x = vertices[i];
         y = vertices[i + 1];
         x += y * 0.25f; // Adjust this multiplier to increase/decrease the italic slant
      }
      break;
   case BOLD:
      // Thicken characters by duplicating the geometry with a slight horizontal offset.
      originalVertSize = vertices.size();
      originalIdxSize = indices.size();

      boldOffset = static_cast<double>(effectiveCellSize * 0.6); // Offset distance
      vertexCount = static_cast<unsigned int>(originalVertSize / 2);

      // Append shifted vertices
      for (size_t i = 0; i < originalVertSize; i += 2)
      {
         vertices.push_back(vertices[i] + boldOffset);
         vertices.push_back(vertices[i + 1]);
      }

      // Append indices for the new vertices
      for (size_t i = 0; i < originalIdxSize; ++i)
      {
         indices.push_back(indices[i] + vertexCount);
      }
      break;
   case UNDERLINE:
      // Add a single long quad just below the baseline to act as an underline.
      yTop = static_cast<double>(-effectiveCellSize * 7.5);
      yBottom = static_cast<double>(-effectiveCellSize * 8.5);
      xStart = 0.0f;
      xEnd = static_cast<double>(textWidth);

      startIdx = static_cast<unsigned int>(vertices.size() / 2);

      // 4 Vertices for the underline rectangle
      vertices.push_back(xStart);
      vertices.push_back(yBottom); // Bottom-Left
      vertices.push_back(xEnd);
      vertices.push_back(yBottom); // Bottom-Right
      vertices.push_back(xEnd);
      vertices.push_back(yTop); // Top-Right
      vertices.push_back(xStart);
      vertices.push_back(yTop); // Top-Left

      // 6 Indices for the two triangles
      indices.push_back(startIdx);
      indices.push_back(startIdx + 1);
      indices.push_back(startIdx + 2);
      indices.push_back(startIdx);
      indices.push_back(startIdx + 2);
      indices.push_back(startIdx + 3);
      break;
   case NONE:
   default:
      break;
   }

   // Update the triangle count AFTER styles have potentially appended more indices
   m_textTriCount = static_cast<unsigned int>(indices.size());

   glBindVertexArray(m_textVAO);
   glBindBuffer(GL_ARRAY_BUFFER, m_textVBO);
   glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_textEBO);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);

   // alignment offset
   double xOffsetAdjustment = 0.0;
   switch (which)
   {
   case CENTER:
      xOffsetAdjustment = textWidth * 0.5;
      break;
   case RIGHT:
      xOffsetAdjustment = textWidth;
      break;
   case LEFT:
   default:
      xOffsetAdjustment = 0.0;
      break;
   }

   // Apply adjusted position
   m_shader->setVec2("u_offset", static_cast<float>(position.first) - xOffsetAdjustment, static_cast<float>(position.second));
   m_shader->setFloat("u_scale", 1.0f);
   m_shader->setFloat("u_aspect", m_aspect);
   m_shader->setFloat("u_sinAngle", 0.0f);
   m_shader->setFloat("u_cosAngle", 1.0f);
   m_shader->setVec3("u_color", std::get<0>(color), std::get<1>(color), std::get<2>(color));
   m_shader->setFloat("u_alpha", std::get<3>(color));

   glDrawElements(GL_TRIANGLES, m_textTriCount, GL_UNSIGNED_INT, 0);
   glBindVertexArray(0);
}

void Draw::setupText()
{
   std::vector<float> vertices;
   std::vector<unsigned int> indices;

   m_textTriCount = 0;

   glGenVertexArrays(1, &m_textVAO);
   glGenBuffers(1, &m_textVBO);
   glGenBuffers(1, &m_textEBO);

   glBindVertexArray(m_textVAO);
   glBindBuffer(GL_ARRAY_BUFFER, m_textVBO);
   glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_textEBO);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);

   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
   glEnableVertexAttribArray(0);
   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
   glEnableVertexAttribArray(1);
   glBindVertexArray(0);
}

void Draw::setAspectRatio(float aspect)
{
   m_aspect = aspect;
}

void Draw::setupRockVertices()
{
   for (size_t i = 0; i < 3; i++)
   {
      std::vector<double> vertices;
      std::vector<unsigned int> indices;

      try
      {
         loadShape(vertices, indices, resolveAssetPath(rockFiles[i]));
         centerVertices(vertices);
      }
      catch (...)
      {
         std::cerr << "Failed to load rock shape: " << rockFiles[i] << "\n";
         constexpr double rockFallbackVerts[] = {
             0.0, 0.2, 0.0, 0.6, 0.6, 0.6,
             0.17, 0.1, 0.0, 0.6, 0.6, 0.6,
             0.17, -0.1, 0.0, 0.6, 0.6, 0.6,
             0.0, -0.2, 0.0, 0.6, 0.6, 0.6,
             -0.17, -0.1, 0.0, 0.6, 0.6, 0.6,
             -0.17, 0.1, 0.0, 0.6, 0.6, 0.6};
         constexpr unsigned int rockFallbackInds[] = {
             0, 1, 5, 1, 2, 5, 2, 3, 4, 2, 4, 5};
      }

      m_rockTriCount[i] = indices.size();
      std::vector<float> floatVerts(vertices.begin(), vertices.end());

      glGenVertexArrays(1, &m_rockVAO[i]);
      glGenBuffers(1, &m_rockVBO[i]);
      glGenBuffers(1, &m_rockEBO[i]);

      glBindVertexArray(m_rockVAO[i]);
      glBindBuffer(GL_ARRAY_BUFFER, m_rockVBO[i]);
      glBufferData(GL_ARRAY_BUFFER, floatVerts.size() * sizeof(float), floatVerts.data(), GL_STATIC_DRAW);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rockEBO[i]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
      glEnableVertexAttribArray(1);
   }
   glBindVertexArray(0);
}

void Draw::setupShipVertices()
{
   std::vector<double> verts;
   std::vector<unsigned int> indices;

   try
   {
      loadShape(verts, indices, resolveAssetPath("assets/shapes/ship.txt"));
      centerVertices(verts);
   }
   catch (...)
   {
      std::cerr << "using fallback ship verts\n";

      constexpr double shipFallbackVerts[] = {
          // Line A
          -1.0f / 25.0f, 22.0f / 25.0f, 0.0f, 1.0f, 1.0f, 1.0f,
          1.0f / 25.0f, 22.0f / 25.0f, 0.0f, 1.0f, 1.0f, 1.0f,
          // Line A2
          0.0f / 25.0f, 16.0f / 25.0f, 0.0f, 1.0f, 1.0f, 1.0f,
          0.0f / 25.0f, 22.0f / 25.0f, 0.0f, 1.0f, 1.0f, 1.0f,
          // Base Polygon (Triangulated)
          -15.0f / 25.0f, 0.0f / 25.0f, 0.0f, 0.5f, 0.5f, 0.5f,
          15.0f / 25.0f, 0.0f / 25.0f, 0.0f, 0.5f, 0.5f, 0.5f,
          15.0f / 25.0f, -10.0f / 25.0f, 0.0f, 0.5f, 0.5f, 0.5f,
          -15.0f / 25.0f, -10.0f / 25.0f, 0.0f, 0.5f, 0.5f, 0.5f};
      constexpr unsigned int shipFallbackInds[] = {
          0, 1,            // Line 1
          2, 3,            // Line 2
          4, 5, 6, 4, 6, 7 // Poly
      };

      verts.assign(std::begin(shipFallbackVerts), std::end(shipFallbackVerts));
      indices.assign(std::begin(shipFallbackInds), std::end(shipFallbackInds));
   }

   if (verts.empty() || indices.empty())
   {
      throw std::runtime_error("Vertices or indices are empty; failed to generate shape data.");
   }

   std::vector<float> vertices(verts.begin(), verts.end());
   m_shipTriCount = indices.size();

   glGenVertexArrays(1, &m_shipVAO);
   glGenBuffers(1, &m_shipVBO);
   glGenBuffers(1, &m_shipEBO);

   glBindVertexArray(m_shipVAO);
   glBindBuffer(GL_ARRAY_BUFFER, m_shipVBO);
   glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_shipEBO);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
   glEnableVertexAttribArray(0);
   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
   glEnableVertexAttribArray(1);
   glBindVertexArray(0);

   m_shipVertexCount = vertices.size() / 6; // 6 floats per vertex
}

void Draw::setupShipThrustVertices()
{
   // set up vertices
   std::vector<double> verts;
   std::vector<unsigned int> indices;

   try
   {
      loadShape(verts, indices, resolveAssetPath("assets/shapes/shipThrust.txt"));
      centerVertices(verts);
   }
   catch (...)
   {
      std::cerr << "using fallback ship thrust verts\n";
      constexpr double shipThrustFallbackVerts[] = {
          // Line A
          -1.0f / 25.0f,
          -10.0f / 25.0f,
          0.0f,
          1.0f,
          0.5f,
          0.0f,
          1.0f / 25.0f,
          -10.0f / 25.0f,
          0.0f,
          1.0f,
          0.5f,
          0.0f,
      };
      constexpr unsigned int shipThrustFallbackInds[] = {
          0, 1};

      verts.assign(std::begin(shipThrustFallbackVerts), std::end(shipThrustFallbackVerts));
      indices.assign(std::begin(shipThrustFallbackInds), std::end(shipThrustFallbackInds));
   }

   if (verts.empty() || indices.empty())
   {
      throw std::runtime_error("Vertices or indices are empty; failed to generate shape data.");
   }

   std::vector<float> vertices(verts.begin(), verts.end());
   m_shipThrustTriCount = indices.size();

   glGenVertexArrays(1, &m_shipThrustVAO);
   glGenBuffers(1, &m_shipThrustVBO);
   glGenBuffers(1, &m_shipThrustEBO);

   glBindVertexArray(m_shipThrustVAO);
   glBindBuffer(GL_ARRAY_BUFFER, m_shipThrustVBO);
   glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_shipThrustEBO);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
   glEnableVertexAttribArray(0);
   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
   glEnableVertexAttribArray(1);
   glBindVertexArray(0);
}

void Draw::pushVertex(std::vector<double> &verts, double x, double y)
{
   verts.push_back(x / 25.0); // Normalize by SVG viewBox scale
   verts.push_back(y / 25.0); // scale is (1, -1)
   verts.push_back(0.0);      // Z
   verts.push_back(1.0);      // R (White)
   verts.push_back(1.0);      // G
   verts.push_back(1.0);      // B
}

void Draw::centerVertices(std::vector<double> &vertices)
{
   if (vertices.empty())
   {
      return;
   }

   const size_t vertexCount = vertices.size() / 6;
   double sumX = 0.0;
   double sumY = 0.0;

   for (size_t i = 0; i < vertexCount; ++i)
   {
      const size_t base = i * 6;
      sumX += vertices[base];
      sumY += vertices[base + 1];
   }

   const double centerX = sumX / static_cast<double>(vertexCount);
   const double centerY = sumY / static_cast<double>(vertexCount);

   for (size_t i = 0; i < vertexCount; ++i)
   {
      const size_t base = i * 6;
      vertices[base] -= centerX;
      vertices[base + 1] -= centerY;
   }
}

void Draw::loadShape(std::vector<double> &vertices, std::vector<unsigned int> &indices, std::string path)
{
   std::ifstream file(path);
   if (file.is_open())
   {
      std::string line;
      unsigned int currentIndex = 0;

      while (std::getline(file, line))
      {
         if (!line.empty() && line[0] != '#')
         {
            std::istringstream iss(line);
            std::string type;
            iss >> type;

            if (type == "LINE")
            {
               double x1, y1, x2, y2;
               if (iss >> x1 >> y1 >> x2 >> y2)
               {
                  pushVertex(vertices, x1, y1);
                  indices.push_back(currentIndex++);
                  pushVertex(vertices, x2, y2);
                  indices.push_back(currentIndex++);
               }
            }

            else if (type == "POLY")
            {
               std::vector<std::pair<double, double>> pts;
               double px, py;
               while (iss >> px >> py)
                  pts.push_back({px, py});

               if (pts.size() >= 3)
               {
                  for (size_t i = 0; i < pts.size(); ++i)
                  {
                     size_t next = (i + 1) % pts.size();
                     pushVertex(vertices, pts[i].first, pts[i].second);
                     pushVertex(vertices, pts[next].first, pts[next].second);
                     indices.push_back(currentIndex++);
                     indices.push_back(currentIndex++);
                  }
               }
            }
         }

      } // end while

      file.close();
   }

   else
   {
      throw std::runtime_error("Warning: Could not load {" + path + "}. Using hardcoded fallback.\n");
   }
}

int run(UI &ui, Game &game, Draw &draw)
{
   if (!ui.getNativeWindow())
      return -1;

   GLFWwindow *window = ui.getNativeWindow();
   glfwSetWindowShouldClose(window, GLFW_FALSE);

   while (!ui.shouldClose())
   {
      double dt = ui.updateDeltaTime();

      // --- Clear Screen ---
      glClearColor(0.05, 0.05, 0.3, 1.0);
      glClear(GL_COLOR_BUFFER_BIT);
      draw.getShader()->use();

      int framebufferWidth = 0;
      int framebufferHeight = 0;
      glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
      const float aspect = (framebufferWidth > 0)
                               ? static_cast<float>(framebufferHeight) / static_cast<float>(framebufferWidth)
                               : 0.75f;
      draw.setAspectRatio(aspect);

      // --- Student Logic ---
      game.input(ui);
      game.update(dt);
      game.output(draw);

      if (ui.isKeyDown(GLFW_KEY_ESCAPE))
         glfwSetWindowShouldClose(window, true); // Close the window if ESC is pressed

      ui.swapBuffersAndPollEvents();
   }

   return 0;
}
