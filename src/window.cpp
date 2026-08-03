#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <filesystem>

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

   glfwWindowHint(GLFW_SAMPLES, 4);

   m_window = glfwCreateWindow((int)width, (int)height, title.c_str(), nullptr, nullptr);
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
namespace {
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
}

Draw::Draw()
{
   // Use 'new' to dynamically allocate the shader so it stays alive
   m_shader = new Shader("assets/shaders/default.vert", "assets/shaders/default.frag");

   setupRockVertices();
   setupShipVertices();
   setupDotVertices();
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
}

void Draw::rock(double x, double y, double angle, double scale, rocks which)
{
   m_shader->setVec2("u_offset", x, y);
   m_shader->setFloat("u_scale", static_cast<float>(scale) * (double)(which + 1));
   m_shader->setFloat("u_aspect", m_aspect);
   m_shader->setFloat("u_sinAngle", static_cast<float>(std::sin(angle)));
   m_shader->setFloat("u_cosAngle", static_cast<float>(std::cos(angle)));

   glBindVertexArray(m_rockVAO[which]);
   glDrawElements(GL_LINES, m_rockTriCount[which], GL_UNSIGNED_INT, 0); // 12 indices for hexagon
   glBindVertexArray(0);
}

void Draw::ship(double x, double y, double angle, double scale)
{
   m_shader->setVec2("u_offset", x, y);
   m_shader->setFloat("u_scale", static_cast<float>(scale));
   m_shader->setFloat("u_aspect", m_aspect);
   m_shader->setFloat("u_sinAngle", static_cast<float>(std::sin(angle)));
   m_shader->setFloat("u_cosAngle", static_cast<float>(std::cos(angle)));

   glBindVertexArray(m_shipVAO);
   glDrawElements(GL_LINES, m_shipTriCount, GL_UNSIGNED_INT, 0);
   // glPointSize(5); // debug: use points instead
   // glDrawArrays(GL_POINTS, 0, m_shipVertexCount);
   glBindVertexArray(0);

   // std::cout << m_shader << '|' 
   //    << angle << '|' 
   //    << m_shipVAO << std::endl; 
}

void Draw::dot(double x, double y, double scale) {
   m_shader->setVec2("u_offset", x, y);
   m_shader->setFloat("u_scale", static_cast<float>(scale));
   m_shader->setFloat("u_aspect", m_aspect);
   m_shader->setFloat("u_sinAngle", 0.0f);
   m_shader->setFloat("u_cosAngle", 1.0f);

   glPointSize(static_cast<float>(10.0 * scale));
   glBindVertexArray(m_dotVAO);
   glDrawElements(GL_POINTS, m_dotTriCount, GL_UNSIGNED_INT, 0);
   glBindVertexArray(0);
}

void Draw::setupDotVertices()
{
   std::vector<float> vertices = {
      0.0f, 0.0f, 0.0f,
      1.0f, 1.0f, 1.0f
   };
   std::vector<unsigned int> indices = { 0 };

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

void Draw::setAspectRatio(float aspect)
{
   m_aspect = aspect;
}

void Draw::setupRockVertices() {
   for(int i = 0; i < 3; i++) {
      std::vector<double> vertices;
      std::vector<unsigned int> indices;
      
      loadShape(vertices, indices, resolveAssetPath(rockFiles[i]));
      centerVertices(vertices);
      
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

   try {
      loadShape(verts, indices, resolveAssetPath("assets/shapes/lander.txt"));
      centerVertices(verts);
   }
   catch (...) {
      std::cerr << "using fallback ship verts\n";
      
      verts.assign(std::begin(shipFallbackVerts), std::end(shipFallbackVerts));
      indices.assign(std::begin(shipFallbackInds), std::end(shipFallbackInds));
   }

   if (verts.empty() || indices.empty()) {
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

   m_shipVertexCount = vertices.size() / 6;   // 6 floats per vertex
}

void Draw::pushVertex(std::vector<double>& verts, double x, double y) {
   verts.push_back(x / 25.0); // Normalize by SVG viewBox scale
   verts.push_back(y / 25.0); // scale is (1, -1)
   verts.push_back(0.0); // Z
   verts.push_back(1.0); // R (White)
   verts.push_back(1.0); // G
   verts.push_back(1.0); // B
}

void Draw::centerVertices(std::vector<double>& vertices)
{
   if (vertices.empty()) {
      return;
   }

   const size_t vertexCount = vertices.size() / 6;
   double sumX = 0.0;
   double sumY = 0.0;

   for (size_t i = 0; i < vertexCount; ++i) {
      const size_t base = i * 6;
      sumX += vertices[base];
      sumY += vertices[base + 1];
   }

   const double centerX = sumX / static_cast<double>(vertexCount);
   const double centerY = sumY / static_cast<double>(vertexCount);

   for (size_t i = 0; i < vertexCount; ++i) {
      const size_t base = i * 6;
      vertices[base] -= centerX;
      vertices[base + 1] -= centerY;
   }
}

void Draw::loadShape(std::vector<double>& vertices, std::vector<unsigned int>& indices, std::string path) {
   std::ifstream file(path);
   if (file.is_open()) {
      std::string line;
      unsigned int currentIndex = 0;
      
      while (std::getline(file, line)) {
         if (!line.empty() && line[0] != '#') {
            std::istringstream iss(line);
            std::string type;
            iss >> type;
            
            if (type == "LINE") {
               double x1, y1, x2, y2;
               if (iss >> x1 >> y1 >> x2 >> y2) {
                  pushVertex(vertices, x1, y1);
                  indices.push_back(currentIndex++);
                  pushVertex(vertices, x2, y2);
                  indices.push_back(currentIndex++);
               }
            }

            else if (type == "POLY") {
               std::vector<std::pair<double, double>> pts;
               double px, py;
               while (iss >> px >> py)
                  pts.push_back({px, py});

               if (pts.size() >= 3) {
                  for (size_t i = 0; i < pts.size(); ++i) {
                     size_t next = (i + 1) % pts.size();
                     pushVertex(vertices, pts[i].first,    pts[i].second);
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

   else {
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
