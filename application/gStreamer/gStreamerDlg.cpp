
// gStreamerDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "gStreamer.h"
#include "gStreamerDlg.h"
#include "afxdialogex.h"
#include <CyAPI.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CgStreamerDlg 대화 상자



CgStreamerDlg::CgStreamerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_GSTREAMER_DIALOG, pParent), m_pEndPt(NULL), m_ppxComboIndex(-1)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CgStreamerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LOG_LIST, m_log);
	DDX_Control(pDX, IDC_DEVICE_COMBO, m_deviceCombo);
	DDX_Control(pDX, IDC_ENDPOINT_COMBO, m_endpointCombo);
	DDX_Control(pDX, IDC_PPX_COMBO, m_ppxCombo);
	DDX_Control(pDX, IDC_QUEUE_COMBO, m_queueCombo);
	DDX_Control(pDX, IDC_START_BUTTON, m_startButton);
}

BEGIN_MESSAGE_MAP(CgStreamerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_CBN_SELCHANGE(IDC_DEVICE_COMBO, &CgStreamerDlg::OnCbnSelchangeDeviceCombo)
	ON_CBN_SELCHANGE(IDC_ENDPOINT_COMBO, &CgStreamerDlg::OnCbnSelchangeEndpointCombo)
	ON_CBN_SELCHANGE(IDC_PPX_COMBO, &CgStreamerDlg::OnCbnSelchangePpxCombo)
	ON_BN_CLICKED(IDC_LOG_CLEAR_BUTTON, &CgStreamerDlg::OnBnClickedLogClearButton)
END_MESSAGE_MAP()


// CgStreamerDlg 메시지 처리기

BOOL CgStreamerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	m_log.AddString(_T("Initializing..."));

	int ppxValues[] = { 1,2,4,8,16,32,64,128,256,512 };
	for (int i = 0; i < sizeof(ppxValues) / sizeof(int); i++) {
		CString strPpxVal;
		strPpxVal.Format(_T("%d"),ppxValues[i]);
		m_ppxCombo.AddString(strPpxVal);
	}
	m_ppxComboIndex = 5;	//5 default PPX index, which is 32
	m_ppxCombo.SetCurSel(m_ppxComboIndex);

	int queueValues[] = { 1,2,4,8,16,32,64 };
	for (int i = 0; i < sizeof(queueValues) / sizeof(int); i++) {
		CString strQueueVal;
		strQueueVal.Format(_T("%d"), queueValues[i]);
		m_queueCombo.AddString(strQueueVal);
	}
	m_queueCombo.SetCurSel(4);	//4 default Queue index, which is 16

	CString errMsg;
	GetStreamerDevice(errMsg)==FALSE ? m_log.AddString(errMsg):m_log.AddString(_T("streamer device ok"));

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CgStreamerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CgStreamerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CgStreamerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CgStreamerDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	delete m_pUsbDev;
}


BOOL CgStreamerDlg::GetStreamerDevice(CString &errMsg)
{
	m_pUsbDev = new CCyUSBDevice(this->m_hWnd, CYUSBDRV_GUID, true);
	if (m_pUsbDev == NULL) {
		errMsg = _T("Can't get USB Device instance");
		return FALSE;
	}

	m_deviceCombo.ResetContent();
	m_deviceCombo.EnableWindow(FALSE);
	int devCnt = m_pUsbDev->DeviceCount();
	for (int i = 0; i < devCnt; i++) {
		m_pUsbDev->Open(i);
		CString strDev;
		strDev.Format(_T("(0x%04X - 0x%04X) %s"),m_pUsbDev->VendorID,m_pUsbDev->ProductID,CString(m_pUsbDev->FriendlyName).GetBuffer());
		m_log.AddString(strDev);
		m_deviceCombo.AddString(strDev);
	}
	if (devCnt > 0) {
		m_deviceCombo.SetCurSel(0);
		GetEndPoints(0);
		m_deviceCombo.EnableWindow(TRUE);
	}
	else {
		m_log.AddString(_T("No device found"));
		m_startButton.EnableWindow(FALSE);
	}
	return TRUE;
}


