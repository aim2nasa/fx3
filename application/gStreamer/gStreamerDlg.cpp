
// gStreamerDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "gStreamer.h"
#include "gStreamerDlg.h"
#include "afxdialogex.h"
#include <CyAPI.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define COUNT_REFRESH_TIMER				1
#define COUNT_REFRESH_TIMER_INTERVAL	10

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
	: CDialogEx(IDD_GSTREAMER_DIALOG, pParent), m_pEndPt(NULL), m_ppxComboIndex(-1), m_pThread(NULL), m_nQueueSize(0), m_nPPX(0)
	, m_ulSuccessCount(0)
	, m_ulFailureCount(0)
	, m_ulBeginDataXferErrCount(0)
	, m_KBps(_T(""))
	, m_strFileName(_T(""))
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
	ON_BN_CLICKED(IDC_FILE_SELECT_BUTTON, &CgStreamerDlg::OnBnClickedFileSelectButton)
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

	CString errMsg;
	GetStreamerDevice(errMsg)==FALSE ? L(errMsg):L(_T("streamer device ok"));

	m_bStart = FALSE;
	m_transferRate.SetRange(0, (short)MAX_KBPS);
	GetDlgItem(IDC_SUCCESS_COUNT_EDIT)->EnableWindow(FALSE);
	GetDlgItem(IDC_FAILURE_COUNT_EDIT)->EnableWindow(FALSE);
	GetDlgItem(IDC_BEGINDATAXFER_ERROR_COUNT_EDIT)->EnableWindow(FALSE);
	m_fileSelect.ShowWindow(SW_HIDE);
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
		L(_T("No device found"));
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
		m_pThread = AfxBeginThread(Xfer, this);
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

