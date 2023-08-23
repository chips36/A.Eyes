
// FrameGeneratorDlg.cpp: 구현 파일
//


#pragma once

#include "pch.h"
#include "framework.h"
#include "FrameGenerator.h"
#include "FrameGeneratorDlg.h"
#include "afxdialogex.h"
//

#include "yolov8.h"

using namespace std;

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


// CFrameGeneratorDlg 대화 상자

CFrameGeneratorDlg::CFrameGeneratorDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_FRAMEGENERATOR_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_pCompBuffQueue = new CImageQueue;
	m_hDecodeThread = NULL;
	m_hYoloProcessingThread = NULL;
	m_lFrameCnt = 0L;
	m_nWidth = 0;
	m_nHeight = 0;
	m_bStopPlay = FALSE;

}

CFrameGeneratorDlg::~CFrameGeneratorDlg()
{
	if (m_pCompBuffQueue) {
		delete m_pCompBuffQueue;
		m_pCompBuffQueue = NULL;
	}
}

void CFrameGeneratorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_EDIT_PATH, m_editPath);
	DDX_Control(pDX, IDC_EDIT_PORT, m_editPort);
	DDX_Control(pDX, IDC_EDIT_ID, m_editID);
	DDX_Control(pDX, IDC_EDIT_PW, m_editPW);
	DDX_Control(pDX, ID_BTN_STOP, m_btnStop);
	DDX_Control(pDX, IDOK, m_btnStart);
}



BEGIN_MESSAGE_MAP(CFrameGeneratorDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CFrameGeneratorDlg::OnBnClickedStart)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDCANCEL, &CFrameGeneratorDlg::OnBnClickedCancel)
	ON_BN_CLICKED(ID_BTN_STOP, &CFrameGeneratorDlg::OnBnClickedBtnStop)
END_MESSAGE_MAP()


// CFrameGeneratorDlg 메시지 처리기

BOOL CFrameGeneratorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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
	MakeControlPos();
	InitTensorRT();

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CFrameGeneratorDlg::InitTensorRT() {
	SendLog(1, "CFrameGeneratorDlg::InitTensorRT ");

	YoloV8Config config;
	std::string onnxModelPath;

	TCHAR path[256] = { 0, };
	GetModuleFileName(NULL, path, 256);

	CString folderPath(path);
	folderPath = folderPath.Left(folderPath.ReverseFind('\\'));
	folderPath = folderPath + "\\models\\";

	CString YoloPath, PosePath;

	YoloPath = folderPath + "yolov8x.onnx";
	//YoloPath = folderPath + "yolov8x-seg.onnx";
	//YoloPath = folderPath + "yolov8n-wheelchair.onnx";
	PosePath = folderPath + "yolov8x-pose.onnx";

	SendLog(1, YoloPath);
	SendLog(1, PosePath);

	m_pYoloV8 = new YoloV8(YoloPath.GetBuffer(), config);
	m_pYoloPose = new YoloV8(PosePath.GetBuffer(), config);

	SendLog(1, "CFrameGeneratorDlg::InitTensorRT END ");
}

void CFrameGeneratorDlg::MakeControlPos() {

	SetWindowText("A.Eyes");

	PreSubclassWindow();
	if (!m_PreviewWnd.SubclassDlgItem(IDC_STATIC_VIEW, this))
		ASSERT(FALSE);
	m_PreviewWnd.SetOSDText("");
	m_editPath.SetWindowText("rtsp://192.168.0.99/test1234.mp4");
	m_editPort.SetWindowText("554");
	m_editID.SetWindowText("admin");
	m_editPW.SetWindowText("admin");
	
}

void CFrameGeneratorDlg::OnSysCommand(UINT nID, LPARAM lParam)
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
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.  

void CFrameGeneratorDlg::OnPaint()
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
HCURSOR CFrameGeneratorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}




UINT CFrameGeneratorDlg::DecodingThread(LPVOID pVoid)
{
	COINITIALIZEEX_MULTI_THREADED

		CFrameGeneratorDlg* pDlg = (CFrameGeneratorDlg*)pVoid;

	pDlg->DecodingProc();

	COUNINITIALIZEEX_MULTI_THREADED;
	return TRUE;
}



int CFrameGeneratorDlg::DecodingProc() {

	CString strURL, strPath, strPort, strID, strPW;
	m_editPath.GetWindowText(strPath);
	m_editPort.GetWindowText(strPort);
	m_editID.GetWindowText(strID);
	m_editPW.GetWindowText(strPW);

	//< RTSP 재생 
	strPath = strPath.MakeLower();
	if (strPath.Find("rtsp:") != -1) {
			
		//< ID / PW 존재 시 
		if (FALSE == strID.IsEmpty() && FALSE == strPW.IsEmpty()) {
			strPath.Replace("rtsp://", "");
			strURL.Format("rtsp://%s:%s@",strID, strPW);
		}
		
		//< PORT 존재시 
		if (FALSE == strPort.IsEmpty()) {
			CString strContext = strPath.Mid(strPath.ReverseFind('/'));
			strPath = strPath.Left(strPath.ReverseFind('/'));			
			strURL = strURL + strPath + ":" + strPort + strContext;

			SendLog(TRACE_INFO, strURL);
		}
	}
	else {		//< File 재생 
		strURL = strPath;
	}

	cv::VideoCapture capture(strURL.GetBuffer());

	if (!capture.isOpened()) {
		//Error
		SendLog(TRACE_ERROR, "VideoCapture Open Failed");
		return -1;
	}

	double dbFps = capture.get(cv::CAP_PROP_FPS);
	SendLog(TRACE_INFO, "[ FPS - %f ]", dbFps);

	int delay = cvRound(1000 / dbFps); 
	int frameCnt = 0;

	cv::Mat frame;

	while (FALSE == m_bStopPlay ) {
		if (!capture.read(frame)) {
			SendLog(TRACE_ERROR, "VideoCapture READ Failed");
		}
		else {

			frameCnt = capture.get(cv::CAP_PROP_POS_FRAMES);
			if (cv::waitKey(delay) >= 0)break;

			//cv::imshow("TEST", frame);
			//cv::waitKey(30);
			if (frame.size().width != m_nWidth || frame.size().height != m_nHeight) {
				m_nWidth = frame.size().width;
				m_nHeight = frame.size().height;
				ChangeWindowSize(frame.size().width, frame.size().height);
			}

			m_lFrameCnt++;

			if (m_lFrameCnt % 2 == 0) {
				SendLog(1, " [ OnVideoFrameReceived ] queue size = %d", m_pCompBuffQueue->size());
				CImageItem* pNewItem = new CImageItem;

				pNewItem->m_cvMatImage = frame;
				m_pCompBuffQueue->Push(pNewItem);
			}
		}
	}

	SendLog(TRACE_INFO, "DecodingProc END");
}


