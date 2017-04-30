#pragma once
#include<opencv2\opencv.hpp>
class KinectBodyCropper
{
public:
	KinectBodyCropper();
	~KinectBodyCropper();

	cv::Mat mOutputMask;
	cv::Mat mOutputClr;

	std::vector<std::vector<cv::Point>>mSrcContours;
	std::vector<std::vector<cv::Point>>mExpandContours;


	int extractBodyImg(uchar *pBodyMask, uchar *pClrImg, int imgWidth, int imgHeight);

	int expandContours(std::vector<cv::Point> &srcCS, int expandSize, cv::Size imgSize);

	void extractContourImage(cv::Mat img, std::vector<cv::Point> &srcContours, cv::Mat &png);

	void smoothContour(std::vector<cv::Point> &cs);

	
};

