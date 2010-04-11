// BmpMan.cpp: implementation of the CBmpMan class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mighty.h"

#include "BmpMan.h"
#include "Board.h"
#include "InfoBar.h"
#include "MainFrm.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CBmpMan::CBmpMan()
{
	m_szCards = m_szBackground = m_szExpandedBackground
		= CSize(0, 0);
	m_pZoomFont = 0;

	m_bmMask.LoadBitmap( IDB_MASK );

	m_bmShadow.LoadBitmap( IDB_GRAY );
	m_brShadow.CreatePatternBrush( &m_bmShadow );
}

CBmpMan::~CBmpMan()
{
	if ( m_font.GetSafeHandle() )
		m_font.DeleteObject();

	if ( m_bmCards.GetSafeHandle() )
		m_bmCards.DeleteObject();
	if ( m_bmBackground.GetSafeHandle() )
		m_bmBackground.DeleteObject();
	if ( m_bmExpandedBackground.GetSafeHandle() )
		m_bmExpandedBackground.DeleteObject();

	m_bmShadow.DeleteObject();
	m_brShadow.DeleteObject();
}

// �ޱ׸��� ������ ��� ī�� �̹����� �о� ���δ�
// �� �� ��Ʈ�� �Բ� ��Ʈ�Ѵ�
bool CBmpMan::LoadAllCards( CDC* pDC, CFont* pFont )
{
	ASSERT( pFont->GetSafeHandle() );
	m_pZoomFont = pFont;

	// ��Ʈ ����
	LOGFONT lf;
	pFont->GetLogFont( &lf );
	lf.lfHeight = -14;
	if ( m_font.GetSafeHandle() ) m_font.DeleteObject();
	VERIFY( m_font.CreateFontIndirect( &lf ) );

	if ( m_bmCards.GetSafeHandle() )
		m_bmCards.DeleteObject();

	CSize szCard = m_szCards = GetBitmapSize( IDB_SA );

	CDC dc; dc.CreateCompatibleDC( pDC );
	m_bmCards.CreateCompatibleBitmap( pDC, szCard.cx*(52+3), szCard.cy );
	dc.SelectObject( &m_bmCards );

	CDC dcTemp; dcTemp.CreateCompatibleDC( pDC );

	for ( int i = 0; i < 52 + 2; i++ ) {

		CBitmap bmCard; bmCard.LoadBitmap( IDB_SA + (unsigned)i );
		CBitmap* pbmTempOld = dcTemp.SelectObject( &bmCard );

		dc.BitBlt(
			szCard.cx * (i+1), 0,
			szCard.cx * (i+2), szCard.cy,
			&dcTemp, 0, 0, SRCCOPY );

		dcTemp.SelectObject( pbmTempOld );
		bmCard.DeleteObject();
	}
	return true;
}

// ī�� �� ���� �׸���
// nCard : int �� ġȯ�� ī��
void CBmpMan::DrawCard( CDC* pDC, int nCard, int x, int y, int cx, int cy )
{
	DrawCardEx( pDC, nCard, x, y, cx, cy,
		0, 0, m_szCards.cx, m_szCards.cy );
}

void CBmpMan::DrawCard( CDC* pDC, int nCard, const CRect& rc )
{
	DrawCard( pDC, nCard, rc.left, rc.top, rc.Width(), rc.Height() );
}

void CBmpMan::DrawCardEx( CDC* pDC, int nCard,
	const CRect& rcTgt, const CRect& rcSrc )
{
	DrawCardEx( pDC, nCard,
		rcTgt.left, rcTgt.top, rcTgt.Width(), rcTgt.Height(),
		rcSrc.left, rcSrc.top, rcSrc.Width(), rcSrc.Height() );
}

