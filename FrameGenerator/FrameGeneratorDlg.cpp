
// FrameGeneratorDlg.cpp: 구현 파일
//


#pragma once

#include "pch.h"
#include "framework.h"
#include "FrameGenerator.h"
#include "FrameGeneratorDlg.h"
#include "afxdialogex.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define _DEF_EVENT_IGNORE_TIME  5000


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


void Mat2CImage(cv::Mat* mat, CImage& img) {
	if (!mat || mat->empty());
	//MessageBox("Error convert Mat to CImage", "error", MB_OK);
	int nBPP = mat->channels() * 8;
	img.Create(mat->cols, mat->rows, nBPP);
	if (nBPP == 8)
	{
		static RGBQUAD pRGB[256];
		for (int i = 0; i < 256; i++)
			pRGB[i].rgbBlue = pRGB[i].rgbGreen = pRGB[i].rgbRed = i;
		img.SetColorTable(0, 256, pRGB);
	}
	uchar* psrc = mat->data;
	uchar* pdst = (uchar*)img.GetBits();
	int imgPitch = img.GetPitch();
	for (int y = 0; y < mat->rows; y++)
	{
		memcpy(pdst, psrc, mat->cols * mat->channels());//mat->step is incorrect for those images created by roi (sub-images!)
		psrc += mat->step;
		pdst += imgPitch;
	}

	return;
}

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
	m_nLastEvtType = EVENT_NONE;
	m_tpLastEvtTime = steady_clock::now();
	m_bStopPlay = FALSE;
	m_pYoloV8 = NULL;
	m_pYoloPose = NULL;

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
	DDX_Control(pDX, IDC_LIST_EVENT, m_evtList);
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
	ON_MESSAGE(WM_USER + 101, &CFrameGeneratorDlg::OnEventCreate)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_EVENT, &CFrameGeneratorDlg::OnNMDblclkListEvent)
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
	InitImageSavePath();
	InitTensorRT();

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}
void CFrameGeneratorDlg::InitImageSavePath()
{
	TCHAR path[256] = { 0, };
	GetModuleFileName(NULL, path, 256);

	CString folderPath(path);
	folderPath = folderPath.Left(folderPath.ReverseFind('\\'));
	m_ImgSavePath = folderPath + "\\save\\";

	if (GetFileAttributes(m_ImgSavePath) == -1) {
		CreateDirectory(m_ImgSavePath,NULL);
	}
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
	//YoloPath = folderPath + "wheelchair_best.onnx";
	PosePath = folderPath + "yolov8x-pose.onnx";

	SendLog(1, YoloPath);
	SendLog(1, PosePath);

	m_pYoloV8 = new YoloV8(YoloPath.GetBuffer(), config, this->GetSafeHwnd());
	m_pYoloPose = new YoloV8(PosePath.GetBuffer(), config, this->GetSafeHwnd());

	SendLog(1, "CFrameGeneratorDlg::InitTensorRT END ");
}


