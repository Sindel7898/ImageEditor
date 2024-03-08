#include "ImageEditor.h"



void ImageProcessor::CheckPixelQuality(cv::Mat& inputImage, int Start, int End)
{
     std::lock_guard<std::mutex>lock(PixelMutex);

     for (int i = Start; i < End; ++i) {
        for (int j = 0; j < inputImage.cols; ++j) {

            cv::Vec3b pixel = inputImage.at<cv::Vec3b>(i, j);
            double saturation = ComputeSaturation(pixel);

            //if (saturation < 1)
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
    const double boostFactor = 1.4;

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



void ImageProcessor::AdjustWhiteBalance(cv::Mat& inputImage, double RedGain, double BlueGain, double Green)
{
    std::vector<cv::Mat> channels;

    cv::split(inputImage, channels);

    channels[0] = channels[0] * RedGain;
    channels[1] = channels[1] * BlueGain;
    channels[2] = channels[2] * Green;

    return cv::merge(channels, inputImage);
}



int ImageProcessor::ImageMainSingleThread()
{

    img = cv::imread("../Images/IMAGE4K2.jpg");

    //CheckPixelQuality(img);
    cv::detailEnhance(img, img, 2.f, 0.1f);
    cv::medianBlur(img, img, 3);
    cv::bilateralFilter(img, bilateralFilterOutPut, -1, 4, 10);
    AdjustWhiteBalance(bilateralFilterOutPut, 1.12, 1.12, 1.12);
  

     
    ResizeImage(bilateralFilterOutPut);
    std::string outputFilePath = "resized_image.png";
    imwrite(outputFilePath, bilateralFilterOutPut);
    
}


int ImageProcessor::ImageMainMultiThread()
{


     // auto clock_start = std::chrono::steady_clock::now();
      
         img = cv::imread("../Images/Image1.png");

        int NumberOfThreads = std::thread::hardware_concurrency();
        int rowsPerThread = img.rows / NumberOfThreads;

        //std::vector<std::thread> threadsVector (NumberOfThreads);

        //for (int i = 0; i < NumberOfThreads; i++) {

            // startY = i * rowsPerThread;
            // EndY = (i + 1) * rowsPerThread;

            
       //  }

        CheckPixelQuality(img, 100,400);


        //auto StartPixel([&]() {

          //  CheckPixelQuality(img, startY, EndY);

            //}); 


       // for (int i = 0; i < NumberOfThreads; ++i) {

       //     threadsVector[i] = std::thread(StartPixel);

       // }

        //for (std::thread& t : threadsVector) {

         //   t.join();
       // }
          //std::thread ColorBoostThread(&ImageProcessor::CheckPixelQuality,this,std::ref(img));
          //ColorBoostThread.join();
            

         // std::thread DetailThread([&]() {
         //       cv::detailEnhance(img, img, 2.f, 0.1f);
         //     });

        //  DetailThread.join();
          
        //  std::thread MedianBlurThread([&]() {
        //      cv::medianBlur(img, img,3);
         //     });

          //MedianBlurThread.join();

        //  std::thread addWeightedthread([&]() {
        //      //cv::addWeighted(img, 1.2, img, -0.1, 0.3, img);
        //      cv::bilateralFilter(img, bilateralFilterOutPut, -1, 4, 10);
//
         //     });

         // addWeightedthread.join();

         // std::thread AdjectWhiteBalance([&]() {
              //cv::addWeighted(img, 1.2, img, -0.1, 0.3, img);
           //   AdjustWhiteBalance(bilateralFilterOutPut, 1.12, 1.12,1.12);
           //   });

          //AdjectWhiteBalance.join();
        
        

        // ResizeImage(bilateralFilter);
         // if (ShouldImageBeShown) {

             cv::imshow("Window", img);
             cv::waitKey(0);
         // }
       
 

          ResizeImage(bilateralFilterOutPut);
          std::string outputFilePath = "resized_image.png";
          cv::imwrite(outputFilePath, bilateralFilterOutPut);

}




int main()
{
       
        ImageProcessor imageProcessor;

        const int numIterations = 20;
        long long totalTime = 0;

        //for (int i = 0; i < numIterations; ++i)
        //{
            auto start = std::chrono::steady_clock::now();

           // if (i == 20) {
              //  imageProcessor.ShouldImageBeShown = true;
           // }

            imageProcessor.ImageMainMultiThread();

            auto end = std::chrono::steady_clock::now();

            auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
           // std::cout << "Iteration " << i + 1 << ": " << elapsed_time.count() << " milliseconds" << std::endl;


            totalTime += elapsed_time.count();

            
       // }

        double averageTime = static_cast<double>(totalTime) / numIterations;
        std::cout << "Average time over " << numIterations << " iterations: " << averageTime << " milliseconds" << std::endl;

        return 0;
    

}


