// DSB_Fill.cpp: implementation of the DSB Filling function
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mighty.h"
#include "BmpMan.h"
#include "DSB.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


static CImageList s_imgl;
static CSize s_szShade;
static bool s_bBlendingFailed = false;
static bool s_bImageListCreated = false;

static bool CreateBlendImageList()
{
	// �ѹ� ���д� ������ ����
	if ( s_bBlendingFailed ) return false;

	s_szShade = ::GetBitmapSize( IDB_SHADE );

	if ( !s_imgl.Create( s_szShade.cx, s_szShade.cy,
						ILC_COLOR16 | ILC_MASK, 0, 1 ) ) {
		s_bBlendingFailed = true;
		return false;
	}

	CBitmap bmShade;
	bmShade.LoadBitmap( IDB_SHADE );

	// �������� ����ũ ����
	s_imgl.Add( &bmShade, RGB(255,0,0) );

	VERIFY( bmShade.DeleteObject() );

	s_bImageListCreated = true;
	return true;
}

static bool DsbBlend( CDC* pDC, const CRect& rc )
{
	if ( s_bBlendingFailed ) return false;

	// 256 �� ���ϸ� �����Ѵ�
	if ( pDC->GetDeviceCaps( NUMCOLORS ) != -1 ) {
		s_bBlendingFailed = true;
		return false;
	}

	if ( !s_bImageListCreated )
		if ( !CreateBlendImageList() )
			return false;

	// Ŭ�� ������ ��´�
	CRect rcClip;
	pDC->GetClipBox( &rcClip );
	rcClip &= rc;

	// rc.left, rc.top ���� s_szShade �� ����� �ǵ���
	// rcClip.left, rcClip.top �� �����Ѵ�
	rcClip.left = ( rcClip.left - rc.left )
				/ s_szShade.cx * s_szShade.cx + rc.left;
	rcClip.top = ( rcClip.top - rc.top )
				/ s_szShade.cy * s_szShade.cy + rc.top;

	// Ÿ�� ���·� ���̵带 ä���
	int nYRem = rcClip.Height();
	for ( int y = rcClip.top; nYRem > 0; y += s_szShade.cy, nYRem -= s_szShade.cy ) {
		int cy = min( s_szShade.cy, nYRem );
		int nXRem = rcClip.Width();
		for ( int x = rcClip.left; nXRem > 0; x += s_szShade.cx, nXRem -= s_szShade.cx ) {
			int cx = min( s_szShade.cx, nXRem );
			ImageList_DrawEx(
				s_imgl.GetSafeHandle(), 0, pDC->GetSafeHdc(),
				x, y, cx, cy, CLR_NONE, CLR_NONE,
				ILD_BLEND50 );
		}
	}

	return true;
}

// ����� �׸���
// nDSBShadeMode : ĥ�ϱ� ��� ( 1 : ������ 2 : ����  �׿� : ��ġ )
void DSB::FillBack( CDC* pDC, LPCRECT prc,
	CBmpMan* pBmpMan, int nDSBShadeMode, bool bBrushOrigin )
{

	if ( nDSBShadeMode == 1 ) // ������
		pDC->FillSolidRect( prc, RGB(64,64,64) );
	else if ( nDSBShadeMode == 2 // ������
		&& DsbBlend( pDC, *prc ) ) ;
	else // ��ġ, �Ǵ� Blending ����
		pBmpMan->DrawShade( pDC,
			prc->left, prc->top,
			prc->right - prc->left,
			prc->bottom - prc->top, bBrushOrigin );
}
