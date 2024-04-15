#include "Farm.h"
#include "ImageProcessor.h"


//add tasks to ask queue
void Farm::add_task(Task task)
{
	//controls acessing to prevent race conditions
	std::lock_guard<std::mutex> lock(queueMutex);
	taskQueue.push(task);
}

//farm to process image 
void Farm::run(ImageProcessor* processor)
{
	//num of threads farm can run 
	 numThreads = 32;
	std::vector<std::thread> ThreadVector;

	// anonymos function to excecute tasks 
	auto Execute = [&]() {

		while (true)
		{
			Task task; 

			{
				// if all the tasks are done break 
				std::lock_guard<std::mutex> lock(queueMutex);
				if (taskQueue.empty()) {

					// sets if the image is able to be resized 
					{
						std::lock_guard<std::mutex> lock(processor->resize_mutex);
						processor->ready_to_resize = true;
					}

					//notivies to resize image
					processor->cv.notify_all();

					break;
				}

				//adds the current task thats in the front to the current task to be done 
				task = taskQueue.front();
				taskQueue.pop();
				
			}

			//effects and processes to be done 

			if (!task.image.empty()) {
				processor->CheckPixelQuality(task.image, task.startY, task.endY);
				cv::Rect roi(0, task.startY, task.image.cols, task.endY - task.startY);
				processor->applyDetailEnhanceToROI(task.image, roi);

			}
			
		}
    };

	//Executes the anonymos function 
	for (int i = 0; i < numThreads; ++i) {
		ThreadVector.push_back(std::thread(Execute));
	}

	// joins threads 
	for (auto& thread : ThreadVector) {
		thread.join(); 
	}
}


// Notify threads to stop and wait for them to finish
void Farm::stop_threads() {
	stopThreads = true;
}