UINT CgStreamerDlg::Xfer(LPVOID pParam)
{
	CgStreamerDlg *pDlg = (CgStreamerDlg*)pParam;
	ASSERT(pDlg);
	ASSERT(pDlg->m_pEndPt);

	ULONG len = pDlg->m_pEndPt->MaxPktSize * pDlg->m_nPPX;
	pDlg->m_pEndPt->SetXferSize(len);

	// Allocate the arrays needed for queueing
	PUCHAR			*buffers = new PUCHAR[pDlg->m_nQueueSize];
	CCyIsoPktInfo	**isoPktInfos = new CCyIsoPktInfo*[pDlg->m_nQueueSize];
	PUCHAR			*contexts = new PUCHAR[pDlg->m_nQueueSize];

	// Allocate all the buffers for the queues
	for (int i = 0; i< pDlg->m_nQueueSize; i++) {
		buffers[i] = new UCHAR[len];
		isoPktInfos[i] = new CCyIsoPktInfo[pDlg->m_nPPX];
		pDlg->m_inOvLap[i].hEvent = CreateEvent(NULL, false, false, NULL);

		memset(buffers[i], 0xEF, len);
	}

	CCyUSBEndPoint *pEndPt = pDlg->m_pEndPt;
	pDlg->m_ulSuccessCount = pDlg->m_ulFailureCount = pDlg->m_ulBeginDataXferErrCount = pDlg->m_ulBytesTransferred = 0;
	pDlg->m_curKBps = 0.0;
	pDlg->m_startTime = clock();

	CFile *pFile = NULL;
	FILEINFO fileInfo;
	memset(&fileInfo, 0, sizeof(fileInfo));
	ASSERT(pEndPt->Attributes == 2);	//BULK���� ����Ѵ�
	if (pEndPt->bIn == FALSE) {	//BULK OUT
		if (!pDlg->m_strFileName.IsEmpty()) pFile = GetFile(pDlg->m_strFileName, fileInfo);
	}

	BYTE sync[4] = { 0x07,0x3a,0xb6,0x99 };	//������� ���Ƿ� ���� sync�ڵ� (���� ���ڸ��� ETI��ũ��)
	//Queue up before loop
	for (int i = 0; i < pDlg->m_nQueueSize; i++) {
		if (pFile) { //BULK OUT�ε� ���Ϸ� ���� �о�鿩 ������ �����
			if (i == 0) {	//���ϸ�ũ��,���ϸ�,���ϻ���� ����, �̵� ũ��� len������ ũ��� ���� �Ѵ�. �׷��� i=0�� ��쿡�� �̷� ��Ÿ������ ��� �Ǹ��ٰ� ����.
				SetFileInfo(buffers[i], len, sync, sizeof(sync), fileInfo);
			} else {
				UINT nRead = Read(pFile, buffers[i], len);
				ASSERT(nRead>0);	//ť�� ���������� EOF�� �ٴٸ��� �ʴ´ٴ� ���� ����, EOF�� �����ߴٸ� ť�� �ʹ� ũ�ų� ������ len���ٵ� ���� �����
			}
		}
		contexts[i] = pEndPt->BeginDataXfer(buffers[i], len, &pDlg->m_inOvLap[i]);
		if (pEndPt->NtStatus || pEndPt->UsbdStatus) pDlg->m_ulBeginDataXferErrCount++;
	}

	BOOL bInitFrame = pEndPt->bIn;	//IN�� ���� ��ó�� �����ϴ� ��Ʈ���� ������ �о� �����°����� �ƴ����� �Ǵ��ؾ� ��
	LONG rLen;
	while (pDlg->m_bStart) {
		for (int i = 0; i < pDlg->m_nQueueSize; i++) {
			pEndPt->WaitForXfer(&pDlg->m_inOvLap[i], INFINITE);

			ASSERT(pEndPt->Attributes == 2);	//Bulk���� ��츸 ����ϴ� ���

			if (pEndPt->FinishDataXfer(buffers[i], rLen, &pDlg->m_inOvLap[i], contexts[i])) {
				pDlg->m_ulSuccessCount++;
				pDlg->m_ulBytesTransferred += rLen;

				if (bInitFrame && i == 0) {
					if (memcmp(buffers[i], sync, sizeof(sync)) == 0) GetFileInfo(buffers[i], len, sizeof(sync), fileInfo);
					bInitFrame = FALSE;
				}
			}else{
				pDlg->m_ulFailureCount++;
				TRACE("Failcount=%u\n", pDlg->m_ulFailureCount);
			}

			//���Ӱ� ����� ť�� ���� ��û�� ����
			if (pFile && !pEndPt->bIn) {
				UINT nRead = Read(pFile, buffers[i], len);
				if (nRead > 0) {
					contexts[i] = pEndPt->BeginDataXfer(buffers[i], len, &pDlg->m_inOvLap[i]);
					if (pEndPt->NtStatus || pEndPt->UsbdStatus) pDlg->m_ulBeginDataXferErrCount++;
				} else { //EOF
				}
			}
			else {
				contexts[i] = pEndPt->BeginDataXfer(buffers[i], len, &pDlg->m_inOvLap[i]);
				if (pEndPt->NtStatus || pEndPt->UsbdStatus) pDlg->m_ulBeginDataXferErrCount++;
			}

			if (fileInfo.size_>0 && pDlg->m_ulBytesTransferred >= fileInfo.size_ + len) {
				pDlg->m_bStart = FALSE;
				break;
			}

			if (i == (pDlg->m_nQueueSize - 1)) {	//ť�� �Ǹ����� ���
				pDlg->showStats();
				if (!pDlg->m_bStart) break;	//���� ���(m_bStart==FALSE)�� �����߰�, ť�� �Ǹ����� ��ұ��� ó���ϰ� ������ for������ Ż��
			}
		}
	}

	delete pFile;

	// Deallocate memories
	for (int i = 0; i < pDlg->m_nQueueSize; i++) {
		if(pFile==NULL) pEndPt->FinishDataXfer(buffers[i], rLen, &pDlg->m_inOvLap[i], contexts[i]);
		delete[] buffers[i];
		delete[] isoPktInfos[i];
	}

	delete [] contexts;
	delete [] isoPktInfos;
	delete [] buffers;

	pDlg->PostMessage(WM_THREAD_TERMINATED);
	return 0;
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
		m_transferRate.SetPos((short)(m_curKBps/1000));	//KBps���� MBps������ ��ȯ
		UpdateData(FALSE);
	}

	CDialogEx::OnTimer(nIDEvent);
}

LRESULT CgStreamerDlg::OnThreadTerminated(WPARAM wParam, LPARAM lParam)
{
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
	L(_T("Xfer thread terminating..."));
	m_bStart = FALSE;
	for (int i = 0; i < m_nQueueSize; i++) SetEvent(m_inOvLap[i].hEvent);
	if (m_pThread) WaitForSingleObject(m_pThread->m_hThread, INFINITE);
}

void CgStreamerDlg::showStats()
{
	clock_t curTime = clock();
	double elapsed = ((double)(curTime - m_startTime)) / CLOCKS_PER_SEC;
	//TRACE("���� �ð� : %f\n", elapsed);

	m_curKBps = ((double)m_ulBytesTransferred / elapsed) / 1024.;
	m_KBps.Format(_T("%.0f KBps"),m_curKBps);
}

