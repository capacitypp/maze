#ifdef ___MAKE_DLL
	#define ___PORT	__declspec(dllexport)
#else
	#define	___PORT	__declspec(dllimport)
	#pragma comment(lib, "jpeg/lib/jpeg.lib")
#endif

//jpeg�t�@�C����ǂݍ���
//fpath : jpeg�t�@�C���p�X�ւ̃|�C���^
//width : jpeg�̉摜��
//height : jpeg�̉摜����
//�߂�l : 24bit�摜�ւ̃|�C���^
//width��4�̔{���ɂȂ�悤�ɒ�������Ă���
//(��)width=7, height=10�̂Ƃ��Cwidth*3=21byte��3byte����Ȃ��̂Ŋe�s��3byte��������Ă���
//���̎��̑S�̂̃T�C�Y��(width*7+3)*10=240byte
___PORT void* load_jpeg(const char* fpath, int* width, int* height);

//jpeg�t�@�C������������
//RGB�̂ݑΉ�
___PORT bool write_jpeg(const char *fpath, unsigned char*** data, int width, int height);

___PORT void delete_jpeg(void* ptr);
