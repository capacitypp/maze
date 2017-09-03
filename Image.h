#ifndef ___ClassImage
#define ___ClassImage

#include "jpeg\\include\\jpeg.h"

#include "Exception.h"
#include "LineImage.h"
#include "GrayPixel.h"
#include "opencv2\opencv.hpp"

#pragma warning(disable:4996)

template <typename Type = RGBPixel<LDR> >
class Image {
	int height;
	int width;
	LineImage<Type> *data;
	void readBMPImage(const char* path);
	void readJPEGImage(const char* path);
	void writeBMPImage(const char* path);
	void writeJPEGImage(const char* path);
	unsigned char* memoryPool;
public:
	static const int INTER_NN;
	static const int INTER_LINEAR;
	static const int INTER_AREA;
	static const int INTER_CUBIC;
	Image(int width = 100, int height = 100);
	Image(const char* path);
	Image(const IplImage* image);
	~Image();
	static int getElementCount() { return LineImage<Type>::getElementCount(); }
	inline int getWidth() const { return width; }
	inline int getHeight() const { return height; }
	inline LineImage<Type>& operator[](int idx) {
		if (idx < 0 || idx >= height)
			throw InvalidIndexException(idx);
		return data[idx];
	}
	inline const LineImage<Type>& operator[](int idx) const {
		if (idx < 0 || idx >= height)
			throw InvalidIndexException(idx);
		return data[idx];
	}
	void resize(int width, int height);
	/* interpolation : 補間方法
		INTER_NN	@最近隣接補間
		INTER_LINEAR@バイリニア補間
		INTER_AREA	@ピクセル領域の関係を用いてリサンプリングする
		INTER_CUBIC	@バイキュービック補間
	*/
	void resize(int width, int height, int interpolation);
	/* 倍率を指定してリサイズする
		power	@倍率:1.0で等倍
	*/
	void resize(double power, int interpolation);
	void readImage(const char* path);
	void readImage(const IplImage* image);
	void writeImage(const char* path);
	Image<GrayPixel<LDR>>& getGrayImage() const;
};

template <typename Type>
Image<Type>::Image(int width, int height) : data(NULL), memoryPool(NULL)
{
	resize(width, height);
}

template <typename Type>
Image<Type>::Image(const char* path) : data(NULL), memoryPool(NULL)
{
	readImage(path);
}

template <typename Type>
Image<Type>::Image(const IplImage* image) : data(NULL), memoryPool(NULL)
{
	readImage(image);
}

template <typename Type>
Image<Type>::~Image()
{
	delete[] data;
	if (memoryPool != NULL)
		delete[] memoryPool;
}

template <typename Type>
void Image<Type>::resize(int width, int height)
{
	if (data != NULL)
		delete[] data;
	LineImage<Type>::setDefaultWidth(width);
	int size = width * height * getElementCount();
	if (memoryPool != NULL)
		delete[] memoryPool;
	memoryPool = new unsigned char[size];
	LineImage<Type>::setMemoryPool(memoryPool, size);
	data = new LineImage<Type>[height];
	if (data == NULL)
		throw BadAllocationException();
	this->width = width;
	this->height = height;
}

template <typename Type>
void Image<Type>::resize(int width, int height, int interpolation)
{
	OpenCVImage<Type> cvImage(*this);
	cvImage.resize(width, height, interpolation);
	readImage(cvImage.getImage());
}

template <typename Type>
void Image<Type>::resize(double power, int interpolation)
{
	resize((int)(width * power), (int)(height * power), interpolation);
}

template <typename Type>
void Image<Type>::readImage(const char* path)
{
	char tmp[256];
	char *dumy;
	string pathString(path);
	strcpy_s(tmp, pathString.c_str());
	strtok_s(tmp, ".", &dumy);
	char* ptr = strtok_s(NULL, ".", &dumy);
	string extension(ptr);
	if (extension == "bmp")
		readBMPImage(path);
	else if (extension == "jpeg" || extension == "jpg")
		readJPEGImage(path);
	else
		throw UnsupportedExtensionException();
}

