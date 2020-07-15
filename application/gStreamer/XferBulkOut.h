#pragma once

#include "XferBulk.h"

class CXferBulkOut : public CXferBulk {
public:
	CXferBulkOut();
	virtual ~CXferBulkOut();

	virtual int open();
	virtual int process();
	virtual void close();
	void processFile(CFile *pFile);

	static CFile* GetFile(CString pathFileName, FILEINFO &fileInfo);
	static int SetFileInfo(UCHAR *buffer, ULONG bufferSize, BYTE *sync, int syncSize, FILEINFO &info);
	static UINT Read(CFile *pFile, UCHAR *buffer, UINT nCount);

	CList<CString> *m_pFileList;
};