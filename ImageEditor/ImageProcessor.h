#pragma once
#include <iostream>
#include "opencv2/core.hpp"
#include <opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>
#include <opencv2/photo.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <chrono>
#include <thread>
#include <vector>
#include<mutex>
#include<future>
#include<algorithm>
#include<atomic>

using namespace std;
using namespace cv;

class Farm;

class ImageProcessor {
private:
	
	string Image1 = "../Images/IMAGE2.png";

	int NumberOfTasks;
	
	float startY;
	float  EndY;

	std::mutex ImageMutex;
	std::mutex PixelMutex;

	cv::Mat img;
	cv::Mat BilateralFilterOutPut;
	cv::Mat ResizeImage(cv::Mat& inputImage);


    
	double ComputeSaturation(cv::Vec3b Pixel);
	
	void Boostcolor(cv::Vec3b& Pixel);
	void ExecuteTasks(cv::Mat image, float starY, float endY);

public:

	Farm* farm;
	ImageProcessor(Farm& farm);

	std::mutex resize_mutex;
    bool ready_to_resize = false;
	std::condition_variable cv;

	void CheckPixelQuality(cv::Mat& inputImage, int Start, int End);
	void applyDetailEnhanceToROI(cv::Mat& inputImage, const cv::Rect& roi);

	int ImageMainMultiThreadWithFarm();
	int ImageMainMultiThread();
	int ImageSingleThread();
};