void CgStreamerDlg::OnBnClickedFileSelectButton()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	ASSERT(m_pEndPt);
	ASSERT(m_pEndPt->Attributes==2);	//���ǻ� ������������ BULK�� �����ϵ��� ����, isochronous�� �����ؾ� ��

	TCHAR szFilter[] = _T("All Files(*.*)|*.*||");
	if (m_pEndPt->bIn) {
		CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter);
		if (IDOK == dlg.DoModal()) {
			m_strFileName = dlg.GetPathName();
			L(_T("File save to:%s"), m_strFileName);
		}else{
			m_strFileName.Empty();
		}
	}
	else {
		CFileDialog dlg(FALSE, NULL, NULL, OFN_HIDEREADONLY, szFilter);
		if (IDOK == dlg.DoModal()) {
			m_strFileName = dlg.GetPathName();
			L(_T("File read from:%s"), m_strFileName);
		}else{
			m_strFileName.Empty();
		}
	}
	UpdateData(FALSE);
}

LRESULT CgStreamerDlg::OnEndOfFile(WPARAM wParam, LPARAM lParam)
{
	L(_T("End of file reached"));
	return 0;
}

BOOL CgStreamerDlg::fullRead(CFile *pFile, UCHAR *buffer,UINT nCount, BOOL bSeekToBegin, BOOL bPostEofMsg, HWND hWnd)
{
	memset(buffer, 0, nCount);	//���۴� nCount���� ��� 0���� �ʱ�ȭ�Ѵ�. nCount���� ���д� ��� ������ ������ 0���� ä������
	UINT read = pFile->Read(buffer, nCount);	//BULK OUT�� ��� ���Ϸ� ���� �д´�
	if (read < nCount) {
		if (bPostEofMsg) ::PostMessage(hWnd, WM_END_OF_FILE,0,0);
		if (bSeekToBegin) {
			pFile->SeekToBegin();
			return TRUE;
		}
		return FALSE;
	}
	return TRUE;
}

UINT CgStreamerDlg::Read(CFile *pFile, UCHAR *buffer, UINT nCount)
{
	memset(buffer, 0, nCount);	//���۴� nCount���� ��� 0���� �ʱ�ȭ�Ѵ�. nCount���� ���д� ��� ������ ������ 0���� ä������
	return pFile->Read(buffer, nCount);	//BULK OUT�� ��� ���Ϸ� ���� �д´�
}

CFile* CgStreamerDlg::GetFile(CString pathFileName, FILEINFO &fileInfo)
{
	CFile *pFile = NULL;
	if (!pathFileName.IsEmpty()) {
		pFile = new CFile(pathFileName, CFile::modeRead | CFile::typeBinary);

		CString name = PathFindFileName(pathFileName.GetBuffer());
		memset(fileInfo.name_, 0, sizeof(fileInfo.name_));
		fileInfo.nameSize_ = name.GetLength() * sizeof(TCHAR);
		memcpy(fileInfo.name_, name.GetBuffer(), fileInfo.nameSize_);
		fileInfo.size_ = GetFileSize(pFile->m_hFile, NULL);

		TRACE("fileName=%S(%d), size=%dbyte\n", fileInfo.name_, fileInfo.nameSize_, fileInfo.size_);
	}
	return pFile;
}

int CgStreamerDlg::SetFileInfo(UCHAR *buffer, ULONG bufferSize, BYTE *sync, int syncSize, FILEINFO &info)
{
	int nOffset = 0;
	memcpy(buffer + nOffset, sync, syncSize); nOffset += syncSize;
	memcpy(buffer + nOffset, &info.nameSize_, sizeof(int)); nOffset += sizeof(int);
	memcpy(buffer + nOffset, info.name_, info.nameSize_); nOffset += info.nameSize_;
	memcpy(buffer + nOffset, &info.size_, sizeof(DWORD)); nOffset += sizeof(DWORD);

	ASSERT((ULONG)nOffset <= bufferSize);	//len���� �۰ų� ���ٴ� ����
	return nOffset;
}

int CgStreamerDlg::GetFileInfo(UCHAR *buffer, ULONG bufferSize, int syncSize, FILEINFO &info)
{
	int nOffset = syncSize;
	memset(info.name_, 0, sizeof(FILEINFO::name_));
	memcpy(&info.nameSize_, buffer + nOffset, sizeof(int)); nOffset += sizeof(int);
	memcpy(info.name_, buffer + nOffset, info.nameSize_); nOffset += info.nameSize_;
	memcpy(&info.size_, buffer + nOffset, sizeof(DWORD)); nOffset += sizeof(DWORD);

	ASSERT((ULONG)nOffset <= bufferSize);	//len���� �۰ų� ���ٴ� ����
	return nOffset;
}