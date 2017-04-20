#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include <Windows.h>
#include <Kinect.h>
#include<iostream>
#include<time.h>
#include "KinectWrapper.h"
#include "PencilSketchFilter.h"
using namespace std;
using namespace cv;



int main(int argc, char **argv[])
{
	KinectWrapper kw;
	PencilSketchFilter filter;
	if (!kw.InitializeDefaultSensor())
		return -1;

	while (true)
	{
		kw.Update();
		Mat bodyImg = kw.mBodyPng;
		if (bodyImg.data)
		{
			
			Mat gray;
			cvtColor(bodyImg, gray, COLOR_RGBA2GRAY);
			Mat pencilImg;
			filter.processImage(gray, pencilImg);

			imshow("pencil", pencilImg);
			waitKey(1);
			
		}
	}

	
	return 0;
}

