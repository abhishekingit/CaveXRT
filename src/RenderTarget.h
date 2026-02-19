#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

class RenderTarget {
public:
	RenderTarget(int width, int height);
	~RenderTarget();

	void Bind() const;
	void UnBind();

	void Resize(int width, int height);


	uint32_t GetColorTexture() const { return colorTexture; }
	int Width() const { return width; }
	int Height() const { return height; }

private:
	void Init();

	uint32_t FBO{ 0 };
	uint32_t colorTexture{ 0 };
	uint32_t depthTexture{ 0 };
	int width{ 0 };
	int height{ 0 };

};