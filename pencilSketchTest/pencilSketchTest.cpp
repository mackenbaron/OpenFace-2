// pencilSketchTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "pencilSketchDll.h"
#include <opencv2\opencv.hpp>
using namespace cv;
int main()
{
	
	
	Mat result = Mat::zeros(Size(640, 480), CV_8UC1);
	Mat result1 = Mat::zeros(Size(640, 480), CV_8UC4);
	int counter = 0;
	while (1)
	{
		
		
		//processGrayImg(0, result.cols, result.rows, result.data);
		processRGBATexture(0, result1.cols, result1.rows, (Color_32*)result1.data);

		imshow("result1", result1);
		char c = waitKey(1);
		if (c == 27 || c == 'q')
			break;	

	}
    return 0;
}

