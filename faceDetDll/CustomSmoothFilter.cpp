
#include "CustomSmoothFilter.h"
using namespace cv;
#define CV_SSE3 1

class BilateralFilter_8u_InvokerX :
	public ParallelLoopBody
{
public:
	BilateralFilter_8u_InvokerX(Mat& _dest, const Mat& _temp, int _radius, int _maxk,
		int* _space_ofs, float *_space_weight, float *_color_weight,int *pAbsTab) :
		temp(&_temp), dest(&_dest), radius(_radius),
		maxk(_maxk), space_ofs(_space_ofs), space_weight(_space_weight), color_weight(_color_weight),mpAbsTab(pAbsTab)
	{

	}

	virtual void operator() (const Range& range) const
	{
		int i, j, cn = dest->channels(), k;
		Size size = dest->size();
#if CV_SSE3
		int CV_DECL_ALIGNED(16) buf[4];
		float CV_DECL_ALIGNED(16) bufSum[4];
		static const int CV_DECL_ALIGNED(16) bufSignMask[] = { 0x80000000, 0x80000000, 0x80000000, 0x80000000 };
		bool haveSSE3 = checkHardwareSupport(CV_CPU_SSE3);
#endif

		const int *pAbs = mpAbsTab + 255;
		for (i = range.start; i < range.end; i++)
		{
			const uchar* sptr = temp->ptr(i + radius) + radius*cn;
			uchar* dptr = dest->ptr(i);

			if (cn == 1)
			{
				for (j = 0; j < size.width; j++)
				{
					float sum = 0, wsum = 0;
					int val0 = sptr[j];
					k = 0;
#if CV_SSE3
					if (haveSSE3)
					{
						__m128 _val0 = _mm_set1_ps(static_cast<float>(val0));
						const __m128 _signMask = _mm_load_ps((const float*)bufSignMask);

						for (; k <= maxk - 4; k += 4)
						{
							__m128 _valF = _mm_set_ps(sptr[j + space_ofs[k + 3]], sptr[j + space_ofs[k + 2]],
								sptr[j + space_ofs[k + 1]], sptr[j + space_ofs[k]]);

							__m128 _val = _mm_andnot_ps(_signMask, _mm_sub_ps(_valF, _val0));
							_mm_store_si128((__m128i*)buf, _mm_cvtps_epi32(_val));

							__m128 _cw = _mm_set_ps(color_weight[buf[3]], color_weight[buf[2]],
								color_weight[buf[1]], color_weight[buf[0]]);
							__m128 _sw = _mm_loadu_ps(space_weight + k);
							__m128 _w = _mm_mul_ps(_cw, _sw);
							_cw = _mm_mul_ps(_w, _valF);

							_sw = _mm_hadd_ps(_w, _cw);
							_sw = _mm_hadd_ps(_sw, _sw);
							_mm_storel_pi((__m64*)bufSum, _sw);

							sum += bufSum[1];
							wsum += bufSum[0];
						}
					}
#endif
					for (; k < maxk; k++)
					{
						int val = sptr[j + space_ofs[k]];
						float w = space_weight[k] * color_weight[std::abs(val - val0)];
						sum += val*w;
						wsum += w;
					}
					// overflow is not possible here => there is no need to use CV_CAST_8U
					dptr[j] = (uchar)cvRound(sum / wsum);
				}
			}
			else
			{
				assert(cn == 3);
				for (j = 0; j < size.width * 3; j += 3)
				{
					float sum_b = 0, sum_g = 0, sum_r = 0, wsum = 0;
					int b0 = sptr[j], g0 = sptr[j + 1], r0 = sptr[j + 2];
					k = 0;
#if CV_SSE3
					if (haveSSE3)
					{
						const __m128i izero = _mm_setzero_si128();
						const __m128 _b0 = _mm_set1_ps(static_cast<float>(b0));
						const __m128 _g0 = _mm_set1_ps(static_cast<float>(g0));
						const __m128 _r0 = _mm_set1_ps(static_cast<float>(r0));
						const __m128 _signMask = _mm_load_ps((const float*)bufSignMask);

						const float zero = 0;
						__m128 sumb = _mm_set1_ps(static_cast<float>(zero));
						__m128 sumg = _mm_set1_ps(static_cast<float>(zero));
						__m128 sumr = _mm_set1_ps(static_cast<float>(zero));
						__m128 sumw = _mm_set1_ps(static_cast<float>(zero));
						for (; k <= maxk - 4; k += 4)
						{
							const int* const sptr_k0 = reinterpret_cast<const int*>(sptr + j + space_ofs[k]);
							const int* const sptr_k1 = reinterpret_cast<const int*>(sptr + j + space_ofs[k + 1]);
							const int* const sptr_k2 = reinterpret_cast<const int*>(sptr + j + space_ofs[k + 2]);
							const int* const sptr_k3 = reinterpret_cast<const int*>(sptr + j + space_ofs[k + 3]);

							__m128 _b = _mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_unpacklo_epi8(_mm_cvtsi32_si128(sptr_k0[0]), izero), izero));
							__m128 _g = _mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_unpacklo_epi8(_mm_cvtsi32_si128(sptr_k1[0]), izero), izero));
							__m128 _r = _mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_unpacklo_epi8(_mm_cvtsi32_si128(sptr_k2[0]), izero), izero));
							__m128 _z = _mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_unpacklo_epi8(_mm_cvtsi32_si128(sptr_k3[0]), izero), izero));

							_MM_TRANSPOSE4_PS(_b, _g, _r, _z);

							__m128 bt = _mm_andnot_ps(_signMask, _mm_sub_ps(_b, _b0));
							__m128 gt = _mm_andnot_ps(_signMask, _mm_sub_ps(_g, _g0));
							__m128 rt = _mm_andnot_ps(_signMask, _mm_sub_ps(_r, _r0));

							bt = _mm_add_ps(rt, _mm_add_ps(bt, gt));
							_mm_store_si128((__m128i*)buf, _mm_cvtps_epi32(bt));

							__m128 _w = _mm_set_ps(color_weight[buf[3]], color_weight[buf[2]],
								color_weight[buf[1]], color_weight[buf[0]]);
							__m128 _sw = _mm_loadu_ps(space_weight + k);

							_w = _mm_mul_ps(_w, _sw);
							_b = _mm_mul_ps(_b, _w);
							_g = _mm_mul_ps(_g, _w);
							_r = _mm_mul_ps(_r, _w);						

							sumb = _mm_add_ps(sumb, _b);
							sumg = _mm_add_ps(sumg, _g);
							sumr = _mm_add_ps(sumr, _r);
							sumw = _mm_add_ps(sumw, _w);
						}
						sumw = _mm_hadd_ps(sumw, sumb);
						sumg = _mm_hadd_ps(sumg, sumr);

						sumw = _mm_hadd_ps(sumw, sumg);
						_mm_store_ps(bufSum, sumw);

						wsum += bufSum[0];
						sum_b += bufSum[1];
						sum_g += bufSum[2];
						sum_r += bufSum[3];
					}
