#pragma once

#include "IXfer.h"
#include "fileInfo.h"

#define MAX_QUEUE_SIZE		64

class CCyUSBEndPoint;

class CXferBulk : public IXfer {
public:
	CXferBulk();
	virtual ~CXferBulk();

	virtual int open();
	virtual int process()=0;
	virtual void close();
	void sendEvent();

	CCyUSBEndPoint *m_pEndPt;
	ULONG m_uLen;
	int m_nPPX;
	PUCHAR	*m_buffers;
	PUCHAR	*m_contexts;
	int m_nQueueSize;
	OVERLAPPED	m_ovLap[MAX_QUEUE_SIZE];
	ULONGLONG m_ulSuccessCount;
	ULONGLONG m_ulFailureCount;
	ULONGLONG m_ulBeginDataXferErrCount;
	ULONGLONG m_ulBytesTransferred;
	clock_t m_startTime;
	double m_curKBps;
	CFile *m_pFile;
	FILEINFO m_fileInfo;
	static BYTE sync[4];
	BOOL m_bStart;
	HWND m_hWnd;
};