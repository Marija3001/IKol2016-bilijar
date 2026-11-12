// Kol1View.h : interface of the CKol1View class  (I kolokvijum 2016 - bilijar)
#pragma once
#include "DImage.h"

class CKol1View : public CView
{
protected:
	CKol1View() noexcept;
	DECLARE_DYNCREATE(CKol1View)

public:
	CKol1Doc* GetDocument() const;

	virtual void OnDraw(CDC* pDC);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual ~CKol1View();

protected:
	// 5., 6. slike se ucitavaju samo jednom (u konstruktoru)
	DImage* m_pFelt;
	DImage* m_pWood;

	// 4. stanje scene
	double m_stickAngle;   // ugao stapa oko pocetnog centra kugle (stepeni)
	double m_stickDist;    // rastojanje vrha stapa od ivice kugle (u pocetnom polozaju)
	double m_ballAngle;    // pravac kretanja kugle (zapamcen u trenutku udarca)
	double m_ballDist;     // koliko se kugla udaljila od pocetnog polozaja

	// 1., 2.
	void DrawStick(CDC* pDC, int w);
	void DrawBall(CDC* pDC, int w);

	// 3.
	void Translate(CDC* pDC, float dX, float dY, bool rightMultiply);
	void Rotate(CDC* pDC, float angle, bool rightMultiply);   // angle u stepenima

	// 5., 6., 7.
	void DrawTable(CDC* pDC, CRect rect);
	void DrawBorder(CDC* pDC, CRect rect, int w);
	void DrawHoles(CDC* pDC, CRect rect, int size);

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};

#ifndef _DEBUG
inline CKol1Doc* CKol1View::GetDocument() const
   { return reinterpret_cast<CKol1Doc*>(m_pDocument); }
#endif
