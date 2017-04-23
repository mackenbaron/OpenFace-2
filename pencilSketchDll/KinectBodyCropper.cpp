#include "KinectBodyCropper.h"
#include<vector>
using namespace cv;
using namespace std;

KinectBodyCropper::KinectBodyCropper()
{
	mOutputMask = Mat::zeros(Size(640, 480), CV_8UC1);
	mOutputClr = Mat::zeros(Size(640, 480), CV_8UC4);
	mOutBodyImg = Mat::zeros(Size(640, 480), CV_8UC4);
}


KinectBodyCropper::~KinectBodyCropper()
{
}

int KinectBodyCropper::extractBodyImg(uchar *pBodyMask, uchar *pClrImg, int imgWidth, int imgHeight)
{

	Mat mask(Size(imgWidth, imgHeight), CV_8UC1, pBodyMask);
	resize(mask, mOutputMask, mOutputMask.size());

	Mat clrImg(Size(imgWidth, imgHeight), CV_8UC4, pClrImg);
	resize(clrImg, mOutputClr, mOutputClr.size());


	//Mat img(Size(nColorWidth, nColorHeight), CV_8UC4, m_pColorRGBX);
	//resize(mask, mask, Size(640, 480));
	//Mat clrImg;
	//resize(img, clrImg, Size(640, 480));

	Mat kernl = getStructuringElement(MORPH_RECT, Size(3, 3));

	erode(mOutputMask, mOutputMask, kernl);
	dilate(mOutputMask, mOutputMask, kernl);

	int numFgPixels = countNonZero(mask);
	if (numFgPixels < 1000)
		return 0;


	//imshow("mask", mOutputMask);
	
	//
	std::vector<std::vector<Point> > ccs;
	findContours(mOutputMask, ccs, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	memset(mOutBodyImg.data, 0, mOutBodyImg.cols*mOutBodyImg.rows*mOutBodyImg.channels());
	printf("contour size %d\n", ccs.size());
	int hasBody = 0;
	for (int i = 0; i < ccs.size(); i++)
	{
		
		if (ccs[i].size()<20/*||arcLength(Mat(_contoursQuery[i]),true) < 500*/)
			continue;
		std::vector<Point> &tempCS=ccs[i];

		int arcLen = 2;//arcLength(Mat(_contoursQuery[i]), true)*0.002
		approxPolyDP(Mat(tempCS), tempCS,arcLen , true);	

		cropBodyImage(tempCS, mOutputClr, mOutBodyImg);
		hasBody = 1;
	}

	return hasBody;
}


int KinectBodyCropper::cropBodyImage(std::vector<cv::Point> &srcCS, cv::Mat srcClrImg, cv::Mat &bodyImg)
{
	Mat tempImg = srcClrImg.clone();

	std::vector<std::vector<cv::Point>> ccs;
	ccs.push_back(srcCS);

	drawContours(tempImg, ccs, 0, Scalar(255, 255, 255), 3);

	Mat tempMask = Mat::zeros(tempImg.size(), CV_8UC3);
	drawContours(tempMask, ccs, 0, Scalar(255, 255, 255), -1);
	drawContours(tempMask, ccs, 0, Scalar(255, 255, 255), 3);

	

	Mat tempMaskGray;
	cvtColor(tempMask, tempMaskGray, COLOR_BGR2GRAY);	
	tempImg.copyTo(bodyImg, tempMaskGray);
	//imshow("expandmask", outImg);
	return 1;


	std::vector<std::vector<cv::Point>> ccs1;
	findContours(tempMaskGray, ccs1, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

	
	
	if (ccs1.size() != 1)
		return 0;

	extractContourImage(tempImg, ccs1[0], bodyImg);
	return 1;
}


void KinectBodyCropper::extractContourImage(cv::Mat img, std::vector<cv::Point> &cs, cv::Mat &png)
{
	
	//get bounding box
	float leftx = 10000, topy = 10000, rightx = 0, bottomy = 0;
	for (int i = 0; i < cs.size(); i++)
	{
		cv::Point pt = cs[i];
		if (pt.x < leftx)
			leftx = pt.x;
		if (pt.x > rightx)
			rightx = pt.x;

		if (pt.y < topy)
			topy = pt.y;
		if (pt.y > bottomy)
			bottomy = pt.y;
	}
	if (leftx == 10000)
		return;

	int expandSize = 4;
	leftx -= expandSize;
	topy -= expandSize;
	rightx += expandSize;
	bottomy += expandSize;

	cv::Rect rct(leftx, topy, rightx - leftx+1 , bottomy - topy+1);
	if (rct.width + rct.x > img.cols)
		rct.width = img.cols - rct.x;
	if (rct.height + rct.y > img.rows)
		rct.height = img.rows - rct.y;

	Mat subImg = img(rct).clone();

	std::vector<cv::Point> subContours;
	for (int i = 0; i < cs.size(); i++)
	{
		cv::Point pt = cs[i];
		pt.x -= rct.x;
		pt.y -= rct.y;
		subContours.push_back(pt);
	}
	Mat mask = Mat::zeros(Size(rct.width, rct.height), CV_8UC1);
	std::vector<std::vector<cv::Point>> subCCs;
	subCCs.push_back(subContours);
	drawContours(mask, subCCs, 0, Scalar(255, 255, 255), -1);

	png = Mat::zeros(subImg.size(), CV_8UC4);
	int dstChannles = png.channels();
	int srcChannels = img.channels();
	for (int r = 0; r < subImg.rows; r++)
	{
		uchar *ptrImgRow = subImg.ptr<uchar>(r);
		uchar *ptrAlphaRow = mask.ptr<uchar>(r);
		uchar *ptrPngRow = png.ptr<uchar>(r);
		for (int c = 0; c < subImg.cols; c++)
		{
			if (ptrAlphaRow[c] == 0)
				continue;
			ptrPngRow[dstChannles * c + 0] = ptrImgRow[srcChannels * c + 0];
			ptrPngRow[dstChannles * c + 1] = ptrImgRow[srcChannels * c + 1];
			ptrPngRow[dstChannles * c + 2] = ptrImgRow[srcChannels * c + 2];
			ptrPngRow[dstChannles * c + 3] = ptrAlphaRow[c] == 0 ? 0 : 255;
		}
	}
	

}