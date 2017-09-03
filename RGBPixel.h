#ifndef ___ClassRGBPixel
#define ___ClassRGBPixel

#include "Pixel.h"

template <typename Type = LDR>
class RGBPixel : virtual public Pixel<Type> {
public:
	inline RGBPixel(Type color = Type()) : Pixel<Type>(3, color) { };
	inline RGBPixel(void* dataPtr, Type color = Type()) : Pixel<Type>(3, dataPtr, color) { };
	inline ~RGBPixel() { };
	inline Type& getR() const { return data[0]; }
	inline Type& getG() const { return data[1]; }
	inline Type& getB() const { return data[2]; }
	inline void setR(Type brightness) { data[0] = brightness; }
	inline void setG(Type brightness) { data[1] = brightness; }
	inline void setB(Type brightness) { data[2] = brightness; }
};

#endif