void CFrameGeneratorDlg::MakeControlPos() {

	SetWindowText("A.Eyes");

	PreSubclassWindow();
	if (!m_PreviewWnd.SubclassDlgItem(IDC_STATIC_VIEW, this))
		ASSERT(FALSE);
	m_PreviewWnd.SetOSDText("");
	m_editPath.SetWindowText("rtsp://desktop-uv7j38l.iptime.org/test1234.mp4");
	m_editPort.SetWindowText("554");
	m_editID.SetWindowText("admin");
	m_editPW.SetWindowText("admin");

	
	m_evtList.SetExtendedStyle(m_evtList.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_imgList.Create(128, 128, ILC_COLOR24, 1, 1);
	m_evtList.SetImageList(&m_imgList, LVSIL_NORMAL);

	ChangeWindowSize(800, 600);
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


LRESULT CFrameGeneratorDlg::OnEventCreate(WPARAM wParam, LPARAM lParam) {

	SendLog(TRACE_INFO, "EVENT CREATE");
	
	m_csEvtImg.Lock();

	cv::Mat copyImg;
	((cv::Mat*)wParam)->copyTo(copyImg);
	EVENT_TYPE evtType = (EVENT_TYPE)lParam;
	m_csEvtImg.Unlock();
	

	steady_clock::time_point  tpNow = steady_clock::now();
	milliseconds interval = duration_cast<milliseconds>(tpNow - m_tpLastEvtTime);

	//SendLog(1, "%d", interval);

	//< 동일한 type의 event가 중복 발생하는 경우 일정 시간 동안 ignore
	if (interval.count() < _DEF_EVENT_IGNORE_TIME && evtType == m_nLastEvtType) {
		SendLog(TRACE_INFO, "IGNORE SAME EVENT.");
		return 0L;
	}

	CString strEvtType = "";
	
	switch (evtType) {
	case EVENT_KNIFE:			  strEvtType = "칼부림";					break;
	case EVENT_COLLAPSE:		  strEvtType = "쓰러짐";				    break;
	case EVENT_VIOLENCE_PUNCH:    strEvtType = "폭력(PUNCH)";			break;
	case EVENT_VIOLENCE_KICK:     strEvtType = "폭력(KICK)";				break;
	}
		
	if (copyImg.size().width == 0 || copyImg.size().height == 0) {
		SendLog(TRACE_ERROR, "EVENT IMAGE ERROR");
		return 0L;
	}
	cv::Mat resizeImg;
	resize(copyImg, resizeImg, cv::Size(128, 128));
		

	CImage image;
	Mat2CImage(&resizeImg, image);
	CBitmap bitmap;
	bitmap.Attach(image.Detach());
		
	int nImgIndex = m_imgList.Add(&bitmap, RGB(0, 0, 0));
		
	//< JPG 파일 저장 
	CString strFileName, strSaveFile;
	CTime   CurTime;
	CurTime = CTime::GetCurrentTime();
	strFileName.Format("%s_%04d%02d%02d%02d%02d%02d", strEvtType, CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay()
		, CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond());

	strSaveFile = m_ImgSavePath + strFileName + ".jpg";
	cv::imwrite(strSaveFile.GetBuffer(), copyImg);
	SendLog(1, strSaveFile);

	//< Event List Insert 
	m_evtList.InsertItem(0, strFileName, nImgIndex);

	m_tpLastEvtTime = tpNow;
	m_nLastEvtType = evtType;

	return 0L;
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
	BOOL bRTSP = FALSE;
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
			strURL.Format("rtsp://%s:%s@", strID, strPW);
		}

		//< PORT 존재시 
		if (FALSE == strPort.IsEmpty()) {
			CString strContext = strPath.Mid(strPath.ReverseFind('/'));
			strPath = strPath.Left(strPath.ReverseFind('/'));
			strURL = strURL + strPath + ":" + strPort + strContext;

			SendLog(TRACE_INFO, strURL);
		}

		bRTSP = TRUE;
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
	//int totalFrameCnt = 0;

	cv::Mat frame;

	while (FALSE == m_bStopPlay) {
		if (!capture.read(frame)) {
			SendLog(TRACE_ERROR, "VideoCapture READ Failed");
		}
		else {						

			if (FALSE == bRTSP) {
				//totalFrameCnt = capture.get(cv::CAP_PROP_POS_FRAMES);
				if (cv::waitKey(delay) >= 0)break;
			}
		
			//cv::imshow("TEST", frame);
			//cv::waitKey(30);
			if (frame.size().width != m_nWidth || frame.size().height != m_nHeight) {
				m_nWidth = frame.size().width;
				m_nHeight = frame.size().height;
				ChangeWindowSize(frame.size().width, frame.size().height);
			}

			m_lFrameCnt++;

			if (m_lFrameCnt % 2 == 1) //< 1/2 fps
			{
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

		/*if (m_lFrameCnt % 2 != 0) {
			
			SAFE_DELETE(pBuffItem);
			continue;
		}*/

		if (pBuffItem)
		{
			//////////////////////////////////// OBJECT DETECTION //////////////////////////////////////////////////


			cv::Mat img;
			img = pBuffItem->m_cvMatImage.clone();
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

			// SendLog(TRACE_INFO, "POSE SIZE = %d", objectsPose.size());

			CRect rtViolence;
			int index = -1;

			bool bPunch = false;
			bool bKick = false;
			// violence check - PUNCH
			for (int i = 0; i < objectsPose.size(); ++i) {
				if (   ( objectsPose[i].LPunchAngle > 140 && objectsPose[i].LArmLineAngle > 220) 
					|| (objectsPose[i].RPunchAngle > 140 && objectsPose[i].RArmLineAngle > 220)) {

					if (objectsPose[i].rect.height < 200) continue;

					SendLog(TRACE_ERROR, "EVENT_VIOLENCE [PUNCH] WARNING  ");

					rtViolence.left = objectsPose[i].rect.x;
					rtViolence.top = objectsPose[i].rect.y;
					rtViolence.right = rtViolence.left + objectsPose[i].rect.width;
					rtViolence.bottom = rtViolence.top + objectsPose[i].rect.height;

					index = i;
					bPunch = true;
					break;
				}
			}

			if (bPunch == false) {

				// violence check - KICK
				for (int i = 0; i < objectsPose.size(); ++i) {
					if ((objectsPose[i].LkickDeg > 150 && (objectsPose[i].LLegLineAngle < 210) && (objectsPose[i].LLegLineAngle > 0))
						|| (objectsPose[i].RkickDeg > 150 && objectsPose[i].RLegLineAngle < 210) && (objectsPose[i].RLegLineAngle > 0)) {

						if (objectsPose[i].rect.height < 200) continue;

						SendLog(TRACE_ERROR, "EVENT_VIOLENCE [ KICK ] WARNING  ");

						rtViolence.left = objectsPose[i].rect.x;
						rtViolence.top = objectsPose[i].rect.y;
						rtViolence.right = rtViolence.left + objectsPose[i].rect.width;
						rtViolence.bottom = rtViolence.top + objectsPose[i].rect.height;

						index = i;
						bKick = true;
						break;
					}
				}
			}
			


			// 중첩된 Rect 체크
			if (index != -1) {

				for (int i = 0; i < objectsPose.size(); ++i) {
					if (i == index) continue;

					CRect personRect;
					personRect.left = objectsPose[i].rect.x;
					personRect.top = objectsPose[i].rect.y;
					personRect.right = personRect.left + objectsPose[i].rect.width;
					personRect.bottom = personRect.top + objectsPose[i].rect.height;

					RECT rtResult;
					if (TRUE == IntersectRect(&rtResult, &rtViolence, &personRect)) {
						
						if (rtResult.right - rtResult.left > 20) {

							SendLog(TRACE_ERROR, "EVENT_VIOLENCE");

							cv::Scalar red(0, 0, 255);
							cv::rectangle(img, cv::Rect(rtViolence.left, rtViolence.top, rtViolence.Width(), rtViolence.Height()), red, 10);

							if (bPunch == true) {
								::SendMessage(this->m_hWnd, WM_USER + 101, (WPARAM)&img, (LPARAM)EVENT_VIOLENCE_PUNCH);
							}
							else {
								::SendMessage(this->m_hWnd, WM_USER + 101, (WPARAM)&img, (LPARAM)EVENT_VIOLENCE_KICK);
							}
							

							break;
						}

					}
					
				}
			}


			m_nWidth = img.size().width;
			m_nHeight = img.size().height;
			m_PreviewWnd.DisplayImage(img, true);

			SAFE_DELETE(pBuffItem);
		}
	}

	SendLog(TRACE_INFO, "YoloProcessingProc END");
	return TRUE;
}


void CFrameGeneratorDlg::OnBnClickedStart()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_bStopPlay = FALSE;

	//< Frame push to Queue Thread
	CWinThread* pTherad = AfxBeginThread(DecodingThread, (LPVOID)this, THREAD_PRIORITY_ABOVE_NORMAL, 384000, CREATE_SUSPENDED | STACK_SIZE_PARAM_IS_A_RESERVATION);
	m_hDecodeThread = pTherad->m_hThread;
	pTherad->ResumeThread();


	//< Yolo 분석 Thread
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

	m_pCompBuffQueue->clear();
	m_evtList.DeleteAllItems();
	
	m_bStopPlay = TRUE;

	m_lFrameCnt = 0;
	Sleep(1000);

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CDialogEx::OnCancel();
}


void CFrameGeneratorDlg::OnBnClickedBtnStop()
{
	m_pCompBuffQueue->clear();
	m_evtList.DeleteAllItems();

	m_bStopPlay = TRUE;

	m_lFrameCnt = 0;

	Sleep(1000);

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void CFrameGeneratorDlg::ChangeWindowSize(int nWidth, int nHeight) {

#define BTN_BUTTON_SIZE 100
#define BTN_BUTTON_MARGIN 10
#define BTN_HEIGHT 25
#define LIST_HEIGHT 165

	CRect rect;
	GetWindowRect(&rect);

	this->MoveWindow(rect.left, rect.top, nWidth + 20, nHeight + 130 + LIST_HEIGHT);
	this->m_PreviewWnd.MoveWindow(2, 50, nWidth, nHeight);

	this->m_evtList.MoveWindow(2, nHeight + 50, nWidth, LIST_HEIGHT);
	this->m_btnStart.MoveWindow(nWidth - BTN_BUTTON_SIZE * 2 - BTN_BUTTON_MARGIN, LIST_HEIGHT + 50 + nHeight  + BTN_BUTTON_MARGIN, BTN_BUTTON_SIZE, BTN_HEIGHT);
	this->m_btnStop.MoveWindow(nWidth - BTN_BUTTON_SIZE, LIST_HEIGHT + 50 + nHeight + BTN_BUTTON_MARGIN, BTN_BUTTON_SIZE, BTN_HEIGHT);
}



void CFrameGeneratorDlg::OnNMDblclkListEvent(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	CString strFileName = m_evtList.GetItemText(pNMListView->iItem, 0);

	if (strFileName.GetLength() != 0)
		ShellExecute(NULL, _T("open"), _T("explorer"), m_ImgSavePath + strFileName + ".jpg", NULL, SW_SHOW);

	SendLog(TRACE_INFO, strFileName);

	*pResult = 0;
}
