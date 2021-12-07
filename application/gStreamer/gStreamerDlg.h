
// gStreamerDlg.h : ��� ����
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
#define MAX_KBPS				625				//625MBps, FX3�� Max 5G bps�̹Ƿ� ����Ʈ�����δ� 5/8 = 0.625 GBps = 625 MBps

// CgStreamerDlg ��ȭ ����
class CgStreamerDlg : public CDialogEx
{
// �����Դϴ�.
public:
	CgStreamerDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GSTREAMER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �����Դϴ�.

	class CEndPointInfo {
	public:
		CEndPointInfo() :m_alt(-1), m_addr(-1) {}
		int m_alt;
		long m_addr;
	};

// �����Դϴ�.
protected:
	HICON m_hIcon;
	CCyUSBDevice *m_pUsbDev;

	// ������ �޽��� �� �Լ�
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnThreadTerminated(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnFileReceived(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDataSent(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnFileSent(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAllFilesReceived(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnFirstHeader(WPARAM wParam, LPARAM lParam);
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
	double m_curKBps;
	CStatic m_fileSelect;
	CButton m_fileSelectBtn;
	afx_msg void OnBnClickedFileSelectButton();
	CString m_strFileName;
	static UINT XferBulk(LPVOID pParam);
	CXferBulk *m_pXfer;
	CList<CString> m_fileList;
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	BOOL m_bPnP_Arrival;
	BOOL m_bPnP_Removal;
	BOOL m_bPnP_DevNodeChange;
	void ResetEndPoint();
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnMainmenuClearlog();
};