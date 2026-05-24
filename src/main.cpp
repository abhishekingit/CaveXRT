#include <algorithm>
#include <iostream>
#include <sstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/string_cast.hpp>
#include "Shader.h"
#include "ModelLoader.h"
#include "CaveXRTConfig.h"
#include "RenderTarget.h"
#include "ParticleSystem.h"
#include "FluidRenderTarget.h"
#include "VideoRecorder.h"
#include "ParticleExporter.h"



CaveXRTConfig caveXRTConfig = loadConfig("../../../CaveXRTConfig.json");

struct OrbitCamera {
	float yaw;
	float pitch;
	float radius;
};

glm::mat4 MakePlaneReflectionY(float planeY) {
	// reflect about y = planeY
	glm::mat4 T1 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -planeY, 0.0f));
	glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, -1.0f, 1.0f)); // flip Y
	glm::mat4 T2 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, planeY, 0.0f));
	return T2 * S * T1;
}

glm::vec3 reflectPoint(const glm::mat4 &R, const glm::vec3 &p) {
    glm::vec4 t = R * glm::vec4(p,1.0f); return glm::vec3(t);
}
glm::vec3 reflectDir(const glm::mat4 &R, const glm::vec3 &v) {
    glm::vec4 t = R * glm::vec4(v,0.0f); return glm::vec3(t);
}

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
bool showBoundaryGhosts = false;
float autoYaw = 0.0f;

glm::vec3 lightPos(1.0f, 1.0f, 3.0f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
glm::vec3 Ka(0.9f, 0.5f, 0.5f);
glm::vec3 Kd(0.9f, 0.2f, 0.1f);
glm::vec3 Ks(1.0f, 1.0f, 1.0f);
float ambientIntensity = 0.2;
float specularIntensity = 1.0;
float glossiness = 128;

glm::vec3 bboxMin(-1.5f, 0.0f, -0.7f);
glm::vec3 bboxMax(1.9f, 4.0f, 0.7f);
float simparticleRadius = 0.020f;

float bboxVerts[] = {
	bboxMin.x, bboxMin.y, bboxMin.z,
	bboxMax.x, bboxMin.y, bboxMin.z,
	bboxMax.x, bboxMax.y, bboxMin.z,
	bboxMin.x, bboxMax.y, bboxMin.z,
	bboxMin.x, bboxMin.y, bboxMax.z,
	bboxMax.x, bboxMin.y, bboxMax.z,
	bboxMax.x, bboxMax.y, bboxMax.z,
	bboxMin.x, bboxMax.y, bboxMax.z
};

uint32_t bboxIndices[] = {
	0, 1,  1, 2,  2, 3,  3, 0,

	4, 5,  5, 6,  6, 7,  7, 4,

	0, 4,  1, 5,  2, 6,  3, 7
};

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
	FluidRenderTarget* fluidRenderTarget{};
	int* framebufferWidth{};
	int* framebufferHeight{};
	ParticleSystem* particleSystem{};
};

struct AppConfig {
	int width = 800;
	int height = 600;
	std::string modelPath;
	std::string normalMapPath;
	std::string displacementMapPath;
};

uint32_t loadTextureFromPath(const std::string& filePath) {
	if (filePath.empty()) {
		return 0;
	}

	std::vector<unsigned char> image;
	uint32_t width, height;
	uint32_t error = lodepng::decode(image, width, height, filePath);

	if (error) {
		std::cout << "Texture failed to load at path: " << filePath << "\n" << "Lodepng error: " << error << ":" << lodepng_error_text(error) << std::endl;
		return 0;
	}

	uint32_t textureID = 0;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data());

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);
	return textureID;

}

void updateTessellationTitle(GLFWwindow* window) {
	static float lastInner = -1.0f;
	static float lastOuter = -1.0f;

	if (lastInner == tesselationInnerLevel && lastOuter == tesselationOuterLevel) {
		return;
	}

	lastInner = tesselationInnerLevel;
	lastOuter = tesselationOuterLevel;

	std::ostringstream title;
	title << "CaveXRT | Tess Inner: " << tesselationInnerLevel << " | Tess Outer: " << tesselationOuterLevel;
	glfwSetWindowTitle(window, title.str().c_str());
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	auto* state = static_cast<RenderState*>(glfwGetWindowUserPointer(window));
	ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	const ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureKeyboard) {
		return;
	}

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
	if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		if (state) {
			state->particleSystem->ResetParticles();
		}
		else {
			std::cout << "Particle System cast error in Key Callback" << std::endl;
		}
	}

	if ((key == GLFW_KEY_1 || key == GLFW_KEY_2 || key == GLFW_KEY_3 || key == GLFW_KEY_4 || key == GLFW_KEY_5) && action == GLFW_PRESS) {
		if (state) {
			if (key == GLFW_KEY_1) {
				state->particleSystem->SetSpawnMode(ParticleSystem::SpawnMode::Random, true);
				std::cout << "Spawn mode: Random" << std::endl;
			}
			else if (key == GLFW_KEY_2) {
				state->particleSystem->SetSpawnMode(ParticleSystem::SpawnMode::SingleDam, true);
				std::cout << "Spawn mode: SingleDam" << std::endl;
			}
			else if (key == GLFW_KEY_3) {
				state->particleSystem->SetSpawnMode(ParticleSystem::SpawnMode::DoubleDam, true);
				std::cout << "Spawn mode: DoubleDam" << std::endl;
			}
			else if (key == GLFW_KEY_4) {
				state->particleSystem->SetSpawnMode(ParticleSystem::SpawnMode::SingleSheet, true);
				std::cout << "Spawn Mode: SingleSheet" << std::endl;

			}
			else if (key == GLFW_KEY_5) {
				state->particleSystem->SetSpawnMode(ParticleSystem::SpawnMode::PourIntoContainer, true);
				std::cout << "Spawn Mode: PourIntoContainer" << std::endl;
			}
		}
		else {
			std::cout << "Particle System cast error in Key Callback" << std::endl;
		}
	}

	if (key == GLFW_KEY_G && action == GLFW_PRESS) {
		showBoundaryGhosts = !showBoundaryGhosts;
		std::cout << "Boundary ghosts: " << (showBoundaryGhosts ? "ON" : "OFF") << std::endl;
	}

	

}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
	const ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureMouse) {
		return;
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT)
		leftMousePressed = (action == GLFW_PRESS);
	if (button == GLFW_MOUSE_BUTTON_RIGHT)
		rightMousePressed = (action == GLFW_PRESS);
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
	const ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureMouse){
		lastX = xpos;
		lastY = ypos;
		return;
	}

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

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
}

