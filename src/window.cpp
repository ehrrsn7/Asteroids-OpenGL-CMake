#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include "window.hpp"
#include "game.hpp" // Required so run() can call game.input, game.update, and game.output

Window::Window(int width, int height, const std::string &title)
{
   if (!glfwInit())
   {
      std::cerr << "Failed to initialize GLFW\n";
      return;
   }

   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

   m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
   if (!m_window)
   {
      std::cerr << "Failed to create GLFW window\n";
      glfwTerminate();
      return;
   }

   glfwMakeContextCurrent(m_window);

   if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
   {
      std::cerr << "Failed to initialize GLAD\n";
      return;
   }

   glViewport(0, 0, width, height);

   // Framebuffer resize callback
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

float Window::updateDeltaTime()
{
   float currentFrame = static_cast<float>(glfwGetTime());
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
Shader::Shader(const char *vertexPath, const char *fragmentPath)
{
   std::string vertexCode, fragmentCode;
   std::ifstream vShaderFile, fShaderFile;

   // Ensure ifstream objects can throw exceptions
   vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
   fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

   try
   {
      vShaderFile.open(vertexPath);
      fShaderFile.open(fragmentPath);

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
                << vertexPath << " or " << fragmentPath << "\n";
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
   setupAsteroidVerticies();
   setupShipVerticies();
}

Draw::Draw()
{
   // Use 'new' to dynamically allocate the shader so it stays alive
   m_shader = new Shader("assets/shaders/default.vert", "assets/shaders/default.frag");

   setupAsteroidVerticies();
   setupShipVerticies();
}

Draw::~Draw()
{
   delete m_shader;
   glDeleteVertexArrays(1, &m_asteroidVAO);
   glDeleteBuffers(1, &m_asteroidVBO);
   glDeleteBuffers(1, &m_asteroidEBO);
   glDeleteVertexArrays(1, &m_shipVAO);
   glDeleteBuffers(1, &m_shipVBO);
   glDeleteBuffers(1, &m_shipEBO);
}

void Draw::asteroid(float x, float y, float angle, float scale)
{
   m_shader->setVec2("u_offset", x, y);
   m_shader->setFloat("u_angle", angle);
   m_shader->setFloat("u_scale", scale);

   glBindVertexArray(m_asteroidVAO);
   glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0); // 12 indices for hexagon
   glBindVertexArray(0);
}

void Draw::ship(float x, float y, float angle, float scale)
{
   m_shader->setVec2("u_offset", x, y);
   m_shader->setFloat("u_angle", angle);
   m_shader->setFloat("u_scale", scale);

   glBindVertexArray(m_shipVAO);
   glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
   glBindVertexArray(0);
}

void Draw::setupAsteroidVerticies()
{
   float vertices[] = {
       0.0f, 0.2f, 0.0f, 0.6f, 0.6f, 0.6f,
       0.17f, 0.1f, 0.0f, 0.6f, 0.6f, 0.6f,
       0.17f, -0.1f, 0.0f, 0.6f, 0.6f, 0.6f,
       0.0f, -0.2f, 0.0f, 0.6f, 0.6f, 0.6f,
       -0.17f, -0.1f, 0.0f, 0.6f, 0.6f, 0.6f,
       -0.17f, 0.1f, 0.0f, 0.6f, 0.6f, 0.6f};
   unsigned int indices[] = {0, 1, 5, 1, 2, 5, 2, 3, 4, 2, 4, 5};

   glGenVertexArrays(1, &m_asteroidVAO);
   glGenBuffers(1, &m_asteroidVBO);
   glGenBuffers(1, &m_asteroidEBO);

   glBindVertexArray(m_asteroidVAO);
   glBindBuffer(GL_ARRAY_BUFFER, m_asteroidVBO);
   glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_asteroidEBO);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
   glEnableVertexAttribArray(0);
   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
   glEnableVertexAttribArray(1);
   glBindVertexArray(0);
}

void Draw::setupShipVerticies()
{
   float vertices[] = {
       0.0f, 0.1f, 0.0f, 0.2f, 1.0f, 0.2f,
       0.1f, -0.1f, 0.0f, 0.2f, 1.0f, 0.2f,
       -0.1f, -0.1f, 0.0f, 0.2f, 1.0f, 0.2f};
   unsigned int indices[] = {0, 1, 2};

   glGenVertexArrays(1, &m_shipVAO);
   glGenBuffers(1, &m_shipVBO);
   glGenBuffers(1, &m_shipEBO);

   glBindVertexArray(m_shipVAO);
   glBindBuffer(GL_ARRAY_BUFFER, m_shipVBO);
   glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_shipEBO);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
   glEnableVertexAttribArray(0);
   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
   glEnableVertexAttribArray(1);
   glBindVertexArray(0);
}

int run(UI &ui, Game &game, Draw &draw)
{
   if (!ui.getNativeWindow())
      return -1;

   GLFWwindow *window = ui.getNativeWindow();
   while (!ui.shouldClose())
   {
      float dt = ui.updateDeltaTime();

      // --- Clear Screen ---
      glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);
      draw.getShader()->use();

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
