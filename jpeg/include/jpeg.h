#ifdef ___MAKE_DLL
	#define ___PORT	__declspec(dllexport)
#else
	#define	___PORT	__declspec(dllimport)
	#pragma comment(lib, "jpeg/lib/jpeg.lib")
#endif

//jpegファイルを読み込む
//fpath : jpegファイルパスへのポインタ
//width : jpegの画像幅
//height : jpegの画像高さ
//戻り値 : 24bit画像へのポインタ
//widthが4の倍数になるように調整されている
//(例)width=7, height=10のとき，width*3=21byteで3byte足りないので各行に3byteずつ足されている
//この時の全体のサイズは(width*7+3)*10=240byte
___PORT void* load_jpeg(const char* fpath, int* width, int* height);

//jpegファイルを書き込む
//RGBのみ対応
___PORT bool write_jpeg(const char *fpath, unsigned char*** data, int width, int height);

___PORT void delete_jpeg(void* ptr);
