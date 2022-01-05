
// gStreamerDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "gStreamer.h"
#include "gStreamerDlg.h"
#include "afxdialogex.h"
#include "XferBulkIn.h"
#include "XferBulkOut.h"
#include <dbt.h>
#include "OScopeCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define COUNT_REFRESH_TIMER				1
#define COUNT_REFRESH_TIMER_INTERVAL	10
#define DATARATE_TIMER					2
#define DATARATE_TIMER_INTERVAL			100

// ���� ���α׷� ������ ���Ǵ� CAboutDlg ��ȭ �����Դϴ�.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

// �����Դϴ�.
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


// CgStreamerDlg ��ȭ ����

CgStreamerDlg::CgStreamerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_GSTREAMER_DIALOG, pParent), m_pUsbDev(NULL), m_pEndPt(NULL), m_ppxComboIndex(-1), m_pThread(NULL), m_nQueueSize(0), m_nPPX(0)
	, m_ulSuccessCount(0)
	, m_ulFailureCount(0)
	, m_ulBeginDataXferErrCount(0)
	, m_KBps(_T(""))
	, m_strFileName(_T(""))
	, m_pXfer(NULL)
	, m_bPnP_Arrival(FALSE),m_bPnP_Removal(FALSE),m_bPnP_DevNodeChange(FALSE)
	, m_pGraph(NULL)
	, m_strSpeed(_T(""))
	, m_bReset(FALSE)
{
	memset(&m_Prev, 0, sizeof(ByteSec));
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
	DDX_Text(pDX, IDC_REALTIME_SPEED_STATIC, m_strSpeed);
	DDX_Control(pDX, IDC_ZING_MODE_COMBO, m_zingModeCombo);
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
	ON_MESSAGE(WM_FILE_RECEIVED, &CgStreamerDlg::OnFileReceived)
	ON_MESSAGE(WM_DATA_SENT, &CgStreamerDlg::OnDataSent)
	ON_MESSAGE(WM_FILE_SENT, &CgStreamerDlg::OnFileSent)
	ON_MESSAGE(WM_ALL_FILES_RECEIVED, &CgStreamerDlg::OnAllFilesReceived)
	ON_MESSAGE(WM_FIRST_HEADER, &CgStreamerDlg::OnFirstHeader)
	ON_BN_CLICKED(IDC_FILE_SELECT_BUTTON, &CgStreamerDlg::OnBnClickedFileSelectButton)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_MAINMENU_CLEARLOG, &CgStreamerDlg::OnMainmenuClearlog)
END_MESSAGE_MAP()


// CgStreamerDlg �޽��� ó����

BOOL CgStreamerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// �ý��� �޴��� "����..." �޴� �׸��� �߰��մϴ�.

	// IDM_ABOUTBOX�� �ý��� ��� ������ �־�� �մϴ�.
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

	// �� ��ȭ ������ �������� �����մϴ�.  ���� ���α׷��� �� â�� ��ȭ ���ڰ� �ƴ� ��쿡��
	//  �����ӿ�ũ�� �� �۾��� �ڵ����� �����մϴ�.
	SetIcon(m_hIcon, TRUE);			// ū �������� �����մϴ�.
	SetIcon(m_hIcon, FALSE);		// ���� �������� �����մϴ�.

	// TODO: ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
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

	m_zingModeCombo.AddString(_T("DEV"));
	m_zingModeCombo.AddString(_T("PPC"));
	GetDlgItem(IDC_ZING_MODE_COMBO)->EnableWindow(FALSE);

	CString errMsg;
	GetStreamerDevice(errMsg)==FALSE ? L(errMsg):L(_T("streamer device ok"));

	m_bStart = FALSE;
	m_transferRate.SetRange(0, (short)MAX_KBPS);
	GetDlgItem(IDC_SUCCESS_COUNT_EDIT)->EnableWindow(FALSE);
	GetDlgItem(IDC_FAILURE_COUNT_EDIT)->EnableWindow(FALSE);
	GetDlgItem(IDC_BEGINDATAXFER_ERROR_COUNT_EDIT)->EnableWindow(FALSE);
	m_fileSelect.ShowWindow(SW_HIDE);

	CRect rtGraph;
	GetDlgItem(IDC_GRAPH_STATIC)->GetWindowRect(rtGraph);
	ScreenToClient(rtGraph);

	m_pGraph = new COScopeCtrl(1);
	m_pGraph->Create(WS_VISIBLE | WS_CHILD, rtGraph, this, IDC_GRAPH_STATIC);
	m_pGraph->SetRanges(0,2.5);
	m_pGraph->autofitYscale = true;
	m_pGraph->SetYUnits(_T("Gbps"));
	m_pGraph->SetLegendLabel(_T("Realtime Transfer Speed"), 0);
	m_pGraph->SetPlotColor(RGB(255, 0, 0), 0);
	m_pGraph->InvalidateCtrl();
	GetDlgItem(IDC_KBPS_PROGRESS1)->ShowWindow(SW_HIDE);
	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
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