template <typename Type>
void Image<Type>::readImage(const IplImage* image)
{
	int width = image->width;
	int height = image->height;

	resize(width, height);

	int step = (4 - width * 3 % 4) % 4;
	const unsigned char* ptr = reinterpret_cast<const unsigned char*>(image->imageData);

	int elementCount = getElementCount();
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++)
			for (int k = 0; k < elementCount; k++)
				(*this)[i][j][elementCount - k - 1] = *ptr++;
		ptr += step;
	}
}

template <typename Type>
void Image<Type>::readBMPImage(const char* path)
{
	const int BFH_SIZE = 14;
	const int BIH_SIZE = 40;
	FILE *fp;
	char c[256];
	unsigned long ul;
	unsigned short us;
	unsigned long off_set;
	int width, height;
	int step;
	int i, j, k;
	fp = fopen(path, "rb");
	if (fp == NULL)
		throw FileNotExistException();
	//ファイルヘッダ
	fread(c, sizeof(c[0]), 2, fp);	//ファイルタイプ
	if (!((c[0] == 'B') && (c[1] == 'M'))) {
		fclose(fp);
		throw InvalidFileFormatException();
	}
	fread(&ul, sizeof(ul), 1, fp);	//ファイルサイズ
	fread(&us, sizeof(us), 1, fp);	//予約1
	fread(&us, sizeof(us), 1, fp);	//予約2
	fread(&off_set, sizeof(off_set), 1, fp);	//オフセット
	//情報ヘッダ
	fread(&ul, sizeof(ul), 1, fp);	//ヘッダサイズ
	if (ul != BIH_SIZE) {
		fclose(fp);
		throw InvalidFileFormatException();
	}
	fread(&width, sizeof(width), 1, fp);	//幅
	fread(&height, sizeof(height), 1, fp);	//高さ
	fread(&us, sizeof(us), 1, fp);	//プレーン数
	fread(&us, sizeof(us), 1, fp);	//ピクセル毎ビット数
	if (us != 24) {
		fclose(fp);
		throw InvalidFileFormatException();
	}
	fread(&ul, sizeof(ul), 1, fp);	//圧縮タイプ
	if (ul != 0) {
		fclose(fp);
		throw InvalidFileFormatException();
	}
	fread(&ul, sizeof(ul), 1, fp);	//イメージデータサイズ
	fread(&ul, sizeof(ul), 1, fp);	//水平解像度
	fread(&ul, sizeof(ul), 1, fp);	//垂直解像度
	fread(&ul, sizeof(ul), 1, fp);	//カラーインデックス数
	fread(&ul, sizeof(ul), 1, fp);	//重要インデックス数
	//イメージデータ
	if (off_set != 0)
		fseek(fp, off_set, SEEK_SET);
	step = (width * 24 + 31) / 32 * 4 - width * 3;	//ゴミ
	resize(width, height);
	for (i = height - 1; i >= 0; i--) {
		unsigned char color[3];
		LineImage<Type>& line = (*this)[i];
		for (j = 0; j < width; j++){
			Type& pixel = line[j];
			int elementCount = pixel.getElementCount();
			fread(color, sizeof(color[0]), 3, fp);
			for (k = 0; k < 3; k++)
				pixel[elementCount - 1 - k] = color[k];
		}
		for (j = 0; j < step; j++)
			fread(color, sizeof(color[0]), 1, fp);
	}

	fclose(fp);
}

template <typename Type>
void Image<Type>::readJPEGImage(const char* path)
{
	int width, height;
	unsigned char* data = (unsigned char*)load_jpeg(path, &width, &height);
	
	unsigned char* ptr = data;
	resize(width, height);
	int step = (4 - (width % 4)) % 4;
	for (int i = height - 1; i >= 0; i--) {
		for (int j = 0; j < width; j++)
			for (int k = 0; k < 3; k++)
				(*this)[i][j][2 - k] = *ptr++;
		ptr += step;
	}
	delete_jpeg(data);
}

template <typename Type>
void Image<Type>::writeImage(const char* path)
{
	char tmp[256];
	char *dumy;
	string pathString(path);
	strcpy_s(tmp, pathString.c_str());
	strtok_s(tmp, ".", &dumy);
	char* ptr = strtok_s(NULL, ".", &dumy);
	string extension(ptr);
	if (extension == "bmp")
		writeBMPImage(path);
	else if (extension == "jpeg" || extension == "jpg")
		writeJPEGImage(path);
	else
		throw UnsupportedExtensionException();
}

