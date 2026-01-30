#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#include "ModelLoader.h"

float yaw = 0.0f;
float pitch = 0.0f;
float distance = 5.0f;

double lastX = 0.0f, lastY = 0.0f;
bool leftMousePressed = false;
bool rightMousePressed = false;
bool firstMouse = true;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (key == GLFW_KEY_F6 && action == GLFW_PRESS) {
		//recompile shaders
		Shader* shader = static_cast<Shader*>(glfwGetWindowUserPointer(window));
		if (shader) {
			shader->reloadShaders();
		}
		else {
			std::cout << "Shader cast error in Key Callback" << std::endl;
		}
		
	}
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT)
		leftMousePressed = (action == GLFW_PRESS);
	if (button == GLFW_MOUSE_BUTTON_RIGHT)
		rightMousePressed = (action == GLFW_PRESS);
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = ypos - lastY;

	lastX = xpos;
	lastY = ypos;

	const float rotationSpeed = 0.005f;
	const float zoomSpeed = 0.01f;

	if (leftMousePressed) {
		yaw -= xoffset * rotationSpeed;
		pitch += yoffset * rotationSpeed;

		pitch = glm::clamp(pitch, -1.5f, 1.5f);
	}

	if (rightMousePressed) {
		distance += yoffset * zoomSpeed;
		distance = glm::max(distance, 0.5f);
	}

	/*float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;*/
	
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	std::cout << "framebuffer size called" << std::endl;
	glViewport(0, 0, width, height);
}

int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "CaveXRT", NULL, NULL);
	if (!window) {
		std::cout << "Failed to create GL window" << std::endl;
		glfwTerminate();
		return -1;
	}

	

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to load GLAD!" << std::endl;
		return -1;
	}

	Shader shaderprog1("../../../src/Shaders/vshader.vert", "../../../src/Shaders/fshader.frag");

	glfwSetWindowUserPointer(window, &shaderprog1);

	glfwSetKeyCallback(window, keyCallback);


	glViewport(0, 0, 800, 600);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetCursorPosCallback(window, cursorPosCallback);

	glEnable(GL_DEPTH_TEST);
	
	

	ModelLoader teapotModel("../../../assets/models/teapot.obj");

	//Computing Model bounding box and center
	glm::vec3 modelBoxMin(FLT_MAX);
	glm::vec3 modelBoxMax(-FLT_MAX);

	for (const Mesh& mesh : teapotModel.meshes) {
		modelBoxMin = glm::min(modelBoxMin, mesh.getBoxMin());
		modelBoxMax = glm::max(modelBoxMax, mesh.getBoxMax());
	}

	glm::vec3 modelCenter = (modelBoxMin + modelBoxMax) * 0.5f;

	while (!glfwWindowShouldClose(window)) {
		/*float timeValue = (float)glfwGetTime();
		float r = sinf(timeValue * 0.8f) * 0.5f + 0.5f;
		float g = sinf(timeValue * 1.1f + 2.0f) * 0.5f + 0.5f;
		float b = sinf(timeValue * 1.5f + 4.0f) * 0.5f + 0.5f;

		glClearColor(r, g, b, 1.0f);*/

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::vec3 cameraPos;
		cameraPos.x = distance * cos(pitch) * sin(yaw);
		cameraPos.y = distance * sin(pitch);
		cameraPos.z = distance * cos(pitch) * cos(yaw);

		glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 perspectiveProjection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
		glm::mat4 model = glm::mat4(1.0f);

		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.05f));
		model = glm::translate(model, -modelCenter);
		glm::mat4 mvp = perspectiveProjection * view * model;

		shaderprog1.use();
		shaderprog1.setMat4("mvp", mvp);

		
		teapotModel.Draw(shaderprog1);

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

