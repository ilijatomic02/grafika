#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>


void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
unsigned  int width1;
unsigned  int height1;

// camera

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct ProgramState {
    glm::vec3 clearColor = glm::vec3(1.0);
    bool ImGuiEnabled = false;
    Camera camera;
    bool CameraMouseMovementUpdateEnabled = true;
    glm::vec3 backpackPosition = glm::vec3(0.0f);
    float backpackScale = 0.05f;
    PointLight pointLight;
    ProgramState()
            : camera(glm::vec3(139.0f, 36.0f, 28.0f)) {}
};

ProgramState *programState;

void DrawImGui(ProgramState *programState);
unsigned int loadTexture(char const * path);


int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    //stbi_set_flip_vertically_on_load(true);

    programState = new ProgramState;
    if (programState->ImGuiEnabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    // Init Imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;



    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
   // Shader ourShader("resources/shaders/5.4.light_casters.vs", "resources/shaders/5.4.light_casters.fs");
    Shader ourShader("resources/shaders/2.model_lighting.vs", "resources/shaders/2.model_lighting.fs");
    Shader shader ("resources/shaders/3.1.blending.vs","resources/shaders/3.1.blending.fs");


    float transparentVertices[] = {
            // positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
            0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
            0.0f, -0.5f,  0.0f,  0.0f,  1.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  1.0f,

            0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  1.0f,
            1.0f,  0.5f,  0.0f,  1.0f,  0.0f
    };
    vector<glm::vec3> vegetation
            {
//                    glm::vec3(25.0f, 15.0f, -30.5f),
//                    glm::vec3(15.0f, 15.0f, -30.5f),
//                    glm::vec3(5.0f, 15.0f, -30.5f),
//                    glm::vec3(-5.0f, 15.0f, -30.5f),
//                    glm::vec3(25.0f, 15.0f, -40.5f)
//                    glm::vec3(25.0f, 11.0f, -32.5f),
//                    glm::vec3(25.0f, 11.0f, -32.5f),
//                    glm::vec3(25.0f, 11.0f, -32.5f),
//                    glm::vec3(25.0f, 11.0f, -32.5f)
            };
    for(int i=0;i<8;i++){
        for(int j=0;j<5;j++){
          //  vegetation.push_back(glm::vec3(25.0f-i*7,15.0f,-8.5f-10*j));
            vegetation.push_back(glm::vec3(32.0f-i*7,15.0f,84.5f+10*j));

        }
    }
    unsigned int transparentVAO, transparentVBO;
    glGenVertexArrays(1, &transparentVAO);
    glGenBuffers(1, &transparentVBO);
    glBindVertexArray(transparentVAO);
    glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);

    //unsigned int transparentTexture = loadTexture(FileSystem::getPath("resources/textures/pngimg.com - wheat_PNG5.png").c_str());
    unsigned int transparentTexture = loadTexture(FileSystem::getPath("resources/textures/7b8345dcb7edc00dfd7ad0fcfc150056.png").c_str());
    shader.use();
    shader.setInt("texture1", 0);

    // load models
    // -----------
   Model platforma("resources/objects/10438_Circular_Grass_Patch_v1_L3.123c72c0e679-bb4b-4162-b0f0-a70f7575d7d8/10438_Circular_Grass_Patch_v1_iterations-2.obj");
   // Model ourModel("resources/objects/UFO_Saucer_v1_L2.123c50bd261a-1751-44c1-b973-f0dd9e11cecd/13884_UFO_Saucer_v1_l2.obj");
    platforma.SetShaderTextureNamePrefix("material.");
   Model ufo ("resources/objects/UFO_Saucer_v1_L2.123c50bd261a-1751-44c1-b973-f0dd9e11cecd/13884_UFO_Saucer_v1_l2.obj");
    ufo.SetShaderTextureNamePrefix("material.");
    Model krava ("resources/objects/cow/cowTM08New00RTime02.obj");
    krava.SetShaderTextureNamePrefix("material.");
    Model barn ("resources/objects/barn1/Rbarn15.obj");
    barn.SetShaderTextureNamePrefix("material.");

    Model mesec ("resources/objects/moon/moon.obj");
    mesec.SetShaderTextureNamePrefix("material.");

    PointLight& pointLight = programState->pointLight;
    pointLight.position = glm::vec3(20.0f);
    pointLight.ambient = glm::vec3(20.0f);
    pointLight.diffuse = glm::vec3(0.6, 0.6, 0.6);
    pointLight.specular = glm::vec3(1.0, 1.0, 1.0);

    pointLight.constant = 1.0f;
    pointLight.linear = 0.09f;
    pointLight.quadratic = 0.032f;

    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



        // don't forget to enable shader before setting uniforms






        ourShader.use();
        pointLight.position = glm::vec3(4.0 * cos(currentFrame), 4.0f, 4.0 * sin(currentFrame));
        ourShader.setVec3("pointLight[0].position", pointLight.position);
        ourShader.setVec3("pointLight[0].ambient", glm::vec3(0.0f));
        ourShader.setVec3("pointLight[0].diffuse", glm::vec3(0.0f));
        ourShader.setVec3("pointLight[0].specular", glm::vec3(0.0f));
        ourShader.setFloat("pointLight[0].constant", pointLight.constant);
        ourShader.setFloat("pointLight[0].linear", pointLight.linear);
        ourShader.setFloat("pointLight[0].quadratic", pointLight.quadratic);
        ourShader.setVec3("viewPosition", programState->camera.Position);
        ourShader.setFloat("material.shininess", 32.0f);

        pointLight.position = glm::vec3(4.0 * cos(currentFrame), 4.0f, 4.0 * sin(currentFrame));
        ourShader.setVec3("pointLight[1].position", glm::vec3(-50.0f, 150.0f, -200.0f));
        ourShader.setVec3("pointLight[1].ambient", glm::vec3(20.0f));
        ourShader.setVec3("pointLight[1].diffuse", pointLight.diffuse);
        ourShader.setVec3("pointLight[1].specular", pointLight.specular);
        ourShader.setFloat("pointLight[1].constant", pointLight.constant);
        ourShader.setFloat("pointLight[1].linear", pointLight.linear);
        ourShader.setFloat("pointLight[1].quadratic", pointLight.quadratic);
        ourShader.setVec3("viewPosition", programState->camera.Position);
        ourShader.setFloat("material.shininess", 32.0f);


      ourShader.setVec3("spotLight.position", glm::vec3(0.0f,61.781075f, 0.0f));
       ourShader.setVec3("spotLight.direction", glm::vec3(0.0f,-1.0f, 0.0f));
        //ourShader.setVec3("spotLight.position", programState->camera.Position);
       //  ourShader.setVec3("spotLight.direction", programState->camera.Front);
        ourShader.setVec3("spotLight.ambient", 0.0f, 10.0f, 0.0f);
        ourShader.setVec3("spotLight.diffuse", 0.0f, 50.0f, 0.0f);
        ourShader.setVec3("spotLight.specular", 0.0f, 10.0f, 0.0f);
        ourShader.setFloat("spotLight.constant", 1.0f);
        ourShader.setFloat("spotLight.linear", 0.09);
        ourShader.setFloat("spotLight.quadratic", 0.032);
        ourShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(20.5f)));
        ourShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(30.0f)));

        //dir lajt

        ourShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
        ourShader.setVec3("dirLight.ambient", 0.2f, 0.2f, 0.2f);
        ourShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
        ourShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);



        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 1000.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);


        glEnable(GL_CULL_FACE);
        // platforma
        glm::mat4 modelplatforma = glm::mat4(1.0f);

        modelplatforma = glm::scale(modelplatforma, glm::vec3(1.0f));
        modelplatforma=glm::rotate(modelplatforma,glm::radians(270.0f),glm::vec3(1,0,0));

        modelplatforma = glm::translate(modelplatforma,glm::vec3(0.0f));
        ourShader.setMat4("model", modelplatforma);
        platforma.Draw(ourShader);

    //ufo
        glm::mat4 modelufo = glm::mat4(1.0f);

        modelufo = glm::scale(modelufo, glm::vec3(0.1));
        modelufo=glm::rotate(modelufo,glm::radians(270.0f),glm::vec3(1,0,0));
        modelufo=glm::rotate(modelufo,(float)glfwGetTime(),glm::vec3(0,0,1));


      //  modelufo = glm::translate(modelufo,glm::vec3(0.0f,0.0f, 600.0f+50*(sin(glfwGetTime()))));
      modelufo = glm::translate(modelufo,glm::vec3(0.0f,0.0f, 600.0f));

        ourShader.setMat4("model", modelufo);
        ufo.Draw(ourShader);

        //krava

        glm::mat4 modelkrava = glm::mat4(1.0f);

        modelkrava = glm::scale(modelkrava, glm::vec3(1.0f));

        modelkrava = glm::translate(modelkrava,glm::vec3(0.0f,25.0f+5*(sin(glfwGetTime()/2)), 0.0f));
        modelkrava=glm::rotate(modelkrava,(float)glfwGetTime(),glm::vec3(0,1,0));
        modelkrava=glm::rotate(modelkrava,(float)glfwGetTime(),glm::vec3(0,0,1));
        modelkrava=glm::rotate(modelkrava,(float)glfwGetTime(),glm::vec3(1,0,0));


        ourShader.setMat4("model", modelkrava);
        krava.Draw(ourShader);

        //barn2

        glm::mat4 modelbarn = glm::mat4(1.0f);

        modelbarn = glm::translate(modelbarn,glm::vec3(0.0f,9.0f,50.0f));
        modelbarn = glm::scale(modelbarn, glm::vec3(0.04f));
        modelbarn=glm::rotate(modelbarn,glm::radians(90.0f),glm::vec3(0,1,0));
        //modelbarn=glm::rotate(modelbarn,(float)glfwGetTime(),glm::vec3(0,0,1));
        ourShader.setMat4("model", modelbarn);
        barn.Draw(ourShader);

        //mesec
        glm::mat4 modelmesec = glm::mat4(1.0f);

        modelmesec = glm::translate(modelmesec,glm::vec3(-50.0f, 150.0f, -200.0f));
        modelmesec = glm::scale(modelmesec, glm::vec3(25.0f));
       // modelbarn=glm::rotate(modelbarn,glm::radians(90.0f),glm::vec3(0,1,0));
        //modelbarn=glm::rotate(modelbarn,(float)glfwGetTime(),glm::vec3(0,0,1));
        ourShader.setMat4("model", modelmesec);
        mesec.Draw(ourShader);




        glDisable(GL_CULL_FACE);
        shader.use();
        glm::mat4 projection1 = glm::perspective(glm::radians(programState->camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view1 = programState->camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        glBindVertexArray(transparentVAO);
        glBindTexture(GL_TEXTURE_2D, transparentTexture);
        glm::mat4 modeltrava = glm::mat4(1.0f);

        for (unsigned int i = 0; i < vegetation.size(); i++)
        {
            modeltrava = glm::mat4(1.0f);
            modeltrava= glm::translate(modeltrava, vegetation[i]);
            modeltrava=glm::scale(modeltrava,glm::vec3 (12.0f));
            modeltrava=glm::rotate(modeltrava,glm::radians(90.0f),glm::vec3(0,1,0));
            ourShader.setMat4("model", modeltrava);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        if (programState->ImGuiEnabled)
            DrawImGui(programState);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete programState;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    width1=width;
    height1=height;
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (programState->CameraMouseMovementUpdateEnabled)
        programState->camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    programState->camera.ProcessMouseScroll(yoffset);
}

void DrawImGui(ProgramState *programState) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Camera info");
    const Camera& c = programState->camera;
    ImGui::Text("Camera position: (%f, %f, %f)", c.Position.x, c.Position.y, c.Position.z);
    ImGui::Text("(Yaw, Pitch): (%f, %f)", c.Yaw, c.Pitch);
    ImGui::Text("Camera front: (%f, %f, %f)", c.Front.x, c.Front.y, c.Front.z);
    ImGui::Checkbox("Camera mouse update", &programState->CameraMouseMovementUpdateEnabled);
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        programState->ImGuiEnabled = !programState->ImGuiEnabled;
        if (programState->ImGuiEnabled) {
            programState->CameraMouseMovementUpdateEnabled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
}

unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

