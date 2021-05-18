#pragma once

#include <filesystem>

namespace fs = std::filesystem;

#include "camera.h"
#include "core.h"
#include "scene.h"

double mx, my;
bool captured = false;
float aspect_ratio;

#define NR_POINT_LIGHTS 1

unsigned selected_light = 0;

void APIENTRY
glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char *message,
              const void *userParam) {
  // ignore non-significant error/warning codes
  if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
    return;

  std::cout << "---------------" << std::endl;
  std::cout << "Debug message (" << id << "): " << message << std::endl;

  switch (source) {
  case GL_DEBUG_SOURCE_API:
    std::cout << "Source: API";
    break;
  case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
    std::cout << "Source: Window System";
    break;
  case GL_DEBUG_SOURCE_SHADER_COMPILER:
    std::cout << "Source: Shader Compiler";
    break;
  case GL_DEBUG_SOURCE_THIRD_PARTY:
    std::cout << "Source: Third Party";
    break;
  case GL_DEBUG_SOURCE_APPLICATION:
    std::cout << "Source: Application";
    break;
  case GL_DEBUG_SOURCE_OTHER:
    std::cout << "Source: Other";
    break;
  }
  std::cout << std::endl;

  switch (type) {
  case GL_DEBUG_TYPE_ERROR:
    std::cout << "Type: Error";
    break;
  case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
    std::cout << "Type: Deprecated Behaviour";
    break;
  case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
    std::cout << "Type: Undefined Behaviour";
    break;
  case GL_DEBUG_TYPE_PORTABILITY:
    std::cout << "Type: Portability";
    break;
  case GL_DEBUG_TYPE_PERFORMANCE:
    std::cout << "Type: Performance";
    break;
  case GL_DEBUG_TYPE_MARKER:
    std::cout << "Type: Marker";
    break;
  case GL_DEBUG_TYPE_PUSH_GROUP:
    std::cout << "Type: Push Group";
    break;
  case GL_DEBUG_TYPE_POP_GROUP:
    std::cout << "Type: Pop Group";
    break;
  case GL_DEBUG_TYPE_OTHER:
    std::cout << "Type: Other";
    break;
  }
  std::cout << std::endl;

  switch (severity) {
  case GL_DEBUG_SEVERITY_HIGH:
    std::cout << "Severity: high";
    break;
  case GL_DEBUG_SEVERITY_MEDIUM:
    std::cout << "Severity: medium";
    break;
  case GL_DEBUG_SEVERITY_LOW:
    std::cout << "Severity: low";
    break;
  case GL_DEBUG_SEVERITY_NOTIFICATION:
    std::cout << "Severity: notification";
    break;
  }
  std::cout << std::endl;
  std::cout << std::endl;
  DEBUG_BREAK;
}

static void cursor_position_callback(GLFWwindow *window, double x, double y) {
  auto *s = static_cast<scene *>(glfwGetWindowUserPointer(window));

  if (captured) {
    float xoffset = mx - x;
    float yoffset = my - y;
    mx = x;
    my = y;

    float sensitivity = 0.09;
    xoffset *= s->cam.sensitivity;
    yoffset *= s->cam.sensitivity;

    s->cam.DelYaw(xoffset);
    s->cam.DelPitch(yoffset);
  }
}

static void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
  if (height != 0)
    aspect_ratio = (float)width / (float)height;
}

static void error_callback(int error, const char *description) {
  fprintf(stderr, "Error: %s\n", description);
}

