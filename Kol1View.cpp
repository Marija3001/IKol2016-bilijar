// Kol1View.cpp : implementation of the CKol1View class  (I kolokvijum 2016 - bilijar)
#include "pch.h"
#include "framework.h"
#ifndef SHARED_HANDLERS
#include "Kol1.h"
#endif

#include "Kol1Doc.h"
#include "Kol1View.h"
#define _USE_MATH_DEFINES
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define BALL_X 600.0f       // 4. pocetni centar kugle
#define BALL_Y 400.0f
#define BALL_D 30           //    precnik kugle
#define STICK_LEN 600       //    duzina stapa

#define ROT_STEP 10.0       // ugao rotacije stapa po pritisku (stepeni)
#define MOVE_STEP 10.0      // pomeraj stapa / kugle po pritisku

#define BORDER_W 40         // 6. sirina okvira
#define HOLE_SIZE 40        // 7. precnik rupa

#define COLOR_OCHER   RGB(255, 190, 128)
#define COLOR_BROWN   RGB(64, 32, 0)
#define COLOR_BALL    RGB(128, 0, 0)
#define COLOR_BALL_BR RGB(64, 0, 0)

IMPLEMENT_DYNCREATE(CKol1View, CView)

BEGIN_MESSAGE_MAP(CKol1View, CView)
	ON_WM_ERASEBKGND()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

// ---------------------------------------------------------------
// 5., 6. Slike se ucitavaju samo jednom - u konstruktoru
// ---------------------------------------------------------------
CKol1View::CKol1View() noexcept
{
	m_pFelt = new DImage();  m_pFelt->Load(CString("felt2.jpg"));
	m_pWood = new DImage();  m_pWood->Load(CString("wood.jpg"));

	m_stickAngle = 160.0;    // stap dole levo od kugle, kao na slici
	m_stickDist = 100.0;
	m_ballAngle = m_stickAngle;
	m_ballDist = 0.0;
}

CKol1View::~CKol1View()
{
	delete m_pFelt;
	delete m_pWood;
}

BOOL CKol1View::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}

// ---------------------------------------------------------------
// 3. Transformacije
//    rightMultiply = false -> MWT_LEFTMULTIPLY  (lokalna, u odnosu na tekuci k.s.)
//    rightMultiply = true  -> MWT_RIGHTMULTIPLY (globalna)
// ---------------------------------------------------------------
void CKol1View::Translate(CDC* pDC, float dX, float dY, bool rightMultiply)
{
	XFORM form = { 1.0f, 0.0f, 0.0f, 1.0f, dX, dY };
	pDC->ModifyWorldTransform(&form, rightMultiply ? MWT_RIGHTMULTIPLY : MWT_LEFTMULTIPLY);
}

void CKol1View::Rotate(CDC* pDC, float angle, bool rightMultiply)
{
	float rad = (float)(angle * M_PI / 180.0);
	XFORM form = { cosf(rad), sinf(rad), -sinf(rad), cosf(rad), 0.0f, 0.0f };
	pDC->ModifyWorldTransform(&form, rightMultiply ? MWT_RIGHTMULTIPLY : MWT_LEFTMULTIPLY);
}

