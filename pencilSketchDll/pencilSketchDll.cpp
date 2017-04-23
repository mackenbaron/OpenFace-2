#include "pencilSketchDll.h"
#include "PencilSketchFilter.h"
#include "KinectWrapper.h"
#include <process.h>
#include <Windows.h>
#include "KinectBodyCropper.h"
KinectWrapper gKinect;
KinectBodyCropper gKinectCropper;

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
	memset(pFilterTexture, 0, width*height * sizeof(Color_32));

	if (!gInit)
	{
		//initKinect();
		int isok = gKinect.InitializeDefaultSensor();
		if (!isok)
		{
			printf("kinect init failed\n");

		}
		gInit = 1;

	}

	int isUpdated = gKinect.Update();
	if (!isUpdated)
		return;

	int hasBody = gKinectCropper.extractBodyImg(gKinect.m_pBodyMask,
		(uchar*)gKinect.m_pColorRGBX, gKinect.cColorWidth, gKinect.cColorHeight);

	if (!hasBody)
		return;

	Mat grayImg;
	cvtColor(gKinectCropper.mOutBodyImg, grayImg, COLOR_RGBA2GRAY);

	PencilSketchFilter sketchFilter;	
	Mat tempFiltered;
	sketchFilter.processImage(grayImg, tempFiltered);
	
	Mat bodyPng = gKinectCropper.mOutBodyImg.clone();
	for (int r = 0; r < height; r++)
	{
		uchar *pDst = bodyPng.ptr<uchar>(r);
		uchar *pSrc = tempFiltered.ptr<uchar>(r);
		for (int c = 0; c < width; c++)
		{
			//update R/G/B
			pDst[4 * c + 0] = pSrc[c];
			pDst[4 * c + 1] = pSrc[c];
			pDst[4 * c + 2] = pSrc[c];
		}
	}

	Mat filterImgOut(Size(width, height), CV_8UC4, (uchar*)pFilterTexture);
	resize(bodyPng, filterImgOut, filterImgOut.size());

	//imshow("bodypng", bodyPng);

	//printf("has body ...............\n");

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
		
	}

	int isUpdated = gKinect.Update();
	if (!isUpdated)
		return;

	int hasBody = gKinectCropper.extractBodyImg(gKinect.m_pBodyMask, 
					(uchar*)gKinect.m_pColorRGBX, gKinect.cColorWidth, gKinect.cColorHeight);
	
	if (hasBody)
	{
		Mat grayImg;
		cvtColor(gKinectCropper.mOutBodyImg, grayImg, COLOR_RGBA2GRAY);

		PencilSketchFilter sketchFilter;
		Mat filterImgOut(Size(width,height), CV_8UC1, pFilterImg);
		Mat tempFiltered;
		sketchFilter.processImage(grayImg, tempFiltered);
		resize(tempFiltered, filterImgOut,filterImgOut.size());
	}

}
