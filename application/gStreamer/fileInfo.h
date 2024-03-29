#pragma once

typedef struct {
	int index_;		//전송하는 여러개의 파일들중에서 몇번째 순번의 파일인지를 알리는 인덱스(0 based order)
	int files_;		//전송하는 총 파일의 숫자 (파일이 두개면 files_=2 index_는 각각 0,1)
	DWORD size_;
	TCHAR name_[512];
	int nameSize_;
} FILEINFO;