// ---------------------------------------------------------------
// 1. Stap u lokalnom k.s.: vrh u (0,0), deblji kraj u (w,0), osa = x osa.
//    Vrh je sirok w/100, dno 2*w/100. Prve 2/3 (od vrha) oker, zadnja 1/3 braon.
// ---------------------------------------------------------------
void CKol1View::DrawStick(CDC* pDC, int w)
{
	double h1 = w / 200.0;                     // pola sirine na vrhu
	double h2 = w / 100.0;                     // pola sirine na dnu
	double xm = 2.0 * w / 3.0;                 // granica oker/braon
	double hm = h1 + (h2 - h1) * 2.0 / 3.0;    // pola sirine na granici

	CPen penBrown(PS_SOLID, 1, COLOR_BROWN);
	CBrush brOcher(COLOR_OCHER);
	CBrush brBrown(COLOR_BROWN);
	CPen* oldPen = pDC->SelectObject(&penBrown);
	CBrush* oldBrush = pDC->SelectObject(&brBrown);

	// kruznica na debljem kraju
	pDC->Ellipse((int)(w - h2), (int)-h2, (int)(w + h2), (int)h2);

	// braon deo (zadnja 1/3)
	POINT brown[4] = {
		{ (int)xm, (int)-hm }, { w, (int)-h2 },
		{ w, (int)h2 },        { (int)xm, (int)hm } };
	pDC->Polygon(brown, 4);

	// oker deo (prve 2/3)
	pDC->SelectObject(&brOcher);
	POINT ocher[4] = {
		{ 0, (int)-h1 },       { (int)xm, (int)-hm },
		{ (int)xm, (int)hm },  { 0, (int)h1 } };
	pDC->Polygon(ocher, 4);

	// odsjaj: bela linija celom duzinom, 2 piksela unutar gornje ivice
	CPen penWhite(PS_SOLID, 1, RGB(255, 255, 255));
	pDC->SelectObject(&penWhite);
	pDC->MoveTo(0, (int)(-h1 + 2));
	pDC->LineTo(w, (int)(-h2 + 2));

	pDC->SelectObject(oldPen);
	pDC->SelectObject(oldBrush);
}

// ---------------------------------------------------------------
// 2. Kugla precnika w sa centrom u (0,0) lokalnog k.s.
// ---------------------------------------------------------------
void CKol1View::DrawBall(CDC* pDC, int w)
{
	int r = w / 2;

	CPen penBall(PS_SOLID, 1, COLOR_BALL_BR);
	CBrush brBall(COLOR_BALL);
	CPen* oldPen = pDC->SelectObject(&penBall);
	CBrush* oldBrush = pDC->SelectObject(&brBall);
	pDC->Ellipse(-r, -r, r, r);

	// odsjaj: bela kruznica 3x manjeg precnika, malo gore desno od centra
	int rr = w / 6;
	int cx = w / 6;
	int cy = -w / 6;
	CPen penWhite(PS_SOLID, 1, RGB(255, 255, 255));
	CBrush brWhite(RGB(255, 255, 255));
	pDC->SelectObject(&penWhite);
	pDC->SelectObject(&brWhite);
	pDC->Ellipse(cx - rr, cy - rr, cx + rr, cy + rr);

	pDC->SelectObject(oldPen);
	pDC->SelectObject(oldBrush);
}

// ---------------------------------------------------------------
// 5. Povrsina stola - felt2.jpg poplocan (bez rastezanja) preko rect
// ---------------------------------------------------------------
void CKol1View::DrawTable(CDC* pDC, CRect rect)
{
	int w = m_pFelt->Width();
	int h = m_pFelt->Height();

	for (int y = rect.top; y < rect.bottom; y += h)
	{
		for (int x = rect.left; x < rect.right; x += w)
		{
			// poslednja slika u redu/koloni se odseca da ne izadje iz rect
			int cw = min(w, rect.right - x);
			int ch = min(h, rect.bottom - y);
			m_pFelt->Draw(pDC, CRect(0, 0, cw, ch), CRect(x, y, x + cw, y + ch));
		}
	}
}

// ---------------------------------------------------------------
// 6. Okvir sirine w - wood.jpg se crta samo van unutrasnjeg pravougaonika
// ---------------------------------------------------------------
void CKol1View::DrawBorder(CDC* pDC, CRect rect, int w)
{
	CRect inner(rect);
	inner.DeflateRect(w, w);

	pDC->ExcludeClipRect(&inner);
	m_pWood->Draw(pDC, CRect(0, 0, m_pWood->Width(), m_pWood->Height()), rect);
	pDC->SelectClipRgn(NULL);
}

