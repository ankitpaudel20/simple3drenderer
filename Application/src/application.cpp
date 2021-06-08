
#include <random>

#include "Shader.h"
#include "Texture.h"
#include "callback.h"
#include "drawable.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "renderer.h"
#include "shapes.h"

int main(int argc, char *argv[]) {

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

    GLFWwindow *mainWin;
    GLFWwindow *imguiWin;

    imguiWin = glfwCreateWindow(800, 600, "imgui", NULL, NULL);
    mainWin = glfwCreateWindow(1000, 750, "main", NULL, NULL);

    if (!(mainWin && imguiWin)) {
        glfwTerminate();
        return -1;
    }
#pragma endregion

#pragma region imgui initialization

    glfwMakeContextCurrent(imguiWin);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(imguiWin, true);
    ImGui_ImplOpenGL3_Init("#version 440");

#pragma endregion

#pragma region Main window initialization
    glfwMakeContextCurrent(mainWin);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glfwSwapInterval(1);

#ifdef MDEBUG
    int flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
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

    // std::cout << glGetString(GL_VERSION) << std::endl;
    auto glversion = glGetString(GL_VERSION);
    auto glrenderer = glGetString(GL_RENDERER);
    auto glvendor = glGetString(GL_VENDOR);
    glEnable(GL_MULTISAMPLE);
    // glEnable(GL_CULL_FACE);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    (glEnable(GL_DEPTH_TEST));

#pragma endregion

    {
        std::string resPath = searchRes();

        scene mainScene;
        glfwSetWindowUserPointer(mainWin, &mainScene);
        mainScene.pointLights.emplace_back(vec3(1), 1);
        //mainScene.dirLights.emplace_back(vec3(-1, -1, 0), 0.8, 1, 1);
        auto lamp = mainScene.loadModel(resPath + "/3dModels/box.obj", "cube_final2", "light");
        for (auto &mesh : lamp->meshes) {
            mesh->setScale(vec3(0.05));
            mesh->doLightCalculations = false;
        }
        mainScene.pointLights[0].setmodel(lamp);
        mainScene.skybox = resPath + "/skybox";

        // auto cyborg = mainScene.loadModel(resPath + "/3dModels/cyborg/cyborg.obj", "cube_final2", "cyborg");
        // auto nanosuit = mainScene.loadModel(resPath + "/3dModels/nanosuit/nanosuit.obj", "cube_final2", "nanosuit", true);
        // auto hammer = mainScene.loadModel(resPath + "/3dModels/hammer/hammer.obj", "cube_final2", "hammer", true);

        // hammer->delpos(vec3(0, 5, -3));
        // hammer->setScale(vec3(0.5));

        // nanosuit->delpos(vec3(3, 0, 0));
        // nanosuit->setRotation(vec3(90, 0, 0));
        // cyborg->delpos(vec3(-3, 0, 0));
        // cyborg->setScale(vec3(3));
        // nanosuit->setScale(vec3(0.9));

        auto colorCube = mainScene.loadModel(resPath + "/3dModels/color/testColored.obj", "cube_final2", "color");

        auto glass = mainScene.loadModel(resPath + "/3dModels/box.obj", "cube_final2", "glass_cube");
         glass->meshes[0]->delpos(-1);
         glass->meshes[0]->setScale(vec3(1, 1, 0.05));

        // drawable<Vertex> side_z, side_not_z, frame;
        // int i = 0;
        // for (int i = 0; i < cube::pos.size(); i += 3) {
        //     auto v1 = cube::pos[i];
        //     auto v2 = cube::pos[i + 1];
        //     auto v3 = cube::pos[i + 2];
        //     vec3 normal;
        //     if (v1.x == v2.x && v2.x == v3.x) {
        //         normal = vec3(v1.x, 0, 0);
        //     } else if (v1.y == v2.y && v2.y == v3.y) {
        //         normal = vec3(0, v1.y, 0);
        //     } else
        //         normal = vec3(0, 0, v1.z);
        //     normal = vec3::normalize(normal);
        //     // auto gen_normal = -vec3::cross(v1 - v2, v1 - v3);
        //     // if (gen_normal != normal) {
        //     //     DEBUG_BREAK;
        //     // }
        //     // normal = gen_normal;
        //     if (normal.z == -1) {
        //         side_z.m_vertices.emplace_back(v1, normal);
        //         side_z.m_vertices.emplace_back(v2, normal);
        //         side_z.m_vertices.emplace_back(v3, normal);
        //         int p = side_z.m_indices.empty() ? 0 : side_z.m_indices.back() + 1;
        //         side_z.m_indices.push_back(p);
        //         side_z.m_indices.push_back(p + 1);
        //         side_z.m_indices.push_back(p + 2);
        //     } else if (normal.z == 1) {
        //         side_not_z.m_vertices.emplace_back(v1, normal);
        //         side_not_z.m_vertices.emplace_back(v2, normal);
        //         side_not_z.m_vertices.emplace_back(v3, normal);
        //         int p = side_not_z.m_indices.empty() ? 0 : side_not_z.m_indices.back() + 1;
        //         side_not_z.m_indices.push_back(p);
        //         side_not_z.m_indices.push_back(p + 1);
        //         side_not_z.m_indices.push_back(p + 2);
        //     } else {
        //         frame.m_vertices.emplace_back(v1, normal);
        //         frame.m_vertices.emplace_back(v2, normal);
        //         frame.m_vertices.emplace_back(v3, normal);
        //         int p = frame.m_indices.empty() ? 0 : frame.m_indices.back() + 1;
        //         frame.m_indices.push_back(p);
        //         frame.m_indices.push_back(p + 1);
        //         frame.m_indices.push_back(p + 2);
        //     }
        // }
        // frame.name = "frame";
        // side_z.name = "side_z";
        // side_not_z.name = "side_not_z";
        // frame.shader = side_not_z.shader = side_z.shader = "cube_final2";
        // frame.delpos(-2);
        // side_z.delpos(-2);
        // side_z.delpos(vec3(0, 0, -1));
        // side_not_z.delpos(-2);
        // side_not_z.delpos(vec3(0, 0, 1));
        // Model::meshes_loaded["frame"] = frame;
        // Model::meshes_loaded["side_z"] = side_z;
        // Model::meshes_loaded["side_not_z"] = side_not_z;
        // node trans;
        // trans.meshes.push_back(&Model::meshes_loaded["frame"]);
        // trans.meshes.push_back(&Model::meshes_loaded["side_z"]);
        // trans.meshes.push_back(&Model::meshes_loaded["side_not_z"]);
        // mainScene.nodes["trans"] = &trans;

        renderer mainRend;
        mainRend.currentScene = &mainScene;
        mainRend.init();

#pragma region setup matrices and vectors for camera

        glm::vec3 translate(0, 0, 0);
        glm::vec3 scale(1, 1, 1);
        glm::vec3 rotate(0, 0, 0);

        glm::vec3 cubePositions[] = {
            glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(2.0f, 5.0f, -15.0f),
            glm::vec3(-1.5f, -2.2f, -2.5f), glm::vec3(-3.8f, -2.0f, -12.3f),
            glm::vec3(2.4f, -0.4f, -3.5f), glm::vec3(-1.7f, 3.0f, -7.5f),
            glm::vec3(1.3f, -2.0f, -2.5f), glm::vec3(1.5f, 2.0f, -2.5f),
            glm::vec3(1.5f, 0.2f, -1.5f), glm::vec3(-1.3f, 1.0f, -1.5f)};

        float near_point{0.1f}, far_point(100);
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
        float shininess = 32, amb = 0.1, diff = 1.0;
        float newshininess = shininess, newamb = amb, newdiff = diff;
        while (!(glfwWindowShouldClose(imguiWin) || glfwWindowShouldClose(mainWin))) {
            glfwMakeContextCurrent(mainWin);
            glfwPollEvents();
            processHoldEvent(mainWin);
            mainScene.deltatime = glfwGetTime() - LastFrame;
            LastFrame = glfwGetTime();

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            rotation = {glm::rotate(
                glm::rotate(glm::rotate(glm::mat4(1.0), glm::radians(rotate.z),
                                        glm::vec3(0.0f, 0.0f, 1.0f)),
                            glm::radians(rotate.y), glm::vec3(0.0f, 1.0f, 0.0f)),
                glm::radians(rotate.x), glm::vec3(1.0f, 0.0f, 0.0f))};

            scaling = glm::scale(glm::mat4(1.0), scale);
            translation = glm::translate(glm::mat4(1.0), translate);

            model = translation * rotation * scaling;

            view = glm::lookAt(mainScene.cam.Camera_Position,
                               mainScene.cam.Camera_Position +
                                   mainScene.cam.Camera_Facing_Direction *
                                       mainScene.cam.Camera_Target_distance,
                               mainScene.cam.Camera_Up);

            projpersp = glm::perspective(glm::radians(mainScene.cam.FOV), aspect_ratio, near_point, far_point);

            mainRend.clear();
            mainRend.Draw();

            glfwSwapBuffers(mainWin);

            glfwMakeContextCurrent(imguiWin);
            {
                glClear(GL_COLOR_BUFFER_BIT);

                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();

                {
                    ImGui::Begin("Controls");

                    ImGui::SliderFloat3("translate", &translate.x, -5.0f, 2.0f);

                    ImGui::SliderFloat3("scale", &scale.x, -3.0f, 3.0f);
                    ImGui::SliderFloat3("rotate", &rotate.x, -180.0f, 180.0f);
                    // ImGui::SliderFloat3("color", &lightColor.x, 0.0f, 1.0f);
                    // ImGui::SliderFloat("near point", &near_point, 0.0f, 5.0f);
                    ImGui::SliderFloat3("ambientLight color", &mainScene.ambientLight.x, 0.0f, 1.0f);
                    vec3 color = mainScene.pointLights[selected_light].getColor();
                    ImGui::SliderFloat3("pointLight color diffused", &color.x, 0.0f, 1.0f);
                    if (color != mainScene.pointLights[selected_light].getColor()) {
                        mainScene.pointLights[selected_light].setColor(color);
                    }
                    ImGui::SliderFloat("pointLight intensity", &mainScene.pointLights[selected_light].intensity, 0.f, 5.f);

                    ImGui::SliderFloat("linear term", &mainScene.pointLights[selected_light].linear, 0.001f, 0.7f);
                    ImGui::SliderFloat("quadratic term", &mainScene.pointLights[selected_light].quadratic, 0.000007f, 1.8f);

                    ImGui::Text("position of light is %.3f %.3f %.3f",
                                mainScene.pointLights[selected_light].getpos().x,
                                mainScene.pointLights[selected_light].getpos().y,
                                mainScene.pointLights[selected_light].getpos().z);
                    // ImGui::SliderFloat3("rotate", &light.direction.x, 0.0f, 1.0f);
                    // ImGui::SliderFloat3("Specular Reflectivity", &material.specular.x,
                    // 0.0f, 1.0f);
                    //ImGui::SliderFloat3("direction light direction", &mainScene.dirLights[0].direction.x, -1.f, 1.f);
                    //ImGui::SliderFloat("direction light intensity", &mainScene.dirLights[0].intensity, 0.f, 1.f);

                    ImGui::SliderFloat("Shineness", &newshininess, 0.0f, 512.0f);
                    ImGui::SliderFloat("ambientStrength", &newamb, 0.0f, 1.f);
                    ImGui::SliderFloat("diffuseStrength", &newdiff, 0.0f, 1.f);

                    if (newshininess != shininess || newamb != amb || newdiff != diff) {
                        shininess = newshininess;
                        amb = newamb;
                        diff = newdiff;
                        // for (auto &mesh : nanosuit->meshes) {
                        //     mesh.material.shininess = newshininess;
                        //     mesh.material.AmbientStrength = newamb;
                        //     mesh.material.DiffuseStrength = newdiff;
                        // }
                    }

                    ImGui ::SliderFloat("far point", &far_point, 0.0f, 200.0f);
                    ImGui::SliderFloat("POV", &mainScene.cam.FOV, 0.0f, 180.0f);
                    ImGui::SliderFloat("cameraSensitivity", &mainScene.cam.sensitivity,
                                       0.0f, 3.0f);
                    ImGui::SliderFloat("camera speed", &mainScene.cam.speed, 0.0f, 5.0f);
                    ImGui::End();

                    ImGui::Begin("Information");
                    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

                    ImGui::Text("aspect Ratio : %f", aspect_ratio);
                    ImGui::Text("value of facing vector is %.3f %.3f %.3f",
                                mainScene.cam.Camera_Facing_Direction.x,
                                mainScene.cam.Camera_Facing_Direction.y,
                                mainScene.cam.Camera_Facing_Direction.z);
                    ImGui::Text(
                        "camera position %.3f %.3f %.3f", mainScene.cam.Camera_Position.x,
                        mainScene.cam.Camera_Position.y, mainScene.cam.Camera_Position.z);
                    ImGui::Text("value of pitch = %.3f yaw = %.3f roll = %.3f",
                                glm::degrees(mainScene.cam.getAngles().m_pitch),
                                glm::degrees(mainScene.cam.getAngles().m_yaw),
                                glm::degrees(mainScene.cam.getAngles().m_roll));
                    ImGui::Text("value of up vector is %.3f %.3f %.3f",
                                mainScene.cam.Camera_Up.x, mainScene.cam.Camera_Up.y,
                                mainScene.cam.Camera_Up.z);
                    ImGui::Text("glrenderer %s", glrenderer);
                    ImGui::Text("glversion %s", glversion);
                    ImGui::Text("glvendor %s", glvendor);

                    ImGui::Text("cursor pos: %f , %f", mx, my);
                    ImGui::Text("captured: %d", captured);

                    //					ImGui::Text("Direction: %f, %f,
                    //%f", light.direction.x, light.direction.y, light.direction.z);
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