// ��ȭ ���ڿ� �ּ�ȭ ���߸� �߰��� ��� �������� �׸�����
//  �Ʒ� �ڵ尡 �ʿ��մϴ�.  ����/�� ���� ����ϴ� MFC ���� ���α׷��� ��쿡��
//  �����ӿ�ũ���� �� �۾��� �ڵ����� �����մϴ�.

void CgStreamerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �׸��⸦ ���� ����̽� ���ؽ�Ʈ�Դϴ�.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Ŭ���̾�Ʈ �簢������ �������� ����� ����ϴ�.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �������� �׸��ϴ�.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ����ڰ� �ּ�ȭ�� â�� ���� ���ȿ� Ŀ���� ǥ�õǵ��� �ý��ۿ���
//  �� �Լ��� ȣ���մϴ�.
HCURSOR CgStreamerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CgStreamerDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰��մϴ�.
	delete m_pGraph;
	terminateThread();
	delete m_pUsbDev;
}


BOOL CgStreamerDlg::GetStreamerDevice(CString &errMsg)
{
	if (m_pUsbDev) delete m_pUsbDev;
	m_pUsbDev = new CCyUSBDevice(this->m_hWnd, CYUSBDRV_GUID, true);
	if (m_pUsbDev == NULL) {
		errMsg = _T("Can't get USB Device instance");
		return FALSE;
	}
	
	if (!m_bReset) {
		if (m_pUsbDev->ControlEndPt) ResetDevice();
	}
	else {
		if (m_pUsbDev->ControlEndPt) UpdateZingMode();
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
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
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
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.

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
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
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
		m_ppxComboIndex = m_ppxCombo.GetCurSel();	//ppx������ �����Ͽ� ���õ� �޺������� m_ppxComboIndex ���� ������Ʈ

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

		m_ppxCombo.SetCurSel(m_ppxComboIndex);	//ppx������ �����Ͽ����Ƿ� ���õ� �޺����� ���������� �ǵ���

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
	return _T("");	//PPX ���� ��� �̻�
}

BOOL CgStreamerDlg::checkMaxTransferLimit(USHORT MaxPktSize, int ppx)
{
	if ( (MaxPktSize*ppx) > MAX_TRANSFER_LENGTH) return FALSE;
	return TRUE;
}

BOOL CgStreamerDlg::checkIsoPpxLimit(int ppx, CString& strErr)
{
	ASSERT(m_pUsbDev != NULL);
	ASSERT(m_pEndPt != NULL);	//�� �Լ��� m_pEndPt�� NULL�� �ƴ� ��쿡�� ȣ��� �� �ִ�

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
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	m_log.ResetContent();
	m_transferRate.SetPos(0);
	m_KBps.Empty();
	m_ulSuccessCount = m_ulFailureCount = m_ulBeginDataXferErrCount = 0;
	UpdateData(FALSE);
}


void CgStreamerDlg::OnBnClickedStartButton()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	if (!m_bStart) {
		m_bStart = TRUE;
		m_pThread = AfxBeginThread(XferBulk, this);
		if (!m_pThread) {
			m_bStart = FALSE;
			L(_T("Failure in creating thread"));
			return;
		}
		m_startButton.SetWindowTextW(_T("Stop"));
		SetTimer(COUNT_REFRESH_TIMER, COUNT_REFRESH_TIMER_INTERVAL, NULL);
		SetTimer(DATARATE_TIMER, DATARATE_TIMER_INTERVAL, NULL);
		GetDlgItem(IDC_DEVICE_COMBO)->EnableWindow(FALSE);
		GetDlgItem(IDC_ENDPOINT_COMBO)->EnableWindow(FALSE);
		GetDlgItem(IDC_PPX_COMBO)->EnableWindow(FALSE);
		GetDlgItem(IDC_QUEUE_COMBO)->EnableWindow(FALSE);
		GetDlgItem(IDC_FILE_SELECT_BUTTON)->EnableWindow(FALSE);
		GetDlgItem(IDC_FILENAME_EDIT)->EnableWindow(FALSE);
		GetDlgItem(IDC_KBPS_PROGRESS1)->ShowWindow(SW_SHOW);
		L(_T("Xfer thread started"));
	}
	else {
		terminateThread();
		KillTimer(DATARATE_TIMER);
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
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
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
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
	if (nIDEvent == COUNT_REFRESH_TIMER) {
		m_transferRate.SetPos((short)(m_curKBps / 1024));	//KBps���� MBps������ ��ȯ	(1KB = 1024byte, 1MB = 1024KB)
		m_KBps.Format(_T("%.0fKBps(%.2fGbps)"), m_curKBps, 8.*m_curKBps*1024. / 1000000000.);	//��ſ����� ���� 1000byte�� 1KB�� ǥ���Ѵ�. 1000000000�� ������ GBps������ ��ȯ
		UpdateData(FALSE);
	}

	if (nIDEvent == DATARATE_TIMER) {
		if (m_Prev.bytes == 0) {
			if (m_pXfer)
				m_Prev.bytes = *m_pXfer->m_pUlBytesTransferred;
			else
				m_Prev.bytes = 0;
		}

		auto stop = std::chrono::high_resolution_clock::now();

		size_t bytes = 0;
		if(m_pXfer) bytes = *m_pXfer->m_pUlBytesTransferred;
		float sec = std::chrono::duration_cast<std::chrono::milliseconds>(stop - m_Prev.now).count() / 1000.;

		int bps = 8 * (int)BpsVal(bytes - m_Prev.bytes, sec);

		m_Prev.now = stop;
		m_Prev.bytes = bytes;
		TRACE("bps=%d\n",bps);

		double value[1] = { bps/1000000000. };
		m_pGraph->AppendPoints(value);

		m_strSpeed.Format(_T("%.3fGbps"), value[0]);
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

void CgStreamerDlg::OnBnClickedFileSelectButton()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	ASSERT(m_pEndPt);
	ASSERT(m_pEndPt->Attributes==2);	//���ǻ� ������������ BULK�� �����ϵ��� ����, isochronous�� �����ؾ� ��
	ASSERT(m_pEndPt->bIn == FALSE);		//BULK OUT��쿡�� ������ �����Ҽ� �ִ�.

	TCHAR szFilter[] = _T("All Files(*.*)|*.*||");
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT, szFilter);

	CString fileName;
	const int c_cMaxFiles = 1024*24;	//24,576
	const int c_cbBuffSize = ((c_cMaxFiles + 1) * MAX_PATH) + 1;
	dlg.GetOFN().lpstrFile = fileName.GetBuffer(c_cbBuffSize);
	dlg.GetOFN().nMaxFile = c_cbBuffSize;

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
		m_strFileName.Format(_T("<<%d>> file(s) selected"),m_fileList.GetCount());
	}else{
		m_fileList.RemoveAll();
		m_strFileName.Empty();
	}
	UpdateData(FALSE);
}

