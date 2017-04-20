#ifndef FILE_PENCIL_SKETCH_DLL_H
#define FILE_PENCIL_SKETCH_DLL_H



struct Color_32
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
};

extern "C" __declspec(dllexport)
void processRGBATexture(Color_32 *pTexImg, int width, int height,Color_32 *pFilterTexture);

extern "C" __declspec(dllexport)
void processGrayImg(unsigned char *pGrayImg, int width, int height, unsigned char *pFilterImg );

extern "C" __declspec(dllexport)
void initKinectSensor(void **pSensor);

#endif


