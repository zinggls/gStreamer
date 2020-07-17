#pragma once

#include "XferBulk.h"

class CBulkInDataProc;

class CXferBulkIn : public CXferBulk {
public:
	CXferBulkIn();
	virtual ~CXferBulkIn();

	virtual int open();
	virtual int process();
	virtual void close();

	static int GetFileInfo(UCHAR *buffer, ULONG bufferSize, int syncSize, FILEINFO &info);

protected:
	CBulkInDataProc *m_pDataProc;
};