#ifndef ___ClassGrayPixel
#define ___ClassGrayPixel

#include "Pixel.h"

template <typename Type = LDR>
class GrayPixel : virtual public Pixel<Type> {
public:
	inline GrayPixel(Type color = Type()) : Pixel<Type>(1, color) { };
	inline GrayPixel(void* dataPtr, Type color = Type()) : Pixel<Type>(1, dataPtr, color) { };
	inline ~GrayPixel() { };
	inline Type& getBrightness() const { return data[0]; }
	inline void setBrightness(Type brightness) { data[0] = brightness; }
};

#endif
