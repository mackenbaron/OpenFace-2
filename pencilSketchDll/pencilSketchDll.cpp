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

	memset(pFilterTexture, 255, width*height * sizeof(Color_32));

	Mat grayImg;
	cvtColor(gKinectCropper.mOutputClr, grayImg, COLOR_RGBA2GRAY);

	PencilSketchFilter sketchFilter;	
	Mat tempFiltered;
	sketchFilter.processImage(grayImg, tempFiltered);
	//imshow("f2", tempFiltered);
	Mat filterClr;
	cvtColor(tempFiltered, filterClr, CV_GRAY2BGR);
	for (int i = 0; i < gKinectCropper.mSrcContours.size(); i++)
		drawContours(filterClr, gKinectCropper.mSrcContours, i, Scalar(0, 0, 0), 1);

	//imshow("f1", filterClr);
	cvtColor(filterClr, tempFiltered, CV_BGR2GRAY);
	
	Mat bodyPng = Mat::zeros(tempFiltered.size(), CV_8UC4);
	for (int r = 0; r < bodyPng.rows; r++)
	{
		uchar *pDst = bodyPng.ptr<uchar>(r);
		uchar *pSrc = tempFiltered.ptr<uchar>(r);
		uchar *pMask = gKinectCropper.mOutputMask.ptr<uchar>(r);
		for (int c = 0; c < bodyPng.cols; c++)
		{
			if (pMask[c] == 0)
			{
				memset(pDst + 4 * c, 255, 3);				
				pDst[4 * c + 3] = 0;
			}
			else
			{
				memset(pDst + 4 * c, pSrc[c], 3);
				pDst[4 * c + 3] = 255;
			}
			//update R/G/B
			/*pDst[4 * c + 0] = pSrc[c];
			pDst[4 * c + 1] = pSrc[c];
			pDst[4 * c + 2] = pSrc[c];*/
		}
	}

	Mat filterImgOut(Size(width, height), CV_8UC4, (uchar*)pFilterTexture);
	resize(bodyPng, filterImgOut, filterImgOut.size());

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
		cvtColor(gKinectCropper.mOutputClr, grayImg, COLOR_RGBA2GRAY);

		PencilSketchFilter sketchFilter;
		Mat filterImgOut(Size(width,height), CV_8UC1, pFilterImg);
		Mat tempFiltered;
		sketchFilter.processImage(grayImg, tempFiltered);
		resize(tempFiltered, filterImgOut,filterImgOut.size());
	}

}
