// pencilSketchTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "pencilSketchDll.h"
#include <opencv2\opencv.hpp>
using namespace cv;
int main()
{
	VideoCapture cap;
	if (!cap.open(0))
		return -1;

	Mat img;
	cap >> img;
	
	while (img.data)
	{
		Mat gray;
		cvtColor(img, gray, COLOR_BGR2GRAY);

		Mat result=gray.clone();
		
		processGrayImg(gray.data, gray.cols, gray.rows, result.data);
		imshow("result", result);
		char c = waitKey(1);
		if (c == 27 || c == 'q')
			break;

		cap >> img;

	}
    return 0;
}