LRESULT CgStreamerDlg::OnFileReceived(WPARAM wParam, LPARAM lParam)
{
	ASSERT(wParam);

	CString str;
	str.Format(_T("%s(%lu bytes), %lu bytes received"), ((FILEINFO*)wParam)->name_, ((FILEINFO*)wParam)->size_,(ULONGLONG)lParam);
	L(str);
	return 0;
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

	//��ü �������� �ʿ��� ���� ����
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
	pDlg->m_pXfer = NULL;
	pDlg->PostMessage(WM_THREAD_TERMINATED);
	return 0;
}

LRESULT CgStreamerDlg::OnDataSent(WPARAM wParam, LPARAM lParam)
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
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
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

LRESULT CgStreamerDlg::OnAllFilesReceived(WPARAM wParam, LPARAM lParam)
{
	ASSERT(wParam);

	//Bulk out������ OnThreadTerminated���� �Ʒ��� ����� �����Ѵ�.
	//Bulk in������ Stop��ư�� ������ �������� ��� �����ϴ� �����
	//���α׷��� ����ϴµ� �������̰� ���Ͽ�
	//������ ���ᰡ �ƴ� ��� ������ ������ ������ �ʱ�ȭ�� �Ѵ�
	m_curKBps = 0.0;
	KillTimer(COUNT_REFRESH_TIMER);

	CString str;
	str.Format(_T("%d files received"), (int)wParam);
	L(str);
	return 0;
}

LRESULT CgStreamerDlg::OnFirstHeader(WPARAM wParam, LPARAM lParam)
{
	ASSERT(wParam);

	//�� �Լ��� BULK IN�� ��쿡�� ���Ǵ� �Լ�
	//������ ������ �ް� Ÿ�̸Ӱ� ����ȴ�. �ٽ� ��Ʈ���� �����ϴ� ��� Ÿ�̸Ӹ� ����� �Ѵ�.
	SetTimer(COUNT_REFRESH_TIMER, COUNT_REFRESH_TIMER_INTERVAL, NULL);

	CString str;
	str.Format(_T("First header of %d files received"), ((FILEINFO*)wParam)->files_);
	L(str);
	return 0;
}

