#pragma once

#include "IDataCallback.h"
#include "fileInfo.h"

class CFile;

class CBulkInDataProc :public IDataCallback {
public:
	CBulkInDataProc();
	virtual ~CBulkInDataProc();

	void OnData(PUCHAR buf, LONG len);
	bool SyncFound(PUCHAR buf, LONG len);
	CFile* OnHeader(PUCHAR buf, LONG len);
	void OnBody(PUCHAR buf, LONG len);
	void OnEof(PUCHAR buf, LONG len);

	CFile *m_pDump;
	ULONG m_nCount;
	FILEINFO m_fileInfo;
	CFile *m_pFile;
	ULONG m_nReceivedFileSize;
	ULONG m_nLen;
	ULONG m_nMaxCount;
};