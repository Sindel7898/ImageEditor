#pragma once
#include <queue>
#include <mutex>
#include <vector>
#include "opencv2/core.hpp"

struct Task {
	cv::Mat image;
	float startY;
	float endY;
};

class ImageProcessor;

class Farm {

private:
	std::queue<Task> taskQueue;
	std::mutex queueMutex;
	std::mutex resultMutex;
	std::atomic<bool> stopThreads = false;
	int numThreads = 0;
public:
	void add_task(Task task);
	void run(ImageProcessor* processor);
	void stop_threads();
    inline	int GetNumOfThreads() { return numThreads; };
};