#ifndef ___ClassPixel
#define ___ClassPixel

#include "Exception.h"

typedef unsigned char LDR;

template <typename Type = LDR>
class Pixel {
protected:
	int elementCount;
	Type *data;
	bool deletable;
	static unsigned char* memoryPool;
	static int memoryPoolSize;
public:
	Pixel(int n, Type color = Type());
	Pixel(int n, void* dataPtr, Type color = Type());
	~Pixel();
	static void setMemoryPool(unsigned char* ptr, int size) {
		memoryPool = ptr;
		memoryPoolSize = size;
	}
	void alloc() {
		if (data != NULL && deletable == true)
			delete[] data;
		deletable = true;
		data = new Type[elementCount];
		if (data == NULL)
			throw BadAllocationException();
	}
	//1ピクセルあたりの要素数を返す(RGBなら3)
	inline int getElementCount() const { return elementCount; }
	inline Type& operator[](int idx) {
		if (idx < 0 || idx >= elementCount)
			throw InvalidIndexException(idx);
		return data[idx];
	}
	inline const Type& operator[](int idx) const {
		if (idx < 0 || idx >= elementCount)
			throw InvalidIndexException(idx);
		return data[idx];
	}
};

template <typename Type>
unsigned char* Pixel<Type>::memoryPool = NULL;

template <typename Type>
int Pixel<Type>::memoryPoolSize = 0;


template <typename Type>
inline Pixel<Type>::Pixel(int n, Type color) : data(NULL), deletable(false)
{
	elementCount = n;
	if (memoryPoolSize >= n) {
		data = memoryPool;
		memoryPool += n;
		memoryPoolSize -= n;
	}
	else {
		alloc();
	}
	for (int i = 0; i < n; i++)
		data[i] = color;
}

template <typename Type>
inline Pixel<Type>::Pixel(int n, void* dataPtr, Type color = Type()) : data(NULL), deletable(false)
{
	elementCount = n;
	data = dataPtr;
	for (int i = 0; i < n; i++)
		data[i] = color;
}


template <typename Type>
inline Pixel<Type>::~Pixel()
{
	if (deletable) {
		delete[] data;
	}
}

#endif
