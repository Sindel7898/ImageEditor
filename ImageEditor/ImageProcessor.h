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
public:
	 ImageProcessor(Farm& farm);
	Farm* farm;

	cv::Mat img;
	std::mutex ImageMutex;
	std::mutex PixelMutex;

	std::mutex ReizeMutex;
	std::condition_variable cv;

	bool readytoresizev = false;

	cv::Mat bilateralFilterOutPut;
	
	float startY;
	float  EndY;
	cv::Mat ResizeImage(cv::Mat& inputImage);
	void applyDetailEnhanceToROI(cv::Mat& inputImage, const cv::Rect& roi);
	int ImageMainMultiThread();
	int ImageSingleThread();

	int ImageMainMultiThreadWithFarm();

	void CheckPixelQuality(cv::Mat& inputImage, int Start, int End);
	double ComputeSaturation(cv::Vec3b Pixel);
	void Boostcolor(cv::Vec3b& Pixel);
	void ExecuteTasks(cv::Mat image, float starY, float endY);
};