void charCallback(GLFWwindow* window, unsigned int c) {
	ImGui_ImplGlfw_CharCallback(window, c);
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

	if (state->fluidRenderTarget) {
		state->fluidRenderTarget->Resize(width, height);
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
		else if (arg == "--normalMap" && i + 1 < argc) {
			config.normalMapPath = argv[++i];
		}
		else if (arg == "--displacementMap" && i + 1 < argc) {
			config.displacementMapPath = argv[++i];
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
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	AppConfig config;
	if (!parseArguments(argc, argv, config)) {
		return -1;
	}
	/*config.modelPath = "../../../assets/models/teapot/teapot.obj";
	config.normalMapPath = "../../../assets/models/teapot/teapot_normal.png";
	config.displacementMapPath = "../../../assets/models/teapot/teapot_disp.png";*/

	std::cout << "Loading model: " << config.modelPath << "\n";
	std::cout << "Window size: " << caveXRTConfig.width << "x" << caveXRTConfig.height << "\n";
	

	GLFWwindow* window = glfwCreateWindow(caveXRTConfig.width, caveXRTConfig.height, "CaveXRT", NULL, NULL);
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

	//imgui setup
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, false);
	ImGui_ImplOpenGL3_Init("#version 430");

	glEnable(GL_PROGRAM_POINT_SIZE);

	Shader shaderprog1("../../../src/Shaders/vshader.vert", "../../../src/Shaders/fshader.frag");
	Shader shaderprog2("../../../src/Shaders/cubevshader.vert", "../../../src/Shaders/cubefshader.frag");
	Shader quadShader("../../../src/Shaders/quadTVshader.vert", "../../../src/Shaders/quadFshader.frag", nullptr, "../../../src/Shaders/quadTCshader.tesc", "../../../src/Shaders/quadTEshader.tese");
	Shader quadLineShader("../../../src/Shaders/quadTVshader.vert", "../../../src/Shaders/quadLineFshader.frag", "../../../src/Shaders/quadGshader.geom", "../../../src/Shaders/quadTCshader.tesc", "../../../src/Shaders/quadTEshader.tese");


	Shader skyboxShader("../../../src/Shaders/skyboxvshader.vert", "../../../src/Shaders/skyboxfshader.frag");
	Shader bboxShader("../../../src/Shaders/bboxvshader.vert", "../../../src/Shaders/bboxfshader.frag");
	Shader fluidRenderShader("../../../src/Shaders/Particles/fluidRender/fluidCompositev.vert", "../../../src/Shaders/Particles/fluidRender/fluidCompositef.frag");
	Shader fluidNormalReconstructShader("../../../src/Shaders/Particles/fluidRender/fluidCompositev.vert", "../../../src/Shaders/Particles/fluidRender/fluidNormalReconstructf.frag");
	Shader fluidNarrowRangeShader("../../../src/Shaders/Particles/fluidRender/fluidCompositev.vert", "../../../src/Shaders/Particles/fluidRender/fluidNarrowRangefilter.frag");

	VideoRecorder videoRecorder(caveXRTConfig.width, caveXRTConfig.height, 120, "CaveXRTFluidSim.mp4");

	ParticleExporter particleExporter;
	bool uiExportParticles = false;
	int exportFrameIndex = 0;
	char exportPath[256] = "../../../exports/CaveXRTCache";
	

	ParticleSystem particleSystem(
		50000,
		simparticleRadius,
		bboxMin,
		bboxMax,
		"../../../src/Shaders/Particles/particlecshader.comp",
		"../../../src/Shaders/Particles/fluidRender/particlevshader.vert",
		"../../../src/Shaders/Particles/particlefshader.frag");

	particleSystem.SetSpawnMode(ParticleSystem::SpawnMode::DoubleDam, true);


	glm::vec3 uiBoxMin = particleSystem.GetGridMin();
	glm::vec3 uiBoxMax = particleSystem.GetGridMax();

	float uiRestDensity = particleSystem.GetRestDensity();
	float uiViscosity = particleSystem.GetViscosityCoeff();
	float uiStiffness = particleSystem.GetStiffness();
	float uiVorticityEpsilon = particleSystem.GetVorticityEpsilon();
	int uiPbfSolverIterations = particleSystem.GetPBFSolverIterations();
	float uiPbfRelaxation = particleSystem.GetPBFRelaxation();
	float uiPbfScorrK = particleSystem.GetPBFCorrK();
	float uiPbfScorrN = particleSystem.GetPBFCorrN();
	float uiPbfScorrDQ = particleSystem.GetPBFCorrDQ();
	float uiPbfEpsilon = particleSystem.GetPBFEpsilon();
	glm::vec3 uiGravity = particleSystem.GetGravity();
	float uiMaxTimeStep = particleSystem.GetMaxTimeStep();
	int uiParticleCount = static_cast<int>(particleSystem.GetMaxParticles());

	bool uiEnableSPH = true;
	bool uiSimulationRunning = false;
	float uiParticleRenderSize = 0.03f;
	float uiWallDamping = 0.5f;
	bool uiEnableParticles = false;

	//Narrow Range filter parameters
	bool uiEnableNarrowRangeFilter = true;
	int uiNRFilterRadius = 6;
	float uiNRSigmaSpatial = 2.0f;
	float uiNRSigmaRangeScale = 0.02f;
	float uiNRSigmaRangeBase = 0.05f;
	float uiNRThicknessEpsilon = 0.0f;
	float uiNRThresholdRatio = 2.0f;
	float uiNRClampRatio = 1.0f;
	int uiNRMinFilterRadius = 2;
	float uiNRDepthAdaptiveScale = 0.15f;
	float uiNRMinSigmaSpatial = 0.5f;
	

	int framebufferWidth = caveXRTConfig.width;
	int framebufferHeight = caveXRTConfig.height;
	RenderTarget renderTarget(framebufferWidth, framebufferHeight);

	RenderTarget reflectionRenderTarget(framebufferWidth, framebufferHeight);

	FluidRenderTarget fluidRenderTarget(framebufferWidth, framebufferHeight);
	
	uint32_t nrFBO[2]{ 0, 0 };
	uint32_t nrTex[2]{ 0, 0 };

	int nrWidth = framebufferWidth;
	int nrHeight = framebufferHeight;

	auto CreateNarrowRangeRenderTargets = [&](int w, int h) {
		if (nrTex[0] != 0) glDeleteTextures(2, nrTex);
		if (nrFBO[0] != 0) glDeleteFramebuffers(2, nrFBO);

		glGenTextures(2, nrTex);
		glGenFramebuffers(2, nrFBO);

		for (int i = 0; i < 2; i++) {
			glBindTexture(GL_TEXTURE_2D, nrTex[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, w, h, 0, GL_RED, GL_FLOAT, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glBindFramebuffer(GL_FRAMEBUFFER, nrFBO[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, nrTex[i], 0);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		nrWidth = w;
		nrHeight = h;


	};

	CreateNarrowRangeRenderTargets(framebufferWidth, framebufferHeight);


	RenderState state{
		.mainShader = &shaderprog1,
		.lightShader = &shaderprog2,
		.quadShader = &quadShader,
		.config = &caveXRTConfig,
		.renderTarget = &renderTarget,
		.fluidRenderTarget = &fluidRenderTarget,
		.framebufferWidth = &framebufferWidth,
		.framebufferHeight = &framebufferHeight,
		.particleSystem = &particleSystem

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
	

	//for bbox
	uint32_t bboxVAO, bboxVBO, bboxEBO;

	glGenVertexArrays(1, &bboxVAO);

	glGenBuffers(1, &bboxVBO);
	glGenBuffers(1, &bboxEBO);

	glBindVertexArray(bboxVAO);

	glBindBuffer(GL_ARRAY_BUFFER, bboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bboxVerts), bboxVerts, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bboxEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(bboxIndices), bboxIndices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindVertexArray(0);

	glfwSetKeyCallback(window, keyCallback);


	glViewport(0, 0, caveXRTConfig.width, caveXRTConfig.height);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetCursorPosCallback(window, cursorPosCallback);
	glfwSetScrollCallback(window, scrollCallback);
	glfwSetCharCallback(window, charCallback);

	glEnable(GL_DEPTH_TEST);
	
	//Parse arguments for obj model

	//ModelLoader teapotModel("../../../assets/models/yoda/yoda.obj");
	ModelLoader mainModel(config.modelPath);
	ModelLoader cubeModel("../../../assets/models/cube.obj");
	ModelLoader lampModel("../../../assets/models/light/light.obj");

	uint32_t cubemapTexture = loadCubemap(caveXRTConfig.skyboxConfig);
	uint32_t normalMapTexture = loadTextureFromPath(config.normalMapPath);
	uint32_t displacementMapTexture = loadTextureFromPath(config.displacementMapPath);

	const int teapotEnvMapUnit = 5;
	const int shadowMapUnit = 8;

	if (normalMapTexture == 0) {
		std::cout << "plane normal map texture disabled\n";
	}

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

	float nearPlane = 0.1f;
	float farPlane = 100.0f;

	/*distance = modelRadius * 2.5f;
	lightRadius = modelRadius * 1.5f;*/	
	const float planeY = -0.1f;
	glm::mat4 reflectionMatrix = MakePlaneReflectionY(planeY);

	while (!glfwWindowShouldClose(window)) {
		float timeValue = (float)glfwGetTime();

		static float lastTime = timeValue;
		float deltaTime = timeValue - lastTime;
		lastTime = timeValue;

		const float autoRotateSpeed = 0.5f;
		autoYaw += autoRotateSpeed * deltaTime;

		/*float timeValue = (float)glfwGetTime();
		float r = sinf(timeValue * 0.8f) * 0.5f + 0.5f;
		float g = sinf(timeValue * 1.1f + 2.0f) * 0.5f + 0.5f;
		float b = sinf(timeValue * 1.5f + 4.0f) * 0.5f + 0.5f;

		glClearColor(r, g, b, 1.0f);*/
		glm::vec3 backgroundColor = caveXRTConfig.backgroundColor;
		glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		auto orbitToPosition = [](const OrbitCamera& cam) {
			float compYaw = cam.yaw;
			return glm::vec3{
				cam.radius * cosf(cam.pitch) * sinf(compYaw),
				cam.radius * sinf(cam.pitch),
				cam.radius * cosf(cam.pitch) * cosf(compYaw)
			};
		};


		

		glm::vec3 cameraPos = orbitToPosition(sceneCam);

		glm::vec3 planeCameraPos = orbitToPosition(planeCam);


		float cameraDistance = glm::length(cameraPos - caveXRTConfig.cameraTarget);
		float scaledModelRadius = 1.0f;
		//float farPlane = cameraDistance + scaledModelRadius + 50.0f;

		glm::vec3 lightPosWorld;
		lightPosWorld.x = lightRadius * cos(lightPitch) * sin(lightYaw);
		lightPosWorld.y = lightRadius * sin(lightPitch);
		lightPosWorld.z = lightRadius * cos(lightPitch) * cos(lightYaw);

		glm::mat4 view = glm::lookAt(cameraPos, caveXRTConfig.cameraTarget, caveXRTConfig.cameraUp);

		float aspectRatio = framebufferHeight > 0 ? (float)framebufferWidth / (float)framebufferHeight : 1.0f;

		glm::mat4 perspectiveProjection = glm::perspective(glm::radians(45.0f), aspectRatio, nearPlane, farPlane);
		glm::mat4 model = glm::mat4(1.0f);

		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(scaleFactor));
		model = glm::translate(model, -modelCenter);
		glm::mat4 mvp = perspectiveProjection * view * model;
		glm::mat4 modelView = view * model;

		glm::vec3 lightPosView = glm::vec3(view * glm::vec4(lightPosWorld, 1.0f));
		glm::vec3 cameraPosView = glm::vec3(view * glm::vec4(cameraPos, 1.0f));

		glm::vec3 lightColor = caveXRTConfig.lightColor;

		/*renderTarget.Bind();
		glViewport(0, 0, renderTarget.Width(), renderTarget.Height());
		glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);*/

		/*reflectionRenderTarget.Bind();
		glViewport(0, 0, reflectionRenderTarget.Width(), reflectionRenderTarget.Height());
		glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);*/

		//

		////glm::vec4 clipPlaneWorld(0.0f, 1.0f, 0.0f, -planeY);

		///*glm::vec3 reflCameraPos = cameraPos;
		//reflCameraPos.y = planeY - (cameraPos.y - planeY);

		//glm::vec3 reflTarget = caveXRTConfig.cameraTarget;
		//reflTarget.y = planeY - (reflTarget.y - planeY);

		//glm::vec3 reflUp = caveXRTConfig.cameraUp;
		//reflUp.y = -caveXRTConfig.y;*/
	

		////glm::vec3 camUp = caveXRTConfig.cameraUp;
		////glm::vec3 reflUp;
		////{ // reflect up as a direction (w=0)
		////	glm::vec4 up4 = reflectionMatrix * glm::vec4(camUp, 0.0f);
		////	reflUp = glm::normalize(glm::vec3(up4));
		////}

		////glm::mat4 reflView = glm::lookAt(reflCameraPos, reflTarget, reflUp);
		//glm::mat4 reflView = view;
		//glm::mat4 reflModel = reflectionMatrix * model;

		//glm::vec3 reflCameraPos = reflectPoint(reflectionMatrix, cameraPos);
		//glm::vec3 reflViewPos = reflectPoint(reflectionMatrix, cameraPosView);
		//

		////std::cout << "Refl model matrix " << glm::to_string(reflModel) << std::endl;

		//glm::mat4 reflMVP = perspectiveProjection * reflView * reflModel;
		//glm::mat4 reflModelView = reflView * reflModel;
		////glm::vec3 reflViewPos = glm::vec3(cameraPosView.x, -cameraPosView.y, cameraPosView.z);

		//shaderprog1.use();
		///*shaderprog1.setVec4("clipPlane", clipPlaneWorld);
		//shaderprog1.setBool("useClipPlane", true);*/

		//shaderprog1.setVec3("light.position", glm::vec3(reflectionMatrix * view * glm::vec4(lightPosWorld, 1.0f)));
		//shaderprog1.setVec3("light.color", lightColor);
		//shaderprog1.setFloat("material.ambientIntensity", caveXRTConfig.ambientIntensity);
		//shaderprog1.setFloat("material.specularIntensity", caveXRTConfig.specularIntensity);
		//shaderprog1.setVec3("viewPos", cameraPosView);
		//shaderprog1.setVec3("cameraPosWorld", reflCameraPos);
		//shaderprog1.setBool("isReflectionPass", true);
		//shaderprog1.setMat4("reflectionMatrix", reflectionMatrix);
		////shaderprog1.setBool("skyboxEnabled", false);
		//shaderprog1.setMat4("mvp", reflMVP);
		//shaderprog1.setMat4("modelView", reflModelView);
		//shaderprog1.setMat4("model", reflModel);


		//mainModel.Draw(shaderprog1);

		//reflectionRenderTarget.UnBind();

		//shaderprog1.use();
		//shaderprog1.setVec3("light.position", lightPosView);
		//shaderprog1.setVec3("light.color", lightColor);

		///*shaderprog1.setVec3("material.ambient", Ka);
		//shaderprog1.setVec3("material.diffuse", Kd);
		//shaderprog1.setVec3("material.specular", Ks);*/
		//shaderprog1.setFloat("material.ambientIntensity", caveXRTConfig.ambientIntensity);
		//shaderprog1.setFloat("material.specularIntensity", caveXRTConfig.specularIntensity);
		////shaderprog1.setFloat("material.glossiness", glossiness);
		//shaderprog1.setVec3("viewPos", cameraPosView);

		//shaderprog1.setBool("skyboxEnabled", caveXRTConfig.skyboxConfig.enabled);
		//shaderprog1.setBool("isReflectionPass", false);

		//glActiveTexture(GL_TEXTURE0 + teapotEnvMapUnit);
		//glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		//shaderprog1.setInt("skybox", teapotEnvMapUnit);

		//shaderprog1.setMat4("mvp", mvp);
		//shaderprog1.setMat4("modelView", modelView);
		//shaderprog1.setMat4("model", model);
		//shaderprog1.setVec3("cameraPosWorld", cameraPos);
		//
		//mainModel.Draw(shaderprog1);

		//renderTarget.UnBind();

		//glViewport(0, 0, framebufferWidth, framebufferHeight);

		//imgui 
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		const float panelWidth = framebufferWidth * 0.25f;
		const float panelMargin = 20.0f;

		ImGui::SetNextWindowPos(
			ImVec2(static_cast<float>(framebufferWidth) - panelWidth - panelMargin, panelMargin),
			ImGuiCond_Always
		);

		ImGui::SetNextWindowSize(
			ImVec2(panelWidth, static_cast<float>(framebufferHeight) * 0.6f),
			ImGuiCond_Always
		);


		ImGui::Begin("Simulation controls", nullptr, ImGuiWindowFlags_NoCollapse);

		ImGui::Checkbox("Enable SPH", &uiEnableSPH);
		ImGui::Checkbox("Show Particles", &uiEnableParticles);
		if (ImGui::Button(uiSimulationRunning ? "Pause Simulation" : "Start Simulation")) {
			uiSimulationRunning = !uiSimulationRunning;
		}
		ImGui::SameLine();
		if (ImGui::Button("Step")) {
			particleSystem.Update(1.0f / 60.0f, uiWallDamping, uiEnableSPH);
		}
		ImGui::SliderFloat("Particle render size", &uiParticleRenderSize, 0.005f, 0.9f);
		ImGui::SliderFloat("Wall Damping", &uiWallDamping, 0.0f, 1.0f);
		if (ImGui::SliderFloat("Max timestep", &uiMaxTimeStep, 1.0f / 240.0f, 1.0f / 30.0f, "%.5f")) {
			particleSystem.SetMaxTimeStep(uiMaxTimeStep);
		}
		ImGui::Text("Sim Timestep: %.1f fps", 1.0f / particleSystem.GetMaxTimeStep());
		ImGui::SetNextItemWidth(180.0f);
		ImGui::SliderInt("Particle Count", &uiParticleCount, 5000, 200000);
		if (ImGui::Button("Apply Particle Count")) {
			particleSystem.SetMaxParticles(static_cast<size_t>(uiParticleCount), true);
		}

		ImGui::SeparatorText("Video Capture");

		if (!videoRecorder.IsRecording()) {
			if (ImGui::Button("Start Recording")) {
				if (!videoRecorder.StartRecording()) {
					std::cout << "Failed to start video recording" << std::endl;
				}
			}
		}
		else {
			ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "REC");
			ImGui::SameLine();
			if (ImGui::Button("Stop Recording")) {
				videoRecorder.StopRecording();
			}
		}

		ImGui::SeparatorText("Particle Exporter");
		ImGui::InputText("Export Path", exportPath, IM_ARRAYSIZE(exportPath));

		if (!particleExporter.IsExporting()) {
			if (ImGui::Button("Start Export")) {
				if (particleExporter.BeginSession(exportPath, 60, simparticleRadius)) {
					exportFrameIndex = 0;
					uiExportParticles = true;
				}
			}
		}
		else {
			if (ImGui::Button("Stop Export")) {
				particleExporter.EndSession();
				uiExportParticles = false;
			}
		}



		
		if (ImGui::DragFloat3("Gravity", &uiGravity.x, 0.05f, -30.0f, 30.0f, "%.2f")) {
			particleSystem.SetGravity(uiGravity);
		}

		if (ImGui::SliderFloat("Vorticity", &uiVorticityEpsilon, 0.0f, 100.0f)) {
			particleSystem.SetVorticityEpsilon(uiVorticityEpsilon);
		}

		if (ImGui::SliderFloat("Viscosity", &uiViscosity, 0.0f, 5.0f)) {
			particleSystem.SetViscosityCoeff(uiViscosity);
		}

		if (ImGui::SliderFloat("Rest Density", &uiRestDensity, 100.0f, 3000.0f)) {
			particleSystem.SetRestDensity(uiRestDensity);
		}

		if (ImGui::CollapsingHeader("SPH Pressure", ImGuiTreeNodeFlags_DefaultOpen)) {

			if (ImGui::SliderFloat("Stiffness", &uiStiffness, 0.0f, 100.0f)) {
				particleSystem.SetStiffness(uiStiffness);
			}			
		}

		if (ImGui::CollapsingHeader("PBF parameters", ImGuiTreeNodeFlags_DefaultOpen)) {
			if (ImGui::SliderFloat("PBF Relaxation", &uiPbfRelaxation, 0.05f, 1.0f, "%.3f")) {
				particleSystem.SetPBFRelaxation(uiPbfRelaxation);
			}

			if (ImGui::DragFloat("SCorrK", &uiPbfScorrK, 0.00001f, 0.0f, 0.001f, "%.6f")) {
				particleSystem.SetPBFCorrK(uiPbfScorrK);
			}

			if (ImGui::SliderFloat("SCorrN", &uiPbfScorrN, 0.0f, 10.0f)) {
				particleSystem.SetPBFCorrN(uiPbfScorrN);
			}

			if (ImGui::SliderFloat("corrDQ", &uiPbfScorrDQ, 0.0f, 1.0f)) {
				particleSystem.SetPBFCorrDQ(uiPbfScorrDQ);
			}

			if (ImGui::DragFloat("pbfEpsilon", &uiPbfEpsilon, 0.000001f, 0.0f, 0.01f, "%.7f")) {
				particleSystem.SetPBFEpsilon(uiPbfEpsilon);
			}

			if (ImGui::SliderInt("Solver Iterations", &uiPbfSolverIterations, 0, 10)) {
				particleSystem.SetPBFSolverIterations(uiPbfSolverIterations);
			}			
			
		}

		ImGui::SeparatorText("Narrow-Range Filter for Screen space fluid rendering");
		ImGui::Checkbox("Enable Narrow-Range Filter", &uiEnableNarrowRangeFilter);
		ImGui::SliderInt("NR Radius", &uiNRFilterRadius, 1, 100);
		ImGui::SliderFloat("NR Sigma Spatial", &uiNRSigmaSpatial, 0.2f, 40.0f, "%.3f");
		ImGui::SliderFloat("NR Sigma Range Base", &uiNRSigmaRangeBase, 0.0001f, 2.0f, "%.5f");
		ImGui::SliderFloat("NR Sigma Range Scale", &uiNRSigmaRangeScale, 0.0f, 0.9f, "%.4f");
		ImGui::SliderFloat("Thickness Epsilon", &uiNRThicknessEpsilon, 0.0f, 1.0f);
		ImGui::SliderFloat("NR Threshold Ratio", &uiNRThresholdRatio, 0.1f, 2.0f, "%.3f");
		ImGui::SliderFloat("NR Clamp Ratio", &uiNRClampRatio, 0.1f, 2.0f, "%.3f");
		ImGui::SliderInt("NR Min Radius", &uiNRMinFilterRadius, 1, 24);
		ImGui::SliderFloat("NR Depth Adaptive Scale", &uiNRDepthAdaptiveScale, 0.01f, 2.0f, "%.3f");
		ImGui::SliderFloat("NR Min Sigma Spatial", &uiNRMinSigmaSpatial, 0.1f, 4.0f, "%.3f");


		if (ImGui::CollapsingHeader("Bounds", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::DragFloat3("Box min", &uiBoxMin.x, 0.01f);
			ImGui::DragFloat3("Box max", &uiBoxMax.x, 0.01f);

			if (ImGui::Button("Apply Bounds")) {
				const glm::vec3 minGap(0.05f);

				glm::vec3 safeMin = glm::min(uiBoxMin, uiBoxMax - minGap);
				glm::vec3 safeMax = glm::max(uiBoxMax, safeMin + minGap);

				uiBoxMin = safeMin;
				uiBoxMax = safeMax;

				bboxMin = safeMin;
				bboxMax = safeMax;

				particleSystem.SetBounds(safeMin, safeMax, true, true);

				bboxVerts[0] = bboxMin.x; bboxVerts[1] = bboxMin.y; bboxVerts[2] = bboxMin.z;
				bboxVerts[3] = bboxMax.x; bboxVerts[4] = bboxMin.y; bboxVerts[5] = bboxMin.z;
				bboxVerts[6] = bboxMax.x; bboxVerts[7] = bboxMax.y; bboxVerts[8] = bboxMin.z;
				bboxVerts[9] = bboxMin.x; bboxVerts[10] = bboxMax.y; bboxVerts[11] = bboxMin.z;
				bboxVerts[12] = bboxMin.x; bboxVerts[13] = bboxMin.y; bboxVerts[14] = bboxMax.z;
				bboxVerts[15] = bboxMax.x; bboxVerts[16] = bboxMin.y; bboxVerts[17] = bboxMax.z;
				bboxVerts[18] = bboxMax.x; bboxVerts[19] = bboxMax.y; bboxVerts[20] = bboxMax.z;
				bboxVerts[21] = bboxMin.x; bboxVerts[22] = bboxMax.y; bboxVerts[23] = bboxMax.z;

				glBindBuffer(GL_ARRAY_BUFFER, bboxVBO);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(bboxVerts), bboxVerts);
				glBindBuffer(GL_ARRAY_BUFFER, 0);

			}

		}

		if (ImGui::Button("Reset Particles")) {
			particleSystem.ResetParticles();
		}

		ImGui::End();

		//BBox
		bboxShader.use();
		bboxShader.setMat4("mvp", perspectiveProjection * view * glm::mat4(1.0f));
		bboxShader.setVec3("lineColor", glm::vec3(0.9f, 0.9f, 0.9f));
		glBindVertexArray(bboxVAO);
		glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);

		//Particle System
		float particleRadius = uiParticleRenderSize;
		float wallDamping = uiWallDamping;
		bool enableSPH = uiEnableSPH;
		if (uiSimulationRunning) {
			particleSystem.Update(deltaTime, wallDamping, enableSPH);
		}
		//write frames
		if (uiExportParticles && particleExporter.IsExporting()) {
			static std::vector<glm::vec4> pos;
			static std::vector<glm::vec4> vel;
			if (particleSystem.ReadbackParticles(pos, vel)) {
				particleExporter.WriteFrame(exportFrameIndex++, static_cast<float>(glfwGetTime()), pos, vel);
			}
		}

		glm::mat4 particleMVP = perspectiveProjection * view * glm::mat4(1.0f);


		if (showBoundaryGhosts) {
			//render boundary ghost particles
			particleSystem.RenderBoundary(particleMVP, glm::vec3(0.7f, 0.0f, 1.0f), 4.0f);
		}

		//plane
		quadShader.use();
		glm::mat4 planeModel = glm::mat4(1.0f);
		planeModel = glm::translate(planeModel, glm::vec3(0.0f, planeY, 0.0f));
		//planeModel = glm::translate(planeModel, -modelCenter);
		planeModel = glm::rotate(planeModel, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		planeModel = glm::scale(planeModel, glm::vec3(8.0f));

		glm::mat4 planeView = glm::lookAt(planeCameraPos, caveXRTConfig.cameraTarget, caveXRTConfig.cameraUp);

		

		glm::mat4 planeMVP = perspectiveProjection * view * planeModel;
		quadShader.setMat4("mvp", planeMVP);
		quadShader.setMat4("model", planeModel);
		quadShader.setMat4("view", view);
		quadShader.setMat4("projection", perspectiveProjection);
		quadShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
		quadShader.setVec3("cameraPosWorld", cameraPos);
		quadShader.setVec3("lightPos", lightPosWorld);
		quadShader.setBool("skyboxEnabled", caveXRTConfig.skyboxConfig.enabled);
		quadShader.setBool("showReflections", false);
		quadShader.setBool("showDepthMap", false);
		quadShader.setBool("useNormalMap", true);
		quadShader.setFloat("tessOuterLevel", tesselationOuterLevel);
		quadShader.setFloat("tessInnerLevel", tesselationInnerLevel);
		quadShader.setBool("useDisplacementMap", hasDisp);
		quadShader.setFloat("displacementScale", displacementScale);
		quadShader.setBool("useDispShadows", false);

		quadShader.setFloat("width", reflectionRenderTarget.Width());
		quadShader.setFloat("height", reflectionRenderTarget.Height());
		quadShader.setFloat("checkerScale", 50.0f);
		quadShader.setFloat("checkerColorStrength", 0.10f);
		quadShader.setFloat("envBlend", 0.04f);
		quadShader.setFloat("desaturationVal", 0.25f);

		quadShader.setVec3("ambient", glm::vec3(0.0f, 0.0f, 0.0f));
		quadShader.setVec3("diffuse", glm::vec3(0.2f, 0.2f, 0.2f));
		quadShader.setVec3("specular", glm::vec3(1.0f, 1.0f, 1.0f));

		quadShader.setFloat("ambientIntensity", 1.0f);
		quadShader.setFloat("specularIntensity", 1.0);
		quadShader.setFloat("glossiness", 32.0f);

		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, reflectionRenderTarget.GetColorTexture());
		//quadShader.setInt("renderTexture", 0);
		//		
		//glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		///*glGenerateMipmap(GL_TEXTURE_2D);*/
		//quadShader.setInt("cubemaptexture", 1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, depthTarget.GetDepthTexture());
		quadShader.setInt("depthMap", 2);

		if (normalMapTexture != 0) {
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, normalMapTexture);
			quadShader.setInt("normalMap", 3);
		}
		
		if (displacementMapTexture != 0) {
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, displacementMapTexture);
			quadShader.setInt("displacementMap", 4);
		}

		glPatchParameteri(GL_PATCH_VERTICES, 4);

		glBindVertexArray(quadVAO);
		//glDrawElements(GL_PATCHES, 4, GL_UNSIGNED_INT, nullptr);
		glDrawArrays(GL_PATCHES, 0, 4);
		glBindVertexArray(0);
	
		if (showTriangulation) {
			quadLineShader.use();
			quadLineShader.setMat4("model", planeModel);
			quadLineShader.setMat4("view", view);
			quadLineShader.setMat4("projection", perspectiveProjection);
			quadLineShader.setFloat("tessOuterLevel", tesselationOuterLevel);
			quadLineShader.setFloat("tessInnerLevel", tesselationInnerLevel);
			quadLineShader.setBool("useDisplacementMap", hasDisp);
			quadLineShader.setFloat("displacementScale", displacementScale);
			quadLineShader.setBool("useDispShadows", false);
			
			if (hasDisp) {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, displacementMapTexture);
				quadLineShader.setInt("displacementMap", 0);

			}


			quadLineShader.setFloat("lineDepthBiasNdc", 0.005f);
			quadLineShader.setVec3("lineColor", glm::vec3(1.0f, 1.0f, 0.0f));
			glBindVertexArray(quadVAO);
			//glDrawElements(GL_PATCHES, 6, GL_UNSIGNED_INT, nullptr);
			glDrawArrays(GL_PATCHES, 0, 4);
			glBindVertexArray(0);
		}

		

		glDepthMask(GL_FALSE);
		glDepthFunc(GL_LEQUAL);

		if (caveXRTConfig.skyboxConfig.enabled) {
			skyboxShader.use();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
			glm::mat4 skyboxView = glm::mat4(glm::mat3(view));
			skyboxShader.setMat4("projection", perspectiveProjection);
			skyboxShader.setMat4("view", skyboxView);
			skyboxShader.setInt("skybox", 0);

			cubeModel.Draw(skyboxShader);
		}
	
		glDepthFunc(GL_LESS);
		glDepthMask(GL_TRUE);
		/*if (caveXRTConfig.skyboxConfig.enabled) {
			
		}*/
		

		shaderprog2.use();

		glm::vec3 lampTarget = caveXRTConfig.cameraTarget;
		glm::vec3 lampForward = glm::normalize(lampTarget - lightPosWorld);
		glm::vec3 lampRight = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), lampForward));
		glm::vec3 lampUp = glm::cross(lampForward, lampRight);

		//Reflection buffer for fluid planar highlights
		reflectionRenderTarget.Bind();
		glViewport(0, 0, reflectionRenderTarget.Width(), reflectionRenderTarget.Height());
		glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_LEQUAL);
		skyboxShader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glm::mat4 reflectionSkyboxView = glm::mat4(glm::mat3(reflView));
		skyboxShader.setMat4("projection", perspectiveProjection);
		skyboxShader.setMat4("view", reflectionSkyboxView);
		skyboxShader.setInt("skybox", 0);
		cubeModel.Draw(skyboxShader);
		glDepthFunc(GL_LESS);
		glDepthMask(GL_TRUE);
		reflectionRenderTarget.UnBind();
		glViewport(0, 0, framebufferWidth, framebufferHeight);

		//Fluid Rendering 

		if (uiEnableParticles) {
			//Particle rendering
			glm::vec3 particleColor(0.2f, 0.0f, 1.0f);
			particleSystem.Render(particleMVP, particleColor, particleRadius, glm::vec2(framebufferWidth, framebufferHeight), perspectiveProjection, view, lightPosWorld);
		}
		else {
			//Fluid surface Rendering 
			fluidRenderTarget.Bind();
			glViewport(0, 0, fluidRenderTarget.Width(), fluidRenderTarget.Height());

			//glClearBufferv() works differently on different GPUs fixed the bug
			//DepthPass for Fluid surface rendering
			glDrawBuffer(GL_COLOR_ATTACHMENT0);
			const float clearDepthVal = 0.0f;
			glClearBufferfv(GL_COLOR, 0, &clearDepthVal);
			glClear(GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);
			glDepthMask(GL_TRUE);
			glDisable(GL_BLEND);
			particleSystem.RenderFluidDepth(particleMVP, particleRadius, glm::vec2(framebufferWidth, framebufferHeight), perspectiveProjection, view);

			//ThicknessPass for Fluid surface rendering
			glDrawBuffer(GL_COLOR_ATTACHMENT1);
			const float clearThicknessVal = 0.0f;
			glClearBufferfv(GL_COLOR, 0, &clearThicknessVal);
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);

			glDisable(GL_DEPTH_TEST);

			glDepthMask(GL_FALSE);
			particleSystem.RenderFluidThickness(particleMVP, particleRadius, glm::vec2(framebufferWidth, framebufferHeight), perspectiveProjection, view);
			glDepthMask(GL_TRUE);
			glDisable(GL_BLEND);

			glEnable(GL_DEPTH_TEST);

			fluidRenderTarget.UnBind();

			//narrow range filtering pass
			if (nrWidth != framebufferWidth || nrHeight != framebufferHeight) {
				CreateNarrowRangeRenderTargets(framebufferWidth, framebufferHeight);
			}

			uint32_t depthForComposite = fluidRenderTarget.GetDepthTexture();

			//if (uiEnableNarrowRangeFilter) {
			//	glDisable(GL_BLEND);
			//	glDisable(GL_DEPTH_TEST);

			//	fluidNarrowRangeShader.use();

			//	// --- repo uniforms ---
			//	fluidNarrowRangeShader.setFloat("u_ParticleRadius", particleRadius);
			//	fluidNarrowRangeShader.setInt("u_FilterSize", uiNRFilterRadius);
			//	fluidNarrowRangeShader.setInt("u_MaxFilterSize", 32); // try 16�32 later
			//	fluidNarrowRangeShader.setInt("u_ScreenWidth", nrWidth);
			//	fluidNarrowRangeShader.setInt("u_ScreenHeight", nrHeight);
			//	fluidNarrowRangeShader.setInt("u_DoFilter1D", 1);

			//	// ======================
			//	// HORIZONTAL PASS
			//	// ======================
			//	glBindFramebuffer(GL_FRAMEBUFFER, nrFBO[0]);
			//	glDrawBuffer(GL_COLOR_ATTACHMENT0);
			//	glReadBuffer(GL_NONE);
			//	glViewport(0, 0, nrWidth, nrHeight);

			//	const float clearNR0 = 0.0f;
			//	glClearBufferfv(GL_COLOR, 0, &clearNR0);

			//	fluidNarrowRangeShader.setInt("u_FilterDirection", 0); // 0 = horizontal

			//	glActiveTexture(GL_TEXTURE0);
			//	glBindTexture(GL_TEXTURE_2D, fluidRenderTarget.GetDepthTexture());
			//	fluidNarrowRangeShader.setInt("u_DepthTex", 0);

			//	glBindVertexArray(quadVAO);
			//	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

			//	// ======================
			//	// VERTICAL PASS
			//	// ======================
			//	glBindFramebuffer(GL_FRAMEBUFFER, nrFBO[1]);
			//	glDrawBuffer(GL_COLOR_ATTACHMENT0);
			//	glReadBuffer(GL_NONE);
			//	glViewport(0, 0, nrWidth, nrHeight);

			//	const float clearNR1 = 0.0f;
			//	glClearBufferfv(GL_COLOR, 0, &clearNR1);

			//	fluidNarrowRangeShader.setInt("u_FilterDirection", 1); // 1 = vertical

			//	glActiveTexture(GL_TEXTURE0);
			//	glBindTexture(GL_TEXTURE_2D, nrTex[0]);
			//	fluidNarrowRangeShader.setInt("u_DepthTex", 0);

			//	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

			//	glBindVertexArray(0);
			//	glBindFramebuffer(GL_FRAMEBUFFER, 0);

			//	depthForComposite = nrTex[1];
			//}

			if (uiEnableNarrowRangeFilter) {
				glDisable(GL_BLEND);
				glDisable(GL_DEPTH_TEST);

				fluidNarrowRangeShader.use();
				fluidNarrowRangeShader.setVec2("texelSize", glm::vec2(1.0f / nrWidth, 1.0f / nrHeight));
				fluidNarrowRangeShader.setInt("filterRadius", uiNRFilterRadius);
				fluidNarrowRangeShader.setFloat("sigmaSpatial", uiNRSigmaSpatial);
				fluidNarrowRangeShader.setFloat("sigmaRangeScale", uiNRSigmaRangeScale);
				fluidNarrowRangeShader.setFloat("sigmaRangeBase", uiNRSigmaRangeBase);
				fluidNarrowRangeShader.setFloat("thicknessEpsilon", uiNRThicknessEpsilon);
				fluidNarrowRangeShader.setFloat("particleRadius", particleRadius);
				fluidNarrowRangeShader.setFloat("thresholdRatio", uiNRThresholdRatio);
				fluidNarrowRangeShader.setFloat("clampRatio", uiNRClampRatio);
				fluidNarrowRangeShader.setInt("minFilterRadius", uiNRMinFilterRadius);
				fluidNarrowRangeShader.setFloat("depthAdaptiveScale", uiNRDepthAdaptiveScale);
				fluidNarrowRangeShader.setFloat("minSigmaSpatial", uiNRMinSigmaSpatial);

				//horizontal pass
				glBindFramebuffer(GL_FRAMEBUFFER, nrFBO[0]);
				glDrawBuffer(GL_COLOR_ATTACHMENT0);
				glReadBuffer(GL_NONE);
				glViewport(0, 0, nrWidth, nrHeight);
				const float clearNR0 = 0.0f;
				glClearBufferfv(GL_COLOR, 0, &clearNR0);

				fluidNarrowRangeShader.setVec2("direction", glm::vec2(1.0f, 0.0f));

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, fluidRenderTarget.GetDepthTexture());
				fluidNarrowRangeShader.setInt("inputDepthTexture", 0);

				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, fluidRenderTarget.GetFluidThicknessTexture());
				fluidNarrowRangeShader.setInt("fluidThicknessTexture", 1);

				glBindVertexArray(quadVAO);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

				//vertical pass
				glBindFramebuffer(GL_FRAMEBUFFER, nrFBO[1]);
				glDrawBuffer(GL_COLOR_ATTACHMENT0);
				glReadBuffer(GL_NONE);
				glViewport(0, 0, nrWidth, nrHeight);
				const float clearNR1 = 0.0f;
				glClearBufferfv(GL_COLOR, 0, &clearNR1);

				fluidNarrowRangeShader.setVec2("direction", glm::vec2(0.0f, 1.0f));

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, nrTex[0]);
				fluidNarrowRangeShader.setInt("inputDepthTexture", 0);

				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, fluidRenderTarget.GetFluidThicknessTexture());
				fluidNarrowRangeShader.setInt("fluidThicknessTexture", 1);

				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
				glBindVertexArray(0);

				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				depthForComposite = nrTex[1];


			}

			//normal reconstruction pass 
			fluidRenderTarget.Bind();
			glDrawBuffer(GL_COLOR_ATTACHMENT2);
			glReadBuffer(GL_NONE);
			glViewport(0, 0, fluidRenderTarget.Width(), fluidRenderTarget.Height());
			const float clearNormal[4]{ 0.0f, 0.0f, 0.0f, 0.0f };
			glClearBufferfv(GL_COLOR, 0, clearNormal);
			glDisable(GL_BLEND);
			glDisable(GL_DEPTH_TEST);

			fluidNormalReconstructShader.use();
			fluidNormalReconstructShader.setVec2("texelSize", glm::vec2(1.0f / framebufferWidth, 1.0f / framebufferHeight));
			fluidNormalReconstructShader.setMat4("projection", perspectiveProjection);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, depthForComposite);
			fluidNormalReconstructShader.setInt("fluidDepthTexture", 0);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, fluidRenderTarget.GetFluidThicknessTexture());
			fluidNormalReconstructShader.setInt("fluidThicknessTexture", 1);

			glBindVertexArray(quadVAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
			glBindVertexArray(0);
			fluidRenderTarget.UnBind();


			glViewport(0, 0, framebufferWidth, framebufferHeight);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDisable(GL_DEPTH_TEST);

			fluidRenderShader.use();
			fluidRenderShader.setVec2("texelSize", glm::vec2(1.0f / framebufferWidth, 1.0f / framebufferHeight));
			fluidRenderShader.setMat4("projection", perspectiveProjection);
			fluidRenderShader.setMat4("inverseView", glm::inverse(view));
			fluidRenderShader.setVec3("absorption", glm::vec3(1.4, 0.55, 0.20));
			fluidRenderShader.setFloat("refractionStrength", 0.95f);
			fluidRenderShader.setFloat("specularIntensity", 0.7f);
			fluidRenderShader.setFloat("shininess", 80.0f);
			fluidRenderShader.setFloat("fresnelPower", 2.5f);
			fluidRenderShader.setFloat("planeReflectionStrength", 0.6f);
			fluidRenderShader.setVec3("lightDirView", glm::normalize(lightPosView));
			fluidRenderShader.setVec3("cameraPosWorld", cameraPos);
			fluidRenderShader.setFloat("planeY", planeY);
			fluidRenderShader.setFloat("planeHalfExtent", 4.0f);
			fluidRenderShader.setVec3("shallowColor", glm::vec3(0.32, 0.68, 1.00));
			fluidRenderShader.setVec3("deepColor", glm::vec3(0.03, 0.18, 0.56));
			fluidRenderShader.setFloat("thicknessEpsilon", 1e-4f);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, depthForComposite);
			fluidRenderShader.setInt("fluidDepthTexture", 0);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, fluidRenderTarget.GetFluidThicknessTexture());
			fluidRenderShader.setInt("fluidThicknessTexture", 1);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, fluidRenderTarget.GetFluidNormalTexture());
			fluidRenderShader.setInt("fluidNormalTexture", 2);

			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
			fluidRenderShader.setInt("skybox", 3);

			glBindVertexArray(quadVAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
			glBindVertexArray(0);

			glDisable(GL_BLEND);
			glEnable(GL_DEPTH_TEST);
		}

		/*shaderprog2.use();
		glm::mat4 lightModel = glm::mat4(1.0f);
		lightModel = glm::translate(lightModel, lightPosWorld);

		glm::mat4 lightRotation(1.0f);
		lightRotation[0] = glm::vec4(lampRight, 0.0f);
		lightRotation[1] = glm::vec4(lampUp, 0.0f);
		lightRotation[2] = glm::vec4(-lampForward, 0.0f);

		lightModel = lightModel * lightRotation;
		lightModel = lightModel * glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

		glm::mat4 lightMVP = perspectiveProjection * view * lightModel;
		shaderprog2.setMat4("mvp", lightMVP);
		shaderprog2.setVec3("lightColor", lightColor);

 		lampModel.Draw(shaderprog2);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (videoRecorder.IsRecording()) {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			videoRecorder.CaptureFrame();
		}

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &quadVAO);
	glDeleteBuffers(1, &quadVBO);
	glDeleteBuffers(1, &quadEBO);

	if (nrTex[0] != 0) glDeleteTextures(2, nrTex);
	if (nrFBO[0] != 0) glDeleteFramebuffers(2, nrFBO);

	if (videoRecorder.IsRecording()) {
		videoRecorder.StopRecording();
	}

	if (particleExporter.IsExporting()) {
		particleExporter.EndSession();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}

