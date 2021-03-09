#define GLEW_STATIC
#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>

#include "shader.h"
#include "camera.h"
#include "texture.h"
#include "model.h"

// Keyboard callback
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
// Mouse movement callback
void mouse_move_callback(GLFWwindow* window, double xpos, double ypos);
// Mouse wheel scroll callback
void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// Move in the scene
void do_movement();

// Widnow dimensions
const int WINDOW_WIDTH = 800, WINDOW_HEIGHT = 600;
// Camera interaction parameters
GLfloat lastX = WINDOW_WIDTH / 2.0f, lastY = WINDOW_HEIGHT / 2.0f;
bool firstMouseMove = true;
bool keyPressedStatus[1024]; // Key pressed or not
GLfloat deltaTime = 0.0f; // Time difference between the current frame and the previous frame
GLfloat lastFrame = 0.0f; // Last frame time
Camera camera(glm::vec3(0.0f, 1.8f, 4.0f));
glm::vec3 lampPos(0.5f, 1.5f, 0.8f);
bool bNormalMapping = true;
bool bParallaxMapping = false;
Model objModel;
GLfloat heightScale = 0.1f;

GLuint quadVAOId, quadVBOId;
void setupQuadVAO();

int main(int argc, char** argv)
{

	if (!glfwInit())	// Init GLFW
	{
		std::cout << "Error::GLFW could not initialize GLFW!" << std::endl;
		return -1;
	}

	// Open OpenGL 3.3 core profile
	std::cout << "Start OpenGL core profile version 3.3" << std::endl;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// Create window
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
		"Demo of normal mapping(Press N to change mapping)", NULL, NULL);
	if (!window)
	{
		std::cout << "Error::GLFW could not create winddow!" << std::endl;
		glfwTerminate();
		return -1;
	}
	// Created window set to current
	glfwMakeContextCurrent(window);

	// Enables key use to be detected in window
	glfwSetKeyCallback(window, key_callback);
	// Enables mouse movement to be detected in window
	glfwSetCursorPosCallback(window, mouse_move_callback);
	// Enables mouse wheel scroll to be detected in window
	glfwSetScrollCallback(window, mouse_scroll_callback);
	// tell GLFW to capture mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Init GLEW for OpenGL functions
	glewExperimental = GL_TRUE; // Let GLEW get all extensions
	GLenum status = glewInit();
	if (status != GLEW_OK)
	{
		std::cout << "Error::GLEW glew version:" << glewGetString(GLEW_VERSION)
			<< " error string:" << glewGetErrorString(status) << std::endl;
		glfwTerminate();
		return -1;
	}

	// Viewpoint parameters
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	// Load in model
	std::ifstream modelPath("modelPath.txt");
	if (!modelPath)
	{
		std::cerr << "Error::could not read model path file." << std::endl;
		glfwTerminate();
		std::system("pause");
		return -1;
	}
	std::string modelFilePath;
	std::getline(modelPath, modelFilePath);
	if (!objModel.loadModel(modelFilePath))
	{
		glfwTerminate();
		std::system("pause");
		return -1;
	}

	setupQuadVAO();

	// Load textures
	GLuint diffuseMap = TextureHelper::load2DTexture("assets/textures/bricks2.jpg");
	GLuint normalMap = TextureHelper::load2DTexture("assets/textures/bricks2_normal.jpg");
	GLuint heightMap = TextureHelper::load2DTexture("assets/textures/bricks2_disp.jpg");


	// Build and compile shaders
	Shader shader("assets/shaders/scene.vertex", "assets/shaders/scene.frag");
	Shader parallaxShader("assets/shaders/parallax.vertex", "assets/shaders/parallax.frag");

	glEnable(GL_DEPTH_TEST);
	// While window is open
	while (!glfwWindowShouldClose(window))
	{
		GLfloat currentFrame = (GLfloat)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		glfwPollEvents(); // Handle events
		do_movement(); // Update camera properties according to user operation

		// Clear colour buffer and reset to specified color
		glClearColor(0.18f, 0.04f, 0.14f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 projection = glm::perspective(camera.mouse_zoom,
			(GLfloat)(WINDOW_WIDTH) / WINDOW_HEIGHT, 1.0f, 100.0f); // 投影矩阵
		glm::mat4 view = camera.getViewMatrix(); // 视变换矩阵

		
		///// CAT MODEL /////
		shader.use();
		// Light source properties
		GLint lightAmbientLoc = glGetUniformLocation(shader.programId, "light.ambient");
		GLint lightDiffuseLoc = glGetUniformLocation(shader.programId, "light.diffuse");
		GLint lightSpecularLoc = glGetUniformLocation(shader.programId, "light.specular");
		GLint lightPosLoc = glGetUniformLocation(shader.programId, "light.position");
		glUniform3f(lightAmbientLoc, 0.3f, 0.3f, 0.3f);
		glUniform3f(lightDiffuseLoc, 0.6f, 0.6f, 0.6f);
		glUniform3f(lightSpecularLoc, 1.0f, 1.0f, 1.0f);
		glUniform3f(lightPosLoc, lampPos.x, lampPos.y, lampPos.z);
		// Observer position
		GLint viewPosLoc = glGetUniformLocation(shader.programId, "viewPos");
		glUniform3f(viewPosLoc, camera.position.x, camera.position.y, camera.position.z);
		// Light source position for vertex shader calculation
		lightPosLoc = glGetUniformLocation(shader.programId, "lightPos");
		glUniform3f(lightPosLoc, lampPos.x, lampPos.y, lampPos.z);
		// Transformation matrix
		glUniformMatrix4fv(glGetUniformLocation(shader.programId, "projection"),
			1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(shader.programId, "view"),
			1, GL_FALSE, glm::value_ptr(view));
		glm::mat4 model;
		glUniformMatrix4fv(glGetUniformLocation(shader.programId, "model"),
			1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(shader.programId, "normalMapping"), bNormalMapping);
		
		// Draw the model
		objModel.draw(shader);

		///// BRICK WALL /////
		parallaxShader.use();
		// Light source properties
		GLint lightAmbientParallax = glGetUniformLocation(parallaxShader.programId, "light.ambient");
		GLint lightDiffuseParallax = glGetUniformLocation(parallaxShader.programId, "light.diffuse");
		GLint lightSpecularParallax = glGetUniformLocation(parallaxShader.programId, "light.specular");
		GLint lightPosParallax = glGetUniformLocation(parallaxShader.programId, "light.position");
		glUniform3f(lightAmbientParallax, 0.2f, 0.2f, 0.2f);
		glUniform3f(lightDiffuseParallax, 0.5f, 0.5f, 0.5f);
		glUniform3f(lightSpecularParallax, 1.0f, 1.0f, 1.0f);
		glUniform3f(lightPosParallax, lampPos.x, lampPos.y, lampPos.z);
		// Observer position
		GLint viewPosParallax = glGetUniformLocation(parallaxShader.programId, "viewPos");
		glUniform3f(viewPosParallax, camera.position.x, camera.position.y, camera.position.z);
		// Light source position for vertex shader calculation
		lightPosParallax = glGetUniformLocation(parallaxShader.programId, "lightPos");
		glUniform3f(lightPosParallax, lampPos.x, lampPos.y, lampPos.z);
		// Transformation matrix
		glUniformMatrix4fv(glGetUniformLocation(parallaxShader.programId, "projection"),
			1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(parallaxShader.programId, "view"),
			1, GL_FALSE, glm::value_ptr(view));
		glm::mat4 model2;
		glUniformMatrix4fv(glGetUniformLocation(parallaxShader.programId, "model"),
			1, GL_FALSE, glm::value_ptr(model2));
		glUniform1f(glGetUniformLocation(parallaxShader.programId, "heightScale"), heightScale);
		glUniform1i(glGetUniformLocation(parallaxShader.programId, "bParallaxMapping"), bParallaxMapping);
		// Draw the wall
		glBindVertexArray(quadVAOId);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseMap);
		glUniform1i(glGetUniformLocation(parallaxShader.programId, "diffuseMap"), 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normalMap);
		glUniform1i(glGetUniformLocation(parallaxShader.programId, "normalMap"), 1);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, heightMap);
		glUniform1i(glGetUniformLocation(parallaxShader.programId, "heightMap"), 2);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		glBindVertexArray(0);
		glUseProgram(0);
		glfwSwapBuffers(window); // Swap the buffers
	}
	// Close window
	glDeleteVertexArrays(1, &quadVAOId);
	glDeleteBuffers(1, &quadVBOId);
	glfwTerminate();
	return 0;
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keyPressedStatus[key] = true;
		else if (action == GLFW_RELEASE)
			keyPressedStatus[key] = false;
	}
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE); // Closes the window
	}
	if (key == GLFW_KEY_N && action == GLFW_PRESS)
	{
		bNormalMapping = !bNormalMapping;
		std::cout << "using normal mapping " << (bNormalMapping ? "true" : "false") << std::endl;
	}
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		bParallaxMapping = !bParallaxMapping;
		std::cout << "using normal mapping " << (bParallaxMapping ? "true" : "false") << std::endl;
	}
	else if (key == GLFW_KEY_UP && action == GLFW_PRESS)
	{
		heightScale += 0.1f;
		std::cout << "Height scale : " << heightScale << std::endl;
	}
	else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
	{
		heightScale -= 0.1f;
		std::cout << "Height scale : " << heightScale << std::endl;
	}
}
void mouse_move_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouseMove)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouseMove = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera.handleMouseMove(xoffset, yoffset);
}
// Mouse wheel control handled by camera class
void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.handleMouseScroll(yoffset);
}
// Keyboard control handled by camera class
void do_movement()
{

	if (keyPressedStatus[GLFW_KEY_W])
		camera.handleKeyPress(FORWARD, deltaTime);
	if (keyPressedStatus[GLFW_KEY_S])
		camera.handleKeyPress(BACKWARD, deltaTime);
	if (keyPressedStatus[GLFW_KEY_A])
		camera.handleKeyPress(LEFT, deltaTime);
	if (keyPressedStatus[GLFW_KEY_D])
		camera.handleKeyPress(RIGHT, deltaTime);
}

