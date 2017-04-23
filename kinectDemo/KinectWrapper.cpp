#include "KinectWrapper.h"


template<class Interface>
inline void safeRelease(Interface *& pInterfaceToRelease)
{
	if (pInterfaceToRelease != NULL)
	{
		pInterfaceToRelease->Release();
		pInterfaceToRelease = NULL;
	}
}

KinectWrapper::KinectWrapper() :	
	m_pKinectSensor(NULL),
	m_pCoordinateMapper(NULL),
	m_pMultiSourceFrameReader(NULL),
	m_pDepthCoordinates(NULL),	
	m_pOutputRGBX(NULL),
	m_pBackgroundRGBX(NULL),
	m_pColorRGBX(NULL)
{
	initMemBuffer();
	mIsInit = 0;
}


KinectWrapper::~KinectWrapper()
{
}


int KinectWrapper::InitializeDefaultSensor()
{
	HRESULT hr;

	hr = GetDefaultKinectSensor(&m_pKinectSensor);
	if (FAILED(hr))
	{
		return 0;
	}

	if (m_pKinectSensor)
	{
		// Initialize the Kinect and get coordinate mapper and the frame reader

		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->get_CoordinateMapper(&m_pCoordinateMapper);
		}

		hr = m_pKinectSensor->Open();

		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->OpenMultiSourceFrameReader(
				FrameSourceTypes::FrameSourceTypes_Depth | FrameSourceTypes::FrameSourceTypes_Color | FrameSourceTypes::FrameSourceTypes_BodyIndex,
				&m_pMultiSourceFrameReader);
		}
	}

	if (!m_pKinectSensor || FAILED(hr))
	{
		return 0;
	}

	
	return !FAILED(hr);
}

