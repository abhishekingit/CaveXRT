#pragma once


#include <glad/glad.h>
#include <cstdio>
#include <cstdint>
#include <vector>


class VideoRecorder {
public:
	
	VideoRecorder(int width, int height, int fps, const char* outputFilePath);

	bool StartRecording();
	void StopRecording();
	void CaptureFrame();
	bool IsRecording() const { return f_isRecording;  }

private:
	int f_width = 0;
	int f_height = 0;
	int fps = 60;
	const char* outputFilePath;

	FILE* ffmpegPipe = nullptr;
	std::vector<std::uint8_t> pixelBuffer;
	bool f_isRecording = false;
};