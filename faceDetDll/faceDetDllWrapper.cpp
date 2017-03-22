#include "faceDetDllWrapper.h"

#include "fceDetWrapper.h"
#include <process.h>
#include <opencv2\opencv.hpp>
#include <Windows.h>
#include <time.h>

#include "CustomSmoothFilter.h"

openFaceDetector gFaceDetector;
int gWidth, gHeight,gFaceImgWidth,gFaceImgHeight;
CustomSmoothFilter gFilter;

cv::VideoCapture gCap;

static void saveFaceImgToFile(cv::Mat facePng, char *pImgFile);
void  initThread(void *para)
{
	gFaceDetector.initFaceDetector(gWidth, gHeight);	
	CreateDirectory("face_images", NULL);
}
int initFaceDetector(int processWidth, int processHeight, int faceImgWidth, int faceImgHeight)
{
	if (processWidth == gWidth && processHeight == gHeight && 
		faceImgWidth== gFaceImgWidth && faceImgHeight == gFaceImgHeight &&gFaceDetector.mIsInited)
		return 1;

	gWidth = processWidth;
	gHeight = processHeight;
	gFaceImgWidth = faceImgWidth;
	gFaceImgHeight = faceImgHeight;
	_beginthread(initThread, 0, NULL);

	return 1;
}


void detectFace(unsigned char*pImg, int width, int height, stFaceDetectResult *result)
{
	memset(result, 0, sizeof(result));
	if (gFaceDetector.mIsInited)
	{
		stDetectResult detResult;
		gFaceDetector.detectFace(pImg, width, height, detResult);
		memcpy(result, &detResult, sizeof(detResult));
		//result.numMarks = detResult.numMarks;
	}
}
//void detectFaceGray(unsigned char*pImg, int width, int height, int &numMarks, float *p2D, float *p3D, float *pPose)
void detectFaceGray(unsigned char*pImg, int width, int height, int &numMarks, float p2D[], float p3D[], float pPose[])
{
	numMarks = 0;
	/*memset(p2D, 0, 200 * sizeof(float));
	memset(p3D, 0, 300 * sizeof(float));
	memset(pPose, 0, 6 * sizeof(float));*/
	if (gFaceDetector.mIsInited)
	{
		stDetectResult detResult;
		gFaceDetector.detectFace(pImg, width, height, detResult);
		numMarks = detResult.numMarks;
		for (int k = 0; k < numMarks*2; k++)
		{
			p2D[k] = detResult.landMarks_2D[k];
		}
		for (int k = 0; k < numMarks * 3; k++)
		{
			p3D[k] = detResult.landMarks_3D[k];
		}
		for (int k = 0; k < 6; k++)
		{
			pPose[k] = detResult.headPose[k];
		}
		/*memcpy(p2D, detResult.landMarks_2D, sizeof(detResult.landMarks_2D));
		memcpy(p3D, detResult.landMarks_3D, sizeof(detResult.landMarks_3D));
		memcpy(pPose, detResult.headPose, sizeof(detResult.headPose));*/
	}
	//pPose[0] = 100;
}

float gp2D[200];
float gp3D[300];
float gpPose[20];
int gNumMarks = 0;
cv::Mat gClrImg;
cv::Mat gFacePng;
int gThreadInit = 0;

int gFlag = 0;
void  threadUnity(void *para)
{
	while (gFlag != -1)
	{
		if (gFlag != 0)
			Sleep(1);
		if (!gCap.isOpened())
		{
			gCap.open(0);
		}

		if (!gCap.isOpened())
		{
			return;
		}

		using namespace cv;


		gCap >> gClrImg;
		/*if (clrImg.cols != width || clrImg.rows != height)
		{
		resize(clrImg, clrImg, Size(width, height));
		}*/

		gFilter.processImage(gClrImg);

		int width = gClrImg.cols;
		int height = gClrImg.rows;

		Mat gray;
		cvtColor(gClrImg, gray, CV_RGB2GRAY);

		detectFaceGray(gray.data, width, height, gNumMarks, gp2D, gp3D, gpPose);
		if (gNumMarks >= 66)
		{
			cv::Mat facePng;
			gFaceDetector.extractFacePng(gClrImg, gp2D, gFacePng);
			if (facePng.data == 0)
				return;
			resize(gFacePng, gFacePng, cv::Size(gFaceImgWidth, gFaceImgHeight));
		}
		if (gFlag == -1)
			break;
		gFlag = 1;
	}
	

}

