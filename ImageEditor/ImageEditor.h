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
	//cv::Mat Detailed;
	//cv::Mat MedianBlur;
	//cv::Mat ResizedImage;
	//cv::Mat AddedWeight;
	cv::Mat bilateralFilterOutPut;
	//cv::Mat ColorHist;
	//cv::Mat result;
	int startY;
	int EndY;
	cv::Mat ResizeImage(cv::Mat& inputImage);
	void AdjustWhiteBalance(cv::Mat& inputImage, double RedGain, double BlueGain, double Green);
	int ImageMainSingleThread();
	int ImageMainMultiThread();
	void CheckPixelQuality(cv::Mat& inputImage, int Start, int End);
	double ComputeSaturation(cv::Vec3b Pixel);
	void Boostcolor(cv::Vec3b& Pixel);
};