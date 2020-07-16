#include "stdafx.h"
#include "XferBulkOut.h"
#include <CyAPI.h>
#include "userDefinedMessage.h"

CXferBulkOut::CXferBulkOut()
{

}

CXferBulkOut::~CXferBulkOut()
{

}

int CXferBulkOut::open()
{
	CXferBulk::open();
	ASSERT(m_pEndPt->bIn == FALSE);
	ASSERT(m_pFileList);
	return 0;
}

int CXferBulkOut::process()
{
	if (m_pFileList->GetCount() == 0) {
		processFile(NULL);
	}
	else {
		int nMaxFiles = m_pFileList->GetCount();
		ASSERT(nMaxFiles > 0);

		for (int i = 0; i < nMaxFiles; i++) {
			m_bStart = TRUE;
			CString strPathName = m_pFileList->GetAt(m_pFileList->FindIndex(i));
			m_fileInfo.index_ = i;
			m_fileInfo.files_ = nMaxFiles;
			CFile *pFile = GetFile(strPathName, m_fileInfo);
			ASSERT(pFile);
			processFile(pFile);
			delete pFile;
			::PostMessage(m_hWnd, WM_FILE_SENT, i, 0);
		}
	}
	return 0;
}

void CXferBulkOut::close()
{
	for (int i = 0; i < m_nQueueSize; i++) {
		delete[] m_buffers[i];
	}
	CXferBulk::close();
}

CFile* CXferBulkOut::GetFile(CString pathFileName, FILEINFO &fileInfo)
{
	CFile *pFile = NULL;
	if (!pathFileName.IsEmpty()) {
		pFile = new CFile(pathFileName, CFile::modeRead | CFile::typeBinary);

		CString name = PathFindFileName(pathFileName.GetBuffer());
		memset(fileInfo.name_, 0, sizeof(fileInfo.name_));
		fileInfo.nameSize_ = name.GetLength() * sizeof(TCHAR);
		memcpy(fileInfo.name_, name.GetBuffer(), fileInfo.nameSize_);
		fileInfo.size_ = GetFileSize(pFile->m_hFile, NULL);

		TRACE("[%d] total files=%d, fileName=%S(%d), size=%dbyte\n", fileInfo.index_, fileInfo.files_, fileInfo.name_, fileInfo.nameSize_, fileInfo.size_);
	}
	return pFile;
}

int CXferBulkOut::SetFileInfo(UCHAR *buffer, ULONG bufferSize, BYTE *sync, int syncSize, FILEINFO &info)
{
	int nOffset = 0;
	memcpy(buffer + nOffset, sync, syncSize); nOffset += syncSize;
	memcpy(buffer + nOffset, &info.index_, sizeof(int)); nOffset += sizeof(int);
	memcpy(buffer + nOffset, &info.files_, sizeof(int)); nOffset += sizeof(int);
	memcpy(buffer + nOffset, &info.nameSize_, sizeof(int)); nOffset += sizeof(int);
	memcpy(buffer + nOffset, info.name_, info.nameSize_); nOffset += info.nameSize_;
	memcpy(buffer + nOffset, &info.size_, sizeof(DWORD)); nOffset += sizeof(DWORD);

	ASSERT((ULONG)nOffset <= bufferSize);	//len보다 작거나 같다는 가정
	return nOffset;
}

UINT CXferBulkOut::Read(CFile *pFile, UCHAR *buffer, UINT nCount)
{
	memset(buffer, 0xEF, nCount);	//버퍼는 nCount까지 모두 0으로 초기화한다. nCount까지 못읽는 경우 나머지 공간은 0으로 채워진다
	return pFile->Read(buffer, nCount);	//BULK OUT인 경우 파일로 부터 읽는다
}

