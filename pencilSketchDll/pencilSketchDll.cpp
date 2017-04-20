#include "pencilSketchDll.h"
#include "PencilSketchFilter.h"
#include "KinectWrapper.h"
#include <process.h>
#include <Windows.h>
KinectWrapper gKinect;
cv::Mat gBodyImg;

int gInit = 0;

void  kinectThread(void *para)
{
	/*while(1)
		gKinect.Update();*/
}
int initKinect()
{
	int isok=gKinect.InitializeDefaultSensor();
	if (!isok)
	{
		printf("kinect init failed\n");
		return -1;
	}
	_beginthread(kinectThread, 0, NULL);
	gInit = 1;
	return 1;
}


extern "C" __declspec(dllexport)
void processRGBATexture(Color_32 *pTexImg, int width, int height, Color_32 *pFilterTexture)
{
	using namespace cv;
	if (!gInit)
	{
		initKinect();
		
	}

	
//	gKinect.Update();
	
	if (gKinect.mBodyPng.data == nullptr)
		return;
	Mat bodyPng = gKinect.mBodyPng.clone();
	resize(bodyPng, bodyPng, Size(width, height));

	Mat grayImg;
	cvtColor(bodyPng, grayImg, COLOR_RGBA2GRAY);

	Mat filterImg;

	PencilSketchFilter sketchFilter;
	sketchFilter.processImage(grayImg, filterImg);

	Mat filterdTex(grayImg.size(), CV_8UC4, pFilterTexture);
	bodyPng.copyTo(filterdTex);

	for (int r = 0; r < height; r++)
	{
		uchar *pDst = filterdTex.ptr<uchar>(r);
		uchar *pSrc = filterImg.ptr<uchar>(r);
		for (int c = 0; c < width; c++)
		{
			//update R/G/B
			pDst[4 * c + 0] = pSrc[c];
			pDst[4 * c + 1] = pSrc[c];
			pDst[4 * c + 2] = pSrc[c];
		}
	}

}
extern "C" __declspec(dllexport)
void processGrayImg(unsigned char *pGrayImg, int width, int height, unsigned char *pFilterImg)
{

	using namespace cv;

	if (!gInit)
	{
		//initKinect();
		int isok=gKinect.InitializeDefaultSensor();
		if (!isok)
		{
			printf("kinect init failed\n");
			
		}
		gInit = 1;
		gBodyImg = cv::Mat::zeros(Size(width, height), CV_8UC4);
	}

	int isUpdated = gKinect.Update(gBodyImg);
	if (!isUpdated)
		return;

	Mat grayImg;
	cvtColor(gBodyImg, grayImg, COLOR_RGBA2GRAY);
	
	
	PencilSketchFilter sketchFilter;
	Mat filterImgOut(grayImg.size(), CV_8UC1, pFilterImg);
	sketchFilter.processImage(grayImg, filterImgOut);

	
	//filterImg.copyTo(filterImgOut);


}