void CgStreamerDlg::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰��մϴ�.
	CMenu popup;
	popup.LoadMenu(IDR_MAIN_MENU);

	CMenu* pMenu = popup.GetSubMenu(0);
	pMenu->TrackPopupMenu(TPM_LEFTALIGN || TPM_RIGHTBUTTON, point.x, point.y, this);
}


void CgStreamerDlg::OnMainmenuClearlog()
{
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	m_log.ResetContent();
}


float CgStreamerDlg::BpsVal(unsigned int size, float sec)
{
	return (float)size / sec;
}

void CgStreamerDlg::ResetDevice()
{
	unsigned char buf[512];
	ZeroMemory(buf, 512);
	sprintf_s(reinterpret_cast<char*>(buf), sizeof(buf), "FX3 RST");
	LONG bytesToSend = 7;

	CCyControlEndPoint* pCEP = m_pUsbDev->ControlEndPt;
	pCEP->ReqCode = 0x3;
	if (sendEP0(pCEP, buf, bytesToSend)) {
		m_bReset = TRUE;
	}
	else {
		m_bReset = FALSE;
	}
}


BOOL CgStreamerDlg::sendEP0(CCyControlEndPoint* pCEP, unsigned char* pBuf, LONG& bufSize)
{
	ASSERT(pCEP);
	return pCEP->Write(pBuf, bufSize);
}


void CgStreamerDlg::ep0DataXfer(CTL_XFER_TGT_TYPE target, CTL_XFER_REQ_TYPE reqType, CTL_XFER_DIR_TYPE direction,
	UCHAR reqCode, WORD value, WORD index, unsigned char* buf, LONG bufSize, ULONG timeOut)
{
	ASSERT(m_pUsbDev);
	CCyControlEndPoint* pCEP = m_pUsbDev->ControlEndPt;
	pCEP->Target = target;
	pCEP->ReqType = reqType;
	pCEP->Direction = direction;
	pCEP->ReqCode = reqCode;
	pCEP->Value = value;
	pCEP->Index = index;

	OVERLAPPED OvLap;
	OvLap.hEvent = CreateEvent(NULL, false, false, NULL);

	PUCHAR Context = pCEP->BeginDataXfer(buf, bufSize, &OvLap);
	pCEP->WaitForXfer(&OvLap, timeOut);
	pCEP->FinishDataXfer(buf, bufSize, &OvLap, Context);

	CloseHandle(OvLap.hEvent);
}


void CgStreamerDlg::UpdateZingMode()
{
	ep0DataXfer(TGT_DEVICE, REQ_VENDOR, DIR_TO_DEVICE, 0x3, 0, 0, (unsigned char*)"DMA MODE SYNC", (LONG)strlen("DMA MODE SYNC"));
	Sleep(100);
	ep0DataXfer(TGT_DEVICE, REQ_VENDOR, DIR_TO_DEVICE, 0x3, 0, 0, (unsigned char*)"GET ZING MODE", (LONG)strlen("GET ZING MODE"));
	Sleep(100);

	unsigned char buf[4] = { 0, };
	ep0DataXfer(TGT_DEVICE, REQ_VENDOR, DIR_FROM_DEVICE, 0x3, 0, 0, buf, 3, 100);
	TRACE("Zing Mode=%s\n", buf);

	if (CString(buf) == _T("DEV")) {
		m_zingModeCombo.SetCurSel(0);
	}
	else if (CString(buf) == _T("PPC")) {
		m_zingModeCombo.SetCurSel(1);
	}
	else {
		ASSERT(false);	//�̷� ���� ������ �� ����
	}
	GetFirmwareVersion();

	ep0DataXfer(TGT_DEVICE, REQ_VENDOR, DIR_TO_DEVICE, 0x3, 0, 0, (unsigned char*)"DMA MODE NORMAL", (LONG)strlen("DMA MODE NORMAL"));
}


void CgStreamerDlg::GetFirmwareVersion()
{
	ep0DataXfer(TGT_DEVICE, REQ_VENDOR, DIR_TO_DEVICE, 0x3, 0, 0, (unsigned char*)"VER", (LONG)strlen("VER"));
	Sleep(100);

	unsigned char buf[6] = { 0, };
	ep0DataXfer(TGT_DEVICE, REQ_VENDOR, DIR_FROM_DEVICE, 0x3, 0, 0, buf, 5, 100);
	L(_T("Firmware version: ")+CString(buf));
}