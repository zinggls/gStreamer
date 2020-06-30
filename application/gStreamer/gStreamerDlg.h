
// gStreamerDlg.h : 헤더 파일
//

#pragma once
#include "afxwin.h"

class CCyUSBDevice;
class CCyUSBEndPoint;

#define MAX_TRANSFER_LENGTH		0x400000		//4MByte

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
};
