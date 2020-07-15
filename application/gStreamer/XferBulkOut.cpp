#include "stdafx.h"
#include "XferBulkOut.h"
#include <CyAPI.h>
#include "userDefinedMessage.h"

CXferBulkOut::CXferBulkOut()
{

}

CXferBulkOut::~CXferBulkOut()
{

}

int CXferBulkOut::open()
{
	CXferBulk::open();
	ASSERT(m_pEndPt->bIn == FALSE);
	ASSERT(m_pFileList);
	return 0;
}

int CXferBulkOut::process()
{
	if (m_pFileList->GetCount() == 0) {
		processFile(NULL);
	}
	else {
		int nMaxFiles = m_pFileList->GetCount();
		ASSERT(nMaxFiles > 0);

		for (int i = 0; i < nMaxFiles; i++) {
			m_bStart = TRUE;
			CString strPathName = m_pFileList->GetAt(m_pFileList->FindIndex(i));
			m_fileInfo.index_ = i;
			m_fileInfo.files_ = nMaxFiles;
			CFile *pFile = GetFile(strPathName, m_fileInfo);
			ASSERT(pFile);
			processFile(pFile);
			delete pFile;
			::PostMessage(m_hWnd, WM_FILE_SENT, i, 0);
		}
	}
	return 0;
}

void CXferBulkOut::close()
{
	for (int i = 0; i < m_nQueueSize; i++) {
		delete[] m_buffers[i];
	}
	CXferBulk::close();
}

CFile* CXferBulkOut::GetFile(CString pathFileName, FILEINFO &fileInfo)
{
	CFile *pFile = NULL;
	if (!pathFileName.IsEmpty()) {
		pFile = new CFile(pathFileName, CFile::modeRead | CFile::typeBinary);

		CString name = PathFindFileName(pathFileName.GetBuffer());
		memset(fileInfo.name_, 0, sizeof(fileInfo.name_));
		fileInfo.nameSize_ = name.GetLength() * sizeof(TCHAR);
		memcpy(fileInfo.name_, name.GetBuffer(), fileInfo.nameSize_);
		fileInfo.size_ = GetFileSize(pFile->m_hFile, NULL);

		TRACE("[%d] total files=%d, fileName=%S(%d), size=%dbyte\n", fileInfo.index_, fileInfo.files_, fileInfo.name_, fileInfo.nameSize_, fileInfo.size_);
	}
	return pFile;
}

int CXferBulkOut::SetFileInfo(UCHAR *buffer, ULONG bufferSize, BYTE *sync, int syncSize, FILEINFO &info)
{
	int nOffset = 0;
	memcpy(buffer + nOffset, sync, syncSize); nOffset += syncSize;
	memcpy(buffer + nOffset, &info.index_, sizeof(int)); nOffset += sizeof(int);
	memcpy(buffer + nOffset, &info.files_, sizeof(int)); nOffset += sizeof(int);
	memcpy(buffer + nOffset, &info.nameSize_, sizeof(int)); nOffset += sizeof(int);
	memcpy(buffer + nOffset, info.name_, info.nameSize_); nOffset += info.nameSize_;
	memcpy(buffer + nOffset, &info.size_, sizeof(DWORD)); nOffset += sizeof(DWORD);

	ASSERT((ULONG)nOffset <= bufferSize);	//len���� �۰ų� ���ٴ� ����
	return nOffset;
}

UINT CXferBulkOut::Read(CFile *pFile, UCHAR *buffer, UINT nCount)
{
	memset(buffer, 0, nCount);	//���۴� nCount���� ��� 0���� �ʱ�ȭ�Ѵ�. nCount���� ���д� ��� ������ ������ 0���� ä������
	return pFile->Read(buffer, nCount);	//BULK OUT�� ��� ���Ϸ� ���� �д´�
}

void CXferBulkOut::processFile(CFile *pFile)
{
	for (int i = 0; i < m_nQueueSize; i++) {
		if (pFile) { //BULK OUT�ε� ���Ϸ� ���� �о�鿩 ������ �����
			if (i == 0) {	//���ϸ�ũ��,���ϸ�,���ϻ���� ����, �̵� ũ��� len������ ũ��� ���� �Ѵ�. �׷��� i=0�� ��쿡�� �̷� ��Ÿ������ ��� �Ǹ��ٰ� ����.
				SetFileInfo(m_buffers[i], m_uLen, sync, sizeof(sync), m_fileInfo);
			}
			else {
				UINT nRead = Read(pFile, m_buffers[i], m_uLen);
				ASSERT(nRead>0);	//ť�װ������� Read�� 0�� ������ �ʾƾ� �Ѵ�. ������ ������ adjustQueueSize()���� ť����� �����Ͽ� �� ������ ���Ѵ�
			}
		}
		m_contexts[i] = m_pEndPt->BeginDataXfer(m_buffers[i], m_uLen, &m_ovLap[i]);
		if (m_pEndPt->NtStatus || m_pEndPt->UsbdStatus) (*m_pUlBeginDataXferErrCount)++;
	}

	LONG rLen;
	while (m_bStart) {
		for (int i = 0; i < m_nQueueSize; i++) {
			m_pEndPt->WaitForXfer(&m_ovLap[i], INFINITE);

			if (m_pEndPt->FinishDataXfer(m_buffers[i], rLen, &m_ovLap[i], m_contexts[i])) {
				(*m_pUlSuccessCount)++;
				(*m_pUlBytesTransferred) += rLen;
				ASSERT(m_hWnd != NULL);
				::PostMessage(m_hWnd, WM_DATA_SENT, 0, 0);
				TRACE("%S, %dbytes sent\n", pFile->GetFileName().GetBuffer(), *m_pUlBytesTransferred);
			}
			else {
				(*m_pUlFailureCount)++;
			}

			if (pFile) {
				if (Read(pFile, m_buffers[i], m_uLen) > 0) {
					m_contexts[i] = m_pEndPt->BeginDataXfer(m_buffers[i], m_uLen, &m_ovLap[i]);
					if (m_pEndPt->NtStatus || m_pEndPt->UsbdStatus) (*m_pUlBeginDataXferErrCount)++;
				}
				else {
					//EOF�̸� ���̻� ���� �ʿ���� ���� BeginDataXfer�� ȣ���� �ʿ䵵 ����
				}
			}
			else {
				m_contexts[i] = m_pEndPt->BeginDataXfer(m_buffers[i], m_uLen, &m_ovLap[i]);
				if (m_pEndPt->NtStatus || m_pEndPt->UsbdStatus) (*m_pUlBeginDataXferErrCount)++;
			}

			if (m_fileInfo.size_>0 && (*m_pUlBytesTransferred >= (m_fileInfo.size_ + m_uLen))) {
				m_bStart = FALSE;
				pFile->Close();
				break;
			}

			if (i == (m_nQueueSize - 1)) {	//ť�� �Ǹ����� ���
				stats();
				if (!m_bStart) break;	//���� ���(m_bStart==FALSE)�� �����߰�, ť�� �Ǹ����� ��ұ��� ó���ϰ� ������ for������ Ż��
			}
		}
	}

	for (int i = 0; i < m_nQueueSize; i++) {
		if (pFile == NULL) m_pEndPt->FinishDataXfer(m_buffers[i], rLen, &m_ovLap[i], m_contexts[i]);
	}
}