#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

static void closeKey_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
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

	GLFWwindow* window = glfwCreateWindow(800, 600, "Project1HelloGL", NULL, NULL);
	if (!window) {
		std::cout << "Failed to create GL window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwSetKeyCallback(window, closeKey_callback);

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to load GLAD!" << std::endl;
		return -1;
	}

	glViewport(0, 0, 800, 600);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	

	while (!glfwWindowShouldClose(window)) {
		

		float timeValue = (float)glfwGetTime();
		float r = sinf(timeValue * 0.8f) * 0.5f + 0.5f;
		float g = sinf(timeValue * 1.1f + 2.0f) * 0.5f + 0.5f;
		float b = sinf(timeValue * 1.5f + 4.0f) * 0.5f + 0.5f;

		glClearColor(r, g, b, 1.0f);

		glClear(GL_COLOR_BUFFER_BIT);
		

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

