
// gStreamer.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CgStreamerApp:
// �� Ŭ������ ������ ���ؼ��� gStreamer.cpp�� �����Ͻʽÿ�.
//

class CgStreamerApp : public CWinApp
{
public:
	CgStreamerApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CgStreamerApp theApp;