static void scroll_callback(GLFWwindow *window, double xoff, double yoff) {
  scene &s = *(static_cast<scene *>(glfwGetWindowUserPointer(window)));
  s.cam.FOV += (s.cam.sensitivity * yoff);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {

  scene &s = *(static_cast<scene *>(glfwGetWindowUserPointer(window)));
  const float cameraSpeed = 2.5 * s.deltatime;
  vec3 left = glm::cross(s.cam.Camera_Up, s.cam.Camera_Facing_Direction);
  switch (action) {
  case GLFW_PRESS:
    switch (key) {
    case GLFW_KEY_ESCAPE:
      glfwSetWindowShouldClose(window, true);
      break;

    case GLFW_KEY_1:
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      captured = false;
      break;
    case GLFW_KEY_2:
      glfwSetCursorPos(window, mx, my);
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      captured = true;
      break;
    case GLFW_KEY_KP_4:
      if (selected_light == 0) {
        selected_light = NR_POINT_LIGHTS - 1;
        break;
      }
      selected_light--;
      break;
    case GLFW_KEY_KP_6:
      if (selected_light == NR_POINT_LIGHTS - 1) {
        selected_light = 0;
        break;
      }
      selected_light++;
      break;
    default:
      break;
    }
    break;
  }
}

void processHoldEvent(GLFWwindow *window) {
  scene &s = *(static_cast<scene *>(glfwGetWindowUserPointer(window)));
  const float cameraSpeed = s.cam.speed * s.deltatime;
  vec3 left = glm::cross(s.cam.Camera_Up, s.cam.Camera_Facing_Direction);

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    s.cam.Camera_Position += cameraSpeed * s.cam.Camera_Facing_Direction;

  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    s.cam.Camera_Position -=
        cameraSpeed * glm::normalize(glm::cross(s.cam.Camera_Facing_Direction, s.cam.Camera_Up));
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    s.cam.Camera_Position -= cameraSpeed * s.cam.Camera_Facing_Direction;

  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    s.cam.Camera_Position +=
        cameraSpeed * glm::normalize(glm::cross(s.cam.Camera_Facing_Direction, s.cam.Camera_Up));
  if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    s.cam.Camera_Position += cameraSpeed * s.cam.Camera_Up;

  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    s.cam.Camera_Position -= cameraSpeed * s.cam.Camera_Up;

  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
    s.pointLights[selected_light].position.x += (0.05 * s.cam.Camera_Facing_Direction.x);
    s.pointLights[selected_light].position.y += (0.05 * s.cam.Camera_Facing_Direction.y);
    s.pointLights[selected_light].position.z += (0.05 * s.cam.Camera_Facing_Direction.z);
  }

  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {

    s.pointLights[selected_light].position.x -= (0.05 * s.cam.Camera_Facing_Direction.x);
    s.pointLights[selected_light].position.y -= (0.05 * s.cam.Camera_Facing_Direction.y);
    s.pointLights[selected_light].position.z -= (0.05 * s.cam.Camera_Facing_Direction.z);
  }

  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
    s.pointLights[selected_light].position.x += (0.05 * left.x);
    s.pointLights[selected_light].position.y += (0.05 * left.y);
    s.pointLights[selected_light].position.z += (0.05 * left.z);
  }

  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
    s.pointLights[selected_light].position.x -= (0.05 * left.x);
    s.pointLights[selected_light].position.y -= (0.05 * left.y);
    s.pointLights[selected_light].position.z -= (0.05 * left.z);
  }

  if (glfwGetKey(window, GLFW_KEY_KP_0) == GLFW_PRESS) {
    s.pointLights[selected_light].position.x += (0.05 * s.cam.Camera_Up.x);
    s.pointLights[selected_light].position.y += (0.05 * s.cam.Camera_Up.y);
    s.pointLights[selected_light].position.z += (0.05 * s.cam.Camera_Up.z);
  }

  if (glfwGetKey(window, GLFW_KEY_KP_1) == GLFW_PRESS) {
    s.pointLights[selected_light].position.x -= (0.05 * s.cam.Camera_Up.x);
    s.pointLights[selected_light].position.y -= (0.05 * s.cam.Camera_Up.y);
    s.pointLights[selected_light].position.z -= (0.05 * s.cam.Camera_Up.z);
  }
}

std::string searchRes() {
  auto currentPath = fs::current_path();
  auto orgiPath = currentPath;
  bool resPathFound = false, appPathFound = false;

  while (true) {
    if (!resPathFound && !appPathFound && currentPath.has_parent_path()) {
      fs::current_path(currentPath.parent_path());
      currentPath = fs::current_path();
    } else if (appPathFound) {
      try {
#ifdef _WIN_32
        auto a = currentPath.string().append("\\Application\\res");
#else
        auto a = currentPath.string().append("/Application/res");
#endif
        fs::current_path(a);
        fs::current_path(orgiPath);
        return a;
      } catch (const std::exception &e) {
        std::cout << e.what() << "\n \"res\" folder not found inside \"Application\" folder \n";
        return std::string();
      }
    } else
      break;

    //std::cout << "currently in path: " << currentPath.string() << std::endl;

    for (auto &dirs : fs::directory_iterator(currentPath)) {
      if (dirs.is_directory() && dirs.path().filename().string() == "Application") {
        appPathFound = true;
        break;
      }
    }
  }

  std::cout << "folder \"Application\" not found in 4 back iterations" << std::endl;
  DEBUG_BREAK;
  return std::string();
}