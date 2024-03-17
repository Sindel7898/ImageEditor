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


using namespace std;
using namespace cv;

class ImageProcessor {
public:

	cv::Mat  img;
	std::mutex PixelMutex;
	bool ShouldImageBeShown = false;
	std::condition_variable cv;
	cv::Mat bilateralFilterOutPut;
	
	float startY;
	float  EndY;
	cv::Mat ResizeImage(cv::Mat& inputImage);
	void applyDetailEnhanceToROI(cv::Mat& inputImage, const cv::Rect& roi);
	int ImageMainMultiThread();
	void CheckPixelQuality(cv::Mat& inputImage, int Start, int End);
	double ComputeSaturation(cv::Vec3b Pixel);
	void Boostcolor(cv::Vec3b& Pixel);
	void ExecuteTasks(cv::Mat image, float starY, float endY);
};