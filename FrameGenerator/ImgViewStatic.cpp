#include "ImgViewStatic.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


#define COLOR_BLACK			RGB(0,0,0)
#define COLOR_GREY			RGB(128,128,128)
#define COLOR_DKGREY			RGB(64,64,64)
#define COLOR_WHITE			RGB(255,255,255)
#define COLOR_YELLOW			RGB(255,255,0)
#define COLOR_DKYELLOW		RGB(192,192,0)
#define COLOR_BRYELLOW		RGB(255,255,128)
#define COLOR_BRBLUE			RGB(128,128,255)
#define COLOR_BR2BLUE		RGB(196,196,255)
#define COLOR_BLUE			RGB(0,0,255)
#define COLOR_DKBLUE			RGB(0,0,128)
#define COLOR_DK2BLUE      RGB(0,0,64)
#define COLOR_RED				RGB(255,0,0)
#define COLOR_DKRED			RGB(128,0,0)
#define COLOR_BRRED			RGB(255,128,128)
#define COLOR_BR2RED			RGB(255,196,196)
#define COLOR_BRGREEN		RGB(128,255,128)
#define COLOR_GREEN			RGB(0,255,0)
#define COLOR_DKGREEN		RGB(0,128,0)
#define COLOR_BRGREY			RGB(196,196,196)
#define COLOR_BR2GREY		RGB(228,228,228)
#define COLOR_BRSKYBLUE		RGB(192,255,255)
#define COLOR_DRKBLUE		RGB(0,70,140)
#define COLOR_SKYBLUE		RGB(0,255,255)
#define COLOR_ORANGE			RGB(255,128,0)
#define COLOR_BRORANGE		RGB(255,192,2)
#define COLOR_PINK			RGB(255,0,255)
#define COLOR_BRPINK       RGB(255,128,255)
#define COLOR_VIOLET       RGB(128,0,255)
#define COLOR_DKORANGE     RGB(255, 128, 64)
#define COLOR_DK2ORANGE    RGB(160, 80, 40)



BOOL gfxCreateFont(LPCSTR szFontName, int nHeight, BOOL bBold, CFont& fontOut)
{
	CFont font;
	font.CreatePointFont(100, szFontName);

	LOGFONT logfont;
	ZeroMemory(&logfont, sizeof(logfont));
	font.GetLogFont(&logfont);
	logfont.lfHeight = nHeight;
	if (bBold) logfont.lfWeight = FW_BOLD;
	return fontOut.CreateFontIndirect(&logfont);
}

CRect gfxGetFittingRect(CRect rect, CSize s_size)
{
	CRect d_rect;
	float wfactor1, wfactor2, sfactor;
	wfactor1 = rect.Width() / (float)rect.Height();
	wfactor2 = s_size.cx / (float)s_size.cy;
	if (wfactor2 > wfactor1) {
		sfactor = rect.Width() / (float)s_size.cx;
		int d_height = (int)(s_size.cy * sfactor);
		d_rect.left = rect.left;
		d_rect.right = rect.right;
		d_rect.top = rect.top + (rect.Height() - d_height) / 2;
		d_rect.bottom = d_rect.top + d_height;
	}
	else {
		sfactor = rect.Height() / (float)s_size.cy;
		int d_width = (int)(s_size.cx * sfactor);
		d_rect.top = rect.top;
		d_rect.bottom = rect.bottom;
		d_rect.left = rect.left + (rect.Width() - d_width) / 2;
		d_rect.right = d_rect.left + d_width;
	}
	return d_rect;
}



IMPLEMENT_DYNAMIC(CImgViewStatic, CStatic)

CImgViewStatic::CImgViewStatic() : CStatic()
{
	m_bDrawCircleRect = FALSE;
	m_nBitmapWidth = m_nBitmapHeight = 0;
}

CImgViewStatic::~CImgViewStatic()
{
	m_Bitmap.DeleteObject();
}

BEGIN_MESSAGE_MAP(CImgViewStatic, CStatic)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

void CImgViewStatic::ClearStatic()
{
	m_Bitmap.DeleteObject();
	m_nBitmapWidth = m_nBitmapHeight = 0;
	m_strOSDText = "";
	RedrawWindow();
}


