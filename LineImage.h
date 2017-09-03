#ifndef ___ClassLineImage
#define ___ClassLineImage

#include "Exception.h"
#include "RGBPixel.h"

template <typename Type = RGBPixel<LDR> >
class LineImage {
	static int defaultWidth;
	static unsigned char* memoryPool;
	static int memoryPoolSize;

	int width;
	Type *data;
	void allocate(int width);
public:
	LineImage() { allocate(defaultWidth); }
	LineImage(int width);
	LineImage(void* dataPtr) : LineImage(defaultWidth, dataPtr) { }
	LineImage(int width, void* dataPtr);
	~LineImage();
	static void setDefaultWidth(int width);
	static void setMemoryPool(unsigned char* ptr, int size) {
		memoryPool = ptr;
		memoryPoolSize = size;
	}
	void resize(int width);
	static int getElementCount() { return Type().getElementCount(); }
	inline int getWidth() const { return width; }
	inline Type& operator[](int idx) {
		if (idx < 0 || idx >= width)
			throw InvalidIndexException(idx);
		return data[idx];
	}
	inline const Type& operator[](int idx) const {
		if (idx < 0 || idx >= width)
			throw InvalidIndexException(idx);
		return data[idx];
	}
};

template <typename Type>
int LineImage<Type>::defaultWidth = 100;

template <typename Type>
unsigned char* LineImage<Type>::memoryPool = NULL;

template <typename Type>
int LineImage<Type>::memoryPoolSize = 0;

template <typename Type>
inline void LineImage<Type>::allocate(int width)
{
	int size = width * Type().getElementCount();
	if (memoryPoolSize >= size) {
		Type::setMemoryPool(memoryPool, size);
		memoryPool += size;
		memoryPoolSize -= size;
	}
	data = new Type[width];
	if (data == NULL)
		throw BadAllocationException();
	this->width = width;
}

template <typename Type>
inline LineImage<Type>::LineImage(int width)
{
	allocate(width);
}

template <typename Type>
inline LineImage<Type>::LineImage(int width, void* dataPtr)
{
	Type::setMemoryPool(dataPtr, width * Type().getElementCount());
	data = new Type[width];
	if (data == NULL)
		throw BadAllocationException();
	this->width = width;
}

template <typename Type>
inline LineImage<Type>::~LineImage()
{
	delete[] data;
}

template <typename Type>
void LineImage<Type>::setDefaultWidth(int width)
{
	defaultWidth = width;
}

template <typename Type>
void LineImage<Type>::resize(int width)
{
	if (width == this->width)
		return;
	delete[] data;
	data = new Type[width];
	if (data == NULL)
		throw BadAllocationException();
	this->width = width;
}

#endif
