#include "stdafx.h"
#include "XferBulkIn.h"
#include <CyAPI.h>
#include "userDefinedMessage.h"

CXferBulkIn::CXferBulkIn()
{

}

CXferBulkIn::~CXferBulkIn()
{

}

int CXferBulkIn::open()
{
	CXferBulk::open();
	ASSERT(m_pEndPt->bIn == TRUE);
	return 0;
}

int CXferBulkIn::process()
{
	for (int i = 0; i < m_nQueueSize; i++) {
		m_contexts[i] = m_pEndPt->BeginDataXfer(m_buffers[i], m_uLen, &m_ovLap[i]);
		if (m_pEndPt->NtStatus || m_pEndPt->UsbdStatus) (*m_pUlBeginDataXferErrCount)++;
	}

	UINT receivedFileSize = 0;
	BOOL bInitFrame = TRUE;
	LONG rLen;
	while (m_bStart) {
		for (int i = 0; i < m_nQueueSize; i++) {
			m_pEndPt->WaitForXfer(&m_ovLap[i], INFINITE);

			if (m_pEndPt->FinishDataXfer(m_buffers[i], rLen, &m_ovLap[i], m_contexts[i])) {
				(*m_pUlSuccessCount)++;
				(*m_pUlBytesTransferred) += rLen;

				if (bInitFrame && i == 0) {
					if (memcmp(m_buffers[i], sync, sizeof(sync)) == 0) {
						GetFileInfo(m_buffers[i], m_uLen, sizeof(sync), m_fileInfo);
						ASSERT(m_hWnd != NULL);
						::PostMessage(m_hWnd,WM_SYNC_FOUND, (WPARAM)&m_fileInfo, 0);
						m_pFile = new CFile(m_fileInfo.name_, CFile::modeCreate | CFile::modeWrite);
						ASSERT(m_pFile);	// TODO 파일 생성과정에서 NULL이 나올수도 있음
					}
					bInitFrame = FALSE;
				}
				else {
					ASSERT(m_pFile);
					if ((receivedFileSize + rLen) <= m_fileInfo.size_) {
						m_pFile->Write(m_buffers[i], rLen);
						receivedFileSize += rLen;
					}
					else {
						UINT size = m_fileInfo.size_ - receivedFileSize;
						m_pFile->Write(m_buffers[i], size);
						receivedFileSize += size;
					}
				}
				ASSERT(m_hWnd != NULL);
				::PostMessage(m_hWnd, WM_DATA_RECEIVED, 0, 0);
			}
			else {
				(*m_pUlFailureCount)++;
			}

			m_contexts[i] = m_pEndPt->BeginDataXfer(m_buffers[i], m_uLen, &m_ovLap[i]);
			if (m_pEndPt->NtStatus || m_pEndPt->UsbdStatus) (*m_pUlBeginDataXferErrCount)++;

			if (m_fileInfo.size_>0 && (*m_pUlBytesTransferred >= (m_fileInfo.size_ + m_uLen))) {
				ASSERT(m_hWnd != NULL);
				::PostMessage(m_hWnd, WM_FILE_RECEIVED, (WPARAM)&m_fileInfo, (LPARAM)*m_pUlBytesTransferred);
				m_bStart = FALSE;
				m_pFile->Close();
				break;
			}

			if (i == (m_nQueueSize - 1)) {	//큐의 맨마지막 요소
				if (!m_bStart) break;	//종료 명령(m_bStart==FALSE)이 도착했고, 큐의 맨마지막 요소까지 처리하고 났으면 for루프를 탈출
			}
		}
	}
	return 0;
}

void CXferBulkIn::close()
{
	LONG rLen;
	for (int i = 0; i < m_nQueueSize; i++) {
		m_pEndPt->FinishDataXfer(m_buffers[i], rLen, &m_ovLap[i], m_contexts[i]);
		delete[] m_buffers[i];
	}
	CXferBulk::close();
}

int CXferBulkIn::GetFileInfo(UCHAR *buffer, ULONG bufferSize, int syncSize, FILEINFO &info)
{
	int nOffset = syncSize;
	memset(info.name_, 0, sizeof(FILEINFO::name_));
	memcpy(&info.nameSize_, buffer + nOffset, sizeof(int)); nOffset += sizeof(int);
	memcpy(info.name_, buffer + nOffset, info.nameSize_); nOffset += info.nameSize_;
	memcpy(&info.size_, buffer + nOffset, sizeof(DWORD)); nOffset += sizeof(DWORD);

	ASSERT((ULONG)nOffset <= bufferSize);	//len보다 작거나 같다는 가정
	return nOffset;
}