void CImgViewStatic::DisplayImage(cv::Mat s_image, BOOL bDrawNow)
{
	ASSERT(IsWindow(GetSafeHwnd()));
	m_Bitmap.DeleteObject();

	CClientDC dc(this);
	if (!s_image.size().width || !s_image.size().height)
		m_nBitmapWidth = m_nBitmapHeight = 0;
	else {
		CRect rect;
		GetClientRect(&rect);
		CRect d_rect = gfxGetFittingRect(rect, CSize(s_image.size().width, s_image.size().height));
		int   d_width = d_rect.Width();
		int   d_height = d_rect.Height();

		CDC mem_dc;
		mem_dc.CreateCompatibleDC(&dc);
		m_Bitmap.CreateCompatibleBitmap(&dc, d_width, d_height);
		CBitmap* old_bitmap = mem_dc.SelectObject(&m_Bitmap);

		BITMAPINFO bmp_info;
		memset((byte*)&bmp_info, 1, sizeof(BITMAPINFO));
		bmp_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmp_info.bmiHeader.biWidth = s_image.size().width;
		bmp_info.bmiHeader.biHeight = -s_image.size().height;
		bmp_info.bmiHeader.biPlanes = 1;
		bmp_info.bmiHeader.biBitCount = 24;
		bmp_info.bmiHeader.biCompression = BI_RGB;
		SetStretchBltMode(mem_dc.m_hDC, COLORONCOLOR);
		StretchDIBits(mem_dc.m_hDC,
			0, 0, d_width, d_height,
			0, 0, s_image.size().width, s_image.size().height,
			s_image.data, (BITMAPINFO*)&bmp_info,
			DIB_RGB_COLORS,
			SRCCOPY);
		mem_dc.SelectObject(old_bitmap);
		mem_dc.DeleteDC();

		m_nBitmapWidth = d_width;
		m_nBitmapHeight = d_height;
	}
	if (bDrawNow)
		DrawImg(&dc);
}

void CImgViewStatic::SetOSDText(LPCSTR szText)
{
	m_strOSDText = szText;
}

BOOL CImgViewStatic::OnEraseBkgnd(CDC* pDC)
{
	DrawImg(pDC);
	return TRUE;
}

void CImgViewStatic::DrawImg(CDC* pDC)
{
	CRect rect;
	GetClientRect(&rect);
	if (m_nBitmapWidth && m_nBitmapHeight) {
		CDC s_dc, d_dc;
		CBitmap d_bitmap;
		CRect d_rect = gfxGetFittingRect(rect, CSize(m_nBitmapWidth, m_nBitmapHeight));
		d_bitmap.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
		s_dc.CreateCompatibleDC(pDC);
		d_dc.CreateCompatibleDC(pDC);
		CBitmap* s_bitmap_old = s_dc.SelectObject(&m_Bitmap);
		CBitmap* d_bitmap_old = d_dc.SelectObject(&d_bitmap);
		d_dc.FillSolidRect(rect, RGB(64, 64, 64));
		d_dc.BitBlt(d_rect.left, d_rect.top, m_nBitmapWidth, m_nBitmapHeight, &s_dc, 0, 0, SRCCOPY);
		s_dc.SelectObject(s_bitmap_old);
		DrawOSDText(&d_dc);
		if (m_bDrawCircleRect)
			DrawCircleRect(&d_dc);
		pDC->BitBlt(0, 0, rect.Width(), rect.Height(), &d_dc, 0, 0, SRCCOPY);
		d_dc.SelectObject(d_bitmap_old);
	}
	else {
		pDC->FillSolidRect(rect, RGB(0, 0, 0));
		DrawOSDText(pDC);
	}
}

void CImgViewStatic::DrawRectangle(CDC* pDC, int x, int y, int width, int height, COLORREF pen_color, int pen_width, int pen_style)
{
	CPen pen(pen_style, pen_width, pen_color);
	pDC->SelectObject(&pen);
	pDC->SelectStockObject(NULL_BRUSH);
	pDC->Rectangle(CRect(x, y, width, height));
	pDC->SelectStockObject(NULL_PEN);
}

void CImgViewStatic::DrawOSDText(CDC* pDC)
{
	if (m_strOSDText.IsEmpty()) return;

	CRect rect;
	GetClientRect(&rect);

	CFont font, * old_font;
	gfxCreateFont("Tahoma", 14, FALSE, font);
	old_font = pDC->SelectObject(&font);
	pDC->SetTextColor(RGB(255, 255, 255));
	pDC->SetBkMode(TRANSPARENT);
	pDC->DrawText(m_strOSDText, rect, DT_RIGHT | DT_TOP | DT_SINGLELINE);
	pDC->SelectObject(old_font);
}

void CImgViewStatic::DrawCircleRect(CDC* pDC)
{
	CPen pen(PS_SOLID, 1, COLOR_WHITE);
	pDC->SelectObject(&pen);
	pDC->SelectStockObject(NULL_BRUSH);
	pDC->Ellipse(m_rectCircle);
	pDC->Rectangle(m_rectCircle);

	for (int i = 0; i < 8; i++) {
		pDC->Rectangle(m_rectMarker[i]);
	}

	int centerX = m_rectCircle.left + (int)((m_rectCircle.right - m_rectCircle.left) / 2);
	int centerY = m_rectCircle.top + (int)((m_rectCircle.bottom - m_rectCircle.top) / 2);
	pDC->MoveTo(centerX, centerY - 10);
	pDC->LineTo(centerX, centerY + 11);
	pDC->MoveTo(centerX - 10, centerY);
	pDC->LineTo(centerX + 11, centerY);

	pDC->SelectStockObject(NULL_PEN);
}
