#pragma once
#include "pch.h"



class CImgViewStatic : public CStatic
{
	DECLARE_DYNAMIC(CImgViewStatic)

public:
	CImgViewStatic();
	virtual	~CImgViewStatic();

	CBitmap	m_Bitmap;
	int		m_nBitmapWidth, m_nBitmapHeight;
	CString	m_strOSDText;

   BOOL     m_bDrawCircleRect;
   CRect    m_rectCircle;
   CRect    m_rectMarker[8];

	void	   DisplayImage(cv::Mat s_image, BOOL bDrawNow);
	void		ClearStatic();
	void		SetOSDText(LPCSTR szText);
	void		DrawOSDText(CDC* pDC);
	void		DrawImg(CDC* pDC);
   void     DrawRectangle(CDC* pDC, int x, int y, int width, int height, COLORREF pen_color, int pen_width = 1, int pen_style = PS_SOLID);
   void     DrawCircleRect(CDC* pDC);

protected:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()
};
