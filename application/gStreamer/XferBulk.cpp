#include "stdafx.h"
#include "XferBulk.h"
#include <CyAPI.h>
#include "userDefinedMessage.h"

BYTE CXferBulk::sync[4] = { 0x07,0x3a,0xb6,0x99 };	//내맘대로 임의로 정한 sync코드 (앞의 세자리는 ETI싱크임)

CXferBulk::CXferBulk()
	:m_pEndPt(NULL), m_uLen(0), m_nPPX(0), m_buffers(NULL), m_contexts(NULL), m_nQueueSize(0),
	m_pUlSuccessCount(NULL), m_pUlFailureCount(NULL), m_pUlBeginDataXferErrCount(NULL), m_pUlBytesTransferred(NULL),
	m_pCurKBps(NULL), m_bStart(FALSE), m_hWnd(NULL)
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

	ASSERT(m_pUlSuccessCount);
	ASSERT(m_pUlFailureCount);
	ASSERT(m_pUlBeginDataXferErrCount);
	ASSERT(m_pUlBytesTransferred);
	ASSERT(m_pCurKBps);
	ASSERT(m_pStartTime);
	return 0;
}

void CXferBulk::close()
{
	delete[] m_contexts;
	delete[] m_buffers;
}

void CXferBulk::sendEvent()
{
	for (int i = 0; i < m_nQueueSize; i++) SetEvent(m_ovLap[i].hEvent);
}

void CXferBulk::stats()
{
	clock_t curTime = clock();
	double elapsed = ((double)(curTime - *m_pStartTime)) / CLOCKS_PER_SEC;
	*m_pCurKBps = ((double)(*m_pUlBytesTransferred) / elapsed) / 1024.;
}

void CXferBulk::initVariables()
{
	*m_pUlSuccessCount = *m_pUlFailureCount = *m_pUlBeginDataXferErrCount = *m_pUlBytesTransferred = 0;
	*m_pCurKBps = 0.0;
	*m_pStartTime = clock();
}