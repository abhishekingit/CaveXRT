#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#include "ModelLoader.h"
#include "CaveXRTConfig.h"
#include "RenderTarget.h"

CaveXRTConfig caveXRTConfig = loadConfig("../../../CaveXRTConfig.json");

struct OrbitCamera {
	float yaw;
	float pitch;
	float radius;
};

OrbitCamera sceneCam{ caveXRTConfig.yaw, caveXRTConfig.pitch, caveXRTConfig.distance };
OrbitCamera planeCam = sceneCam;

float yaw = caveXRTConfig.yaw;
float pitch = caveXRTConfig.pitch;
float distance = caveXRTConfig.distance;

float lightYaw = caveXRTConfig.lightYaw;
float lightPitch = caveXRTConfig.lightPitch;
float lightRadius = caveXRTConfig.lightRadius;

float planeYaw = yaw;
float planePitch = pitch;
float planeRadius = distance;

double lastX = 0.0f, lastY = 0.0f;
bool leftMousePressed = false;
bool rightMousePressed = false;
bool firstMouse = true;
bool planeViewMode = false;
bool planeCamDetached = false;
bool lightMode = false;

glm::vec3 lightPos(1.0f, 1.0f, 3.0f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
glm::vec3 Ka(0.9f, 0.5f, 0.5f);
glm::vec3 Kd(0.9f, 0.2f, 0.1f);
glm::vec3 Ks(1.0f, 1.0f, 1.0f);
float ambientIntensity = 0.2;
float specularIntensity = 1.0;
float glossiness = 128;

constexpr float quadPlaneVertices[] = {
	-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
	1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
	-1.0f, 1.0f, 0.0f, 0.0f, 1.0f
};

constexpr uint32_t quadPlaneIndices[] = {
	0, 1, 2,
	0, 2, 3
};



struct RenderState {
	Shader* mainShader{};
	Shader* lightShader{};
	Shader* quadShader{};
	CaveXRTConfig* config{};
	RenderTarget* renderTarget{};
	int* framebufferWidth{};
	int* framebufferHeight{};
};

struct AppConfig {
	int width = 800;
	int height = 600;
	std::string modelPath;
};

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	auto* state = static_cast<RenderState*>(glfwGetWindowUserPointer(window));
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (key == GLFW_KEY_F6 && action == GLFW_PRESS) {
		//recompile shaders
		if (state) {
			state->mainShader->reloadShaders();
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
	if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		if (state) {
			//shader->reloadShaders();
			*state->config = loadConfig("../../../CaveXRTConfig.json");
			state->config->apply(*state->mainShader, *state->lightShader);
		}
		else {
			std::cout << "Shader cast error in Key Callback" << std::endl;
		}

	}
	if (key == GLFW_KEY_LEFT_ALT || key == GLFW_KEY_RIGHT_ALT) {
		if (action == GLFW_PRESS) {
			planeViewMode = true;
			if (!planeCamDetached) {
				planeCam = sceneCam;
			}
		}		
		if (action == GLFW_RELEASE) {
			planeViewMode = false;
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

	const float rotationSpeed = caveXRTConfig.rotationSpeed;
	const float zoomSpeed = caveXRTConfig.zoomSpeed;

	OrbitCamera& active = planeViewMode ? planeCam : sceneCam;

	if (leftMousePressed) {
		if (lightMode) {
			lightYaw += xoffset * rotationSpeed;
			lightPitch -= yoffset * rotationSpeed;
			lightPitch = glm::clamp(lightPitch, -1.2f, 1.2f);
		}
		else {
			active.yaw -= xoffset * rotationSpeed;
			active.pitch = glm::clamp(active.pitch + yoffset * rotationSpeed, -1.5f, 1.5f);

			if (planeViewMode) {
				planeCamDetached = true;
			}
			/*else if (!planeCamDetached) {
				planeCam = sceneCam;
			}*/
		}
		
	}

	if (rightMousePressed) {
		active.radius = glm::max(active.radius + yoffset * zoomSpeed, 0.5f);

		if (planeViewMode) {
			planeCamDetached = true;

		}
		/*else if (!planeCamDetached) {
			planeCam = sceneCam;
		}*/
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
	auto* state = static_cast<RenderState*>(glfwGetWindowUserPointer(window));
	std::cout << "framebuffer size called" << std::endl;
	glViewport(0, 0, width, height);

	if (!state) {
		return;
	}

	if (state->renderTarget) {
		state->renderTarget->Resize(width, height);
	}

	if (state->framebufferWidth) {
		*state->framebufferWidth = width;
	}

	if (state->framebufferHeight) {
		*state->framebufferHeight = height;
	}
}

bool parseArguments(int argc, char* argv[], AppConfig& config) {
	for (int i = 1; i < argc; i++) {
		std::string arg = argv[i];
		std::cout << "Parsing argument: " << arg << "\n";
		if (arg == "--width" && i + 1 < argc) {
			config.width = std::stoi(argv[++i]);
		}
		else if (arg == "--height" && i + 1 < argc) {
			config.height = std::stoi(argv[++i]);
		}
		else if (arg == "--help") {
			std::cout << "Usage: \n" << " CaveXRT.exe [options] <model.obj> \n\n" << "Options: \n" << "--width <int> Window width(default 800) \n" << "--height <int> Window height(default 600) \n";
			return false;
		}
		else {
			config.modelPath = arg;
		}
	}

	if (config.modelPath.empty()) {
		std::cerr << "Error: no obj file specified\n";
		return false;
	}

	return true;
}


int main(int argc, char* argv[]) {
	

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	AppConfig config;
	if (!parseArguments(argc, argv, config)) {
		return -1;
	}

	std::cout << "Loading model: " << config.modelPath << "\n";
	std::cout << "Window size: " << config.width << "x" << config.height << "\n";
	

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
	Shader shaderprog2("../../../src/Shaders/cubevshader.vert", "../../../src/Shaders/cubefshader.frag");
	Shader quadShader("../../../src/Shaders/quadVshader.vert", "../../../src/Shaders/quadFshader.frag");

	int framebufferWidth = caveXRTConfig.width;
	int framebufferHeight = caveXRTConfig.height;
	RenderTarget renderTarget(framebufferWidth, framebufferHeight);

	RenderState state{
		.mainShader = &shaderprog1,
		.lightShader = &shaderprog2,
		.quadShader = &quadShader,
		.config = &caveXRTConfig,
		.renderTarget = &renderTarget,
		.framebufferWidth = &framebufferWidth,
		.framebufferHeight = &framebufferHeight	

	};


	glfwSetWindowUserPointer(window, &state);

	//for quad plane
	uint32_t quadVAO, quadVBO, quadEBO;
	glGenVertexArrays(1, &quadVAO);

	glGenBuffers(1, &quadVBO);

	glGenBuffers(1, &quadEBO);

	glBindVertexArray(quadVAO);

	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadPlaneVertices), quadPlaneVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadPlaneIndices), quadPlaneIndices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glBindVertexArray(0);


	glfwSetKeyCallback(window, keyCallback);


	glViewport(0, 0, 800, 600);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetCursorPosCallback(window, cursorPosCallback);

	glEnable(GL_DEPTH_TEST);
	
	//Parse arguments for obj model

	//ModelLoader teapotModel("../../../assets/models/yoda/yoda.obj");
	ModelLoader mainModel(config.modelPath);
	ModelLoader cubeModel("../../../assets/models/cube.obj");

	//Computing Model bounding box and center
	glm::vec3 modelBoxMin(FLT_MAX);
	glm::vec3 modelBoxMax(-FLT_MAX);

	for (const Mesh& mesh : mainModel.meshes) {
		modelBoxMin = glm::min(modelBoxMin, mesh.getBoxMin());
		modelBoxMax = glm::max(modelBoxMax, mesh.getBoxMax());
	}

	glm::vec3 modelCenter = (modelBoxMin + modelBoxMax) * 0.5f;
	glm::vec3 modelSize = modelBoxMax - modelBoxMin;
	float modelRadius = glm::length(modelSize) * 0.5f;
	float maxExtent = glm::max(modelSize.x, glm::max(modelSize.y, modelSize.z));
	float scaleFactor = 1.0f / modelRadius;

	/*distance = modelRadius * 2.5f;
	lightRadius = modelRadius * 1.5f;*/	

	while (!glfwWindowShouldClose(window)) {
		float timeValue = (float)glfwGetTime();

		/*float timeValue = (float)glfwGetTime();
		float r = sinf(timeValue * 0.8f) * 0.5f + 0.5f;
		float g = sinf(timeValue * 1.1f + 2.0f) * 0.5f + 0.5f;
		float b = sinf(timeValue * 1.5f + 4.0f) * 0.5f + 0.5f;

		glClearColor(r, g, b, 1.0f);*/
		glm::vec3 backgroundColor = caveXRTConfig.backgroundColor;

		auto orbitToPosition = [](const OrbitCamera& cam) {
			return glm::vec3{
				cam.radius * cosf(cam.pitch) * sinf(cam.yaw),
				cam.radius * sinf(cam.pitch),
				cam.radius * cosf(cam.pitch) * cosf(cam.yaw)
			};
		};


		

		glm::vec3 cameraPos = orbitToPosition(sceneCam);

		glm::vec3 planeCameraPos = orbitToPosition(planeCam);


		glm::vec3 lightPosWorld;
		lightPosWorld.x = lightRadius * cos(lightPitch) * sin(lightYaw);
		lightPosWorld.y = lightRadius * sin(lightPitch);
		lightPosWorld.z = lightRadius * cos(lightPitch) * cos(lightYaw);

		glm::mat4 view = glm::lookAt(cameraPos, caveXRTConfig.cameraTarget, caveXRTConfig.cameraUp);

		float aspectRatio = framebufferHeight > 0 ? (float)framebufferWidth / (float)framebufferHeight : 1.0f;

		glm::mat4 perspectiveProjection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
		glm::mat4 model = glm::mat4(1.0f);

		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(scaleFactor));
		model = glm::translate(model, -modelCenter);
		glm::mat4 mvp = perspectiveProjection * view * model;
		glm::mat4 modelView = view * model;

		glm::vec3 lightPosView = glm::vec3(view * glm::vec4(lightPosWorld, 1.0f));
		glm::vec3 cameraPosView = glm::vec3(view * glm::vec4(cameraPos, 1.0f));

		glm::vec3 lightColor = caveXRTConfig.lightColor;

		renderTarget.Bind();
		glViewport(0, 0, renderTarget.Width(), renderTarget.Height());
		glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shaderprog1.use();
		shaderprog1.setVec3("light.position", lightPosView);
		shaderprog1.setVec3("light.color", lightColor);

		/*shaderprog1.setVec3("material.ambient", Ka);
		shaderprog1.setVec3("material.diffuse", Kd);
		shaderprog1.setVec3("material.specular", Ks);*/
		shaderprog1.setFloat("material.ambientIntensity", caveXRTConfig.ambientIntensity);
		shaderprog1.setFloat("material.specularIntensity", caveXRTConfig.specularIntensity);
		//shaderprog1.setFloat("material.glossiness", glossiness);
		shaderprog1.setVec3("viewPos", cameraPosView);

		shaderprog1.setMat4("mvp", mvp);
		shaderprog1.setMat4("modelView", modelView);
		
		mainModel.Draw(shaderprog1);

		renderTarget.UnBind();

		glViewport(0, 0, framebufferWidth, framebufferHeight);
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//plane
		quadShader.use();
		glm::mat4 planeModel = glm::mat4(1.0f);
		planeModel = glm::translate(planeModel, glm::vec3(0.0f, 0.0f, 0.0f));
		planeModel = glm::rotate(planeModel, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		planeModel = glm::scale(planeModel, glm::vec3(2.0f));

		glm::mat4 planeView = glm::lookAt(planeCameraPos, caveXRTConfig.cameraTarget, caveXRTConfig.cameraUp);

		glm::mat4 planeMVP = perspectiveProjection * planeView * planeModel;
		quadShader.setMat4("mvp", planeMVP);
	

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, renderTarget.GetColorTexture());
		glGenerateMipmap(GL_TEXTURE_2D);
		quadShader.setInt("renderTexture", 0);

		glBindVertexArray(quadVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);

		/*shaderprog2.use();
		glm::mat4 lightModel = glm::mat4(1.0f);
		lightModel = glm::translate(lightModel, lightPosWorld);
		lightModel = glm::scale(lightModel, glm::vec3(0.02f));

		glm::mat4 lightMVP = perspectiveProjection * view * lightModel;
		shaderprog2.setMat4("mvp", lightMVP);
		shaderprog2.setVec3("lightColor", lightColor);

		cubeModel.Draw(shaderprog2);*/

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &quadVAO);
	glDeleteBuffers(1, &quadVBO);
	glDeleteBuffers(1, &quadEBO);
	glfwTerminate();
	return 0;
}

