#include "stdafx.h"
#include "XferBulk.h"
#include <CyAPI.h>
#include "userDefinedMessage.h"

BYTE CXferBulk::sync[4] = { 0x07,0x3a,0xb6,0x99 };	//내맘대로 임의로 정한 sync코드 (앞의 세자리는 ETI싱크임)

CXferBulk::CXferBulk()
	:m_pEndPt(NULL), m_uLen(0), m_nPPX(0), m_buffers(NULL), m_contexts(NULL), m_nQueueSize(0),
	m_ulSuccessCount(0), m_ulFailureCount(0), m_ulBeginDataXferErrCount(0), m_ulBytesTransferred(0),
	m_curKBps(0.0), m_pFile(NULL), m_bStart(FALSE), m_hWnd(NULL)
{
	memset(&m_fileInfo, 0, sizeof(FILEINFO));
}

CXferBulk::~CXferBulk()
{

}

int CXferBulk::open()
{
	ASSERT(m_pEndPt);
	ASSERT(m_pEndPt->Attributes == 2);

	ASSERT(m_pEndPt->MaxPktSize > 0);
	ASSERT(m_nPPX > 0);
	m_uLen = m_pEndPt->MaxPktSize * m_nPPX;
	m_pEndPt->SetXferSize(m_uLen);

	ASSERT(m_nQueueSize > 0);
	m_buffers = new PUCHAR[m_nQueueSize];
	m_contexts = new PUCHAR[m_nQueueSize];

	for (int i = 0; i< m_nQueueSize; i++) {
		m_buffers[i] = new UCHAR[m_uLen];
		m_ovLap[i].hEvent = CreateEvent(NULL, false, false, NULL);
		memset(m_buffers[i], 0xEF, m_uLen);
	}

	m_ulSuccessCount = m_ulFailureCount = m_ulBeginDataXferErrCount = m_ulBytesTransferred = 0;
	m_curKBps = 0.0;
	m_startTime = clock();
	return 0;
}

void CXferBulk::close()
{
	delete m_pFile;
	delete[] m_contexts;
	delete[] m_buffers;

	ASSERT(m_hWnd != NULL);
	::PostMessage(m_hWnd, WM_THREAD_TERMINATED, 0, 0);
}

void CXferBulk::sendEvent()
{
	for (int i = 0; i < m_nQueueSize; i++) SetEvent(m_ovLap[i].hEvent);
}