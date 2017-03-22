#pragma once

#include<opencv2\opencv.hpp>
#include<vector>
class CustomSmoothFilter
{
public:
	CustomSmoothFilter();
	~CustomSmoothFilter();


	void initFilter(int imgWidth,int imgHeight,int cn);

	void processImage(cv::Mat &img);

	void balanceClr(cv::Mat img,cv::Mat &dist);


private:	

	bool mbIsInited;
	std::vector<float> mvClrWeight;
	std::vector<float> mvSpaceWeight;
	std::vector<int> mvSpaceOffset;

	int miRadius;
	int miNumOfNeigs;
	cv::Mat mTempFilterResult;

	int mAbsTabList[512];

};

