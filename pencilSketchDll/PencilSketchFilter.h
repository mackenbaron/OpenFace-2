#pragma once

#include <opencv2/opencv.hpp>

class PencilSketchFilter
{
public:
	PencilSketchFilter(void);
	~PencilSketchFilter(void);

	
	void processImage(cv::Mat srcImg, cv::Mat &pencilImg);
	void gradientImage(cv::Mat grayImg,cv::Mat &gradImg);
};

