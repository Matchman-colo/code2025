#include "opencv2/opencv.hpp"
#include <string.h>
#include <cstdio>
#include <iostream>
#include <vector>
extern "C" {
    extern void convertToGray(const void* imageData, int width, int height, int channels);
    extern int whitePixelCount;
    extern void saveContourImage(const char* referPath, const char* inputPath, const char* outputPath,const char* outputPath_refer);
}

int whitePixelCount;
// C++ 函数实现
void convertToGray(const void* imageData, int width, int height, int channels)
 {
    cv::Mat image(height, width, CV_8UC3, (uchar*)imageData);
    if (image.empty()) {
        fprintf(stderr, "Input image is empty!\n");
        return;
    }
    
    //cv::imwrite("/mnt/pic.png", image);

    int x = 660;
    int y = 75;
    int w = 1369 - 660; // 感兴趣区域的宽度
    int h = 500 - 75;  // 感兴趣区域的高度
    
    // 提取感兴趣区域
    cv::Mat roi = image(cv::Rect(x, y, w, h));
   //cv::imwrite("/mnt/roi.png", roi);
    cv::Mat grayImage;
    cv::cvtColor(roi, grayImage, cv::COLOR_BGR2GRAY);

    cv::Mat blurredImage;
    cv::GaussianBlur(grayImage, blurredImage, cv::Size(1, 1), 0);
    //cv::imwrite("/mnt/bur.png", blurredImage);
    // 应用阈值分割
    cv::Mat thresholdedImage;
    // double thresholdValue = 100;// 阈值可以根据实际情况调整

    // cv::threshold(blurredImage, thresholdedImage, thresholdValue, 255, cv::THRESH_BINARY);
    // cv::imwrite("/mnt/thre.png", thresholdedImage);


    // 使用 inRange 函数将 75 到 100 之间的像素值设为白色，其他设为黑色
    cv::inRange(blurredImage, cv::Scalar(75), cv::Scalar(100), thresholdedImage);

    // 保存结果
    //cv::imwrite("/mnt/thre.png", thresholdedImage);

    whitePixelCount = 0;
     for (int y = 0; y < thresholdedImage.rows; ++y) {
        for (int x = 0; x < thresholdedImage.cols; ++x) {
            if (thresholdedImage.at<uchar>(y, x) == 255) {
                ++whitePixelCount;
            }
        }
    }
    
}



void saveContourImage(const char* referPath, const char* inputPath, const char* outputPath,const char* outputPath_refer) {
  // 读取图像
    //cv::Mat image = cv::imread(inputPath);
    cv::Mat image_refer = cv::imread(referPath);


    // 转换为灰度图像
    cv::Mat grayImage, grayImage_refer;
    //cv::cvtColor(image, grayImage, cv::COLOR_BGR2GRAY);
    cv::cvtColor(image_refer, grayImage_refer, cv::COLOR_BGR2GRAY);

    // 应用高斯模糊
    cv::Mat blurredImage, blurredImage_refer;
    //cv::GaussianBlur(grayImage, blurredImage, cv::Size(1, 1), 0);
    cv::GaussianBlur(grayImage_refer, blurredImage_refer, cv::Size(1, 1), 0);

    // 应用阈值分割
    cv::Mat thresholdedImage, thresholdedImage_refer;
    double thresholdValue = 100; // 阈值可以根据实际情况调整
    //cv::threshold(blurredImage, thresholdedImage, thresholdValue, 255, cv::THRESH_BINARY);
    cv::threshold(blurredImage_refer, thresholdedImage_refer, thresholdValue, 255, cv::THRESH_BINARY);
   
    int whitePixelCount = 0;
     for (int y = 0; y < thresholdedImage_refer.rows; ++y) {
        for (int x = 0; x < thresholdedImage_refer.cols; ++x) {
            if (thresholdedImage_refer.at<uchar>(y, x) == 255) {
                ++whitePixelCount;
            }
        }
    }
    std::cout << "Number of white pixels: " << whitePixelCount << std::endl;


    

    // 保存结果图像
    bool writeSuccess = cv::imwrite(outputPath_refer, thresholdedImage_refer);
    
}



