#include "ImageEditor.h"



void ImageProcessor::CheckPixelQuality(cv::Mat& inputImage, int Start, int End)
{
     std::lock_guard<std::mutex>lock(PixelMutex);

     for (int i = Start; i < End; ++i) {
        for (int j = 0; j < inputImage.cols; ++j) {

            cv::Vec3b pixel = inputImage.at<cv::Vec3b>(i, j);
            double saturation = ComputeSaturation(pixel);

            if (saturation < 1)
            {
                Boostcolor(pixel);
            }

            inputImage.at<cv::Vec3b>(i, j) = pixel;
        }
    }
}

double ImageProcessor::ComputeSaturation(cv::Vec3b Pixel)
{
    cv::Mat3b hsv;
    cv::cvtColor(cv::Mat3b(1, 1, const_cast<cv::Vec3b*>(&Pixel)), hsv, cv::COLOR_BGR2HSV);

    double saturation = hsv(0, 0)[1] / 255.0;
    //std::cout << "CurrentPixel Intensity   " << saturation << std::endl;

    return saturation;
}

void ImageProcessor::Boostcolor(cv::Vec3b& Pixel)
{
    const double boostFactor = 2;

    Pixel[0] = static_cast<uchar>(std::min(boostFactor * Pixel[0], 255.0));  // Boost blue channel
    Pixel[1] = static_cast<uchar>(std::min(boostFactor * Pixel[1], 255.0));  // Boost green channel
    Pixel[2] = static_cast<uchar>(std::min(boostFactor * Pixel[2], 255.0));  // Boost red channel

}


cv::Mat ImageProcessor::ResizeImage(cv::Mat& inputImage)
{
    int OriginalWidth = inputImage.cols;
    int OriginalHeight = inputImage.rows;
    double aspectratio = static_cast<double>(OriginalWidth / OriginalHeight);                                     

    int NewWidth = 3840;
    int NewHeight = 2160;

    std:Mat ResizedImage;
    cv::resize(inputImage, ResizedImage, cv::Size(NewWidth, NewHeight));
    return ResizedImage;
}



void ImageProcessor::applyDetailEnhanceToROI(cv::Mat& inputImage, const cv::Rect& roi)
{
    // Ensure the ROI is within the image bounds
    cv::Rect validROI = roi & cv::Rect(0, 0, inputImage.cols, inputImage.rows);

    // Create a mask for the ROI
    cv::Mat mask = cv::Mat::zeros(inputImage.size(), CV_8U);
    mask(validROI) = 255;

    // Apply detailEnhance only to the ROI
    cv::Mat roiImage = inputImage(validROI);
    cv::detailEnhance(roiImage, roiImage, 2.f, 0.1f);
    cv::medianBlur(roiImage, roiImage,3);
    cv::addWeighted(roiImage, 1.2, roiImage, -0.1, 0.3, roiImage);
}


void ImageProcessor::ExecuteTasks(cv::Mat image, float starY, float endY)
{
    CheckPixelQuality(image, starY, endY);
    cv::Rect roi(0, starY, img.cols, endY - starY);
    applyDetailEnhanceToROI(img, roi);
}


int ImageProcessor::ImageMainMultiThread()
{


        auto clock_start = std::chrono::steady_clock::now();
      
         img = cv::imread("../Images/IMAGE2.png");  

        int NumberOfThreads = 2;//std::thread::hardware_concurrency();
        int rowsPerThread = img.rows / NumberOfThreads;

        std::vector<std::thread> threadsVector;

        for (int i = 0; i < NumberOfThreads; i++) {

             startY = i * rowsPerThread;
             EndY = (i + 1) * rowsPerThread;

             threadsVector.push_back(std::thread(&ImageProcessor::ExecuteTasks, this, std::ref(img), startY, EndY));    
        }
       

       for (auto& t : threadsVector) {
          t.join();
        }
        

         // ResizeImage(img);
         // if (ShouldImageBeShown) {
       
           //  cv::imshow("Window", img);
           //  cv::waitKey(0);
         // }
       
 

          std::string outputFilePath = "resized_image.png";
          cv::imwrite(outputFilePath, img);

}




int main()
{
       
        ImageProcessor imageProcessor;

        const int numIterations = 20;
        long long totalTime = 0;

        for (int i = 0; i < numIterations; ++i)
        {
            auto start = std::chrono::steady_clock::now();

           if (i == 20) {
                imageProcessor.ShouldImageBeShown = true;
            }

            imageProcessor.ImageMainMultiThread();

            auto end = std::chrono::steady_clock::now();

            auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << "Iteration " << i + 1 << ": " << elapsed_time.count() << " milliseconds" << std::endl;


            totalTime += elapsed_time.count();

            
        }

        double averageTime = static_cast<double>(totalTime) / numIterations;
        std::cout << "Average time over " << numIterations << " iterations: " << averageTime << " milliseconds" << std::endl;

        return 0;
    

}