BOOL CgStreamerDlg::GetEndPoints(int nSelect)
{
	ASSERT(m_pUsbDev);
	if (nSelect < 0) return FALSE;

	if(m_pUsbDev->IsOpen()==true) m_pUsbDev->Close();
	if (!m_pUsbDev->Open(nSelect)) return FALSE;

	int interfaces = m_pUsbDev->AltIntfcCount() + 1;

	m_endpointCombo.ResetContent();
	for (int i = 0; i < interfaces; i++) {
		if (m_pUsbDev->SetAltIntfc(i) == true) {

			// Fill the EndPointsBox
			for (int j = 1; j < m_pUsbDev->EndPointCount(); j++) {
				CCyUSBEndPoint *ept = m_pUsbDev->EndPoints[j];

				// INTR, BULK and ISO endpoints are supported.
				if ((ept->Attributes >= 1) && (ept->Attributes <= 3)) {
					CString strEpt;
					strEpt += (AttributesToString(ept->Attributes) + _T(" "));
					strEpt += (BinToString(ept->bIn) + _T(", "));
					strEpt += (MaxPktSizeToString(ept->MaxPktSize) + _T(" Bytes, "));

					if (m_pUsbDev->BcdUSB == USB30MAJORVER) 
						strEpt += (ssmaxburstToString(ept->ssmaxburst) + _T(" MaxBurst, "));

					strEpt += ((_T("(") + interfaceToString(i)) + _T(" - "));
					strEpt += (AddressToString(ept->Address) + _T(")"));

					m_endpointCombo.AddString(strEpt);

					CString order;
					order.Format(_T("[%d] "),j);
					m_log.AddString(order+strEpt);
				}
			}
		}
	}

	if (m_endpointCombo.GetCount() > 0) {
		m_endpointCombo.SetCurSel(0);
		OnCbnSelchangeEndpointCombo();
		OnCbnSelchangePpxCombo();
		m_endpointCombo.EnableWindow(TRUE);
	}
	else {
		m_log.AddString(_T("No EndPoint found"));
		m_startButton.EnableWindow(FALSE);
	}
	return TRUE;
}


void CgStreamerDlg::OnCbnSelchangeDeviceCombo()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	GetEndPoints(m_deviceCombo.GetCurSel());
}


CString CgStreamerDlg::AttributesToString(UCHAR attributes)
{
	if (attributes == 0)
		return CString(_T("CONT"));
	else if (attributes == 1)
		return CString(_T("ISOC"));
	else if (attributes == 2)
		return CString(_T("BULK"));
	else if (attributes == 3)
		return CString(_T("INTR"));
	else
		ASSERT(FALSE);	//Not defined. Never should be here

	return CString(_T(""));	//Just to make compiler happy
}


CString CgStreamerDlg::BinToString(bool bIn)
{
	if (bIn) return CString(_T("IN   ")); else return CString(_T("OUT"));
}

CString CgStreamerDlg::MaxPktSizeToString(USHORT MaxPktSize)
{
	CString size;
	size.Format(_T("%u"), MaxPktSize);
	return size;
}

CString CgStreamerDlg::ssmaxburstToString(UCHAR ssmaxburst)
{
	CString burst;
	burst.Format(_T("%d"), ssmaxburst);
	return burst;
}

CString CgStreamerDlg::interfaceToString(int iface)
{
	CString ifaceStr;
	ifaceStr.Format(_T("%d"), iface);
	return ifaceStr;
}

CString CgStreamerDlg::AddressToString(UCHAR address)
{
	CString addrStr;
	addrStr.Format(_T("0x%02X"), address);
	return addrStr;
}

