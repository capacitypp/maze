#ifndef ___ClassOpenCVManager
#define ___ClassOpenCVManager

#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <string>

#include "opencv2\opencv.hpp"

#include "Exception.h"
#include "RGBPixel.h"
#include "Image.h"
#include "OpenCVImage.h"

template <typename Type = RGBPixel<LDR>>
class DrawCommand {
	int index;
	OpenCVImage<Type> image;
public:
	DrawCommand(int idx, const OpenCVImage<Type>& img) : index(idx), image(OpenCVImage<Type>(img)) { }
	DrawCommand(const DrawCommand<Type>& src) : index(src.index), image(OpenCVImage<Type>(src.image)) { }
	int getIndex() const { return index; }
	const OpenCVImage<Type>& getImage() const { return image; }
	DrawCommand<Type>& operator=(const DrawCommand<Type>& src) { index = src.index; image = src.image; }
};

class ResizeCommand {
	int idx;
	int w, h;
public:
	ResizeCommand(int index, int width, int height) : idx(index), w(width), h(height) { }
	ResizeCommand(const ResizeCommand& src) : idx(src.idx), w(src.w), h(src.h) { }
	int getIndex() const { return idx; }
	int getWidth() const { return w; }
	int getHeight() const { return h; }
	ResizeCommand& operator=(const ResizeCommand& src) { idx = src.idx; w = src.w; h = src.h; }
};

template <typename Type = RGBPixel<LDR>>
class OpenCVManager {
	std::thread* th;
	static std::mutex mtx;
	static std::vector<std::string> windowNames;
	static std::vector<std::string> createWindowNames;
	static std::vector<DrawCommand<Type>> drawCommands;
	static std::vector<ResizeCommand> resizeCommands;
	static std::vector<int> destroyWindows;
	static void thread();
public:
	OpenCVManager() : th(NULL) { windowNames.clear(); createWindowNames.clear(); drawCommands.clear(); resizeCommands.clear(); }
	~OpenCVManager() { if (th != NULL) delete th; }
	void run() { th = new std::thread(&OpenCVManager<Type>::thread); th->detach(); }
	int createWindow(char* windowName);
	void showImage(int idx, const Image<Type>& image);
	void showImage(const Image<Type>& image);
	void showImage(int idx, const Image<GrayPixel<LDR>>& gray);
	void showImage(const Image<GrayPixel<LDR>>& gray);
	void resizeWindow(int idx, int width, int height);
	void destroyWindow(int idx);
};

template <typename Type>
void OpenCVManager<Type>::thread()
{
	while (1) {
		std::lock_guard<std::mutex> lock(mtx);
		if (!createWindowNames.empty()) {
			std::string windowName = createWindowNames.back();
			windowNames.push_back(windowName);
			cv::namedWindow(windowName);
			createWindowNames.pop_back();
		}
		if (!drawCommands.empty()) {
			DrawCommand<Type> command = drawCommands.back();
			cvShowImage(windowNames[command.getIndex()].c_str(), command.getImage().getImage());
			drawCommands.pop_back();
		}
		if (!resizeCommands.empty()) {
			ResizeCommand command = resizeCommands.back();
			cvResizeWindow(windowNames[command.getIndex()].c_str(), command.getWidth(), command.getHeight());
			resizeCommands.pop_back();
		}
		if (!destroyWindows.empty()) {
			int idx = destroyWindows.back();
			cvDestroyWindow(windowNames[idx].c_str());
			destroyWindows.pop_back();
		}
		cv::waitKey(100);
	}
}

template <typename Type>
int OpenCVManager<Type>::createWindow(char* windowName)
{
	std::lock_guard<std::mutex> lock(mtx);
	createWindowNames.push_back(windowName);
	int ret = windowNames.size();
	return ret;
}

template <typename Type>
void OpenCVManager<Type>::showImage(int idx, const Image<Type>& image)
{
	std::lock_guard<std::mutex> lock(mtx);
	drawCommands.push_back(DrawCommand<Type>(idx, OpenCVImage<Type>(image)));
}

template <typename Type>
void OpenCVManager<Type>::showImage(const Image<Type>& image)
{
	int idx = createWindow("window");
	showImage(idx, image);
}

template <typename Type>
void OpenCVManager<Type>::showImage(int idx, const Image<GrayPixel<LDR>>& gray)
{
	int width = gray.getWidth();
	int height = gray.getHeight();
	Image<RGBPixel<LDR>> image(width, height);
	int elementCount = image.getElementCount();
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
			for (int k = 0; k < elementCount; k++)
				image[i][j][k] = gray[i][j].getBrightness();
	showImage(idx, image);
}

template <typename Type>
void OpenCVManager<Type>::showImage(const Image<GrayPixel<LDR>>& gray)
{
	int idx = createWindow("window");
	showImage(idx, gray);
}

template <typename Type>
void OpenCVManager<Type>::resizeWindow(int idx, int width, int height)
{
	std::lock_guard<std::mutex> lock(mtx);
	resizeCommands.push_back(ResizeCommand(idx, width, height));
}

template <typename Type>
void OpenCVManager<Type>::destroyWindow(int idx)
{
	std::lock_guard<std::mutex> lock(mtx);
	destroyWindows.push_back(idx);
}

template <typename Type>
std::mutex OpenCVManager<Type>::mtx;

template <typename Type>
std::vector<std::string> OpenCVManager<Type>::windowNames;

template <typename Type>
std::vector<std::string> OpenCVManager<Type>::createWindowNames;

template <typename Type>
std::vector<DrawCommand<Type>> OpenCVManager<Type>::drawCommands;

template <typename Type>
std::vector<ResizeCommand> OpenCVManager<Type>::resizeCommands;

template <typename Type>
std::vector<int> OpenCVManager<Type>::destroyWindows;

#endif
