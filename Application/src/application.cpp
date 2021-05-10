
#include <memory>
#include <chrono>
#include <random>

#include "callback.h"
#include "Shader.h"
#include "Texture.h"
#include "drawable.h"
//#include "model.h"
#include "shapes.h"
#include "skybox.h"
#include "renderer.h"
#include "imgui/imgui.h"

#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

int main(int argc, char* argv[])
{

#pragma region windowInitialization
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef MDEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif

    GLFWwindow* mainWin;
    GLFWwindow* imguiWin;

    imguiWin = glfwCreateWindow(800, 600, "imgui", NULL, NULL);
    mainWin = glfwCreateWindow(1000, 750, "main", NULL, NULL);


    if (!(mainWin && imguiWin))
    {
        glfwTerminate();
        return -1;
    }
#pragma endregion

#pragma region imgui initialization

    glfwMakeContextCurrent(imguiWin);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(imguiWin, true);
    ImGui_ImplOpenGL3_Init("#version 440");

#pragma endregion

#pragma region Main window initialization
    glfwMakeContextCurrent(mainWin);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

#ifdef MDEBUG
    int flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
#endif

    glfwSetFramebufferSizeCallback(mainWin, framebuffer_size_callback);
    glfwSetCursorPosCallback(mainWin, cursor_position_callback);
    glfwSetKeyCallback(mainWin, key_callback);


    int height, width;
    glfwGetFramebufferSize(mainWin, &width, &height);

    aspect_ratio = width / height;

    //std::cout << glGetString(GL_VERSION) << std::endl;
    auto glversion = glGetString(GL_VERSION);
    auto glrenderer = glGetString(GL_RENDERER);
    auto glvendor = glGetString(GL_VENDOR);
    glEnable(GL_MULTISAMPLE);
    //glEnable(GL_CULL_FACE);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    GLcall(glEnable(GL_DEPTH_TEST));
#pragma endregion


    {
        std::string resPath = searchRes();
        Shader lampShader(resPath + "/shaders/lamp.vert", resPath + "/shaders/lamp.frag");
        Shader cubeShader(resPath + "/shaders/cube_final.vert", resPath + "/shaders/cube_final.frag");
        Shader cubeShader2(resPath + "/shaders/cube_final2.vert", resPath + "/shaders/cube_final2.frag");
        skyBox skybox(resPath+pathDelemeter+"skybox", resPath + pathDelemeter + "shaders");


#pragma region setup matrices and vectors for camera

        glm::vec3 translate(0, 0, 0);
        glm::vec3 scale(1, 1, 1);
        glm::vec3 rotate(0, 0, 0);

        glm::vec3 cubePositions[] = {
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(2.0f, 5.0f, -15.0f),
                glm::vec3(-1.5f, -2.2f, -2.5f),
                glm::vec3(-3.8f, -2.0f, -12.3f),
                glm::vec3(2.4f, -0.4f, -3.5f),
                glm::vec3(-1.7f, 3.0f, -7.5f),
                glm::vec3(1.3f, -2.0f, -2.5f),
                glm::vec3(1.5f, 2.0f, -2.5f),
                glm::vec3(1.5f, 0.2f, -1.5f),
                glm::vec3(-1.3f, 1.0f, -1.5f) };

        float near_point{ 0.1f }, far_point(100);
        glm::mat4 final_rotation(1.0);
        glm::mat4 rotation(1.0);
        glm::mat4 final_translation(1.0);
        glm::mat4 translation(1.0);
        glm::mat4 scaling(1.0);

        glm::mat4 model(1.0);
        glm::mat4 model_const(1.0);
        glm::mat4 view(1.0);
        glm::mat4 projpersp(1.0);
#pragma endregion

        double LastFrame = 0;


        while (!(glfwWindowShouldClose(imguiWin) || glfwWindowShouldClose(mainWin)))
        {
            glfwMakeContextCurrent(mainWin);
            glfwPollEvents();
            deltatime = glfwGetTime() - LastFrame;
            LastFrame = glfwGetTime();

            processInput(mainWin);

            GLcall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

            rotation = {
                    glm::rotate(
                            glm::rotate(
                                    glm::rotate(
                                            glm::mat4(1.0), glm::radians(rotate.z), glm::vec3(0.0f, 0.0f, 1.0f)),
                                    glm::radians(rotate.y), glm::vec3(0.0f, 1.0f, 0.0f)),
                            glm::radians(rotate.x), glm::vec3(1.0f, 0.0f, 0.0f))
            };

            scaling = glm::scale(glm::mat4(1.0), scale);
            translation = glm::translate(glm::mat4(1.0), translate);

            model = translation * rotation * scaling;

            view = glm::lookAt(cam.Camera_Position, cam.Camera_Position + cam.Camera_Facing_Direction * cam.Camera_Target_distance, cam.Camera_Up);

            projpersp = glm::perspective(glm::radians(perspective_fov), aspect_ratio, near_point, far_point);

            //TODO: drawing here

            glfwSwapBuffers(mainWin);

            glfwMakeContextCurrent(imguiWin);
            {
                glClear(GL_COLOR_BUFFER_BIT);

                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();

                {
                    static float f = 0.0f;
                    static int counter = 0;

                    ImGui::Begin("Controls");

                    ImGui::SliderFloat3("translate", &translate.x, -5.0f, 2.0f);

                    ImGui::SliderFloat3("scale", &scale.x, -3.0f, 3.0f);
                    ImGui::SliderFloat3("rotate", &rotate.x, -180.0f, 180.0f);
                    //ImGui::SliderFloat3("color", &lightColor.x, 0.0f, 1.0f);
                    //ImGui::SliderFloat("near point", &near_point, 0.0f, 5.0f);
                    ImGui::SliderFloat3("pointLight color ambient", &AmbientLight.x, 0.0f, 1.0f);
                    ImGui::SliderFloat3("pointLight color diffused", &pointLights[selected_light].diffusecolor.x, 0.0f, 1.0f);
                    ImGui::SliderFloat3("pointLight color specular", &pointLights[selected_light].specularcolor.x, 0.0f, 1.0f);
                    ImGui::Text("selected Lamp :%d", selected_light);
                    ImGui::Text("position of light is %.3f %.3f %.3f", pointLights[selected_light].position.x, pointLights[selected_light].position.y, pointLights[selected_light].position.z);
                    //ImGui::SliderFloat3("rotate", &light.direction.x, 0.0f, 1.0f);
                    //ImGui::SliderFloat3("Specular Reflectivity", &material.specular.x, 0.0f, 1.0f);
                    ImGui::SliderFloat("liear term", &pointLights[selected_light].linear, 0.001f, 0.7f);
                    ImGui::SliderFloat("quadratic term", &pointLights[selected_light].quadratic, 0.000007f, 1.8f);
                    //ImGui::SliderFloat("Shineness", &material.shininess, 0.0f, 512.0f);

                    ImGui::SliderFloat("far point", &far_point, 0.0f, 200.0f);
                    ImGui::SliderFloat("POV", &perspective_fov, 0.0f, 180.0f);
                    ImGui::End();

                    ImGui::Begin("Information");
                    ImGui::Text("no of Drawcalls :%d",noDrawCalls);
                    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

                    ImGui::Text("aspect Ratio : %f", aspect_ratio);
                    ImGui::Text("value of facing vector is %.3f %.3f %.3f", cam.Camera_Facing_Direction.x, cam.Camera_Facing_Direction.y, cam.Camera_Facing_Direction.z);
                    ImGui::Text("value of pitch = %.3f yaw = %.3f roll = %.3f", glm::degrees(cam.getAngles().m_pitch), glm::degrees(cam.getAngles().m_yaw), glm::degrees(cam.getAngles().m_roll));
                    ImGui::Text("value of up vector is %.3f %.3f %.3f", cam.Camera_Up.x, cam.Camera_Up.y, cam.Camera_Up.z);
                    ImGui::Text("glrenderer %s", glrenderer);
                    ImGui::Text("glversion %s", glversion);
                    ImGui::Text("glvendor %s", glvendor);

                    // ImGui::Text("( %.3f ,%.3f, %.3f, %.3f )\n( %.3f ,%.3f, %.3f, %.3f )\n( %.3f, %.3f, %.3f, %.3f )\n( %.3f, %.3f, %.3f, %.3f )",
                    // 	model[0][0], model[0][1], model[0][2], model[0][3],
                    // 	model[1][0], model[1][1], model[1][2], model[1][3],
                    // 	model[2][0], model[2][1], model[2][2], model[2][3],
                    // 	model[3][0], model[3][1], model[3][2], model[3][3]);
                    // ImGui::Text("View matrix: ");
                    // ImGui::Text("( %.3f ,%.3f, %.3f, %.3f )\n( %.3f ,%.3f, %.3f, %.3f )\n( %.3f, %.3f, %.3f, %.3f )\n( %.3f, %.3f, %.3f, %.3f )",
                    // 	view[0][0], view[0][1], view[0][2], view[0][3],
                    // 	view[1][0], view[1][1], view[1][2], view[1][3],
                    // 	view[2][0], view[2][1], view[2][2], view[2][3],
                    // 	view[3][0], view[3][1], view[3][2], view[3][3]);
                    // ImGui::Text("Projection matrix: ");
                    // ImGui::Text("( %.3f ,%.3f, %.3f, %.3f )\n( %.3f ,%.3f, %.3f, %.3f )\n( %.3f, %.3f, %.3f, %.3f )\n( %.3f, %.3f, %.3f, %.3f )",
                    // 	projpersp[0][0], projpersp[0][1], projpersp[0][2], projpersp[0][3],
                    // 	projpersp[1][0], projpersp[1][1], projpersp[1][2], projpersp[1][3],
                    // 	projpersp[2][0], projpersp[2][1], projpersp[2][2], projpersp[2][3],
                    // 	projpersp[3][0], projpersp[3][1], projpersp[3][2], projpersp[3][3]);

                    ImGui::Text("cursor pos: %f , %f", mx, my);
                    ImGui::Text("captured: %d", captured);

                    //					ImGui::Text("Direction: %f, %f, %f", light.direction.x, light.direction.y, light.direction.z);
                    ImGui::End();
                }
                // Rendering
                ImGui::Render();
                int display_w, display_h;
                glfwGetFramebufferSize(imguiWin, &display_w, &display_h);
                glViewport(0, 0, display_w, display_h);
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

                glfwSwapBuffers(imguiWin);
            }
        }
        glfwDestroyWindow(mainWin);
        glfwDestroyWindow(imguiWin);
    }
    glfwTerminate();
    return 0;
}