void CgStreamerDlg::OnCbnSelchangeEndpointCombo()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	CString selStr;
	m_endpointCombo.GetLBText(m_endpointCombo.GetCurSel(), selStr);

	CgStreamerDlg::CEndPointInfo info;
	if (getEndPointInfo(selStr, info) == FALSE) {
		m_log.AddString(_T("getEndPointInfo failed"));
		return;
	}

	if (!m_pUsbDev->SetAltIntfc(info.m_alt)) {
		CString str;
		str.Format(_T("SetAltIntfc failed with alt:%d"),info.m_alt);
		m_log.AddString(str);
		return;
	}

	m_pEndPt = m_pUsbDev->EndPointOf((UCHAR)info.m_addr);
}

BOOL CgStreamerDlg::getEndPointInfo(CString strCombo, CgStreamerDlg::CEndPointInfo &info)
{
	int nLoc = strCombo.Find(_T("("));
	if (nLoc < 0) return FALSE;

	CString alt = strCombo.Mid(nLoc+1, 1);
	CString addr = strCombo.Mid(nLoc + 7, 2);

	info.m_alt = _ttoi(alt.GetBuffer());
	info.m_addr = _tcstol(addr,NULL,16);	//Hex(16) to Dec(10)
	return TRUE;
}

void CgStreamerDlg::OnCbnSelchangePpxCombo()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (m_pEndPt == NULL) {
		m_log.AddString(_T("EndPointer is NULL"));
		m_startButton.EnableWindow(FALSE);
		return;
	}

	if (m_pEndPt->MaxPktSize == 0) {
		m_log.AddString(_T("MaxPktSize is 0"));
		m_startButton.EnableWindow(FALSE);
		return;
	}

	CString strRtn = checkPpxValidity();
	if (strRtn == CString(_T(""))) {
		m_ppxComboIndex = m_ppxCombo.GetCurSel();	//ppx검증이 성공하여 선택된 콤보값으로 m_ppxComboIndex 값을 업데이트
		m_log.AddString(_T("ppx validated ok"));
		m_startButton.EnableWindow(TRUE);
	}
	else {
		m_log.AddString(strRtn);
		m_ppxCombo.SetCurSel(m_ppxComboIndex);	//ppx검증이 실패하였으므로 선택된 콤보값을 이전값으로 되돌림
		m_log.AddString(_T("Chosen ppx is invalid, restore ppx value"));
		m_startButton.EnableWindow(FALSE);
	}
}

CString CgStreamerDlg::checkPpxValidity()
{
	CString strPpx;
	m_ppxCombo.GetLBText(m_ppxCombo.GetCurSel(), strPpx);
	if (!checkMaxTransferLimit(m_pEndPt->MaxPktSize, _ttoi(strPpx))) {
		CString err;
		err.Format(_T("Total Xfer length limited to 4Mbyte, Lower PPX value:"));
		return (err+ strPpx);
	}

	CString err;
	if (!checkIsoPpxLimit(_ttoi(strPpx),err)) {
		return err;
	}
	return _T("");	//PPX 검증 결과 이상무
}

BOOL CgStreamerDlg::checkMaxTransferLimit(USHORT MaxPktSize, int ppx)
{
	if ( (MaxPktSize*ppx) > MAX_TRANSFER_LENGTH) return FALSE;
	return TRUE;
}

BOOL CgStreamerDlg::checkIsoPpxLimit(int ppx, CString& strErr)
{
	ASSERT(m_pUsbDev != NULL);
	ASSERT(m_pEndPt != NULL);	//이 함수는 m_pEndPt가 NULL이 아닌 경우에만 호출될 수 있다

	// HS/SS ISOC Xfers must use PPX >= 8
	if ((m_pUsbDev->bSuperSpeed || m_pUsbDev->bHighSpeed) && (m_pEndPt->Attributes == 1)) {
		if (ppx < 8) {
			strErr.Format(_T("Highspeed or Superspeed ISOC xfers require at least 8 Packets per Xfer"));
			return FALSE;
		}

		if (m_pUsbDev->bHighSpeed) {
			if (ppx >128)
			{
				strErr.Format(_T("Hish Speed ISOC xfers does not support more than 128 Packets per transfer"));
				return FALSE;
			}
		}
	}
	return TRUE;
}

void CgStreamerDlg::OnBnClickedLogClearButton()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_log.ResetContent();
}
