#include "stdafx.h"
#include "XferBulkIn.h"
#include <CyAPI.h>
#include "userDefinedMessage.h"
#include "BulkInDataProc.h"

CXferBulkIn::CXferBulkIn()
	:m_pDataProc(NULL)
{
	m_pDataProc = new CBulkInDataProc();
}

CXferBulkIn::~CXferBulkIn()
{
	delete m_pDataProc;
}

int CXferBulkIn::open()
{
	CXferBulk::open();
	ASSERT(m_pEndPt->bIn == TRUE);
	ASSERT(m_uLen>0);
	ASSERT(m_hWnd!=NULL);
	m_pDataProc->m_nLen = m_uLen;
	m_pDataProc->m_hWnd = m_hWnd;
	return 0;
}

int CXferBulkIn::process()
{
	for (int i = 0; i < m_nQueueSize; i++) {
		m_contexts[i] = m_pEndPt->BeginDataXfer(m_buffers[i], m_uLen, &m_ovLap[i]);
		ASSERT(m_pEndPt->NtStatus == 0 && m_pEndPt->UsbdStatus == 0);
	}

	int i = 0;
	for (; m_bStart;)
	{
		long rLen = m_uLen;
		bool bRtn = m_pEndPt->WaitForXfer(&m_ovLap[i], INFINITE);
		ASSERT(bRtn);

		bRtn = m_pEndPt->FinishDataXfer(m_buffers[i], rLen, &m_ovLap[i], m_contexts[i]);
		if (bRtn) {
			(*m_pUlSuccessCount)++;
			(*m_pUlBytesTransferred) += rLen;
			m_pDataProc->OnData(m_buffers[i], rLen);
		}
		else {
			(*m_pUlFailureCount)++;
		}

		m_contexts[i] = m_pEndPt->BeginDataXfer(m_buffers[i], m_uLen, &m_ovLap[i]);
		ASSERT(m_pEndPt->NtStatus == 0 && m_pEndPt->UsbdStatus == 0);

		if (!bRtn) {
			//블럭상태(WaitForXfer에서 블럭상태)에서 종료를 누르면 블럭을 빠져나오기 위해 sendEvent함수가 호출되고
			//이때 FinishDataXfer함수는 false를 리턴한다. 이때 루프를 빠져나온다
			break;
		}
		i++;
		if (i == m_nQueueSize) i = 0;
	}
	return 0;
}

void CXferBulkIn::close()
{
	LONG rLen = m_uLen;
	for (int i = 0; i < m_nQueueSize; i++) {
		m_pEndPt->FinishDataXfer(m_buffers[i], rLen, &m_ovLap[i], m_contexts[i]);
		delete [] m_buffers[i];
	}
	CXferBulk::close();
}

int CXferBulkIn::GetFileInfo(UCHAR *buffer, ULONG bufferSize, int syncSize, FILEINFO &info)
{
	int nOffset = syncSize;
	memset(info.name_, 0, sizeof(FILEINFO::name_));
	memcpy(&info.index_, buffer + nOffset, sizeof(int)); nOffset += sizeof(int);
	memcpy(&info.files_, buffer + nOffset, sizeof(int)); nOffset += sizeof(int);
	memcpy(&info.nameSize_, buffer + nOffset, sizeof(int)); nOffset += sizeof(int);
	memcpy(info.name_, buffer + nOffset, info.nameSize_); nOffset += info.nameSize_;
	memcpy(&info.size_, buffer + nOffset, sizeof(DWORD)); nOffset += sizeof(DWORD);

	ASSERT((ULONG)nOffset <= bufferSize);	//len보다 작거나 같다는 가정
	return nOffset;
}