void CBmpMan::DrawCardEx( CDC* pDC, int nCard,
	int xTgt, int yTgt, int cxTgt, int cyTgt,
	int xSrc, int ySrc, int cxSrc, int cySrc )
{
	ASSERT( m_bmCards.GetSafeHandle() );

	if ( nCard < 0 || nCard > 53 ) {
		ASSERT(0); nCard = 0;
	}

	CDC dc; dc.CreateCompatibleDC( pDC );

	dc.SelectObject( &m_bmCards );

	pDC->StretchBlt( xTgt, yTgt, cxTgt, cyTgt,
		&dc, xSrc+54*m_szCards.cx, ySrc, cxSrc, cySrc,
		SRCPAINT );
	pDC->StretchBlt( xTgt, yTgt, cxTgt, cyTgt,
		&dc, xSrc+nCard*m_szCards.cx, ySrc, cxSrc, cySrc,
		SRCAND );

	if( cxTgt > cyTgt ) {
		POINT pt[3];
		pt[0].x = xTgt+cxTgt;
		pt[0].y = yTgt;
		pt[1].x = xTgt+cxTgt;
		pt[1].y = yTgt+cyTgt;
		pt[2].x = xTgt;
		pt[2].y = yTgt;
		pDC->PlgBlt( pt, &dc, xSrc+nCard*m_szCards.cx, ySrc, cxSrc, cySrc, m_bmMask, 0, 0 );
	}

	// ���ڸ� ����
	if ( Mo()->bCardHelp && CCard::GetState() ) {
		// ī�� ���� ���
		CCard c(nCard);
		if ( c.IsMighty() || c.IsJokercall() ) {
			// Ư��ī���϶�
			CString sText = c.GetString( Mo()->bUseTerm );

			// ������ ����(���� ���)�� ����Ѵ�
			int x = xTgt-(cxTgt*xSrc)/cxSrc+m_szCards.cx*cxTgt/cxSrc/2;
			int y = yTgt-(cyTgt*ySrc)/cySrc+m_szCards.cy*cyTgt/cySrc/4;
			CRect rcText( xTgt, yTgt, xTgt+cxTgt, yTgt+cyTgt );

			// ���ڸ� ���
			// ǥ�� ũ���̸� m_font ��, �׷��� ������ m_pZoomFont �� ����
			ASSERT( m_font.GetSafeHandle() && m_pZoomFont
					&& m_pZoomFont->GetSafeHandle() );
			if ( cxSrc == cxTgt && cySrc == cyTgt )
				pDC->SelectObject( &m_font );
			else pDC->SelectObject( m_pZoomFont );

			pDC->SetBkMode( TRANSPARENT );
			pDC->SetTextAlign( TA_CENTER );
			pDC->SetTextColor( RGB( 255, 0, 0 ) );	// ������
			pDC->ExtTextOut( x, y, ETO_CLIPPED,
				&rcText, sText, sText.GetLength(), 0 );
		}
	}

	if( cxTgt > cyTgt ) {
		XFORM xform;
		dc.GetWorldTransform( &xform );
		xform.eM11 = 1.0f;
		xform.eM12 = 0.0f;
		xform.eM21 = 0.0f;
		xform.eM22 = 1.0f;
		xform.eDx = 0.0f;
		xform.eDy = 0.0f;

		dc.SetWorldTransform( &xform );
	}

	dc.SetGraphicsMode(GM_COMPATIBLE);

}

// �� �׸��� �д´�
bool CBmpMan::LoadBackPicture( CDC* pDC, int nIndex, LPCTSTR sFileName )
{
	ASSERT( m_bmCards.GetSafeHandle() );
	ASSERT( nIndex >= 0 && nIndex <= 10
		|| nIndex == -1 );

	HBITMAP hBitmap;

	if ( nIndex >= 0 && nIndex <= 10 ) {
		hBitmap = (HBITMAP) LoadImage( AfxGetInstanceHandle(),
			MAKEINTRESOURCE( IDB_BACK1 + nIndex ), IMAGE_BITMAP,
			0, 0, LR_DEFAULTCOLOR );
	}
	else {
		hBitmap = (HBITMAP) LoadImage( 0, sFileName,
			IMAGE_BITMAP, 0, 0,
			LR_DEFAULTCOLOR|LR_LOADFROMFILE );
	}

	if ( !hBitmap ) return false;

	CDC dc; dc.CreateCompatibleDC( pDC );
	dc.SelectObject( &m_bmCards );

	CDC dcSrc; dcSrc.CreateCompatibleDC( pDC );
	CBitmap bmSrc; bmSrc.Attach( hBitmap );
	BITMAP bmInfo; bmSrc.GetBitmap( &bmInfo );
	CBitmap* pbmOldSrc = dcSrc.SelectObject( &bmSrc );

	dc.StretchBlt( 0, 0, m_szCards.cx, m_szCards.cy,
		&dcSrc, 0, 0, bmInfo.bmWidth, bmInfo.bmHeight, SRCCOPY );

	// �׵θ��� �׸���
	dc.MoveTo( 2, 0 );
	dc.LineTo( 0, 2 );
	dc.LineTo( 0, m_szCards.cy-1 - 2 );
	dc.LineTo( 2, m_szCards.cy-1 );
	dc.LineTo( m_szCards.cx-1 - 2, m_szCards.cy-1 );
	dc.LineTo( m_szCards.cx-1, m_szCards.cy-1 - 2 );
	dc.LineTo( m_szCards.cx-1, 2 );
	dc.LineTo( m_szCards.cx-1 - 2, 0 );
	dc.LineTo( 2, 0 );

	dcSrc.SelectObject( pbmOldSrc );

	bmSrc.DeleteObject();

	return true;
}

