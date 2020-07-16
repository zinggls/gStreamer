#pragma once

#include "IDataCallback.h"

class CFile;

class CBulkInDataProc :public IDataCallback {
public:
	CBulkInDataProc();
	virtual ~CBulkInDataProc();

	void OnData(PUCHAR buf, LONG len);

	CFile *m_pDump;
};