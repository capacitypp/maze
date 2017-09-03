#ifndef ___ClassException
#define ___ClassException

//new出来なかった場合に発生
class BadAllocationException { };
//operator[]における不正なインデックス指定時に発生
class InvalidIndexException {
	int idx;
public:
	InvalidIndexException(int index) : idx(index) {};
	inline int getIdx() const { return idx; }
};
//未対応の拡張子
class UnsupportedExtensionException { };

//ファイルがない
class FileNotExistException { };

//ファイルフォーマットが正しくない
class InvalidFileFormatException { };

#endif
