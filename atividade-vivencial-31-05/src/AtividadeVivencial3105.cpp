#include <iostream>
#include <string>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>




#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


using namespace std;
using namespace glm;




#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


const GLuint WIDTH = 800, HEIGHT = 600;


const char* vertexShaderSource = R"(
#version 400
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;


uniform mat4 projection;
uniform float offsetX;


out vec2 TexCoord;


void main()
{
    vec3 pos = position;
    pos.x += offsetX;
    gl_Position = projection * vec4(pos, 1.0);
    TexCoord = texCoord;
}
)";


const char* fragmentShaderSource = R"(
#version 400
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D texture1;


void main()
{
    FragColor = texture(texture1, TexCoord);
}
)";


struct Layer {
    GLuint textureID;
    float speedFactor;
    float offsetX = 0.0;
};


vector<Layer> layers;
GLuint quadVAO, quadVBO;
GLuint shaderProgram;


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
int loadTexture(string path);
void initQuad();
GLuint createShaderProgram();


int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);


    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Parallax Scrolling", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);


    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);


    glViewport(0, 0, WIDTH, HEIGHT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    shaderProgram = createShaderProgram();
    initQuad();


    const string textures[] = {
        "../assets/sprites/sky.png",
        "../assets/sprites/clouds_1.png",
        "../assets/sprites/clouds_2.png",
        "../assets/sprites/clouds_3.png",
        "../assets/sprites/clouds_4.png",
        "../assets/sprites/rocks_1.png",
        "../assets/sprites/rocks_2.png"
    };


    float speeds[] = { 0.1, 0.2, 0.3, 0.4, 0.5, 0.8, 1.0 };


    for (int i = 0; i < 7; i++) {
        Layer layer;
        layer.textureID = loadTexture(textures[i]);
        layer.speedFactor = speeds[i];
        layers.push_back(layer);
    }


    GLuint vampTex = loadTexture("../assets/sprites/Vampirinho.png");


    mat4 projection = glm::ortho(0.0, 800.0, 600.0 , 0.0, -1.0, 1.0);


    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);


        glUseProgram(shaderProgram);
        glBindVertexArray(quadVAO);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, value_ptr(projection));


        for (auto& layer : layers) {
            glBindTexture(GL_TEXTURE_2D, layer.textureID);
            glUniform1f(glGetUniformLocation(shaderProgram, "offsetX"), layer.offsetX);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }


        // Vampirinho no centro
        glBindTexture(GL_TEXTURE_2D, vampTex);
        glUniform1f(glGetUniformLocation(shaderProgram, "offsetX"), 0.0);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


        glfwSwapBuffers(window);
    }


    glfwTerminate();
    return 0;
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        double delta = 10.0;
        if (key == GLFW_KEY_LEFT || key == GLFW_KEY_A) {
            for (auto& layer : layers) {
                layer.offsetX += delta * layer.speedFactor;
            }
        }
        if (key == GLFW_KEY_RIGHT || key == GLFW_KEY_D) {
            for (auto& layer : layers) {
                layer.offsetX -= delta * layer.speedFactor;
            }
        }
    }
}


GLuint createShaderProgram()
{
    GLint success;
    char infoLog[512];


    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexShaderSource, NULL);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        cout << "Vertex Shader Error: " << infoLog << endl;
    }


    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        cout << "Fragment Shader Error: " << infoLog << endl;
    }


    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    return program;
}


void initQuad()
{
    float quad[] = {
        0, 0, 0.0, 0.0, 0.0,
        0, 600, 0.0, 0.0, 1.0,
        800, 0, 0.0, 1.0, 0.0,
        800, 600, 0.0, 1.0, 1.0
    };


    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);


    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}


int loadTexture(string path)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    int width, height, nrChannels;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = nrChannels == 4 ? GL_RGBA : GL_RGB;
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    } else {
        cout << "Failed to load texture: " << path << endl;
    }
    return textureID;
}