void CXferBulkOut::processFile(CFile *pFile)
{
	initVariables();

	ASSERT(pFile);
	int nQueueSize = adjustQueueSize(pFile);
	for (int i = 0; i < nQueueSize; i++) {
		if (pFile) { //BULK OUT인데 파일로 부터 읽어들여 보내는 경우임
			if (i == 0) {	//파일명크기,파일명,파일사이즈를 보냄, 이들 크기는 len이하의 크기로 가정 한다. 그래서 i=0인 경우에만 이런 메타정보가 모두 실린다고 가정.
				SetFileInfo(m_buffers[i], m_uLen, sync, sizeof(sync), m_fileInfo);
			}
			else {
				UINT nRead = Read(pFile, m_buffers[i], m_uLen);
				ASSERT(nRead>0);	//큐잉과정에서 Read가 0이 나오지 않아야 한다. 스레드 시작전 adjustQueueSize()에서 큐사이즈를 조절하여 이 문제를 피한다
			}
		}
		m_contexts[i] = m_pEndPt->BeginDataXfer(m_buffers[i], m_uLen, &m_ovLap[i]);
		if (m_pEndPt->NtStatus || m_pEndPt->UsbdStatus) (*m_pUlBeginDataXferErrCount)++;
	}

#ifdef DEBUG
	CFile dump;
	dump.Open(_T("BulkOut_header+body_")+pFile->GetFileName(), CFile::modeCreate | CFile::modeWrite);
	CFile dumpBody;
	dumpBody.Open(_T("BulkOut_body_") + pFile->GetFileName(), CFile::modeCreate | CFile::modeWrite);
	UINT sentFileSize = 0;
#endif
	BOOL bFirst = TRUE;
	LONG rLen;
	while (m_bStart) {
		for (int i = 0; i < nQueueSize; i++) {
			m_pEndPt->WaitForXfer(&m_ovLap[i], INFINITE);

			if (m_pEndPt->FinishDataXfer(m_buffers[i], rLen, &m_ovLap[i], m_contexts[i])) {
				(*m_pUlSuccessCount)++;
				(*m_pUlBytesTransferred) += rLen;
				ASSERT(m_hWnd != NULL);
				::PostMessage(m_hWnd, WM_DATA_SENT, 0, 0);
#ifdef DEBUG
				dump.Write(m_buffers[i], rLen);
				if (bFirst) {
					bFirst = FALSE;
				}
				else {
					if ((sentFileSize + rLen) <= m_fileInfo.size_) {
						dumpBody.Write(m_buffers[i], rLen);
						sentFileSize += rLen;
					}
					else {
						UINT size = m_fileInfo.size_ - sentFileSize;
						dumpBody.Write(m_buffers[i], size);
						sentFileSize += size;
					}
				}
#endif
				TRACE("%S, %dbytes sent\n", pFile->GetFileName().GetBuffer(), *m_pUlBytesTransferred);
			}
			else {
				(*m_pUlFailureCount)++;
			}

			if (pFile) {
				if (Read(pFile, m_buffers[i], m_uLen) > 0) {
					m_contexts[i] = m_pEndPt->BeginDataXfer(m_buffers[i], m_uLen, &m_ovLap[i]);
					if (m_pEndPt->NtStatus || m_pEndPt->UsbdStatus) (*m_pUlBeginDataXferErrCount)++;
				}
				else {
					//EOF이면 더이상 읽을 필요없고 따라서 BeginDataXfer를 호출할 필요도 없다
				}
			}
			else {
				m_contexts[i] = m_pEndPt->BeginDataXfer(m_buffers[i], m_uLen, &m_ovLap[i]);
				if (m_pEndPt->NtStatus || m_pEndPt->UsbdStatus) (*m_pUlBeginDataXferErrCount)++;
			}

			if (m_fileInfo.size_>0 && (*m_pUlBytesTransferred >= (m_fileInfo.size_ + m_uLen))) {
				m_bStart = FALSE;
				pFile->Close();
				break;
			}

			if (i == (nQueueSize - 1)) {	//큐의 맨마지막 요소
				stats();
				if (!m_bStart) break;	//종료 명령(m_bStart==FALSE)이 도착했고, 큐의 맨마지막 요소까지 처리하고 났으면 for루프를 탈출
			}
		}
	}
#ifdef DEBUG
	dumpBody.Close();
	dump.Close();
#endif

	for (int i = 0; i < nQueueSize; i++) {
		if (pFile == NULL) m_pEndPt->FinishDataXfer(m_buffers[i], rLen, &m_ovLap[i], m_contexts[i]);
	}
}

int CXferBulkOut::adjustQueueSize(CFile *pFile)
{
	DWORD fileSize = GetFileSize(pFile->m_hFile, NULL);

	if (fileSize < (m_uLen*m_nQueueSize)) {
		//맨처음 큐요소 : 파일사이즈등의 정보 전송, 큐에는 두번째 이후부터 파일로 부터 읽은 데이터가 들어감.
		//파일 전송의 경우 정상적으로 파일이 수신되려면 전송시 큐는 최소 2개 이상이어야 함
		//이는 본 프로그램에서 파일을 전송하고 수신하는 규칙을 정의한 것에 따라 생긴 제약임
		return 2;
	}
	return m_nQueueSize;
}