#pragma once
#include <Kinect.h>
#include <vector>
#include <opencv2\opencv.hpp>
#include <mutex>

class KinectWrapper
{
public:
	KinectWrapper();
	~KinectWrapper();

	static const int        cDepthWidth = 512;
	static const int        cDepthHeight = 424;
	static const int        cColorWidth = 1920;
	static const int        cColorHeight = 1080;


public:
	
	// Current Kinect
	IKinectSensor*          m_pKinectSensor;
	ICoordinateMapper*      m_pCoordinateMapper;
	DepthSpacePoint*        m_pDepthCoordinates;

	// Frame reader
	IMultiSourceFrameReader*m_pMultiSourceFrameReader;

	RGBQUAD*                m_pOutputRGBX;
	RGBQUAD*                m_pBackgroundRGBX;
	RGBQUAD*                m_pColorRGBX;

	BYTE *mpMask;

	cv::Mat mBodyPng;
	int mIsInit;

	std::mutex mImgMutex;
	/// <summary>
	/// Main processing function
	/// </summary>
	int    Update(cv::Mat &bodyImg);

	cv::Mat getBodyImage();


	int InitializeDefaultSensor();
	void initMemBuffer();
	
	void  processFrame(INT64 nTime,
		const UINT16* pDepthBuffer, int nDepthWidth, int nDepthHeight,
		const RGBQUAD* pColorBuffer, int nColorWidth, int nColorHeight,
		const BYTE* pBodyIndexBuffer, int nBodyIndexWidth, int nBodyIndexHeight);
	void extractIPImage(cv::Mat img, std::vector<cv::Point> &srcContours, cv::Mat &png);
	
};

