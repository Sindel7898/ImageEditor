#include "Farm.h"
#include "ImageProcessor.h"


void Farm::add_task(Task task)
{
	std::lock_guard<std::mutex> lock(queueMutex);
	taskQueue.push(task);
}

void Farm::run(ImageProcessor* processor)
{
	int numThreads = 64; //std::thread::hardware_concurrency();
	std::vector<std::thread> ThreadVector;

	auto Execute = [&]() {

		while (true)
		{
			Task task; 

			{
				std::lock_guard<std::mutex> lock(queueMutex);
				if (taskQueue.empty()) {
					break;
				}

				task = taskQueue.front();
				taskQueue.pop();
			}

			processor->CheckPixelQuality(task.image, task.startY, task.endY);
			cv::Rect roi(0, task.startY, task.image.cols, task.endY - task.startY);
			processor->applyDetailEnhanceToROI(task.image, roi);
		}
    };

	for (int i = 0; i < numThreads; ++i) {
		ThreadVector.push_back(std::thread(Execute));
	}

	for (auto& thread : ThreadVector) {
		thread.join(); // Join threads
	
	}
}
