#include "stdafx.h"
#include "BulkInDataProc.h"

CBulkInDataProc::CBulkInDataProc()
{
#ifdef DEBUG
	m_pDump = new CFile(_T("BulkIn.dump"), CFile::modeCreate | CFile::modeWrite);
	ASSERT(m_pDump);
#endif
}

CBulkInDataProc::~CBulkInDataProc()
{
#ifdef DEBUG
	m_pDump->Close();
	delete m_pDump;
#endif
}

void CBulkInDataProc::OnData(PUCHAR buf, LONG len)
{
#ifdef DEBUG
	m_pDump->Write(buf, len);
#endif
}