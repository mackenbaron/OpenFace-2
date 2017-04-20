// pencilSketchTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "pencilSketchDll.h"
#include <opencv2\opencv.hpp>
using namespace cv;
int main()
{
	
	
	Mat result = Mat::zeros(Size(200, 360), CV_8UC1);

	while (1)
	{
		
		
		processGrayImg(0, result.cols, result.rows, result.data);

		imshow("result", result);
		char c = waitKey(1);
		if (c == 27 || c == 'q')
			break;

		//cap >> img;

	}
    return 0;
}

