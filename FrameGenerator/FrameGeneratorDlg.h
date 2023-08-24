
// FrameGeneratorDlg.h: 헤더 파일
//

#pragma once

#include "ImgViewStatic.h"


#include "yolov8.h"

#include "ImageQueue.h"

//TensorRT
#include <NvInfer.h>
#include <NvOnnxParser.h>
#include <cuda_runtime_api.h>
using namespace nvinfer1;

// CFrameGeneratorDlg 대화 상자
class CFrameGeneratorDlg : public CDialogEx
{
// 생성입니다.
public:
	CFrameGeneratorDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.
	virtual  ~CFrameGeneratorDlg();
// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FRAMEGENERATOR_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;
		

public:

	long m_lFrameCnt;
	int m_nWidth;
	int m_nHeight;

	CImgViewStatic	m_PreviewWnd;
		
	HANDLE m_hDecodeThread;
	HANDLE m_hYoloProcessingThread;


	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnEventCreate(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	
	afx_msg void OnClose();
	void InitTensorRT();
	void ChangeWindowSize(int nWidth, int nHeight);

	CEdit m_editPath;
	CEdit m_editPort;
	CEdit m_editID;
	CEdit m_editPW;
	CListCtrl m_evtList;
	CImageList m_imgList;
	CButton m_btnStart;
	CButton m_btnStop;
	CString m_strCurrGuid;

	YoloV8* m_pYoloV8;	//< Object Detect
	YoloV8* m_pYoloPose; //< Pose

	BOOL m_bStopPlay;

	CImageQueue* m_pCompBuffQueue;


	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedBtnStop();
	afx_msg void OnBnClickedStart();

	static UINT DecodingThread(LPVOID pVoid);
	static UINT YoloProcessingThread(LPVOID pVoid);
	int DecodingProc();
	int YoloProcessingProc();
	void MakeControlPos();
};
