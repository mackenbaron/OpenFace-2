#include "pencilSketchDll.h"
#include "PencilSketchFilter.h"


void processRGBATexture(Color_32 *pTexImg, int width, int height, Color_32 *pFilterTexture)
{
	

	using namespace cv;
	
	Mat img(Size(width, height), CV_8UC4, pTexImg);

	Mat grayImg;
	cvtColor(img, grayImg, COLOR_RGBA2GRAY);

	Mat filterImg;

	PencilSketchFilter sketchFilter;
	sketchFilter.processImage(grayImg, filterImg);

	Mat filterdTex(img.size(), CV_8UC4, pFilterTexture);
	img.copyTo(filterdTex);

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

	Mat grayImg(Size(width, height), CV_8UC1, pGrayImg);

	
	Mat filterImg;

	PencilSketchFilter sketchFilter;
	sketchFilter.processImage(grayImg, filterImg);

	Mat filterImgOut(grayImg.size(), CV_8UC1, pFilterImg);
	filterImg.copyTo(filterImgOut);


}
