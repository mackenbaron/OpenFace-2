#pragma once
#include<opencv2\opencv.hpp>
class KinectBodyCropper
{
public:
	KinectBodyCropper();
	~KinectBodyCropper();

	cv::Mat mOutputMask;
	cv::Mat mOutputClr;
	cv::Mat mOutBodyImg;

	int extractBodyImg(uchar *pBodyMask, uchar *pClrImg, int imgWidth, int imgHeight);

	int cropBodyImage(std::vector<cv::Point> &srcCS, cv::Mat srcClrImg, cv::Mat &bodyImg);

	void extractContourImage(cv::Mat img, std::vector<cv::Point> &srcContours, cv::Mat &png);
};