void setupQuadVAO()
{
	// Vertex position
	glm::vec3 pos1(-1.0, 1.0, -2.0);
	glm::vec3 pos2(-1.0, -1.0, -2.0);
	glm::vec3 pos3(1.0, -1.0, -2.0);
	glm::vec3 pos4(1.0, 1.0, -2.0);
	// Texture coords
	glm::vec2 uv1(0.0, 1.0);
	glm::vec2 uv2(0.0, 0.0);
	glm::vec2 uv3(1.0, 0.0);
	glm::vec2 uv4(1.0, 1.0);
	// Normal vecotr
	glm::vec3 nm(0.0, 0.0, 1.0);

	// Tangent & bitangent vector for 2 trianges
	glm::vec3 tangent1, bitangent1;
	glm::vec3 tangent2, bitangent2;

	//// FIRST TRIANGLE ////
	glm::vec3 edge1 = pos2 - pos1;
	glm::vec3 edge2 = pos3 - pos1;
	glm::vec2 deltaUV1 = uv2 - uv1;
	glm::vec2 deltaUV2 = uv3 - uv1;

	GLfloat f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

	tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
	tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
	tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
	tangent1 = glm::normalize(tangent1);

	bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
	bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
	bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
	bitangent1 = glm::normalize(bitangent1);

	//// SECOND TRIANGLE ////
	edge1 = pos3 - pos1;
	edge2 = pos4 - pos1;
	deltaUV1 = uv3 - uv1;
	deltaUV2 = uv4 - uv1;

	f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

	tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
	tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
	tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
	tangent2 = glm::normalize(tangent2);


	bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
	bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
	bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
	bitangent2 = glm::normalize(bitangent2);

	// TB vector calculated in advance
	GLfloat quadVertices[] = {
		// Position           | Normal      | Texture  | Tangent  | Bitangent
		pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y,
		tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
		pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y,
		tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
		pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y,
		tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

		pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y,
		tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
		pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y,
		tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
		pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y,
		tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
	};
	glGenVertexArrays(1, &quadVAOId);
	glGenBuffers(1, &quadVBOId);
	glBindVertexArray(quadVAOId);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBOId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
		14 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
		14 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
		14 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE,
		14 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE,
		14 * sizeof(GLfloat), (GLvoid*)(11 * sizeof(GLfloat)));
	glBindVertexArray(0);
}