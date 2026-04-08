#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

class FluidRenderTarget {
public:
	FluidRenderTarget(int width, int height);
	~FluidRenderTarget();

	void Bind() const;
	void UnBind();

	void Resize(int width, int height);

	uint32_t GetDepthTexture() const { return fluidDepthTexture; }
	uint32_t GetFluidThicknessTexture() const { return fluidThicknessTexture; }
	uint32_t GetFluidNormalTexture() const { return fluidNormalTexture; }


	int Width() const { return width; }
	int Height() const { return height; }


private:
	void Init();

	uint32_t FBO{ 0 };
	uint32_t fluidDepthTexture{ 0 };
	uint32_t fluidThicknessTexture{ 0 };
	uint32_t fluidNormalTexture{ 0 };
	uint32_t depthRBO{ 0 };
	int width{ 0 };
	int height{ 0 };


};