template <typename Type>
void Image<Type>::writeJPEGImage(const char* path)
{
	unsigned char***data = new unsigned char**[height];
	if (data == NULL)
		throw BadAllocationException();
	int elementCount = getElementCount();
	for (int i = 0; i < height; i++) {
		data[i] = new unsigned char*[width];
		if (data[i] == NULL)
			throw BadAllocationException();
		for (int j = 0; j < width; j++) {
			data[i][j] = new unsigned char[elementCount];
			if (data[i][j] == NULL)
				throw BadAllocationException();
			for (int k = 0; k < elementCount; k++)
				data[i][j][k] = (*this)[i][j][k];
		}
	}
	write_jpeg(path, data, width, height);
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++)
			delete[] data[i][j];
		delete[] data[i];
	}
	delete[] data;
}

template <typename Type>
void Image<Type>::writeBMPImage(const char* path)
{
	const int BFH_SIZE = 14;
	const int BIH_SIZE = 40;
	FILE *fp;
	char c[256];
	int i;
	signed int si;
	signed long sl;
	unsigned long ul;
	unsigned short us;
	if ((fp = fopen(path, "wb")) == NULL)
		throw FileNotExistException();
	//ファイルヘッダ
	strcpy(c, "BM");
	fwrite(c, sizeof(c[0]), strlen(c), fp);
	ul = 0;//ファイルサイズ
	fwrite(&ul, sizeof(ul), 1, fp);
	us = 0;
	fwrite(&us, sizeof(us), 1, fp);
	fwrite(&us, sizeof(us), 1, fp);
	ul = 0;//オフセット
	fwrite(&ul, sizeof(ul), 1, fp);
	//情報ヘッダ
	ul = BIH_SIZE;
	fwrite(&ul, sizeof(ul), 1, fp);
	sl = width;
	fwrite(&sl, sizeof(sl), 1, fp);
	sl = height;
	fwrite(&sl, sizeof(sl), 1, fp);
	us = 1;
	fwrite(&us, sizeof(us), 1, fp);
	us = 24;
	fwrite(&us, sizeof(us), 1, fp);
	ul = 0;
	fwrite(&ul, sizeof(ul), 1, fp);
	ul = 0;
	fwrite(&ul, sizeof(ul), 1, fp);
	fwrite(&ul, sizeof(ul), 1, fp);
	fwrite(&ul, sizeof(ul), 1, fp);
	fwrite(&ul, sizeof(ul), 1, fp);
	fwrite(&ul, sizeof(ul), 1, fp);
	//イメージデータ
	si = (width * 24 + 31) / 32 * 4 - width * 3;	//行末に詰めるゴミのサイズ(4の倍数バイトにする)
	c[0] = 0;	//ゴミ
	for (i = height - 1; i >= 0; i--) {
		int j;
		for (j = 0; j < width; j++) {
			char tmp[3];
			tmp[0] = (*this)[i][j].getB();
			tmp[1] = (*this)[i][j].getG();
			tmp[2] = (*this)[i][j].getR();
			fwrite(tmp, sizeof(tmp), 1, fp);
		}
		fwrite(c, sizeof(c[0]), si, fp);
	}
	fclose(fp);
}

template <typename Type>
Image<GrayPixel<LDR>>& Image<Type>::getGrayImage() const
{
	Image<GrayPixel<LDR>>* image = new Image<GrayPixel<LDR>>(width, height);
	if (image == NULL)
		throw BadAllocationException();
	int elementCount = getElementCount();
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			double tmp = 0.0;
			for (int k = 0; k < elementCount; k++)
				tmp += (*this)[i][j][k];
			(*image)[i][j].setBrightness((LDR)(tmp / elementCount));
		}
	}
	return *image;
}

template <typename Type>
const int Image<Type>::INTER_NN = CV_INTER_NN;

template <typename Type>
const int Image<Type>::INTER_LINEAR = CV_INTER_LINEAR;

template <typename Type>
const int Image<Type>::INTER_AREA = CV_INTER_AREA;

template <typename Type>
const int Image<Type>::INTER_CUBIC = CV_INTER_CUBIC;

#endif
