#include "stdafx.h"
#include "BulkInDataProc.h"
#include "XferBulkIn.h"
#include "userDefinedMessage.h"

CBulkInDataProc::CBulkInDataProc()
	:m_pDump(NULL), m_nCount(0),m_pFile(NULL), m_nReceivedFileSize(0), m_nLen(0), m_nMaxCount(0), m_hWnd(NULL)
{
#ifdef BULK_IN_DEBUG
	m_pDump = new CFile(_T("BulkIn.dump"), CFile::modeCreate | CFile::modeWrite);
	ASSERT(m_pDump);
#endif
}

CBulkInDataProc::~CBulkInDataProc()
{
#ifdef BULK_IN_DEBUG
	m_pDump->Close();
	delete m_pDump;
#endif
}

void CBulkInDataProc::OnData(PUCHAR buf, LONG len)
{
#ifdef BULK_IN_DEBUG
	m_pDump->Write(buf, len);
#endif

	m_nCount++;
	if (SyncFound(buf, len))
		m_pFile = OnHeader(buf, len);
	else {
		if (m_nCount < m_nMaxCount) {
			OnBody(buf, len);
		}
		else {
			OnEof(buf, m_fileInfo.size_ - m_nReceivedFileSize);
		}
	}
}

bool CBulkInDataProc::SyncFound(PUCHAR buf, LONG len)
{
	if (m_nCount == 1 && memcmp(buf, CXferBulk::sync, sizeof(CXferBulk::sync)) == 0) return true;
	return false;
}

CFile* CBulkInDataProc::OnHeader(PUCHAR buf, LONG len)
{
	m_nReceivedFileSize = 0;
	memset(&m_fileInfo, 0, sizeof(FILEINFO));
	CXferBulkIn::GetFileInfo(buf, len, sizeof(CXferBulkIn::sync), m_fileInfo);
	ASSERT(m_nLen>0);
	ASSERT(m_fileInfo.size_ > 0);
	m_nMaxCount = m_fileInfo.size_ / m_nLen;
	if ((m_fileInfo.size_%m_nLen) != 0) m_nMaxCount++;
	m_nMaxCount++;	//맨처음 보내지는 헤더를 고려해서 하나를 더함

	if (m_fileInfo.index_==0)
		::SendMessage(m_hWnd, WM_FIRST_HEADER, (WPARAM)&m_fileInfo, 0);

	return new CFile(m_fileInfo.name_, CFile::modeCreate | CFile::modeWrite);
}

void CBulkInDataProc::OnBody(PUCHAR buf, LONG len)
{
	ASSERT(m_pFile);
	m_pFile->Write(buf, len);
	m_nReceivedFileSize += len;
}

void CBulkInDataProc::OnEof(PUCHAR buf, LONG len)
{
	ASSERT(m_pFile);
	m_pFile->Write(buf, len);
	m_nReceivedFileSize += len;

	m_pFile->Close();
	delete m_pFile;
	m_pFile = NULL;
	m_nCount = 0;

	ASSERT(m_hWnd);
	::SendMessage(m_hWnd, WM_FILE_RECEIVED, (WPARAM)&m_fileInfo, m_nReceivedFileSize);

	if((m_fileInfo.index_+1) == m_fileInfo.files_)
		::SendMessage(m_hWnd, WM_ALL_FILES_RECEIVED, (WPARAM)m_fileInfo.files_,0);
}