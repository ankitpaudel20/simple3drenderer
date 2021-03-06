
// #define USEASSIMP
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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
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
    // glfwSwapInterval(1);

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
    // glfwSwapInterval(1);

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

    glfwGetFramebufferSize(mainWin, &winWIDTH, &winHEIGHT);

    aspect_ratio = winWIDTH / winHEIGHT;

    // std::cout << glGetString(GL_VERSION) << std::endl;
    auto glversion = glGetString(GL_VERSION);
    auto glrenderer = glGetString(GL_RENDERER);
    auto glvendor = glGetString(GL_VENDOR);
    glEnable(GL_MULTISAMPLE);
    //glEnable(GL_CULL_FACE);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    (glEnable(GL_DEPTH_TEST));

#pragma endregion

    {
        std::string resPath = searchRes();

        scene mainScene;
        glfwSetWindowUserPointer(mainWin, &mainScene);

        mainScene.pointLights.emplace_back(vec3(1), 5);
        mainScene.pointLights.emplace_back(vec3(1, 1, 1), 5, vec3(1, 0, 0));
        mainScene.pointLights.emplace_back(vec3(1, 1, -1), 5, vec3(0, 1, 0));
        std::vector<node *> lightmodels;
        lightmodels.push_back(mainScene.loadModel_obj(resPath + "/3dModels/sphere.obj", "cube_final2", "light"));
        lightmodels.push_back(mainScene.loadModel_obj(resPath + "/3dModels/sphere.obj", "cube_final2", "light1"));
        lightmodels.push_back(mainScene.loadModel_obj(resPath + "/3dModels/sphere.obj", "cube_final2", "light2"));
        for (auto &mesh : lightmodels) {
            mesh->setScale(vec3(0.1));
            for (auto &m : mesh->meshes) {
                m->doLightCalculations = false;
            }
        }
        for (int i = 0; i < mainScene.pointLights.size(); i++) {
            mainScene.pointLights[i].setmodel(lightmodels[i]);
        }

        mainScene.dir_light.direction = vec3(0.5, -1, 0.5);

        mainScene.cam.Camera_Position = glm::vec3(10, 10, 10);
        mainScene.cam.DelYaw(45);
        mainScene.cam.DelPitch(-30);

        mainScene.skybox = resPath + "/skybox";

        // auto cyborg1 = mainScene.loadModel_obj(resPath + "/3dModels/cyborg/cyborg1.obj", "cube_final2", "cyborg1");
        // auto nanosuit = mainScene.loadModel(resPath + "/3dModels/nanosuit/nanosuit.obj", "cube_final2", "nanosuit", true);
        // auto nanosuit1 = mainScene.loadModel_obj(resPath + "/3dModels/nanosuit/nanosuit1.obj", "cube_final2", "nanosuit1", true);
        // auto hammer = mainScene.loadModel_obj(resPath + "/3dModels/hammer/hammer.obj", "cube_final2", "hammer", true);

        // hammer->delpos(vec3(0, 5, -3));
        // hammer->setScale(vec3(0.5));

        // cyborg1->setScale(vec3(3));
        // nanosuit->setScale(vec3(0.9));
        // cyborg->delpos(vec3(-3, 1, 0));

        auto colorCube = mainScene.loadModel_obj(resPath + "/3dModels/color/testColored.obj", "cube_final2", "color");
        colorCube->delpos(vec3(3, 3, 3));
        // colorCube2->delpos(vec3(5, 3, 3));
        // auto trans = mainScene.loadModel_obj(resPath + "/3dModels/transparency/transparent.obj", "cube_final2", "trans");
        // auto gallery = mainScene.loadModel_obj(resPath + "/3dModels/gallery/gallery.obj", "cube_final2", "gallery");
        // auto pine = mainScene.loadModel_obj(resPath + "/3dModels/pine/scrubPine.obj", "cube_final2", "pineTree");
        // pine->setScale(vec3(0.01));
        //auto rungholt = mainScene.loadModel_obj(resPath + "/3dModels/rungholt/rungholt.obj", "cube_final2", "rungholt");
        // auto house = mainScene.loadModel_obj(resPath + "/3dModels/rungholt/house.obj", "cube_final2", "house");
        //auto bugatti = mainScene.loadModel_obj(resPath + "/3dModels/bugatti/bugatti.obj", "cube_final2", "bugatti");
        // auto l = mainScene.loadModel_obj(resPath + "/3dModels/box.obj", "cube_final2", "light");
        // l->delpos(3);
        // auto tree = mainScene.loadModel_obj(resPath + "/3dModels/tree/Tree1.obj", "cube_final2", "tree");
        auto sponza = mainScene.loadModel_obj(resPath + "/3dModels/sponza/sponza.obj", "cube_final2", "sponza");
        sponza->setScale(vec3(0.03));
        // auto plane = mainScene.loadModel_obj(resPath + "/3dModels/plane.obj", "cube_final2", "plane");

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

        /* std::vector<std::string> pointlightString(mainScene.pointLights.size());
        std::vector<const char *> pointlightChar(mainScene.pointLights.size());
        for (int i = 0; i < pointlightString.size(); i++) {
            pointlightString[i] = ("pointlight " + std::to_string(i)).c_str();
            pointlightChar[i] = pointlightString[i].c_str();
        }*/

        while (!(glfwWindowShouldClose(imguiWin) || glfwWindowShouldClose(mainWin))) {
            glfwMakeContextCurrent(mainWin);
            glfwPollEvents();
            processHoldEvent(mainWin);
            mainScene.deltatime = glfwGetTime() - LastFrame;
            LastFrame = glfwGetTime();

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
                    ImGui::Checkbox("render depth: ", &mainRend.enable_shadows);
                    ImGui::Checkbox("enable normals", &mainRend.enable_normals);
                    ImGui::SliderFloat("shadow bias", &mainRend.bias, 0.00001f, 1.0f);

                    // ImGui::DragFloat("camera left", &mainRend.left, 0.1);
                    // ImGui::DragFloat("camera right", &mainRend.right, 0.1);
                    // ImGui::DragFloat("camera top", &mainRend.top, 0.1);
                    // ImGui::DragFloat("camera bottom", &mainRend.bottom, 0.1);
                    // ImGui::DragFloat("camera near", &mainScene.cam.nearPoint, 0.1);
                    // ImGui::DragFloat("camera far", &mainScene.cam.farPoint, 0.1);

                    //ImGui::Combo("pointlights", &selected_light, &pointlightChar[0], pointlightChar.size());

                    // ImGui::DragFloat3("translate second colorcube", &translate.x, 0.1, -10.f, 10.f);
                    // colorCube2->setpos(translate);

                    ImGui::SliderFloat3("scale", &scale.x, -3.0f, 3.0f);
                    ImGui::SliderFloat3("rotate", &rotate.x, -180.0f, 180.0f);
                    // ImGui::SliderFloat3("color", &lightColor.x, 0.0f, 1.0f);
                    // ImGui::SliderFloat("near point", &near_point, 0.0f, 5.0f);
                    ImGui::SliderFloat3("ambientLight color", &mainScene.ambientLight.x, 0.0f, 1.0f);
                    ImGui::SliderFloat("ambientLight intensity", &mainScene.ambientStrength, 0.0f, 1.0f);
                    ImGui::SliderFloat("directional intensity", &mainScene.dir_light.intensity, 0.f, 50.f);

                    if (ImGui::SliderFloat3("Light Direction", &mainScene.dir_light.direction.x, -1, +1)) {
                        mainScene.dir_light.direction = vec3::normalize(mainScene.dir_light.direction);
                    }

                    vec3 color = mainScene.pointLights[selected_light].get_diffuse_color();
                    ImGui::ColorEdit3("lightColor", &color.x);
                    if (color != mainScene.pointLights[selected_light].get_diffuse_color()) {
                        mainScene.pointLights[selected_light].setColor(color);
                    }

                    ImGui::SliderFloat("pointLight intensity", &mainScene.pointLights[selected_light].intensity, 0.f, 50.f);
                    ImGui::SliderFloat("pointLight radius", &mainScene.pointLights[selected_light].radius, 0.f, 100.f);
                    ImGui::SliderFloat("linear term", &mainScene.pointLights[selected_light].linear, 0.001f, 0.7f);
                    ImGui::SliderFloat("quadratic term", &mainScene.pointLights[selected_light].quadratic, 0.000007f, 0.5f);

                    ImGui::Text("position of light is %.3f %.3f %.3f",
                                mainScene.pointLights[selected_light].getpos().x,
                                mainScene.pointLights[selected_light].getpos().y,
                                mainScene.pointLights[selected_light].getpos().z);

                    // ImGui::SliderFloat3("rotate", &light.direction.x, 0.0f, 1.0f);
                    // ImGui::SliderFloat3("Specular Reflectivity", &material.specular.x,0.0f, 1.0f);
                    //ImGui::SliderFloat3("direction light direction", &mainScene.dirLights[0].direction.x, -1.f, 1.f);
                    //ImGui::SliderFloat("direction light intensity", &mainScene.dirLights[0].intensity, 0.f, 1.f);

                    // ImGui::SliderFloat("Shineness", &newshininess, 0.0f, 512.0f);
                    // ImGui::SliderFloat("ambientStrength", &newamb, 0.0f, 1.f);
                    // ImGui::SliderFloat("diffuseStrength", &newdiff, 0.0f, 1.f);

                    // if (newshininess != shininess || newamb != amb || newdiff != diff) {
                    //     shininess = newshininess;
                    //     amb = newamb;
                    //     diff = newdiff;
                    //     // for (auto &mesh : nanosuit->meshes) {
                    //     //     mesh.material.shininess = newshininess;
                    //     //     mesh.material.AmbientStrength = newamb;
                    //     //     mesh.material.DiffuseStrength = newdiff;
                    //     // }
                    // }

                    ImGui ::SliderFloat("far point", &far_point, 0.0f, 200.0f);
                    ImGui::SliderFloat("POV", &mainScene.cam.FOV, 0.0f, 180.0f);
                    ImGui::SliderFloat("cameraSensitivity", &mainScene.cam.sensitivity, 0.0f, 3.0f);
                    ImGui::SliderFloat("camera speed", &mainScene.cam.speed, 0.0f, 50.0f);
                    ImGui::SliderFloat("light Speed", &light_speed, 0.0f, 5.0f);
                    ImGui::End();

                    ImGui::Begin("Information");
                    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

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
