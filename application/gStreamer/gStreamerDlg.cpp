
// gStreamerDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "gStreamer.h"
#include "gStreamerDlg.h"
#include "afxdialogex.h"
#include <CyAPI.h>
#include "XferBulkIn.h"
#include "XferBulkOut.h"
#include <dbt.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define COUNT_REFRESH_TIMER				1
#define COUNT_REFRESH_TIMER_INTERVAL	10

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
	: CDialogEx(IDD_GSTREAMER_DIALOG, pParent), m_pEndPt(NULL), m_ppxComboIndex(-1), m_pThread(NULL), m_nQueueSize(0), m_nPPX(0)
	, m_ulSuccessCount(0)
	, m_ulFailureCount(0)
	, m_ulBeginDataXferErrCount(0)
	, m_KBps(_T(""))
	, m_strFileName(_T(""))
	, m_pXfer(NULL)
	, m_bPnP_Arrival(FALSE),m_bPnP_Removal(FALSE),m_bPnP_DevNodeChange(FALSE)
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
	DDX_Text(pDX, IDC_SUCCESS_COUNT_EDIT, m_ulSuccessCount);
	DDX_Text(pDX, IDC_FAILURE_COUNT_EDIT, m_ulFailureCount);
	DDX_Text(pDX, IDC_BEGINDATAXFER_ERROR_COUNT_EDIT, m_ulBeginDataXferErrCount);
	DDX_Control(pDX, IDC_KBPS_PROGRESS1, m_transferRate);
	DDX_Text(pDX, IDC_KBPS_STATIC, m_KBps);
	DDX_Control(pDX, IDC_FILE_SELECT_STATIC, m_fileSelect);
	DDX_Control(pDX, IDC_FILE_SELECT_BUTTON, m_fileSelectBtn);
	DDX_Text(pDX, IDC_FILENAME_EDIT, m_strFileName);
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
	ON_BN_CLICKED(IDC_START_BUTTON, &CgStreamerDlg::OnBnClickedStartButton)
	ON_CBN_SELCHANGE(IDC_QUEUE_COMBO, &CgStreamerDlg::OnCbnSelchangeQueueCombo)
	ON_WM_TIMER()
	ON_MESSAGE(WM_THREAD_TERMINATED, &CgStreamerDlg::OnThreadTerminated)
	ON_MESSAGE(WM_END_OF_FILE, &CgStreamerDlg::OnEndOfFile)
	ON_MESSAGE(WM_SYNC_FOUND, &CgStreamerDlg::OnSyncFound)
	ON_MESSAGE(WM_FILE_RECEIVED, &CgStreamerDlg::OnFileReceived)
	ON_MESSAGE(WM_DATA_SENT, &CgStreamerDlg::OnDataSent)
	ON_MESSAGE(WM_DATA_RECEIVED, &CgStreamerDlg::OnDataReceived)
	ON_MESSAGE(WM_FILE_SENT, &CgStreamerDlg::OnFileSent)
	ON_BN_CLICKED(IDC_FILE_SELECT_BUTTON, &CgStreamerDlg::OnBnClickedFileSelectButton)
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
	L(_T("Initializing..."));

	int ppxValues[] = { 1,2,4,8,16,32,64,128,256,512 };
	for (int i = 0; i < sizeof(ppxValues) / sizeof(int); i++) {
		CString strPpxVal;
		strPpxVal.Format(_T("%d"),ppxValues[i]);
		m_ppxCombo.AddString(strPpxVal);
	}
	m_ppxComboIndex = 5;	//5 default PPX index, which is 32
	m_ppxCombo.SetCurSel(m_ppxComboIndex);

	CString strPpx;
	m_ppxCombo.GetLBText(m_ppxComboIndex, strPpx);
	m_nPPX = _ttoi(strPpx);

	int queueValues[] = { 1,2,4,8,16,32,64 };
	for (int i = 0; i < sizeof(queueValues) / sizeof(int); i++) {
		CString strQueueVal;
		strQueueVal.Format(_T("%d"), queueValues[i]);
		m_queueCombo.AddString(strQueueVal);
	}
	m_queueCombo.SetCurSel(4);	//4 default Queue index, which is 16
	OnCbnSelchangeQueueCombo();

	CString errMsg;
	GetStreamerDevice(errMsg)==FALSE ? L(errMsg):L(_T("streamer device ok"));

	m_bStart = FALSE;
	m_transferRate.SetRange(0, (short)MAX_KBPS);
	GetDlgItem(IDC_SUCCESS_COUNT_EDIT)->EnableWindow(FALSE);
	GetDlgItem(IDC_FAILURE_COUNT_EDIT)->EnableWindow(FALSE);
	GetDlgItem(IDC_BEGINDATAXFER_ERROR_COUNT_EDIT)->EnableWindow(FALSE);
	m_fileSelect.ShowWindow(SW_HIDE);
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
	terminateThread();
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
		L(strDev);
		m_deviceCombo.AddString(strDev);
	}
	if (devCnt > 0) {
		m_deviceCombo.SetCurSel(0);
		GetEndPoints(0);
		m_deviceCombo.EnableWindow(TRUE);
	}
	else {
		ResetEndPoint();
		errMsg = _T("No device found");
		m_startButton.EnableWindow(FALSE);
		return FALSE;
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

	ResetEndPoint();
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
					L(order+strEpt);
				}
			}
		}
	}

	if (m_endpointCombo.GetCount() > 0) {
		m_endpointCombo.SetCurSel(0);
		OnCbnSelchangeEndpointCombo();
		m_endpointCombo.EnableWindow(TRUE);
	}
	else {
		m_fileSelectBtn.ShowWindow(SW_HIDE);
		GetDlgItem(IDC_FILENAME_EDIT)->ShowWindow(SW_HIDE);
		UpdateData(FALSE);
		L(_T("No EndPoint found"));
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
		L(_T("getEndPointInfo failed"));
		return;
	}

	if (!m_pUsbDev->SetAltIntfc(info.m_alt)) {
		L(_T("SetAltIntfc failed with alt:%d"), info.m_alt);
		return;
	}

	m_pEndPt = m_pUsbDev->EndPointOf((UCHAR)info.m_addr);
	if (m_pEndPt->Attributes == 2) { //BULK
		if (!m_pEndPt->bIn) {
			m_strFileName = _T("");
			m_fileSelectBtn.ShowWindow(SW_SHOW);
			m_fileSelect.ShowWindow(SW_SHOW);
			GetDlgItem(IDC_FILENAME_EDIT)->ShowWindow(SW_SHOW);
		} else {
			m_strFileName = _T("");
			m_fileSelectBtn.ShowWindow(SW_HIDE);
			GetDlgItem(IDC_FILENAME_EDIT)->ShowWindow(SW_HIDE);
			m_fileSelect.ShowWindow(SW_HIDE);
		}
	}else{
		m_fileSelectBtn.ShowWindow(SW_HIDE);
		GetDlgItem(IDC_FILENAME_EDIT)->ShowWindow(SW_HIDE);
		m_fileSelect.ShowWindow(SW_HIDE);
	}
	UpdateData(FALSE);
	OnCbnSelchangePpxCombo();
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
		L(_T("EndPointer is NULL"));
		m_startButton.EnableWindow(FALSE);
		return;
	}

	if (m_pEndPt->MaxPktSize == 0) {
		L(_T("MaxPktSize is 0"));
		m_startButton.EnableWindow(FALSE);
		return;
	}

	CString strRtn = checkPpxValidity();
	if (strRtn == CString(_T(""))) {
		m_ppxComboIndex = m_ppxCombo.GetCurSel();	//ppx검증이 성공하여 선택된 콤보값으로 m_ppxComboIndex 값을 업데이트

		CString strPpx;
		m_ppxCombo.GetLBText(m_ppxComboIndex, strPpx);
		m_nPPX = _ttoi(strPpx);

		L(_T("ppx(%d) validated ok"), m_nPPX);
		m_startButton.EnableWindow(TRUE);
	}
	else {
		L(strRtn);

		CString strPpx;
		m_ppxCombo.GetLBText(m_ppxCombo.GetCurSel(), strPpx);

		m_ppxCombo.SetCurSel(m_ppxComboIndex);	//ppx검증이 실패하였으므로 선택된 콤보값을 이전값으로 되돌림

		L(_T("Chosen ppx(%d) is invalid, restore ppx value"), _ttoi(strPpx));
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
	m_transferRate.SetPos(0);
	m_KBps.Empty();
	m_ulSuccessCount = m_ulFailureCount = m_ulBeginDataXferErrCount = 0;
	UpdateData(FALSE);
}


