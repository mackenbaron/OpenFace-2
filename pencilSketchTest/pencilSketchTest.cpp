// pencilSketchTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "PencilSketchFilter.h"
using namespace cv;
int main()
{
	VideoCapture cap;
	if (!cap.open(0))
		return -1;

	Mat img;
	cap >> img;
	PencilSketchFilter sketchFilter;
	while (img.data)
	{
		
		Mat result;
		sketchFilter.processImage(img, result);
		imshow("result", result);
		char c = waitKey(1);
		if (c == 27 || c == 'q')
			break;

		cap >> img;

	}
    return 0;
}

