#include <iostream>
#include <string>
#include <assert.h>
#include <cmath>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// STB_IMAGE
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

//GLM
#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;
struct Sprite
{
	GLuint VAO;
	GLuint texID;
	vec3 position;
	vec3 dimensions; //tamanho do frame
	float ds, dt;
	int iAnimation, iFrame;
	int nAnimations, nFrames;
	bool isWalking = false;
};

Sprite vampirao;

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);

int setupShader();
int setupSprite(int nAnimations, int nFrames, float &ds, float &dt);
int loadTexture(string filePath, int &width, int &height);

const GLuint WIDTH = 800, HEIGHT = 600;

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded
const GLchar *vertexShaderSource = R"(
 #version 400
 layout (location = 0) in vec3 position;
 layout (location = 1) in vec2 texc;
 out vec2 tex_coord;
 uniform mat4 model;
 uniform mat4 projection;
 void main()
 {
	tex_coord = vec2(texc.s, 1.0 - texc.t);
	gl_Position = projection * model * vec4(position, 1.0);
 }
 )";

// Código fonte do Fragment Shader (em GLSL): ainda hardcoded
const GLchar *fragmentShaderSource = R"(
 #version 400
 in vec2 tex_coord;
 out vec4 color;
 uniform sampler2D tex_buff;
 uniform vec2 offsetTex;

 void main()
 {
	 color = texture(tex_buff,tex_coord + offsetTex);
 }
 )";