void CgStreamerDlg::OnBnClickedStartButton()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (!m_bStart) {
		m_bStart = TRUE;
		adjustQueueSize();
		m_pThread = AfxBeginThread(XferBulk, this);
		if (!m_pThread) {
			m_bStart = FALSE;
			L(_T("Failure in creating thread"));
			return;
		}
		m_startButton.SetWindowTextW(_T("Stop"));
		SetTimer(COUNT_REFRESH_TIMER, COUNT_REFRESH_TIMER_INTERVAL, NULL);
		GetDlgItem(IDC_DEVICE_COMBO)->EnableWindow(FALSE);
		GetDlgItem(IDC_ENDPOINT_COMBO)->EnableWindow(FALSE);
		GetDlgItem(IDC_PPX_COMBO)->EnableWindow(FALSE);
		GetDlgItem(IDC_QUEUE_COMBO)->EnableWindow(FALSE);
		GetDlgItem(IDC_FILE_SELECT_BUTTON)->EnableWindow(FALSE);
		GetDlgItem(IDC_FILENAME_EDIT)->EnableWindow(FALSE);
		L(_T("Xfer thread started"));
	}
	else {
		terminateThread();
		GetDlgItem(IDC_DEVICE_COMBO)->EnableWindow(TRUE);
		GetDlgItem(IDC_ENDPOINT_COMBO)->EnableWindow(TRUE);
		GetDlgItem(IDC_PPX_COMBO)->EnableWindow(TRUE);
		GetDlgItem(IDC_QUEUE_COMBO)->EnableWindow(TRUE);
		GetDlgItem(IDC_FILE_SELECT_BUTTON)->EnableWindow(TRUE);
		GetDlgItem(IDC_FILENAME_EDIT)->EnableWindow(TRUE);
	}
}