// ��� �׸��� �д´�
bool CBmpMan::LoadBackground( CDC* pDC, LPCTSTR sFileName )
{
	HBITMAP hBitmap;
	
	hBitmap = (HBITMAP) LoadImage( 0, sFileName,
		IMAGE_BITMAP, 0, 0,
		LR_DEFAULTCOLOR|LR_LOADFROMFILE );

	if ( !hBitmap ) return false;

	if ( m_bmBackground.GetSafeHandle() )
		m_bmBackground.DeleteObject();

	CDC dcSrc; dcSrc.CreateCompatibleDC( pDC );
	CBitmap bmSrc; bmSrc.Attach( hBitmap );
	BITMAP bmInfo; bmSrc.GetBitmap( &bmInfo );
	m_szBackground.cx = bmInfo.bmWidth;
	m_szBackground.cy = bmInfo.bmHeight;
	CBitmap* pbmOldSrc = dcSrc.SelectObject( &bmSrc );

	CDC dc; dc.CreateCompatibleDC( pDC );
	m_bmBackground.CreateCompatibleBitmap(
		pDC, bmInfo.bmWidth, bmInfo.bmHeight );
	dc.SelectObject( &m_bmBackground );

	dc.BitBlt( 0, 0, bmInfo.bmWidth, bmInfo.bmHeight,
		&dcSrc, 0, 0, SRCCOPY );

	dcSrc.SelectObject( pbmOldSrc );

	bmSrc.DeleteObject();

	// Ȯ���� ��浵 ��ȿ�� �ȴ�
	if ( m_bmExpandedBackground.GetSafeHandle() ) {
		m_bmExpandedBackground.DeleteObject();
		m_szExpandedBackground = CSize(0,0);
	}

	return true;
}

// ��� �׸��� �׸���
// �� �Լ��� Invalid ������ ���� ��� �׸� ������
// �׸��� ���ؼ� ���ȴ�
void CBmpMan::DrawBackground( bool bUseBackground,
							 bool bTile, bool bExpand, COLORREF colBack,
							 CDC* pDC, int x, int y, int cx, int cy,
							 int xView, int yView, int xOffset, int yOffset )
{
	CRect rc( x, y, x+cx, y+cy );
	DrawBackground( bUseBackground, bTile, bExpand, colBack,
		pDC, rc, xView, yView, xOffset, yOffset );
}