void KinectWrapper::Update()
{

	if (!m_pMultiSourceFrameReader)
	{
		return;
	}
	
	IMultiSourceFrame* pMultiSourceFrame = NULL;
	IDepthFrame* pDepthFrame = NULL;
	IColorFrame* pColorFrame = NULL;
	IBodyIndexFrame* pBodyIndexFrame = NULL;

	HRESULT hr = m_pMultiSourceFrameReader->AcquireLatestFrame(&pMultiSourceFrame);

	if (SUCCEEDED(hr))
	{
		IDepthFrameReference* pDepthFrameReference = NULL;

		hr = pMultiSourceFrame->get_DepthFrameReference(&pDepthFrameReference);
		if (SUCCEEDED(hr))
		{
			hr = pDepthFrameReference->AcquireFrame(&pDepthFrame);
			//printf("get depth frame\n");
		}

		safeRelease(pDepthFrameReference);
	}

	if (SUCCEEDED(hr))
	{
		IColorFrameReference* pColorFrameReference = NULL;

		hr = pMultiSourceFrame->get_ColorFrameReference(&pColorFrameReference);
		if (SUCCEEDED(hr))
		{
			hr = pColorFrameReference->AcquireFrame(&pColorFrame);
			//printf("get color frame\n");
		}

		safeRelease(pColorFrameReference);
	}

	if (SUCCEEDED(hr))
	{
		IBodyIndexFrameReference* pBodyIndexFrameReference = NULL;

		hr = pMultiSourceFrame->get_BodyIndexFrameReference(&pBodyIndexFrameReference);
		if (SUCCEEDED(hr))
		{
			hr = pBodyIndexFrameReference->AcquireFrame(&pBodyIndexFrame);
			//printf("get body frame\n");
		}

		safeRelease(pBodyIndexFrameReference);
	}

	if (SUCCEEDED(hr))
	{
		INT64 nDepthTime = 0;
		IFrameDescription* pDepthFrameDescription = NULL;
		int nDepthWidth = 0;
		int nDepthHeight = 0;
		UINT nDepthBufferSize = 0;
		UINT16 *pDepthBuffer = NULL;

		IFrameDescription* pColorFrameDescription = NULL;
		int nColorWidth = 0;
		int nColorHeight = 0;
		ColorImageFormat imageFormat = ColorImageFormat_None;
		UINT nColorBufferSize = 0;
		RGBQUAD *pColorBuffer = NULL;

		IFrameDescription* pBodyIndexFrameDescription = NULL;
		int nBodyIndexWidth = 0;
		int nBodyIndexHeight = 0;
		UINT nBodyIndexBufferSize = 0;
		BYTE *pBodyIndexBuffer = NULL;

		// get depth frame data

		hr = pDepthFrame->get_RelativeTime(&nDepthTime);

		if (SUCCEEDED(hr))
		{
			hr = pDepthFrame->get_FrameDescription(&pDepthFrameDescription);
		}

		if (SUCCEEDED(hr))
		{
			hr = pDepthFrameDescription->get_Width(&nDepthWidth);
		}

		if (SUCCEEDED(hr))
		{
			hr = pDepthFrameDescription->get_Height(&nDepthHeight);
		}

		if (SUCCEEDED(hr))
		{
			hr = pDepthFrame->AccessUnderlyingBuffer(&nDepthBufferSize, &pDepthBuffer);
			//printf("get depth frame\n");
		}

		// get color frame data

		if (SUCCEEDED(hr))
		{
			hr = pColorFrame->get_FrameDescription(&pColorFrameDescription);
		}

		if (SUCCEEDED(hr))
		{
			hr = pColorFrameDescription->get_Width(&nColorWidth);
		}

		if (SUCCEEDED(hr))
		{
			hr = pColorFrameDescription->get_Height(&nColorHeight);
		}

		if (SUCCEEDED(hr))
		{
			hr = pColorFrame->get_RawColorImageFormat(&imageFormat);
		}

		if (SUCCEEDED(hr))
		{
			if (imageFormat == ColorImageFormat_Bgra)
			{
				hr = pColorFrame->AccessRawUnderlyingBuffer(&nColorBufferSize, reinterpret_cast<BYTE**>(&pColorBuffer));
			}
			else if (m_pColorRGBX)
			{
				pColorBuffer = m_pColorRGBX;
				nColorBufferSize = cColorWidth * cColorHeight * sizeof(RGBQUAD);
				hr = pColorFrame->CopyConvertedFrameDataToArray(nColorBufferSize, reinterpret_cast<BYTE*>(pColorBuffer), ColorImageFormat_Bgra);
			}
			else
			{
				hr = E_FAIL;
				printf("fail----------------\n");
			}
		}

		// get body index frame data

		if (SUCCEEDED(hr))
		{
			hr = pBodyIndexFrame->get_FrameDescription(&pBodyIndexFrameDescription);
		}

		if (SUCCEEDED(hr))
		{
			hr = pBodyIndexFrameDescription->get_Width(&nBodyIndexWidth);
		}

		if (SUCCEEDED(hr))
		{
			hr = pBodyIndexFrameDescription->get_Height(&nBodyIndexHeight);
		}

		if (SUCCEEDED(hr))
		{
			hr = pBodyIndexFrame->AccessUnderlyingBuffer(&nBodyIndexBufferSize, &pBodyIndexBuffer);
		}

		if (SUCCEEDED(hr))
		{
			//printf("start processing\n");
			processFrame(nDepthTime, pDepthBuffer, nDepthWidth, nDepthHeight,
				pColorBuffer, nColorWidth, nColorHeight,
				pBodyIndexBuffer, nBodyIndexWidth, nBodyIndexHeight);
		}

		safeRelease(pDepthFrameDescription);
		safeRelease(pColorFrameDescription);
		safeRelease(pBodyIndexFrameDescription);
	}

	safeRelease(pDepthFrame);
	safeRelease(pColorFrame);
	safeRelease(pBodyIndexFrame);
	safeRelease(pMultiSourceFrame);
}


