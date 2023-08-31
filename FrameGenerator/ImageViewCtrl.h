#pragma once
#include "pch.h"

class CImageViewCtrl : public CStatic
{
	DECLARE_DYNAMIC(CImageViewCtrl)

public:
	CImageViewCtrl();
	virtual	~CImageViewCtrl();

	CBitmap	m_Bitmap;
	int		m_nBitmapWidth, m_nBitmapHeight;
	CString	m_strOSDText;

	BOOL     m_bDrawCircleRect;
	CRect    m_rectCircle;
	CRect    m_rectMarker[8];

	
	void ClearStatic();
	void SetOSDText(LPCSTR szText);
	void DrawOSDText(CDC* pDC);
	void DisplayImage(cv::Mat s_image, BOOL bDrawNow);
	void DrawImg(CDC* pDC);
	void DrawRectangle(CDC* pDC, int x, int y, int width, int height, COLORREF pen_color, int pen_width = 1, int pen_style = PS_SOLID);
	void DrawCircleRect(CDC* pDC);
	BOOL CreateFont(LPCSTR szFontName, int nHeight, BOOL bBold, CFont& fontOut);
	CRect GetFittingRect(CRect rect, CSize s_size);


protected:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()
};
