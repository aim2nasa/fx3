#pragma once

typedef struct {
	int index_;		//�����ϴ� �������� ���ϵ��߿��� ���° ������ ���������� �˸��� �ε���(0 based order)
	int files_;		//�����ϴ� �� ������ ���� (������ �ΰ��� files_=2 index_�� ���� 0,1)
	DWORD size_;
	TCHAR name_[512];
	int nameSize_;
} FILEINFO;