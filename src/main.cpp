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

float lightYaw = 0.0f;
float lightPitch = 0.3f;
float lightRadius = 3.0f;

double lastX = 0.0f, lastY = 0.0f;
bool leftMousePressed = false;
bool rightMousePressed = false;
bool firstMouse = true;

bool lightMode = false;

glm::vec3 lightPos(1.0f, 1.0f, 3.0f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
glm::vec3 Ka(0.9f, 0.5f, 0.5f);
glm::vec3 Kd(0.9f, 0.2f, 0.1f);
glm::vec3 Ks(1.0f, 1.0f, 1.0f);
float ambientIntensity = 0.2;
float specularIntensity = 1.0;
float glossiness = 128;

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
	if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS) {
		lightMode = true;
	}
	if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_RELEASE) {
		lightMode = false;
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
		if (lightMode) {
			lightYaw += xoffset * rotationSpeed;
			lightPitch -= yoffset * rotationSpeed;
			lightPitch = glm::clamp(lightPitch, -1.2f, 1.2f);
		}
		else {
			yaw -= xoffset * rotationSpeed;
			pitch += yoffset * rotationSpeed;

			pitch = glm::clamp(pitch, -1.5f, 1.5f);
		}
		
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
	//Shader shaderprog2("../../../src/Shaders/cubevshader.vert", "../../../src/Shaders/cubefshader.frag");

	glfwSetWindowUserPointer(window, &shaderprog1);

	glfwSetKeyCallback(window, keyCallback);


	glViewport(0, 0, 800, 600);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetCursorPosCallback(window, cursorPosCallback);

	glEnable(GL_DEPTH_TEST);
	
	

	ModelLoader teapotModel("../../../assets/models/teapot.obj");
	//ModelLoader cubeModel("../../../assets/models/cube.obj");

	//Computing Model bounding box and center
	glm::vec3 modelBoxMin(FLT_MAX);
	glm::vec3 modelBoxMax(-FLT_MAX);

	for (const Mesh& mesh : teapotModel.meshes) {
		modelBoxMin = glm::min(modelBoxMin, mesh.getBoxMin());
		modelBoxMax = glm::max(modelBoxMax, mesh.getBoxMax());
	}

	glm::vec3 modelCenter = (modelBoxMin + modelBoxMax) * 0.5f;

	while (!glfwWindowShouldClose(window)) {
		float timeValue = (float)glfwGetTime();

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

		glm::vec3 lightPosWorld;
		lightPosWorld.x = lightRadius * cos(lightPitch) * sin(lightYaw);
		lightPosWorld.y = lightRadius * sin(lightPitch);
		lightPosWorld.z = lightRadius * cos(lightPitch) * cos(lightYaw);
		
		glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 perspectiveProjection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
		glm::mat4 model = glm::mat4(1.0f);

		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.05f));
		model = glm::translate(model, -modelCenter);
		glm::mat4 mvp = perspectiveProjection * view * model;
		glm::mat4 modelView = view * model;

		glm::vec3 lightPosView = glm::vec3(view * glm::vec4(lightPosWorld, 1.0f));
		glm::vec3 cameraPosView = glm::vec3(view * glm::vec4(cameraPos, 1.0f));

		shaderprog1.use();
		shaderprog1.setVec3("light.position", lightPosView);
		shaderprog1.setVec3("light.color", lightColor);

		shaderprog1.setVec3("material.ambient", Ka);
		shaderprog1.setVec3("material.diffuse", Kd);
		shaderprog1.setVec3("material.specular", Ks);
		shaderprog1.setFloat("material.ambientIntensity", ambientIntensity);
		shaderprog1.setFloat("material.specularIntensity", specularIntensity);
		shaderprog1.setFloat("material.glossiness", glossiness);
		shaderprog1.setVec3("viewPos", cameraPosView);

		shaderprog1.setMat4("mvp", mvp);
		shaderprog1.setMat4("modelView", modelView);
		
		teapotModel.Draw(shaderprog1);

		//shaderprog2.use();
		//glm::mat4 lightModel = glm::mat4(1.0f);
		////lightModel = glm::translate(lightModel, lightPosWorld);
		//lightModel = glm::scale(lightModel, glm::vec3(0.5f));

		//glm::mat4 lightMVP = perspectiveProjection * view * lightModel;
		//shaderprog2.setMat4("mvp", lightMVP);
		//shaderprog2.setVec3("lightColor", lightColor);

		//cubeModel.Draw(shaderprog2);

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