void KinectWrapper::processFrame(INT64 nTime,
	const UINT16* pDepthBuffer, int nDepthWidth, int nDepthHeight,
	const RGBQUAD* pColorBuffer, int nColorWidth, int nColorHeight,
	const BYTE* pBodyIndexBuffer, int nBodyIndexWidth, int nBodyIndexHeight)
{
	using namespace cv;
	Mat mask = Mat::zeros(Size(nColorWidth, nColorHeight), CV_8UC1);
	uchar *pMask = mask.data;
	Mat body;
	// Make sure we've received valid data
	printf("ndepth:(%d,%d)\n", nDepthWidth, nDepthHeight);
	bool isOK = m_pCoordinateMapper && m_pDepthCoordinates && m_pOutputRGBX&&
		pDepthBuffer && (nDepthWidth == cDepthWidth) && (nDepthHeight == cDepthHeight) ;
	if (isOK)
	{
		printf("isok\n");
	}

	if (m_pCoordinateMapper && m_pDepthCoordinates && m_pOutputRGBX &&
		pDepthBuffer && (nDepthWidth == cDepthWidth) && (nDepthHeight == cDepthHeight) &&
		pColorBuffer && (nColorWidth == cColorWidth) && (nColorHeight == cColorHeight) &&
		pBodyIndexBuffer && (nBodyIndexWidth == cDepthWidth) && (nBodyIndexHeight == cDepthHeight))
	{
		printf("enter.....\n");
		HRESULT hr = m_pCoordinateMapper->MapColorFrameToDepthSpace(nDepthWidth * nDepthHeight, (UINT16*)pDepthBuffer, nColorWidth * nColorHeight, m_pDepthCoordinates);
		if (SUCCEEDED(hr))
		{
			// loop over output pixels
			for (int colorIndex = 0; colorIndex < (nColorWidth*nColorHeight); ++colorIndex)
			{
				// default setting source to copy from the background pixel
				const RGBQUAD* pSrc = m_pBackgroundRGBX + colorIndex;

				DepthSpacePoint p = m_pDepthCoordinates[colorIndex];
				
				// Values that are negative infinity means it is an invalid color to depth mapping so we
				// skip processing for this pixel
				//if (p.X > -5000 && p.Y > -5000)
				if (p.X != -std::numeric_limits<float>::infinity() && p.Y != -std::numeric_limits<float>::infinity())
				{
					int depthX = static_cast<int>(p.X + 0.5f);
					int depthY = static_cast<int>(p.Y + 0.5f);

					if ((depthX >= 0 && depthX < nDepthWidth) && (depthY >= 0 && depthY < nDepthHeight))
					{
						BYTE player = pBodyIndexBuffer[depthX + (depthY * cDepthWidth)];

						// if we're tracking a player for the current pixel, draw from the color camera
						if (player != 0xff)
						{
							// set source for copy to the color pixel
							pSrc = m_pColorRGBX + colorIndex;
							if(colorIndex/nColorWidth < nColorHeight-12)
							pMask[colorIndex] = 255;
						}
					}
				}

				// write output
				m_pOutputRGBX[colorIndex] = *pSrc;
			}

			// Draw the data with Direct2D
			//m_pDrawCoordinateMapping->Draw(reinterpret_cast<BYTE*>(m_pOutputRGBX), cColorWidth * cColorHeight * sizeof(RGBQUAD));

		}
	}

	Mat img(Size(nColorWidth, nColorHeight), CV_8UC4, m_pColorRGBX);
	resize(mask, mask, Size(640, 480));
	Mat clrImg;
	resize(img, clrImg, Size(640, 480));

	Mat kernl = getStructuringElement(MORPH_RECT, Size(3, 3));

	erode(mask, mask, kernl);
	dilate(mask, mask, kernl);
	
	std::vector<std::vector<Point> > _contoursQuery;

	int numFgPixels = countNonZero(mask);
	if (numFgPixels < 1000)
		return;
	//imshow("mask", mask);
	//waitKey(0);

	Mat maskClr;
	cvtColor(mask, maskClr, COLOR_GRAY2BGR);

	findContours(mask, _contoursQuery, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	printf("contour size %d\n", _contoursQuery.size());
	for (int i = 0; i < _contoursQuery.size(); i++)
	{
		
		if (_contoursQuery[i].size()<20)//||arcLength(Mat(_contoursQuery[i]),true) < 500)
			continue;
		std::vector<Point> tempCS;

		int arcLen = 2;//arcLength(Mat(_contoursQuery[i]), true)*0.002
		//approxPolyDP(Mat(_contoursQuery[i]), tempCS,arcLen , true);
		
		drawContours(clrImg, _contoursQuery, i, Scalar(0, 0, 255),2);
		extractIPImage(clrImg, _contoursQuery[i], body);
		break;
	}
	
	mBodyPng = body;
	if (mBodyPng.data)
	{
		resize(mBodyPng, mBodyPng, Size(200, 360));
		imshow("body", mBodyPng);
	}
	
	//waitKey(1);

}

void extendContour(std::vector<cv::Point> &srcContours, std::vector<cv::Point> &dstContours,
					int imgWidth,int imgHeight, int numExtend)
{
	using namespace cv;
	dstContours.clear();
	Point2f centerPt(0, 0);
	for (int i = 0; i < srcContours.size(); i++)
	{
		centerPt.x += srcContours[i].x;
		centerPt.y += srcContours[i].y;
	}
	centerPt.x /= srcContours.size();
	centerPt.y /= srcContours.size();
	for (int i = 0; i < srcContours.size(); i++)
	{
		Point2f dir;
		dir.x = srcContours[i].x - centerPt.x;
		dir.y = srcContours[i].y - centerPt.x;
		float len = norm(dir);
		Point newPt;
		newPt.x = centerPt.x + dir.x*(len + numExtend) / len;
		newPt.y = centerPt.y + dir.y*(len + numExtend) / len;

		if (newPt.x < 0)
			newPt.x = 0;
		else if (newPt.x >= imgWidth)
			newPt.x = imgWidth - 1;

		if (newPt.y < 0)
			newPt.y = 0;
		else if (newPt.y >= imgHeight)
			newPt.y = imgHeight - 1;


		dstContours.push_back(newPt);

	}
}

void KinectWrapper::extractIPImage(cv::Mat img, std::vector<cv::Point> &srcContours, cv::Mat &png)
{
	std::vector<cv::Point> newCs;
	extendContour(srcContours, newCs, img.cols,img.rows,2);
	using namespace cv;
	//get bounding box
	float leftx = 10000, topy = 10000, rightx = 0, bottomy = 0;
	for (int i = 0; i < newCs.size(); i++)
	{
		cv::Point pt = newCs[i];
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

	cv::Rect rct(leftx, topy, rightx - leftx+1 , bottomy - topy+1);
	if (rct.width + rct.x > img.cols)
		rct.width = img.cols - rct.x;
	if (rct.height + rct.y > img.rows)
		rct.height = img.rows - rct.y;

	Mat subImg = img(rct).clone();

	std::vector<cv::Point> subContours;
	for (int i = 0; i < newCs.size(); i++)
	{
		cv::Point pt = srcContours[i];
		pt.x -= rct.x;
		pt.y -= rct.y;
		subContours.push_back(pt);
	}
	Mat mask = Mat::zeros(Size(rct.width, rct.height), CV_8UC1);
	std::vector<std::vector<cv::Point>> subCCs;
	subCCs.push_back(subContours);
	drawContours(mask, subCCs, 0, Scalar(25, 255, 255), -1);

	png = Mat::zeros(subImg.size(), CV_8UC4);
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
			ptrPngRow[4 * c + 0] = ptrImgRow[srcChannels * c + 0];
			ptrPngRow[4 * c + 1] = ptrImgRow[srcChannels * c + 1];
			ptrPngRow[4 * c + 2] = ptrImgRow[srcChannels * c + 2];
			ptrPngRow[4 * c + 3] = ptrAlphaRow[c] == 0 ? 0 : 255;
		}
	}
	

}

void KinectWrapper::initMemBuffer()
{
	m_pBodyMask = new BYTE[cColorWidth*cColorHeight];
	m_pOutputRGBX = new RGBQUAD[cColorWidth * cColorHeight];

	// create heap storage for background image pixel data in RGBX format
	m_pBackgroundRGBX = new RGBQUAD[cColorWidth * cColorHeight];

	// create heap storage for color pixel data in RGBX format
	m_pColorRGBX = new RGBQUAD[cColorWidth * cColorHeight];

	// create heap storage for the coorinate mapping from color to depth
	m_pDepthCoordinates = new DepthSpacePoint[cColorWidth * cColorHeight];
}