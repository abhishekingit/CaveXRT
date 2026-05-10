#pragma once

#include "VideoRecorder.h"
#include <iostream>

VideoRecorder::VideoRecorder(int width, int height, int fps, const char* outputFilePath): f_width(width), f_height(height), fps(fps), outputFilePath(outputFilePath) {
	pixelBuffer.resize(static_cast<size_t>(f_width) * static_cast<size_t>(f_height) * 4u);
}

bool VideoRecorder::StartRecording() {
	if (f_isRecording) {
		std::cerr << "Already recording!" << std::endl;
		return false;
	}

	const char* ffmpegCommand = "ffmpeg -y -f rawvideo -vcodec rawvideo -pix_fmt rgba -s %dx%d -r %d -i - -c:v libx264 -preset slow -crf 12 -profile:v high -level 4.2 -pix_fmt yuv420p -vf vflip %s";
	char commandBuffer[512];
	snprintf(commandBuffer, sizeof(commandBuffer), ffmpegCommand, f_width, f_height, fps, outputFilePath);

#ifdef _WIN32
	ffmpegPipe = _popen(commandBuffer, "wb");
#else
	ffmpegPipe = popen(commandBuffer, "w");
#endif

	if (!ffmpegPipe) {
		std::cerr << "Failed to start ffmpeg process\n";
		return false;
	}

	f_isRecording = true;
	return true;

}

void VideoRecorder::CaptureFrame() {
	if (!f_isRecording) {
		std::cerr << "Not Recording! Start recording before capturing frames." << std::endl;
		return;

	}

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadBuffer(GL_BACK);
	glReadPixels(0, 0, f_width, f_height, GL_RGBA, GL_UNSIGNED_BYTE, pixelBuffer.data());

	const size_t bytesToWrite = pixelBuffer.size();
	const size_t bytesWritten = fwrite(pixelBuffer.data(), 1, bytesToWrite, ffmpegPipe);
	if (bytesWritten != bytesToWrite) {
		std::cerr << "Failed to write bytes to ffmpeg pipe\n";
		StopRecording();
	}
}

void VideoRecorder::StopRecording() {
	if (!f_isRecording) {
		std::cerr << "Not Recording! Start recording before stopping." << std::endl;
		return;
	}

	fflush(ffmpegPipe);

#ifdef  _WIN32
	_pclose(ffmpegPipe);
#else
	pclose(ffmpegPipe);
#endif 

	ffmpegPipe = nullptr;
	f_isRecording = false;

}