void CgStreamerDlg::OnCbnSelchangeQueueCombo()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString strQ;
	m_queueCombo.GetLBText(m_queueCombo.GetCurSel(), strQ);
	m_nQueueSize = _ttoi(strQ);
}

void CgStreamerDlg::L(const TCHAR* str, ...)
{
	if (m_log.GetCount() >= MAX_LOG) {
		m_log.AddString(_T("Log >= MAX_LOG, Reset log"));
		m_log.ResetContent();
	}

	va_list args;
	va_start(args, str);

	int len = _vsctprintf(str, args)+1;	//_vscprintf doesn't count terminating '\0'
	TCHAR* buffer = new TCHAR[len];
	_vstprintf(buffer,len, str, args);
	va_end(args);

	m_log.AddString(buffer);
	delete[](buffer);

	m_log.SetTopIndex(m_log.GetCount() - 1);
}

void CgStreamerDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (nIDEvent == COUNT_REFRESH_TIMER) {
		m_transferRate.SetPos((short)(m_curKBps/1000));	//KBps에서 MBps단위로 변환
		m_KBps.Format(_T("%.0f KBps"), m_curKBps);
		UpdateData(FALSE);
	}

	CDialogEx::OnTimer(nIDEvent);
}

LRESULT CgStreamerDlg::OnThreadTerminated(WPARAM wParam, LPARAM lParam)
{
	m_bStart = FALSE;
	m_curKBps = 0.0;
	m_startButton.SetWindowTextW(_T("Start"));
	GetDlgItem(IDC_DEVICE_COMBO)->EnableWindow(TRUE);
	GetDlgItem(IDC_ENDPOINT_COMBO)->EnableWindow(TRUE);
	GetDlgItem(IDC_PPX_COMBO)->EnableWindow(TRUE);
	GetDlgItem(IDC_QUEUE_COMBO)->EnableWindow(TRUE);
	GetDlgItem(IDC_FILE_SELECT_BUTTON)->EnableWindow(TRUE);
	GetDlgItem(IDC_FILENAME_EDIT)->EnableWindow(TRUE);
	KillTimer(COUNT_REFRESH_TIMER);
	UpdateData(FALSE);
	L(_T("Xfer thread terminated"));
	return 0;
}