int main()
{
	// Inicialização da GLFW
	glfwInit();

	// Ativa a suavização de serrilhado (MSAA) com 8 amostras por pixel
	glfwWindowHint(GLFW_SAMPLES, 8);

	// Criação da janela GLFW
	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Ola Triangulo! -- Rossana", nullptr, nullptr);
	if (!window)
	{
		std::cerr << "Falha ao criar a janela GLFW" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Fazendo o registro da função de callback para a janela GLFW
	glfwSetKeyCallback(window, key_callback);

	// GLAD: carrega todos os ponteiros d funções da OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Falha ao inicializar GLAD" << std::endl;
		return -1;
	}

	// Obtendo as informações de versão
	const GLubyte *renderer = glGetString(GL_RENDERER); /* get renderer string */
	const GLubyte *version = glGetString(GL_VERSION);	/* version as a string */
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	// Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	// Compilando e buildando o programa de shader
	GLuint shaderID = setupShader();

	//Carregando uma textura 
	int imgWidth, imgHeight;
	GLuint texID = loadTexture("../assets/sprites/Vampires1_Walk_full.png",imgWidth,imgHeight);

	// Gerando um buffer simples, com a geometria de um triângulo
	vampirao.nAnimations = 4;
	vampirao.nFrames = 6;
	vampirao.VAO = setupSprite(vampirao.nAnimations,vampirao.nFrames,vampirao.ds,vampirao.dt);
	vampirao.position = vec3(400.0, 150.0, 0.0);
	vampirao.dimensions = vec3(imgWidth/vampirao.nFrames*3,imgHeight/vampirao.nAnimations*3,1.0);
	vampirao.texID = texID;
	vampirao.iAnimation = 1;
	vampirao.iFrame = 0;

	Sprite background;
	background.nAnimations = 1;
	background.nFrames = 1;
	background.VAO = setupSprite(background.nAnimations,background.nFrames,background.ds,background.dt);
	background.position = vec3(400.0, 300.0, 0.0);
	background.texID = loadTexture("../assets/backgrounds/vampiro-fundo.png",imgWidth,imgHeight);
	background.dimensions = vec3(800, 600, 1.0);
	background.iAnimation = 0;
	background.iFrame = 0;
	

	glUseProgram(shaderID);

	double prev_s = glfwGetTime();
	double title_countdown_s = 0.1;

	float colorValue = 0.0;

	// Ativando o primeiro buffer de textura do OpenGL
	glActiveTexture(GL_TEXTURE0);

	// Criando a variável uniform pra mandar a textura pro shader
	glUniform1i(glGetUniformLocation(shaderID, "tex_buff"), 0);

	// Matriz de projeção paralela ortográfica
	mat4 projection = ortho(0.0, 800.0, 0.0, 600.0, -1.0, 1.0);
	glUniformMatrix4fv(glGetUniformLocation(shaderID, "projection"), 1, GL_FALSE, value_ptr(projection));

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	double lastTime = 0.0;
	double deltaT = 0.0;
	double currTime = glfwGetTime();
	double FPS = 12.0;


	vec2 offsetTexBg = vec2(0.0,0.0);
	// Loop da aplicação - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		// Este trecho de código é totalmente opcional: calcula e mostra a contagem do FPS na barra de título
		{
			double curr_s = glfwGetTime();
			double elapsed_s = curr_s - prev_s;
			prev_s = curr_s;

			// Exibe o FPS, mas não a cada frame, para evitar oscilações excessivas.
			title_countdown_s -= elapsed_s;
			if (title_countdown_s <= 0.0 && elapsed_s > 0.0)
			{
				double fps = 1.0 / elapsed_s; // Calcula o FPS com base no tempo decorrido.

				// Cria uma string e define o FPS como título da janela.
				char tmp[256];
				sprintf(tmp, "Vampirinho por ai\tFPS %.2lf", fps);
				glfwSetWindowTitle(window, tmp);

				title_countdown_s = 0.1;
			}
		}

		glfwPollEvents();

		// Limpa o buffer de cor
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // cor de fundo
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glLineWidth(10);
		glPointSize(20);

		mat4 model = mat4(1);
		model = translate(model,background.position);
		model = rotate(model, radians(0.0f), vec3(0.0, 0.0, 1.0));
		model = scale(model,background.dimensions);
		glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));

		

		currTime = glfwGetTime();
		deltaT = currTime - lastTime;
		
		offsetTexBg.s = background.iFrame * 0.01;
		offsetTexBg.t = 0.0;
		glUniform2f(glGetUniformLocation(shaderID, "offsetTex"),offsetTexBg.s, offsetTexBg.t);

		glBindVertexArray(background.VAO);
		glBindTexture(GL_TEXTURE_2D, background.texID);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


		// Desenho do vampirao
		model = mat4(1);
		model = translate(model,vampirao.position);
		model = rotate(model, radians(0.0f), vec3(0.0, 0.0, 1.0));
		model = scale(model,vampirao.dimensions);
		glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));

		vec2 offsetTex;
		
			if (deltaT >= 1.0/FPS)
			{
				if(vampirao.isWalking) {
				vampirao.iFrame = (vampirao.iFrame + 1) % vampirao.nFrames; // incremento "circular"
				}
				lastTime = currTime;
			}
		
		offsetTex.s = vampirao.iFrame * vampirao.ds;
		offsetTex.t = (vampirao.iAnimation) * vampirao.dt;
		glUniform2f(glGetUniformLocation(shaderID, "offsetTex"),offsetTex.s, offsetTex.t);

		glBindVertexArray(vampirao.VAO);
		glBindTexture(GL_TEXTURE_2D, vampirao.texID);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glfwSwapBuffers(window);
	}
		
	glfwTerminate();
	return 0;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	vampirao.isWalking = true;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
	glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (action != GLFW_PRESS && action != GLFW_REPEAT){
		vampirao.isWalking = false;
	} 
	
		if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT)){
			vampirao.iAnimation = 3;
			if(vampirao.position.x >= 50) {
				vampirao.position.x -= 10.0;
			}
			
		}
		if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT)){
			vampirao.iAnimation = 4;
			if(vampirao.position.x > 0 && vampirao.position.x <= 750) {
				vampirao.position.x += 10.0;
			}
		}
		if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT)){
			vampirao.iAnimation = 2;
			if(vampirao.position.y <= 550) {
				vampirao.position.y += 10.0;
                vampirao.dimensions -= 2.0;
			}
		}
		if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT)){
			vampirao.iAnimation = 1;
			if(vampirao.position.y >= 50) {
				vampirao.position.y -= 10.0;
                vampirao.dimensions += 2.0;
			}
		}	
	}

int setupShader()
{
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
				  << infoLog << std::endl;
	}
	// Fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
				  << infoLog << std::endl;
	}

    GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
				  << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

int setupSprite(int nAnimations, int nFrames, float &ds, float &dt)
{

	ds = 1.0 / (float) nFrames;
	dt = 1.0 / (float) nAnimations;
	
	GLfloat vertices[] = {
		// x   y    z    s     t
		-0.5,  0.5, 0.0, 0.0, dt, //V0
		-0.5, -0.5, 0.0, 0.0, 0.0, //V1
		 0.5,  0.5, 0.0, ds, dt, //V2
		 0.5, -0.5, 0.0, ds, 0.0  //V3
		};

	GLuint VBO, VAO;

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);

	// Ponteiro pro atributo 0 - Posição - coordenadas x, y, z
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *)0);
	glEnableVertexAttribArray(0);

	// Ponteiro pro atributo 1 - Coordenada de textura s, t
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return VAO;
}

int loadTexture(string filePath, int &width, int &height)
{
	GLuint texID;

	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	int nrChannels;

	unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);

	if (data)
	{
		if (nrChannels == 3) // jpg, bmp
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		else // png
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, 0);

	return texID;
}
