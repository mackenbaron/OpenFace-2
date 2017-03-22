// dllTester.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "faceDetDllWrapper.h"
#include <opencv2\opencv.hpp>
#include <stdio.h>


using namespace cv;
using namespace std;

int main()
{
	initFaceDetector(320, 240,160,200);
	
	
	cv::VideoCapture video_capture;	
	if (!video_capture.open(0))
		return -1;

	video_capture.set(CV_CAP_PROP_FRAME_WIDTH, 320);

	video_capture.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
	Mat img;
	video_capture >> img;

	
	char *pImgFile = 0;

	double freq = getTickFrequency();
	int64 lastTick = 0;
	
	while (true)
	{
		//resize(img, img, Size(320, 240));
		//beautifyImage(img);

		int64 tick = getTickCount();
	
	
		stFaceDetectResult result;
		
		memset(&result, 0, sizeof(result));
		//detectFace(gray.data, gray.cols, gray.rows, &result);
		//detectFaceGray(gray.data, gray.cols, gray.rows, result.numMarks, result.landMarks_2D, result.landMarks_3D, result.headPose);
		detectFaceRGB(img.data, img.cols, img.rows, result.numMarks, result.landMarks_2D, result.landMarks_3D, result.headPose,pImgFile);
		
		float ts = (getTickCount() - tick) / freq;
		lastTick = tick;
		printf("ts=%f\n", ts);

		Mat facePng;
		//extractFacePng(srcImg, img.size(), result.landMarks_2D, facePng);
		//for (int k = 0; k < result.numMarks; k++)

		Mat dspImg = img.clone();

		for (int k = 0; k < 27; k++)
		{
			cv::Point pt(result.landMarks_2D[2 * k], result.landMarks_2D[2 * k + 1]);
			cv::circle(dspImg, pt, 3, Scalar(0, 0, 255));
		}
		
		for (int k = 27; k < 36; k++)
		{
			cv::Point pt(result.landMarks_2D[2 * k], result.landMarks_2D[2 * k + 1]);
			cv::circle(dspImg, pt, 3, Scalar(0,255, 0));
		}
		for (int k = 36; k < 48; k++)
		{
			cv::Point pt(result.landMarks_2D[2 * k], result.landMarks_2D[2 * k + 1]);
			cv::circle(dspImg, pt, 3, Scalar(255, 255, 0));
		}
		for (int k = 48; k < 60; k++)
		{
			cv::Point pt(result.landMarks_2D[2 * k], result.landMarks_2D[2 * k + 1]);
			cv::circle(dspImg, pt, 3, Scalar(0, 255, 255));
		}
		for (int k = 60; k < result.numMarks; k++)
		{
			cv::Point pt(result.landMarks_2D[2 * k], result.landMarks_2D[2 * k + 1]);
			cv::circle(dspImg, pt, 3, Scalar(255, 0, 0));
		}
		
		imshow("img", img);
		char c = waitKey(1);
		if (c == 27 || c == 'q')
			break;
		if (c == 's')
		{
			//imwrite("face.png", facePng);
			pImgFile = new char[128];
		}
		else
		{
			pImgFile = 0;
		}
		video_capture >> img;
		//printf("headpose,%.1f,%.1f,%.1f\n", result.headPose[0], result.headPose[1], result.headPose[2]);
	}
    return 0;
}

