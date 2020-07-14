
// gStreamerDlg.h : 헤더 파일
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "fileInfo.h"
#include "userDefinedMessage.h"

class CCyUSBDevice;
class CCyUSBEndPoint;
class CXferBulk;

#define MAX_TRANSFER_LENGTH		0x400000		//4MByte
#define MAX_QUEUE_SIZE			64
#define MAX_LOG					1000
#define MAX_KBPS				625				//625MBps, FX3는 Max 5G bps이므로 바이트단위로는 5/8 = 0.625 GBps = 625 MBps

// CgStreamerDlg 대화 상자
class CgStreamerDlg : public CDialogEx
{
// 생성입니다.
public:
	CgStreamerDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GSTREAMER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.

	class CEndPointInfo {
	public:
		CEndPointInfo() :m_alt(-1), m_addr(-1) {}
		int m_alt;
		long m_addr;
	};

// 구현입니다.
protected:
	HICON m_hIcon;
	CCyUSBDevice *m_pUsbDev;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnThreadTerminated(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEndOfFile(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSyncFound(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnFileReceived(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDataSent(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDataReceived(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	CListBox m_log;
	BOOL GetStreamerDevice(CString &errMsg);
	CComboBox m_deviceCombo;
	BOOL GetEndPoints(int nSelect);
	afx_msg void OnCbnSelchangeDeviceCombo();
	CString AttributesToString(UCHAR attributes);
	CString BinToString(bool bIn);
	CString MaxPktSizeToString(USHORT MaxPktSize);
	CString ssmaxburstToString(UCHAR ssmaxburst);
	CString interfaceToString(int iface);
	CString AddressToString(UCHAR address);
	CComboBox m_endpointCombo;
	afx_msg void OnCbnSelchangeEndpointCombo();
	BOOL getEndPointInfo(CString strCombo, CEndPointInfo &info);
	CCyUSBEndPoint *m_pEndPt;
	CComboBox m_ppxCombo;
	CComboBox m_queueCombo;
	CButton m_startButton;
	afx_msg void OnCbnSelchangePpxCombo();
	CString checkPpxValidity();
	int m_ppxComboIndex;
	BOOL checkMaxTransferLimit(USHORT MaxPktSize,int ppx);
	BOOL checkIsoPpxLimit(int ppx, CString& strErr);
	afx_msg void OnBnClickedLogClearButton();
	afx_msg void OnBnClickedStartButton();
	CWinThread *m_pThread;
	static UINT Xfer(LPVOID pParam);
	int m_nQueueSize;
	afx_msg void OnCbnSelchangeQueueCombo();
	int m_nPPX;
	void L(const TCHAR* str, ...);
	BOOL m_bStart;
	OVERLAPPED	m_inOvLap[MAX_QUEUE_SIZE];
	ULONGLONG m_ulSuccessCount;
	ULONGLONG m_ulFailureCount;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	ULONGLONG m_ulBeginDataXferErrCount;
	void terminateThread();
	CProgressCtrl m_transferRate;
	CString m_KBps;
	ULONGLONG m_ulBytesTransferred;
	clock_t m_startTime;
	void showStats();
	double m_curKBps;
	CStatic m_fileSelect;
	CButton m_fileSelectBtn;
	afx_msg void OnBnClickedFileSelectButton();
	CString m_strFileName;
	static BOOL fullRead(CFile *pFile, UCHAR *buffer, UINT nCount, BOOL bSeekToBegin, BOOL bPostEofMsg, HWND hWnd);
	static CFile* GetFile(CString pathFileName, FILEINFO &fileInfo);
	static int SetFileInfo(UCHAR *buffer, ULONG bufferSize, BYTE *sync, int syncSize, FILEINFO &info);
	static UINT Read(CFile *pFile, UCHAR *buffer, UINT nCount);
	static int GetFileInfo(UCHAR *buffer, ULONG bufferSize, int syncSize, FILEINFO &info);
	static BYTE sync[4];
	void adjustQueueSize();
	static UINT XferBulkOut(LPVOID pParam);
	static UINT XferBulkIn(LPVOID pParam);
	static UINT XferBulk(LPVOID pParam);
	CXferBulk *m_pXfer;
};