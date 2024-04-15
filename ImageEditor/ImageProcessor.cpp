#include "ImageProcessor.h"
#include "Farm.h"

ImageProcessor::ImageProcessor(Farm& farm) : farm(&farm) {}

//initial check to decide if pixel color should be boosted 
void ImageProcessor::CheckPixelQuality(cv::Mat& inputImage, int Start, int End)
{
    std::lock_guard<std::mutex>lock(PixelMutex);
     //loops through the image
     for (int i = Start; i < End; ++i) {
        for (int j = 0; j < inputImage.cols; ++j) {

            //extracts pixel
            cv::Vec3b pixel = inputImage.at<cv::Vec3b>(i, j);

            double saturation = ComputeSaturation(pixel);

            if (saturation < 1)
            {
                Boostcolor(pixel);
            }
            //replace the edited pixel witht the one that was prevoiusly there
            inputImage.at<cv::Vec3b>(i, j) = pixel;
        }
    }
}

//calculate image saturation
double ImageProcessor::ComputeSaturation(cv::Vec3b Pixel)
{
    cv::Mat3b hsv;
    cv::cvtColor(cv::Mat3b(1, 1, const_cast<cv::Vec3b*>(&Pixel)), hsv, cv::COLOR_BGR2HSV);
    double saturation = hsv(0, 0)[1] / 255.0;
    return saturation;
}

//bosts image color by increasing the color on each pixel by a set value
void ImageProcessor::Boostcolor(cv::Vec3b& Pixel)
{
    const double boostFactor = 2;

    Pixel[0] = static_cast<uchar>(std::min(boostFactor * Pixel[0], 255.0));  // Boost blue channel
    Pixel[1] = static_cast<uchar>(std::min(boostFactor * Pixel[1], 255.0));  // Boost green channel
    Pixel[2] = static_cast<uchar>(std::min(boostFactor * Pixel[2], 255.0));  // Boost red channel

}

//resizes Image to 4k resolution 
cv::Mat ImageProcessor::ResizeImage(cv::Mat& inputImage)
{
    int NewWidth = 3840;
    int NewHeight = 2160;

    cv::resize(inputImage, img, cv::Size(NewWidth, NewHeight));
    return img;
}


//apply enhancements on only speic parts of the image so image effects can be ran on seperate threads
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

//tasks to excecute
void ImageProcessor::ExecuteTasks(cv::Mat image, float starY, float endY)
{
    CheckPixelQuality(image, starY, endY);
    cv::Rect roi(0, starY, img.cols, endY - starY);
    applyDetailEnhanceToROI(img, roi);
}



int ImageProcessor::ImageMainMultiThread()
{      
         //loads image
         img = cv::imread(Image1);

         //sets the amount of threads to run change depending on how many you want 
         int NumberOfThreads = 32;
        int rowsPerThread = img.rows / NumberOfThreads;

        std::vector<std::thread> threadsVector;

        //devides the imaged into different parts for a thread to compute
        for (int i = 0; i < NumberOfThreads; i++) {

             startY = i * rowsPerThread;
             EndY = (i + 1) * rowsPerThread;

             threadsVector.push_back(std::thread(&ImageProcessor::ExecuteTasks, this, std::ref(img), startY, EndY));    
        }
       
        //joinds back all threads
       for (auto& t : threadsVector) {
          t.join();
        }
       
        
          //resize image then output it writen to a file 
          img =  ResizeImage(img);
          std::string outputFilePath = "resized_image.png";
          cv::imwrite(outputFilePath, img);
}

int ImageProcessor::ImageSingleThread()
{
    //loads image
    img = cv::imread(Image1);
    //sets number of splits needed set to 1 becuase we are only running 1 thread
    int NumberOfThreads = 1;
    int rowsPerThread = img.rows / NumberOfThreads;

    // runs pixel thread based on split
    for (int i = 0; i < NumberOfThreads; i++) {

        startY = i * rowsPerThread;
        EndY = (i + 1) * rowsPerThread;

        CheckPixelQuality(img, startY, EndY);
    }

    // applies different enhancements to image
    cv::detailEnhance(img, img, 2.f, 0.1f);
    cv::medianBlur(img, img, 3);
    cv::addWeighted(img, 1.2, img, -0.1, 0.3, img);

    //resize image then output it writen to a file 
    img = ResizeImage(img);
    std::string outputFilePath = "resized_image.png";
    cv::imwrite(outputFilePath, img);
}


int ImageProcessor::ImageMainMultiThreadWithFarm() {

    //loads image
    img = cv::imread(Image1);

    //sets the amount of threads to run change depending on how many you want 
    int Numofthreads = farm->GetNumOfThreads();

    if (Numofthreads == 64) { NumberOfTasks = 70; }
    

    if (Numofthreads == 32) { NumberOfTasks = 130; }
    

    if (Numofthreads == 16) { NumberOfTasks = 900;}
 

    if (Numofthreads < 16) { NumberOfTasks = 900; }
    

    float  rowsPerThread = img.rows / static_cast<float>(NumberOfTasks);

    std::vector<Task> tasks;

    //devides the imaged into different parts for a thread to compute
    // converts the data into the task struct and add it into the tasks vector
    for (float  i = 0; i < NumberOfTasks; i++) {
        float startY = i * rowsPerThread;
        float endY = (i + 1) * rowsPerThread;
        
        //controls acessing to prevent race conditions
        std::lock_guard<std::mutex>lock(ImageMutex);
        tasks.push_back({ img,startY,endY });
    }

    // Add tasks to the farm's task queue
    for (const auto& task : tasks) {
        farm->add_task(task);
    }
    

    //async to resize image 
     auto Risizer = std::async(std::launch::async, [this]()
        {
            return ResizeImage(img);
        });

     // runs the farm
     farm->run(this);
     farm->stop_threads();

     std::unique_lock<std::mutex> lock(resize_mutex);
     cv.wait(lock, [this] { return ready_to_resize; });

     // gets the resized image from the promise
     cv::Mat resizedImage = Risizer.get();

 
     //writes the outputed image to a file
    std::string outputFilePath = "FinalImage.png";
    cv::imwrite(outputFilePath, resizedImage);
}



int main()
{
       // initialise farm an image procesor 
        Farm farm;
        ImageProcessor imageProcessor(farm);
       
        const int numIterations = 20;
        long long totalTime = 0;

        // perform tasks the number of iterations and them record the time an canculate the average
        for (int i = 0; i < numIterations; ++i)
        {        
               auto start = std::chrono::steady_clock::now(); //starts tyimer

               //imageProcessor.ImageMainMultiThreadWithFarm();
               //imageProcessor.ImageSingleThread();
              imageProcessor.ImageMainMultiThread();
            auto end = std::chrono::steady_clock::now();//emd timer

            auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << "Iteration " << i + 1 << ": " << elapsed_time.count() << " milliseconds" << std::endl;

            totalTime += elapsed_time.count();
        }

        double averageTime = static_cast<double>(totalTime) / numIterations;
        std::cout << "Average time over " << numIterations << " iterations: " << averageTime << " milliseconds" << std::endl;

        return 0;
}