// ---------------------------------------------------------------
// 7. 6 rupa precnika size, centri odmaknuti za size od ivica prozora
// ---------------------------------------------------------------
void CKol1View::DrawHoles(CDC* pDC, CRect rect, int size)
{
	int r = size / 2;
	int xs[3] = { rect.left + size, rect.CenterPoint().x, rect.right - size };
	int ys[2] = { rect.top + size, rect.bottom - size };

	CBrush brBlack(RGB(0, 0, 0));
	CPen penBlack(PS_SOLID, 1, RGB(0, 0, 0));
	CPen* oldPen = pDC->SelectObject(&penBlack);
	CBrush* oldBrush = pDC->SelectObject(&brBlack);

	for (int j = 0; j < 2; j++)
		for (int i = 0; i < 3; i++)
			pDC->Ellipse(xs[i] - r, ys[j] - r, xs[i] + r, ys[j] + r);

	pDC->SelectObject(oldPen);
	pDC->SelectObject(oldBrush);
}

// ---------------------------------------------------------------
// 8. Crtanje bez treperenja: sve u memorijski DC, pa BitBlt
// ---------------------------------------------------------------
void CKol1View::OnDraw(CDC* pDC)
{
	CKol1Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	CRect rect;
	GetClientRect(&rect);

	CDC memDC;
	memDC.CreateCompatibleDC(pDC);
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
	CBitmap* oldBmp = memDC.SelectObject(&bmp);

	int oldMode = memDC.SetGraphicsMode(GM_ADVANCED);
	XFORM oldForm;
	memDC.GetWorldTransform(&oldForm);

	CRect inner(rect);
	inner.DeflateRect(BORDER_W, BORDER_W);
	DrawBorder(&memDC, rect, BORDER_W);
	DrawTable(&memDC, inner);
	DrawHoles(&memDC, rect, HOLE_SIZE);

	// kugla: od pocetnog centra se pomera suprotno od stapa (ballAngle + 180)
	double rad = m_ballAngle * M_PI / 180.0;
	float bx = (float)(BALL_X - m_ballDist * cos(rad));
	float by = (float)(BALL_Y - m_ballDist * sin(rad));
	Translate(&memDC, bx, by, false);
	DrawBall(&memDC, BALL_D);
	memDC.SetWorldTransform(&oldForm);

	// stap: rotira se oko pocetnog centra kugle, vrh je na (r + dist) od centra
	Translate(&memDC, BALL_X, BALL_Y, false);
	Rotate(&memDC, (float)m_stickAngle, false);
	Translate(&memDC, (float)(BALL_D / 2.0 + m_stickDist), 0.0f, false);
	DrawStick(&memDC, STICK_LEN);
	memDC.SetWorldTransform(&oldForm);

	memDC.SetGraphicsMode(oldMode);

	pDC->BitBlt(0, 0, rect.Width(), rect.Height(), &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(oldBmp);
}

BOOL CKol1View::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

// ---------------------------------------------------------------
// 4. Tastatura
//    <- / ->  rotacija stapa oko centra kugle
//    gore     stap ide ka kugli; kad je dodirne staje, a kugla krece
//             u pravcu stapa (pravac se pamti u trenutku udarca)
//    dole     kugla se vraca na pocetak; kad stigne staje, a stap se udaljava
// ---------------------------------------------------------------
void CKol1View::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar)
	{
	case VK_LEFT:
		m_stickAngle -= ROT_STEP;
		break;

	case VK_RIGHT:
		m_stickAngle += ROT_STEP;
		break;

	case VK_UP:
		if (m_stickDist > 0.0)
		{
			m_stickDist = max(0.0, m_stickDist - MOVE_STEP);   // stap staje na kugli
		}
		else
		{
			if (m_ballDist == 0.0)
				m_ballAngle = m_stickAngle;                      // udarac - pamti pravac
			m_ballDist += MOVE_STEP;
		}
		break;

	case VK_DOWN:
		if (m_ballDist > 0.0)
			m_ballDist = max(0.0, m_ballDist - MOVE_STEP);       // kugla staje na pocetku
		else
			m_stickDist += MOVE_STEP;
		break;

	default:
		CView::OnKeyDown(nChar, nRepCnt, nFlags);
		return;
	}

	Invalidate();
}