#endif
					for (; k < maxk; k++)
					{
						const uchar* sptr_k = sptr + j + space_ofs[k];
						int b = sptr_k[0], g = sptr_k[1], r = sptr_k[2];						
						float w = space_weight[k] * color_weight[pAbs[b - b0] +
							pAbs[g - g0] + pAbs[r - r0]];

						sum_b += b*w; sum_g += g*w; sum_r += r*w;
						wsum += w;
					}
					wsum = 1.f / wsum;
					dptr[j] = (sum_b*wsum);
					dptr[j + 1] = (sum_g*wsum);
					dptr[j + 2] = (sum_r*wsum);
					
				}
			}
		}
	}

private:
	const Mat *temp;
	Mat *dest;
	int radius, maxk, *space_ofs;
	float *space_weight, *color_weight;
	int *mpAbsTab;

};
CustomSmoothFilter::CustomSmoothFilter()
{
	mbIsInited = false;
}


CustomSmoothFilter::~CustomSmoothFilter()
{
}


void CustomSmoothFilter::initFilter(int imgWidth, int imgHeight, int cn)
{
	const int windowSize = 9;
	const float sigmaSpace = 37;
	const float sigmaClr = 37;

	
	mvClrWeight.resize(cn * 256);
	mvSpaceOffset.resize(windowSize*windowSize);
	mvSpaceWeight.resize(windowSize*windowSize);

	float* color_weight = &mvClrWeight[0];
	float* space_weight = &mvSpaceWeight[0];
	int* space_ofs = &mvSpaceOffset[0];

	double gauss_color_coeff = -0.5 / (sigmaClr*sigmaClr);
	double gauss_space_coeff = -0.5 / (sigmaSpace*sigmaSpace);
	// initialize color-related bilateral filter coefficients
	int i,j,maxk;
	for (i = 0; i < 256 * cn; i++)
		color_weight[i] = (float)std::exp(i*i*gauss_color_coeff);

	int radius = windowSize / 2;
	// initialize space-related bilateral filter coefficients
	
	cv::Mat src = Mat::zeros(Size(imgWidth,imgHeight),CV_8UC(cn));
	
	cv::Mat temp;
	copyMakeBorder(src, temp, radius, radius, radius, radius, 4);
	
	int copyBorderStep = imgWidth + 2 * radius;

	for (i = -radius, maxk = 0; i <= radius; i++)
	{
		j = -radius;

		for (; j <= radius; j++)
		{
			double r = std::sqrt((double)i*i + (double)j*j);
			if (r > radius)
				continue;
			space_weight[maxk] = (float)std::exp(r*r*gauss_space_coeff);
			space_ofs[maxk++] = (int)(i*temp.step + j*cn);
		}
	}

	for (int i = -255; i <= 255; i++)
	{
		mAbsTabList[i + 255] = std::abs(i);
	}
	miRadius = radius;
	mTempFilterResult = Mat::zeros(Size(imgWidth, imgHeight), CV_8UC(cn));
	miNumOfNeigs = maxk;

	mbIsInited = true;
}


