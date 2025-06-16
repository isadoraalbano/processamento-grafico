#include <iostream>
#include <vector>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace std;
using namespace glm;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
int setupShader();
int setupSprite();
int loadTexture(string filePath);

const GLuint WIDTH = 800, HEIGHT = 600;

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded
const GLchar *vertexShaderSource = R"(
 #version 400
 layout (location = 0) in vec3 position;
 layout (location = 1) in vec2 texc;
 out vec2 tex_coord;
 uniform mat4 projection;
 uniform mat4 model;
 void main()
 {
	tex_coord = texc;
	gl_Position = projection * model * vec4(position.x, position.y, position.z, 1.0);
 }
 )";

// Código fonte do Fragment Shader (em GLSL): ainda hardcoded
const GLchar *fragmentShaderSource = R"(
 #version 400
 in vec2 tex_coord;
 out vec4 color;
 uniform sampler2D tex_buff;
 void main()
 {
	 color = texture(tex_buff,tex_coord);
 }
 )";

class Sprite {
private:
    GLuint VAO;
    GLuint textureID;
    vec2 position;
    vec2 size;
    float rotation;

public:
    Sprite(const string& texturePath) {
        this->VAO = setupSprite();
        this->textureID = loadTexture(texturePath);
        this->position = vec2(0.0);
        this->size = vec2(1.0);
        this->rotation = 0.0;
    }

    void Draw(GLuint shaderProgram) {
        mat4 model = mat4(1.0);
        model = translate(model, vec3(position, 0.0));
        model = rotate(model, radians(rotation), vec3(0.0, 0.0, 1.0));
        model = scale(model, vec3(size, 1.0));

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, value_ptr(model));
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }

    void SetPosition(vec2 pos) { position = pos; }
    void SetSize(vec2 s) { size = s; }
    void SetRotation(float rot) { rotation = rot; }
};

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Texturizacoes", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }

    GLuint shaderProgram = setupShader();
    
    vector<Sprite> sprites = {
        Sprite("../assets/sprites/sky_back.png"),
        Sprite("../assets/sprites/aurora.png"),
        Sprite("../assets/sprites/clouds_1.png"),
        Sprite("../assets/sprites/clouds_2.png"),
        Sprite("../assets/sprites/rocks_tex.png"),
        Sprite("../assets/sprites/coruja.png"),
        Sprite("../assets/sprites/vampirinho.png")
    };

    sprites[0].SetPosition(vec2(400, 300)); sprites[0].SetSize(vec2(800, 600));
    sprites[1].SetPosition(vec2(400, 300)); sprites[1].SetSize(vec2(800, 600));
    sprites[2].SetPosition(vec2(150, 100)); sprites[2].SetSize(vec2(200, 150));
    sprites[3].SetPosition(vec2(650, 150)); sprites[3].SetSize(vec2(200, 150));
    sprites[4].SetPosition(vec2(400, 500)); sprites[4].SetSize(vec2(800, 200));
    sprites[5].SetPosition(vec2(200, 400)); sprites[5].SetSize(vec2(100, 100));
    sprites[6].SetPosition(vec2(600, 585)); sprites[6].SetSize(vec2(80, 80));

    glUseProgram(shaderProgram);
    
    mat4 projection = ortho(0.0, 800.0, 600.0, 0.0, -1.0, 1.0);

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, value_ptr(projection));
    glUniform1i(glGetUniformLocation(shaderProgram, "tex_buff"), 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        for (auto& sprite : sprites) {
            sprite.Draw(shaderProgram);
        }

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

int setupShader() {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

int setupSprite() {
    GLfloat vertices[] = {
        -0.5,  0.5, 0.0,  0.0, 1.0,
        -0.5, -0.5, 0.0,  0.0, 0.0,
         0.5,  0.5, 0.0,  1.0, 1.0,
         0.5, -0.5, 0.0,  1.0, 0.0
    };

    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}

int loadTexture(string filePath) {
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);

    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        cout << "Failed to load texture: " << filePath << endl;
    }
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texID;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

