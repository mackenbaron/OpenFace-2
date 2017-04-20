
#include "PencilSketchFilter.h"

using namespace cv;


PencilSketchFilter::PencilSketchFilter(void)
{
}


PencilSketchFilter::~PencilSketchFilter(void)
{
}



void PencilSketchFilter::gradientImage(Mat grayImg,Mat &gradImg)
{
	gradImg = Mat::zeros(grayImg.size(),CV_8UC1);
	int w = grayImg.cols;
	for(int r = 1; r < grayImg.rows-1;r++)
	{
		uchar *ptrSrc = grayImg.ptr<uchar>(r);
		uchar *ptrDst = gradImg.ptr<uchar>(r);
		for(int c = 1;c<grayImg.cols-1;c++)
		{
			int grad = abs(ptrSrc[c+1]-ptrSrc[c-1]) + abs(ptrSrc[c+w]-ptrSrc[c-w]);
			if(grad>255)
				grad=255;
			ptrDst[c] = grad;//>>1;
		}
	}
}


void PencilSketchFilter::processImage(cv::Mat srcImg, cv::Mat &pencilImg)
{

	Mat grayImg = srcImg;
	//cvtColor(srcImg,grayImg,COLOR_BGR2GRAY);

	Mat ivtImg;
	addWeighted(grayImg,-1,NULL,0,255,ivtImg);	
	GaussianBlur(ivtImg,ivtImg,Size(11,11),0); 
	
	Mat edge; 
	gradientImage(ivtImg,edge);
	
	pencilImg = grayImg.clone();
	for (int y=0; y<grayImg.rows; y++)  
	{  
		uchar* pSrc  = grayImg.ptr<uchar>(y);  
		uchar* pInv  = ivtImg.ptr<uchar>(y);  
		uchar *pEdge = edge.ptr<uchar>(y);
		uchar* pDst  = pencilImg.ptr<uchar>(y);  
		
		for (int x=0; x<grayImg.cols; x++)  
		{  
			int tmp0=pSrc[x];  
			int tmp1=pInv[x];  
			int edgeVal = pEdge[x];			
			int resultVal = tmp0+(tmp0*tmp1)/(256-tmp1) - edgeVal;
			if(resultVal>255)
				resultVal = 255;
			if(resultVal <0)
				resultVal=0;
			pDst[x] =resultVal;  
		}  
	}  
}
