
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <ctime>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;
using namespace glm;

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 800, HEIGHT = 600;
const GLuint ROWS = 6, COLS = 8;
const GLuint QUAD_WIDTH = 100, QUAD_HEIGHT = 100;
const float dMax = sqrt(3.0);

// Protótipos das funções
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
GLuint createQuad();
int setupShader();
void eliminarSimilares(float tolerancia);
void reiniciarJogo();
void validaFimJogo();

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded
const GLchar* vertexShaderSource = R"(
#version 400
layout (location = 0) in vec3 position;
uniform mat4 projection;
uniform mat4 model;
void main()
{
	gl_Position = projection * model * vec4(position, 1.0);
}
)";

// Código fonte do Fragment Shader (em GLSL): ainda hardcoded
const GLchar* fragmentShaderSource = R"(
#version 400
uniform vec4 inputColor;
out vec4 color;
void main()
{
	color = inputColor;
}
)";

struct Quad {
	vec3 position;
	vec3 dimensions;
	vec3 color;
	bool eliminated;
};

Quad grid[ROWS][COLS];
int iSelected = -1;
int scoreFinal = 0;
int tentativas = 0;

// Função MAIN
int main()
{
	srand(time(0));

	// Inicialização da GLFW
	glfwInit();

	// Criação da janela GLFW
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Jogo das cores! ❤️🩷🧡💛💚", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Registro das funções de callback
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	// GLAD: carrega todos os ponteiros da OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
	}

	// Informações da versão do driver
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	GLuint shaderID = setupShader();
	GLuint VAO = createQuad();
	reiniciarJogo();

	glUseProgram(shaderID);
	GLint colorLoc = glGetUniformLocation(shaderID, "inputColor");
	mat4 projection = ortho(0.0, 800.0, 600.0, 0.0, -1.0, 1.0);
	glUniformMatrix4fv(glGetUniformLocation(shaderID, "projection"), 1, GL_FALSE, value_ptr(projection));

	// Loop principal
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glLineWidth(10);
		glPointSize(20);
		glBindVertexArray(VAO);

		if (iSelected > -1)
		{
			eliminarSimilares(0.2f);
		}

		// Inicializar a grid
		for (int i = 0; i < ROWS; i++)
		{
			for (int j = 0; j < COLS; j++)
			{
				if (!grid[i][j].eliminated)
				{
					mat4 model = mat4(1);
					model = translate(model, grid[i][j].position);
					model = scale(model, grid[i][j].dimensions);
					glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));
					glUniform4f(colorLoc, grid[i][j].color.r, grid[i][j].color.g, grid[i][j].color.b, 1.0f);
					glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
				}
			}
		}

		glBindVertexArray(0);
		validaFimJogo();
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}

// Verifica se todos os retangulos foram eliminados
void validaFimJogo()
{
	bool allEliminated = true;
	for (int i = 0; i < ROWS; i++) {
		for (int j = 0; j < COLS; j++) {
			if (!grid[i][j].eliminated) {
				allEliminated = false;
				break;
			}
		}
		if (!allEliminated) break;
	}
	if (allEliminated) {
		cout << "FIM DE JOGO - Pontuacao final: " << scoreFinal << ", Tentativas: " << tentativas << endl;
		reiniciarJogo();
	}
};

// Callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
		reiniciarJogo();
}

// Callback de clique do mouse
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		int x = xpos / QUAD_WIDTH;
		int y = ypos / QUAD_HEIGHT;
		if (!grid[y][x].eliminated)
		{
			iSelected = x + y * COLS;
			tentativas++;
			scoreFinal -= 2;
		}
	}
}

GLuint createQuad()
{
	GLuint VAO, VBO;
	GLfloat vertices[] = {
		-0.5f,  0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
		 0.5f,  0.5f, 0.0f,
		 0.5f, -0.5f, 0.0f
	};
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	return VAO;
}

int setupShader()
{
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

void eliminarSimilares(float tolerancia)
{
	int x = iSelected % COLS;
	int y = iSelected / COLS;
	vec3 C = grid[y][x].color;
	grid[y][x].eliminated = true;
	int pontos = 0;

	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLS; j++)
		{
			if (!grid[i][j].eliminated)
			{
				vec3 O = grid[i][j].color;
				float d = sqrt(pow(C.r - O.r, 2) + pow(C.g - O.g, 2) + pow(C.b - O.b, 2));
				float dd = d / dMax;
				if (dd <= tolerancia)
				{
					grid[i][j].eliminated = true;
					pontos += 5;
				}
			}
		}
	}
	scoreFinal += pontos;
	cout << "Pontuacao: " << scoreFinal << " | Tentativas: " << tentativas << endl;
	iSelected = -1;
}

void reiniciarJogo()
{
	scoreFinal = 0;
	tentativas = 0;
	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLS; j++)
		{
			grid[i][j].position = vec3(QUAD_WIDTH/2 + j*QUAD_WIDTH, QUAD_HEIGHT/2 + i*QUAD_HEIGHT, 0);
            grid[i][j].dimensions = vec3(QUAD_WIDTH, QUAD_HEIGHT, 1);
            grid[i][j].color = vec3(
                rand() % 256 / 255.0f,
                rand() % 256 / 255.0f,
                rand() % 256 / 255.0f
            );
            grid[i][j].eliminated = false;
		}
	}
}