UINT CFrameGeneratorDlg::YoloProcessingThread(LPVOID pVoid)
{
	COINITIALIZEEX_MULTI_THREADED

		CFrameGeneratorDlg* pDlg = (CFrameGeneratorDlg*)pVoid;

	pDlg->YoloProcessingProc();

	COUNINITIALIZEEX_MULTI_THREADED;
	return TRUE;
}


int CFrameGeneratorDlg::YoloProcessingProc() {
	while (m_bStopPlay == false)
	{
		CImageItem* pBuffItem = m_pCompBuffQueue->Pop();


		if (pBuffItem)
		{
			//////////////////////////////////// OBJECT DETECTION //////////////////////////////////////////////////

			cv::Mat img = pBuffItem->m_cvMatImage;

			//SendLog(1, "Object Detection Start ");

			auto objects = m_pYoloV8->detectObjects(img);
			auto objectsPose = m_pYoloPose->detectObjects(img);

			//SendLog(1, "Object Detection End ");

			if (objects.size() > 0) {
				m_pYoloV8->drawObjectLabels(img, objects);
			}


			/* 1 - 왼쪽 허벅지 2- 왼다리 3 - 오른쪽 허벅지 4- 오른쪽 다리
			*  5 - 왼쪽몸통 6-오른쪽 몸통(어깨에서 골반)
			*  8 - 왼쪽 팔뚝   9 -오른쪽 팔뚝  10- 왼쪽 팔  11 - 오른쪽 팔
			   12 - 미간 13 - 왼쪽 눈에서 귀 14 -왼쪽눈에서 코 15-  오른쪽눈에서 코
			   17,18 - 귀에서 어깨
			*/

			if (objectsPose.size() > 0) {
				m_pYoloPose->drawObjectLabels(img, objectsPose);
			}

			//cv::imshow("Object Detection", img);
			//cv::waitKey(30);

			m_nWidth = img.size().width;
			m_nHeight = img.size().height;
			m_PreviewWnd.DisplayImage(img, true);

			delete pBuffItem;
			pBuffItem = NULL;
		}
	}

	SendLog(TRACE_INFO, "DecodingProc END");
	return TRUE;
}


void CFrameGeneratorDlg::OnBnClickedStart()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_bStopPlay = FALSE;

	//< Frame push Queue
	CWinThread* pTherad = AfxBeginThread(DecodingThread, (LPVOID)this, THREAD_PRIORITY_ABOVE_NORMAL, 384000, CREATE_SUSPENDED | STACK_SIZE_PARAM_IS_A_RESERVATION);
	m_hDecodeThread = pTherad->m_hThread;
	pTherad->ResumeThread();


	CWinThread* pYoloTherad = AfxBeginThread(YoloProcessingThread, (LPVOID)this, THREAD_PRIORITY_ABOVE_NORMAL, 384000, CREATE_SUSPENDED | STACK_SIZE_PARAM_IS_A_RESERVATION);
	m_hYoloProcessingThread = pYoloTherad->m_hThread;
	pYoloTherad->ResumeThread();
}

void CFrameGeneratorDlg::OnClose()
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	CDialogEx::OnClose();
}


void CFrameGeneratorDlg::OnBnClickedCancel()
{
	m_bStopPlay = TRUE;
	
	m_lFrameCnt = 0;
	Sleep(1000);

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CDialogEx::OnCancel();
}


void CFrameGeneratorDlg::OnBnClickedBtnStop()
{
	m_bStopPlay = TRUE;

	m_lFrameCnt = 0;

	Sleep(1000);

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void CFrameGeneratorDlg::ChangeWindowSize(int nWidth, int nHeight) {

#define BTN_BUTTON_SIZE 100
#define BTN_BUTTON_MARGIN 10
#define BTN_HEIGHT 25

	CRect rect;
	GetWindowRect(&rect);

	this->MoveWindow(rect.left, rect.top, nWidth + 20, nHeight + 130);
	this->m_PreviewWnd.MoveWindow(2, 50, nWidth, nHeight);
	this->m_btnStart.MoveWindow(nWidth - BTN_BUTTON_SIZE * 2 - BTN_BUTTON_MARGIN, 50 + nHeight + BTN_BUTTON_MARGIN, BTN_BUTTON_SIZE, BTN_HEIGHT);
	this->m_btnStop.MoveWindow(nWidth - BTN_BUTTON_SIZE, 50 + nHeight + BTN_BUTTON_MARGIN, BTN_BUTTON_SIZE, BTN_HEIGHT);
}
