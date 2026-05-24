#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

class DepthTarget {
public:
	DepthTarget(int width, int height);
	~DepthTarget();

	void Bind() const;
	void UnBind();

	uint32_t GetDepthTexture() const { return depthTexture; }

	int Width() const { return width; }
	int Height() const { return height; }

private:
	void Init();
	uint32_t FBO{ 0 };
	uint32_t depthTexture{ 0 };
	int width{ 0 };
	int height{ 0 };

};