//void detectFaceRGBA(Color_32 *pImg, int width, int height, int &numMarks,
//	float p2D[], float p3D[], float pPose[], char *pFaceImgPath, Color_32 *pFaceTexture)
//{
//	if (gThreadInit == 0)
//	{
//		_beginthread(threadUnity, 0, NULL);
//		gThreadInit = 1;
//	}
//	if (gFlag == 1)
//	{
//		using namespace cv;
//		uchar *pImgRaw = (uchar*)pImg;
//		Mat img(Size(width, height), CV_8UC4, pImgRaw);
//		cvtColor(gClrImg, img, CV_BGR2RGBA);
//		numMarks = gNumMarks;
//		memcpy(p2D, gp2D, numMarks * 2 * sizeof(float));
//		memcpy(p3D, gp3D, numMarks * 3 * sizeof(float));
//		memcpy(pPose, gpPose, 6 * sizeof(float));
//		if (gNumMarks >= 66)
//		{
//			
//			if (pFaceImgPath)
//			{
//				saveFaceImgToFile(gFacePng, pFaceImgPath);
//			}
//			if (pFaceTexture)
//			{
//				uchar *pDst = (uchar *)pFaceTexture;
//				Mat tempPng;
//				cvtColor(gFacePng, tempPng, CV_BGRA2RGBA);
//				memcpy(pDst, tempPng.data, tempPng.cols*tempPng.rows*tempPng.channels());
//			}
//		}
//		gFlag = 0;
//	}
//
//}
void detectFaceRGBA(Color_32 *pImg, int width, int height, int &numMarks,
					float p2D[], float p3D[], float pPose[], char *pFaceImgPath,Color_32 *pFaceTexture)
{
	if (!gCap.isOpened())
	{
		gCap.open(0);
		gCap.set(CV_CAP_PROP_FRAME_WIDTH, 640);
		gCap.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
	}

	if (!gCap.isOpened())
	{
		return;
	}

	using namespace cv;
	uchar *pImgRaw = (uchar*)pImg;
	Mat img(Size(width, height), CV_8UC4, pImgRaw);



	Mat clrImg;

	gCap >> clrImg;
	if (clrImg.cols != width || clrImg.rows != height)
	{
		resize(clrImg, clrImg, Size(width, height));
	}
	
	gFilter.processImage(clrImg);
	cvtColor(clrImg, img, CV_BGR2RGBA);

	//flip(clrImg, clrImg, 0);

	Mat gray;
	cvtColor(clrImg, gray, CV_RGB2GRAY);

	detectFaceGray(gray.data, width, height, numMarks, p2D, p3D, pPose);		
	if (numMarks >= 66)
	{
		cv::Mat facePng;
		gFaceDetector.extractFacePng(clrImg, p2D, facePng);
		if (facePng.data == 0)
			return;
		resize(facePng, facePng, cv::Size(gFaceImgWidth, gFaceImgHeight));
		if (pFaceImgPath)
		{
			saveFaceImgToFile(facePng, pFaceImgPath);
		}
		if (pFaceTexture)
		{
			uchar *pDst = (uchar *)pFaceTexture;
			cvtColor(facePng, facePng, CV_BGRA2RGBA);
			memcpy(pDst, facePng.data, facePng.cols*facePng.rows*facePng.channels());
		}
	}
		

	if (gFaceDetector.mIsInited)
	{
		gFaceDetector.drawResult(clrImg);
	}
		
	//imshow("result", clrImg);
	//waitKey(1);
}
extern "C" __declspec(dllexport)
void detectFaceRGB(unsigned char *pImg, int width, int height, int &numMarks, float p2D[], float p3D[], float pPose[], char *pFaceImgPath)
{

	using namespace cv;
	uchar *pImgRaw = (uchar*)pImg;
	Mat img(Size(width, height), CV_8UC3, pImgRaw);
	gFilter.processImage(img);
	
	Mat gray;
	cvtColor(img, gray, CV_BGRA2GRAY);
	
	detectFaceGray(gray.data, width, height, numMarks, p2D, p3D, pPose);

	if (numMarks >=66)
	{
		cv::Mat facePng;
		gFaceDetector.extractFacePng(img, p2D, facePng);
		if (facePng.data == nullptr)
		{
			printf("empty face\n");
			return;
		}
		resize(facePng, facePng, cv::Size(gFaceImgWidth, gFaceImgHeight));
		if (pFaceImgPath)
		{
			saveFaceImgToFile(facePng, pFaceImgPath);
		}

		//normalize(facePng, facePng, 0, 255, CV_MINMAX);

		imshow("face", facePng);
		
		
	}
	else if (numMarks > 0)
	{
		printf("found %d marks\n", numMarks);
	}
	
	//imshow("beati", beautifyImg);
	/*if (gFaceDetector.mIsInited)	
		gFaceDetector.drawResult(img);
	
		
	imshow("box", img);
	waitKey(1);*/

}

static void saveFaceImgToFile(cv::Mat facePng, char *pImgFile)
{
	time_t t1 = time(0);
	struct tm * now = localtime(&t1);
	now->tm_year += 1900;
	now->tm_mon += 1;

	sprintf_s(pImgFile,128, "face_images/%d-%02d-%02d_%02d%02d%02d.png\0", now->tm_year, now->tm_mon, now->tm_mday,
		now->tm_hour, now->tm_min, now->tm_sec);
	
	if (facePng.size().area() > 100)
	{
		//
		cv::imwrite(pImgFile, facePng);
		//printf("save image:%s\n", pImgFile);
	}
		
	//return png;
}
void uinitFaceDetector()
{
	gFaceDetector.releaseDetector();
	if(gCap.isOpened())
		gCap.release();
}