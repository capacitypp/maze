#ifndef ___ClassOpenCVImage
#define ___ClassOpenCVImage

#include "opencv2\opencv.hpp"
#include "opencv2\highgui\highgui.hpp"

#include "Exception.h"
#include "Image.h"

#pragma comment(lib, "opencv_core2411.lib")
#pragma comment(lib, "opencv_highgui2411.lib")
#pragma comment(lib, "opencv_imgproc2411.lib")

template <typename Type = RGBPixel<LDR>>
class OpenCVImage {
	IplImage* image;
public:
	OpenCVImage() : image(NULL) { }
	OpenCVImage(const OpenCVImage<Type>& src);
	OpenCVImage(const Image<Type>& src);
	OpenCVImage(const IplImage* image);
	~OpenCVImage();
	const IplImage* getImage() const { return image; }
	IplImage* getImage() { return image; }
	OpenCVImage<Type>& operator=(const OpenCVImage<Type>& src);
	/* interpolation : 補間方法
	INTER_NN	@最近隣接補間
	INTER_LINEAR@バイリニア補間
	INTER_AREA	@ピクセル領域の関係を用いてリサンプリングする
	INTER_CUBIC	@バイキュービック補間
	*/
	void resize(int width, int height, int interpolation);
};

template <typename Type>
OpenCVImage<Type>::OpenCVImage(const OpenCVImage<Type>& src)
{
	image = cvCloneImage(src.image);
	if (image == NULL)
		throw BadAllocationException();
}

template <typename Type>
OpenCVImage<Type>::OpenCVImage(const Image<Type>& src)
{
	unsigned char* ptr;
	IplImage* image = cvCreateImage(cvSize(src.getWidth(), src.getHeight()), IPL_DEPTH_8U, Type().getElementCount());
	int step = (4 - src.getWidth() * 3 % 4) % 4;
	ptr = reinterpret_cast<unsigned char*>(image->imageData);
	for (int i = 0; i < src.getHeight(); i++) {
		const LineImage<Type>& line = src[i];
		for (int j = 0; j < src.getWidth(); j++) {
			const Type& pixel = line[j];
			int elementCount = pixel.getElementCount();
			for (int k = 0; k < elementCount; k++)
				*ptr++ = pixel[elementCount - 1 - k];
		}
		for (int j = 0; j < step; j++)
			ptr++;
	}
	this->image = image;
}

template <typename Type>
OpenCVImage<Type>::OpenCVImage(const IplImage* image)
{
	this->image = cvCloneImage(image);
}

template <typename Type>
OpenCVImage<Type>::~OpenCVImage()
{
	cvReleaseImage(&image);
}

template <typename Type>
OpenCVImage<Type>& OpenCVImage<Type>::operator=(const OpenCVImage<Type>& src)
{
	image = cvCloneImage(src.image);
	return *this;
}

template <typename Type>
void OpenCVImage<Type>::resize(int width, int height, int interpolation)
{
	IplImage* image = cvCreateImage(cvSize(width, height), this->image->depth, this->image->nChannels);
	cvResize(this->image, image);
	cvReleaseImage(&this->image);
	this->image = image;
}

#endif