void CgStreamerDlg::terminateThread()
{
	if (!m_pXfer) return;

	L(_T("Xfer thread terminating..."));
	m_bStart = FALSE;
	m_pXfer->m_bStart = m_bStart;
	for (int i = 0; i < m_nQueueSize; i++) SetEvent(m_inOvLap[i].hEvent);
	m_pXfer->sendEvent();
	if (m_pThread) WaitForSingleObject(m_pThread->m_hThread, INFINITE);
}

void CgStreamerDlg::showStats()
{
	clock_t curTime = clock();
	double elapsed = ((double)(curTime - m_startTime)) / CLOCKS_PER_SEC;
	//TRACE("수행 시간 : %f\n", elapsed);

	m_curKBps = ((double)m_ulBytesTransferred / elapsed) / 1024.;
	m_KBps.Format(_T("%.0f KBps"),m_curKBps);
}

void CgStreamerDlg::OnBnClickedFileSelectButton()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	ASSERT(m_pEndPt);
	ASSERT(m_pEndPt->Attributes==2);	//편의상 현시점에서는 BULK만 지원하도록 구현, isochronous도 지원해야 함
	ASSERT(m_pEndPt->bIn == FALSE);		//BULK OUT경우에만 파일을 선택할수 있다.

	TCHAR szFilter[] = _T("All Files(*.*)|*.*||");
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT, szFilter);
	if (IDOK == dlg.DoModal()) {
		m_fileList.RemoveAll();
		UINT nCount = 1;
		POSITION pos = dlg.GetStartPosition();
		while (pos != NULL) {
			CString str = dlg.GetNextPathName(pos);
			m_fileList.AddTail(str);
			L(_T("[%d] File read from: %s"), nCount,str);
			nCount++;
		}
		m_strFileName = m_fileList.GetAt(m_fileList.GetHeadPosition());
	}else{
		m_fileList.RemoveAll();
		m_strFileName.Empty();
	}
	UpdateData(FALSE);
}

LRESULT CgStreamerDlg::OnEndOfFile(WPARAM wParam, LPARAM lParam)
{
	L(_T("End of file reached"));
	return 0;
}

LRESULT CgStreamerDlg::OnSyncFound(WPARAM wParam, LPARAM lParam)
{
	FILEINFO *pFileInfo = (FILEINFO*)wParam;
	CString name(pFileInfo->name_);
	L(_T("Sync found,FileName=")+name);
	return 0;
}

LRESULT CgStreamerDlg::OnFileReceived(WPARAM wParam, LPARAM lParam)
{
	ASSERT(wParam);

	CString str;
	str.Format(_T("%s(%d bytes), %d bytes received"), ((FILEINFO*)wParam)->name_, ((FILEINFO*)wParam)->size_,(ULONGLONG)lParam);
	L(str);
	return 0;
}

void CgStreamerDlg::adjustQueueSize()
{
	ASSERT(m_pEndPt);
	if (m_pEndPt->bIn) return;	//IN의 경우는 조정 필요없음

	ULONG len = m_pEndPt->MaxPktSize * m_nPPX;

	if (!m_strFileName.IsEmpty()) {
		CFile f(m_strFileName, CFile::modeRead | CFile::typeBinary);
		DWORD fileSize = GetFileSize(f.m_hFile, NULL);
		f.Close();

		if (fileSize < (len*m_nQueueSize)) {
			//맨처음 큐요소 : 파일사이즈등의 정보 전송, 큐에는 두번째 이후부터 파일로 부터 읽은 데이터가 들어감.
			//파일 전송의 경우 정상적으로 파일이 수신되려면 전송시 큐는 최소 2개 이상이어야 함
			//이는 본 프로그램에서 파일을 전송하고 수신하는 규칙을 정의한 것에 따라 생긴 제약임
			int nPrevQueueSize = m_nQueueSize;
			m_nQueueSize = 2;
			L(_T("fileSize(%d) is less then len*queue size(%d), queue size decreased as %d"), fileSize, len*nPrevQueueSize, m_nQueueSize);

			m_queueCombo.SetCurSel(1);	//하드코딩 1은 큐사이즈 m_nQueueSize=2에 해당함
		}
	}
}

