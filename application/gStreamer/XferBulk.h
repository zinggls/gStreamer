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
	virtual int process() = 0;
	virtual void close();
	void sendEvent();

public:
	CCyUSBEndPoint *m_pEndPt;
	int m_nPPX;
	int m_nQueueSize;
	ULONGLONG m_ulSuccessCount;
	ULONGLONG m_ulFailureCount;
	ULONGLONG m_ulBeginDataXferErrCount;
	ULONGLONG m_ulBytesTransferred;
	clock_t m_startTime;
	double m_curKBps;
	FILEINFO m_fileInfo;
	BOOL m_bStart;
	HWND m_hWnd;
	static BYTE sync[4];

protected:
	ULONG m_uLen;
	PUCHAR	*m_buffers;
	PUCHAR	*m_contexts;
	OVERLAPPED	m_ovLap[MAX_QUEUE_SIZE];
	CFile *m_pFile;
};