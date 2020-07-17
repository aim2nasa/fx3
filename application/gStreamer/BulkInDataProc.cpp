#include "stdafx.h"
#include "BulkInDataProc.h"
#include "XferBulkIn.h"

CBulkInDataProc::CBulkInDataProc()
	:m_pDump(NULL), m_nCount(0),m_pFile(NULL), m_nReceivedFileSize(0), m_nLen(0), m_nMaxCount(0), m_hWnd(NULL)
{
#ifdef DEBUG
	m_pDump = new CFile(_T("BulkIn.dump"), CFile::modeCreate | CFile::modeWrite);
	ASSERT(m_pDump);
#endif
}

CBulkInDataProc::~CBulkInDataProc()
{
#ifdef DEBUG
	m_pDump->Close();
	delete m_pDump;
#endif
}

void CBulkInDataProc::OnData(PUCHAR buf, LONG len)
{
#ifdef DEBUG
	m_pDump->Write(buf, len);
#endif

	m_nCount++;
	if (SyncFound(buf, len))
		m_pFile = OnHeader(buf, len);
	else {
		if (m_nCount < m_nMaxCount) {
			OnBody(buf, len);
		}
		else {
			OnEof(buf, m_fileInfo.size_ - m_nReceivedFileSize);
		}
	}
}

bool CBulkInDataProc::SyncFound(PUCHAR buf, LONG len)
{
	if (m_nCount == 1 && memcmp(buf, CXferBulk::sync, sizeof(CXferBulk::sync)) == 0) return true;
	return false;
}

CFile* CBulkInDataProc::OnHeader(PUCHAR buf, LONG len)
{
	m_nReceivedFileSize = 0;
	memset(&m_fileInfo, 0, sizeof(FILEINFO));
	CXferBulkIn::GetFileInfo(buf, len, sizeof(CXferBulkIn::sync), m_fileInfo);
	ASSERT(m_nLen>0);
	ASSERT(m_fileInfo.size_ > 0);
	m_nMaxCount = m_fileInfo.size_ / m_nLen;
	if ((m_fileInfo.size_%m_nLen) != 0) m_nMaxCount++;
	m_nMaxCount++;	//��ó�� �������� ����� �����ؼ� �ϳ��� ����
	return new CFile(m_fileInfo.name_, CFile::modeCreate | CFile::modeWrite);
}

void CBulkInDataProc::OnBody(PUCHAR buf, LONG len)
{
	ASSERT(m_pFile);
	m_pFile->Write(buf, len);
	m_nReceivedFileSize += len;
}

void CBulkInDataProc::OnEof(PUCHAR buf, LONG len)
{
	ASSERT(m_pFile);
	m_pFile->Write(buf, len);
	m_nReceivedFileSize += len;

	m_pFile->Close();
	delete m_pFile;
	m_pFile = NULL;
	m_nCount = 0;
}