UINT CgStreamerDlg::XferBulk(LPVOID pParam)
{
	CgStreamerDlg *pDlg = (CgStreamerDlg*)pParam;
	ASSERT(pDlg);
	CCyUSBEndPoint *pEndPt = pDlg->m_pEndPt;
	ASSERT(pEndPt);
	ASSERT(pEndPt->Attributes == 2);

	pDlg->m_pXfer = NULL;
	if(pEndPt->bIn) {
		pDlg->m_pXfer = new CXferBulkIn();
	} else {
		CXferBulkOut *pBulkOut = new CXferBulkOut();
		pBulkOut->m_pFileList = &pDlg->m_fileList;
		pDlg->m_pXfer = pBulkOut;
	}

	//객체 시작전에 필요한 값들 설정
	pDlg->m_pXfer->m_pEndPt = pDlg->m_pEndPt;
	pDlg->m_pXfer->m_nPPX = pDlg->m_nPPX;
	pDlg->m_pXfer->m_nQueueSize = pDlg->m_nQueueSize;
	pDlg->m_pXfer->m_bStart = pDlg->m_bStart;
	pDlg->m_pXfer->m_hWnd = pDlg->m_hWnd;
	pDlg->m_pXfer->m_pUlSuccessCount = &pDlg->m_ulSuccessCount;
	pDlg->m_pXfer->m_pUlFailureCount = &pDlg->m_ulFailureCount;
	pDlg->m_pXfer->m_pUlBeginDataXferErrCount = &pDlg->m_ulBeginDataXferErrCount;
	pDlg->m_pXfer->m_pUlBytesTransferred = &pDlg->m_ulBytesTransferred;
	pDlg->m_pXfer->m_pCurKBps = &pDlg->m_curKBps;
	pDlg->m_pXfer->m_pStartTime = &pDlg->m_startTime;

	pDlg->m_pXfer->open();
	pDlg->m_pXfer->process();
	pDlg->m_pXfer->close();

	delete pDlg->m_pXfer;
	pDlg->PostMessage(WM_THREAD_TERMINATED);
	return 0;
}

LRESULT CgStreamerDlg::OnDataSent(WPARAM wParam, LPARAM lParam)
{
	return 0;
}

LRESULT CgStreamerDlg::OnDataReceived(WPARAM wParam, LPARAM lParam)
{
	return 0;
}

LRESULT CgStreamerDlg::OnFileSent(WPARAM wParam, LPARAM lParam)
{
	int nIndex = (int)wParam;
	CString str;
	str.Format(_T("[%d] "), nIndex);
	str += m_fileList.GetAt(m_fileList.FindIndex(nIndex));
	L(str+_T(" sent"));
	return 0;
}

LRESULT CgStreamerDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if (message == WM_DEVICECHANGE)
	{
		// Tracks DBT_DEVNODES_CHANGED followed by DBT_DEVICEREMOVECOMPLETE
		if (wParam == DBT_DEVNODES_CHANGED)
		{
			m_bPnP_DevNodeChange = TRUE;
			m_bPnP_Removal = FALSE;
		}

		// Tracks DBT_DEVICEARRIVAL followed by DBT_DEVNODES_CHANGED
		if (wParam == DBT_DEVICEARRIVAL)
		{
			m_bPnP_Arrival = TRUE;
			m_bPnP_DevNodeChange = FALSE;
		}

		CString errMsg;
		if (m_bPnP_DevNodeChange && m_bPnP_Removal) {
			m_bPnP_DevNodeChange = m_bPnP_Removal = FALSE;
			GetStreamerDevice(errMsg) == FALSE ? L(errMsg) : L(_T("streamer device ok"));
		}

		if (m_bPnP_DevNodeChange && m_bPnP_Arrival) {
			m_bPnP_DevNodeChange = m_bPnP_Arrival = FALSE;
			GetStreamerDevice(errMsg) == FALSE ? L(errMsg) : L(_T("streamer device ok"));
		}

		if (wParam == DBT_DEVICEREMOVECOMPLETE) m_bPnP_Removal = TRUE;
	}
	return CDialogEx::WindowProc(message, wParam, lParam);
}

void CgStreamerDlg::ResetEndPoint()
{
	m_endpointCombo.ResetContent();
}