void CBmpMan::DrawBackground( bool bUseBackground,
							 bool bTile, bool bExpand, COLORREF colBack,
							 CDC* pDC, const CRect& _rc, int xView, int yView,
							 int xOff, int yOff )
{
	CRect rc(_rc);
	rc.NormalizeRect();

	if ( !bUseBackground ) {
		// ����� ������� �ʴ´� ( �ܻ����� ĥ�� )
		pDC->FillSolidRect(
			rc.left + xOff,
			rc.top + yOff,
			rc.Width(), rc.Height(),
			colBack );
		return;
	}

	CDC dc; dc.CreateCompatibleDC( pDC );
	dc.SelectObject( &m_bmBackground );

	if ( !bTile ) {
		// Ÿ�� ���� �ƴ� ��� �ѹ��� �׸���
		if ( bExpand ) {
			// Ȯ��

			CDC dcExp; dcExp.CreateCompatibleDC( pDC );

			if ( m_szExpandedBackground != CSize( xView, yView )
				|| !m_bmExpandedBackground.GetSafeHandle() ) {
				// ���� Ȯ��� ����� ���� �ȸ������ �ְų�
				// ũ�Ⱑ ���� �ʴٸ� ���� �����

				if ( m_szExpandedBackground != CSize( xView, yView )
					&& m_bmExpandedBackground.GetSafeHandle() )
					// �̹� �����Ѵٸ� �����
					m_bmExpandedBackground.DeleteObject();
				// ��Ʈ�� ��ü ����
				if ( !m_bmExpandedBackground.GetSafeHandle() )
					m_bmExpandedBackground.CreateCompatibleBitmap(
						pDC, xView, yView );
				// ũ�� �缳��
				m_szExpandedBackground = CSize( xView, yView );

				// ��Ʈ�� Ȯ�� ����
				dcExp.SelectObject( &m_bmExpandedBackground );
				dcExp.StretchBlt( 0, 0, xView, yView,
					&dc, 0, 0, m_szBackground.cx, m_szBackground.cy,
					SRCCOPY );
			}
			else dcExp.SelectObject( &m_bmExpandedBackground );

			pDC->BitBlt( rc.left+xOff, rc.top+yOff,
				rc.Width(), rc.Height(),
				&dcExp, rc.left, rc.top, SRCCOPY );
		}
		else {
			// ���

			// �׸� ���� : �׸� ������ ������ �׷��� ������
			// �������� ��Ÿ���� �簢��
			CRect rcDraw;

			// ��� ��Ʈ�� ũ��
			CRect rcCenter( CPoint( xView/2-m_szBackground.cx/2+xOff,
									yView/2-m_szBackground.cy/2+yOff ),
							CSize(	m_szBackground.cx,
									m_szBackground.cy ) );

			if ( rcDraw.IntersectRect( &rc, &rcCenter ) )
				// ��� ��Ʈ���� �׷��� �Ѵٸ� (rcDraw ��
				// ������� �ʴ´ٸ�) �׸��� 1:1�� �׸���
				pDC->BitBlt(
					rcDraw.left+xOff, rcDraw.top+yOff,
					rcDraw.Width(), rcDraw.Height(),
					&dc,
					rcDraw.left - rcCenter.left,
					rcDraw.top - rcCenter.top,
					SRCCOPY );

			// ������ ������ ���������� ä���
			COLORREF col = colBack;
			if ( yView > m_szBackground.cy ) {
				pDC->FillSolidRect(
					( CRect( 0, 0, xView, yView/2-m_szBackground.cy/2 )
						& rc ) + CPoint( xOff, yOff ), col );
				pDC->FillSolidRect(
					( CRect( 0, yView/2-m_szBackground.cy/2+m_szBackground.cy, xView, xView)
						& rc ) + CPoint( xOff, yOff ), col );
			}
			if ( xView > m_szBackground.cx ) {
				pDC->FillSolidRect(
					( CRect( 0, 0, xView/2-m_szBackground.cx/2, yView )
						& rc ) + CPoint( xOff, yOff ), col );
				pDC->FillSolidRect(
					( CRect( xView/2-m_szBackground.cx/2+m_szBackground.cx, 0, xView, yView )
						& rc ) + CPoint( xOff, yOff ), col );
			}
		}
		return;
	}

	// Ÿ�� ��

	int yOffset = rc.top;
	while ( yOffset < rc.bottom ) {
		int ySize = max(
			min( m_szBackground.cy, rc.bottom - yOffset )
				- yOffset % m_szBackground.cy,
			rc.bottom - yOffset );

		int xOffset = rc.left;
		while ( xOffset < rc.right ) {
			int xSize = max(
				min( m_szBackground.cx, rc.right - xOffset )
					- xOffset % m_szBackground.cx,
				rc.right - xOffset );

			pDC->BitBlt( xOffset+xOff, yOffset+yOff, xSize, ySize,
				&dc,
				xOffset % m_szBackground.cx, yOffset % m_szBackground.cy,
				SRCCOPY );

			if ( xOffset % m_szBackground.cx )
				xOffset += m_szBackground.cx - xOffset % m_szBackground.cx;
			else xOffset += m_szBackground.cx;
		}

		if ( yOffset % m_szBackground.cy )
			yOffset += m_szBackground.cy - yOffset % m_szBackground.cy;
		else yOffset += m_szBackground.cy;
	}
}

// ������ �׸���
void CBmpMan::DrawShade( CDC* pDC, const CRect& rc, bool bOrigin )
{
	DrawShade( pDC, rc.left, rc.top, rc.Width(), rc.Height(), bOrigin );
}

void CBmpMan::DrawShade( CDC* pDC, int x, int y, int cx, int cy, bool bOrigin )
{
	// �귯�� ������ �����Ѵ�
	VERIFY( m_brShadow.UnrealizeObject() );
	pDC->SetBrushOrg( bOrigin ? 1 : 0, 0 );

	CBrush* pbr = pDC->SelectObject( &m_brShadow );

	COLORREF colText = pDC->SetTextColor( RGB(0,0,0) );	// ������ �׸���
	COLORREF colBk = pDC->SetBkColor( RGB(255,255,255) );

	pDC->BitBlt( x, y, cx, cy, NULL, 0, 0, 0x00A000C9 /*DPa*/ );

	pDC->SelectObject( pbr );
	pDC->SetBkColor( colBk );
	pDC->SetTextColor( colText );
}