void CustomSmoothFilter::processImage(cv::Mat &img)
{
	//normalize(img, img, 0, 255, CV_MINMAX);
	//balanceClr(img, img);
	if (!mbIsInited)
	{
		initFilter(img.cols, img.rows, img.channels());
	}

	Mat temp;
	copyMakeBorder(img, temp, miRadius, miRadius, miRadius, miRadius, 4);

	float* color_weight = &mvClrWeight[0];
	float* space_weight = &mvSpaceWeight[0];
	int* space_ofs = &mvSpaceOffset[0];

	BilateralFilter_8u_InvokerX body(mTempFilterResult, temp, miRadius, miNumOfNeigs, space_ofs, space_weight, color_weight, mAbsTabList);
	parallel_for_(Range(0, mTempFilterResult.rows), body, mTempFilterResult.total() / (double)(1 << 16));

	Mat temp2 = mTempFilterResult - img + 128;
	Mat temp3;
	GaussianBlur(temp2, temp3, Size(3,3), 0, 0);
	
	Mat dist = img + temp3 - 128;
	dist.copyTo(img);
	
}



void CustomSmoothFilter::balanceClr(cv::Mat img, cv::Mat &dst)
{
	Mat hsvM;
	cvtColor(img, hsvM, CV_BGR2HSV);

	std::vector<Mat> hsv;
	split(hsvM, hsv);

	//int histSize = 256;
	//float range[2] = { 0,256 };
	//const float *histRange = { range };
	//Mat hist;
	//calcHist(&hsv[2], 1, 0, Mat(), hist, 1, &histSize, &histRange, true, false);

	//
	//int total = img.cols*img.rows;
	//float minCount = 0.5*total / 100.0f;
	//float maxCount = (100 - 0.5)*total / 100.0f;
	//int n1 = 0, n2 = total;
	//int p1 = 0, p2 = histSize - 1;
	//int minValue = 0, maxValue = 255;

	//float *pHistData = (float*)hist.data;
	//while (n1 + pHistData[p1] <100)// minCount)
	//{
	//	n1 += pHistData[p1++];
	//	minValue++;
	//}

	//while (n2 - pHistData[p2] > (total-100))//maxCount)
	//{
	//	n2 -= pHistData[p2--];
	//	maxValue--;
	//}

	//hsv[2] = 255 * (hsv[2] - minValue)/ (maxValue - minValue);

	//GaussianBlur(hsv[2], hsv[2], Size(3, 3), 0, 0);

	normalize(hsv[2], hsv[2], 0, 255, CV_MINMAX);
	cv::merge(hsv, hsvM);
	cvtColor(hsvM, dst, CV_HSV2BGR);
}





