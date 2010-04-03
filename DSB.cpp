// DSB.cpp: implementation of the DSB class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mighty.h"
#include "DSB.h"

#include "BoardWrap.h"
#include "BmpMan.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

COLORREF DSB::s_colWhite = RGB(255,255,255);
COLORREF DSB::s_colCyan = RGB(0,255,255);
COLORREF DSB::s_colYellow = RGB(255,255,0);
COLORREF DSB::s_colGray = RGB(192,192,192);
COLORREF DSB::s_colLightGray = RGB(192,192,192);
bool DSB::s_bColorLoaded = false;
const int DSB::s_tdNormal = 0;
const int DSB::s_tdShade = (TD_SHADE|TD_SUNKEN);
const int DSB::s_tdOutline = TD_OUTLINE;
const int DSB::s_tdShadeOutline = (TD_SHADE|TD_OUTLINE);
const int DSB::s_tdShadeOpaque = (TD_SHADE|TD_OPAQUE);
const int DSB::s_tdOutlineOpaque = (TD_OUTLINE|TD_OPAQUE);
const int DSB::s_tdShadeOutlineOpaque = (TD_SHADE|TD_OUTLINE|TD_OPAQUE);
const int DSB::s_tdMidium = TD_SIZEMIDIUM;
const int DSB::s_tdMidiumShade = (TD_SIZEMIDIUM|TD_SHADE|TD_SUNKEN);
const int DSB::s_tdMidiumOutline = (TD_SIZEMIDIUM|TD_OUTLINE);
const int DSB::s_tdMidiumShadeOutline = (TD_SIZEMIDIUM|TD_SHADE|TD_OUTLINE);

CList<int,int> DSB::s_lID;
CList<DSB*,DSB*> DSB::s_lPtr;
CCriticalSection DSB::s_csDSB;

DSB::GarbageCollector DSB::s_gc;

// DSB �� �⺻ ������ ��Ʈ�Ѵ�
void DSB::SetDefaultColor(
	COLORREF colText, COLORREF colStrong1,
	COLORREF colStrong2, COLORREF colGray )
{
	s_colWhite = colText;
	s_colCyan = colStrong1;
	s_colYellow = colStrong2;
	s_colGray = colGray;
	s_colLightGray =
		RGB( ( GetRValue(colText) + GetRValue(colGray) ) / 2,
			( GetGValue(colText) + GetGValue(colGray) ) / 2,
			( GetBValue(colText) + GetBValue(colGray) ) / 2 );
}


DSB::DSB( CBoardWrap* pBoard ) : m_pBoard(pBoard)
{
	m_rcDSB.SetRectEmpty();
	m_pBmpMan = 0;
	m_nTimeOut = -1;
	m_pntOffset.x = m_pntOffset.y = 0;
	m_posSel = 0;
	m_bFixed = false;
	m_bModal = false;
	m_bBelow = false;

	// ���ο� ID �� �Ҵ��Ѵ� ( s_lID �� �ִ� �ִ밪 + 1 )
	s_csDSB.Lock();
	int nMaxID = -1;
	POSITION pos = s_lID.GetHeadPosition();
	while (pos) {
		int nID = s_lID.GetNext(pos);
		nMaxID = max(nID,nMaxID);
	}
	m_nID = nMaxID+1;
	m_pEvent = 0;
	m_pResult = 0;
	s_lID.AddTail(m_nID);
	s_lPtr.AddTail( this );	// ������ ����Ʈ���� �߰�
	s_csDSB.Unlock();
}

DSB::~DSB()
{
	// �� ID �� s_lID ���� �����Ѵ�
	s_csDSB.Lock();
	POSITION pos = s_lID.GetHeadPosition();
	for (; pos; s_lID.GetNext(pos) )
		if ( s_lID.GetAt(pos) == m_nID ) {
			s_lID.RemoveAt(pos);
			break;
		}
	ASSERT( pos );
	// ������ ����Ʈ������ ����
	pos = s_lPtr.Find( this ); ASSERT(pos);
	s_lPtr.RemoveAt(pos);
	s_csDSB.Unlock();

	if ( m_pEvent ) m_pEvent->SetEvent();
}

// ���ο� ���ڸ� �����Ѵ�
// CBoard �� �ڽ��� ����ϰ� ���� �׸���
// ��ġ xDSB, yDSB �� �»�� �𼭸��� �ƴ϶�
// ��ũ�� �߽ɿ��� ���� �߽��� ������� ����°�
// �ϴ� ���̴� !!!
void DSB::Create(
	int xDSB, int yDSB,				// ��ġ
	int cxDSB, int cyDSB,			// ũ��
	int nTimeOut )					// ���� (�и���:-1�� ����)
{
	if ( !s_bColorLoaded ) {
		s_bColorLoaded = true;
		DSB::SetDefaultColor( (COLORREF)Mo()->nDSBText, (COLORREF)Mo()->nDSBStrong1,
			(COLORREF)Mo()->nDSBStrong2, (COLORREF)Mo()->nDSBGray );
	}
	ASSERT( m_pBoard );

	if ( (HWND)*m_pBoard == 0 ) delete this;
	else {
		m_nTimeOut = nTimeOut;
		m_rcDSB.SetRect( xDSB, yDSB, cxDSB, cyDSB );
		m_pBoard->AddDSB( this );
	}
}

// ���ο� ���ڸ� �����Ѵ�
// CBoard �� �ڽ��� ����ϰ� ���� �׸���
// x, y �� DP ��ǥ
void DSB::Create2(
	int x, int y,					// ��ġ
	int cxDSB, int cyDSB,			// DSB ������ ����� ũ��
	int nTimeOut )					// ���� (�и���:-1�� ����)
{
	// ���� x, y �� DSB �� ǥ���Ϸ��� �߽ɿ���
	// ��� ������ �̵��ؾ� �ұ�

	int nMyUnit = GetDSBUnit();
	int nBoardUnit = m_pBoard->GetDSBUnit();

	// ���� ũ��
	int cx = cxDSB*nMyUnit, cy = cyDSB*nMyUnit;
	// CBoard ũ��
	CRect rc; m_pBoard->GetClientRect(&rc);
	// ȭ���� ����� �Ǹ� ��ġ�� �����Ѵ�
	x = max( rc.left, min( rc.right - cx, x ) );
	y = max( rc.top, min( rc.bottom - cy, y ) );
	// �׳� �̵� ������ �� ǥ�õǴ� ��ġ
	int xBefore = rc.left + rc.Width()/2 - cx/2,
		yBefore = rc.top + rc.Height()/2 - cy/2;
	// �̵��ؾ� �ϴ� ��
	int xToMove = x - xBefore, yToMove = y - yBefore;
	// �̵����� DSB ������
	int xToMoveDSB = (xToMove+nBoardUnit-1) / nBoardUnit;
	int yToMoveDSB = (yToMove+nBoardUnit-1) / nBoardUnit;
	// ���̴� offset ���� �����Ѵ�
	m_pntOffset.x = xToMove - xToMoveDSB*nBoardUnit;
	m_pntOffset.y = yToMove - yToMoveDSB*nBoardUnit;

	DSB::Create( xToMoveDSB, yToMoveDSB, cxDSB, cyDSB, nTimeOut );
}

// ���ڸ� �����ϰ� �ڽ��� delete �Ѵ�
// CBoard ���� ����� ����� ���� �׸���
void DSB::Destroy()
{
	OnDestroy();
	ASSERT( m_pBoard );
	VERIFY( m_pBoard->RemoveDSB( this ) );
	delete this;
}

// �� ���ڴ� ���� ũ�Ⱑ ���ص�
// ũ�Ⱑ ������ �ʵ��� �����
// ( Create ���� �� �Լ��� ȣ���ؾ� �Ѵ� )
void DSB::SetFixed()
{
	m_bFixed = true;
}

// ���콺�� �� �簢���� �� ��(CBoard���� ������ǥ)��
// Ŭ���ϰų� ������ �� ������ ȿ���� �ش�
// Ŭ��(bCheckOnly==true)�ߴٸ� �� ���� �̺�Ʈ��
// ����ġ �ϸ�, ���콺�� �������ٸ�(bCheckOnly==false)
// �� �� ������ ���̶���Ʈ �Ѵ�
// ���� � �� ���̿� �ɷ� �ִٸ� ����,
// �׷��� �ʴٸ� ������ �����Ѵ�
bool DSB::DispatchHotspot( int x, int y, bool bCheckOnly )
{
	// �� DSB �� �簢��
	CRect rcBox; GetRect( &rcBox );

	// ������ ���� x ��ư�� Ư���ϰ� ó���Ѵ�
	if ( m_nTimeOut < 0 ) {
		// timeout �� ���Ѵ��� ���� x ��ư�� ��Ÿ����
		CRect rcX( CPoint( rcBox.left + rcBox.Width() - 17, rcBox.top + 5 ),
					CSize( 14, 14 ) );
		if ( rcX.PtInRect( CPoint(x,y) ) ) {
			if ( !bCheckOnly ) {
				PlaySound( IDW_BEEP );
				OnClick( 0 );	// ���� �ڵ�
			}
			return true;
		}
	}

	// �ٸ� �� ������ ã�´�
	POSITION pos = m_lHotspot.GetHeadPosition();
	for (; pos; m_lHotspot.GetNext(pos) ) {

		// 10000, 10000 ���� �簢�� ��ǥ
		CRect rc10000 = m_lHotspot.GetAt(pos).rc;
		// �̰��� DP ��ǥ�� �ٲ۴�
		CRect rc(	rcBox.left + rcBox.Width() * rc10000.left/10000,
					rcBox.top + rcBox.Height() * rc10000.top/10000,
					rcBox.left + rcBox.Width() * rc10000.right/10000,
					rcBox.top + rcBox.Height() * rc10000.bottom/10000 );

		if ( rc.PtInRect( CPoint(x,y) )
				&& *m_lHotspot.GetAt(pos).pCol != s_colGray ) {
			// �� ������ �ְ� disabled ���� ���� �ֽ���

			if ( !bCheckOnly ) {
				OnClickSound();
				OnClick( m_lHotspot.GetAt(pos).pVoid );
			}
			else {
				// ������ ���϶���Ʈ �� �� ������ ��ַ� �ٲٰ�
				// �� �� ������ ���϶���Ʈ �Ѵ�
				POSITION posLastSel = m_posSel;
				m_posSel = pos;
				if ( posLastSel != m_posSel ) {
					if ( posLastSel )
						OnHighlight( m_lHotspot.GetAt( posLastSel ), false );
					if ( m_posSel )
						OnHighlight( m_lHotspot.GetAt( m_posSel ), true );
					UpdateHotspot( posLastSel, m_posSel );
				}
			}
			return true;
		}
	}
	// ���϶���Ʈ �Ǿ��� �� ������ ��ַ� �ٲ۴�
	POSITION posLastSel = m_posSel;
	m_posSel = 0;
	if ( posLastSel ) {
		OnHighlight( m_lHotspot.GetAt( posLastSel ), false );
		UpdateHotspot( posLastSel );
	}
	return false;
}

// Ÿ�̸Ӹ� ��Ʈ ( SetPermanent �� �ݴ� �Լ� )
void DSB::SetTimeout( int nTimeOut )
{
	m_nTimeOut = nTimeOut;
	m_pBoard->SetDSBTimer( this );

	CRect rc; GetRect( &rc );
	m_pBoard->UpdateDSB( &rc, true );
}

// �� ������ ����Ѵ�
// �� ������ Ŭ���ϸ� CBoard �� ������ ���� ������
// �� Ŭ������ OnClick �� ȣ���Ѵ�
// ��ġ�� ������� (Box�� �»���� 0,0)
// DSB ��ǥ
RECT DSB::RegisterHotspot(
	int x, int y,					// ��ġ
	int cx, int cy,					// ũ�� (-1,-1 �̸� �ڵ� ���)
	bool bDSB,						// ��ġ ��ǥ�� DSB ��ǥ�ΰ�
	UINT idBitmap, LPCTSTR s,		// �׸� ��Ʈ�ʰ� �� ���ڿ� (������ ���� -1, 0 )
	const COLORREF* pCol,
	const int* pDeco,				// ���ڿ� ����� ��� & ũ��
	const COLORREF* pColSel,
	const int* pDecoSel,			// ���콺�� ������ ���� ����� ��� & ũ��
	LPVOID pVoid )					// OnClick �� �Ѱ����� ��
{
	HOTSPOT hs;

	// �� DSB �� �簢��
	CRect rcDSB;
	GetRect( &rcDSB );

	if ( bDSB ) {
		CPoint pnt = DSBtoDP( CPoint(x,y) );
		x = pnt.x; y = pnt.y;
	}

	// �� ���� ������ ����Ѵ�
	CRect rcSpot;
	if ( cx == -1 || cy == -1 ) {
		CClientDC dc(*m_pBoard);
		rcSpot = idBitmap ?
				PutBitmap( &dc, idBitmap, s, 0, 0, false, *pCol, *pDeco, true )
			:	PutText( &dc, s, 0, 0, false, *pCol, *pDeco, true );
	}
	else {
		int nUnit = bDSB ? GetDSBUnit() : 1;
		rcSpot = CRect( CPoint(x,y), CSize(cx*nUnit,cy*nUnit) );
	}

	// �簢���� ������� 10000, 10000 ���� ��ǥ�� ��ȯ�Ѵ�
	hs.rc.left = ( x - rcDSB.left ) * 10000 / rcDSB.Width();
	hs.rc.top = ( y - rcDSB.top ) * 10000 / rcDSB.Height();
	hs.rc.right = hs.rc.left + rcSpot.Width() * 10000 / rcDSB.Width();
	hs.rc.bottom = hs.rc.top + rcSpot.Height() * 10000 / rcDSB.Height();

	hs.s = s;
	hs.idBitmap = idBitmap;
	hs.pCol = pCol;
	hs.pDeco = pDeco;
	hs.pColSel = pColSel;
	hs.pDecoSel = pDecoSel;
	hs.pVoid = pVoid;

	m_lHotspot.AddTail(hs);

	return rcSpot + CPoint(x,y);
}

// �־��� pVoid ���� ������ �� ������ ã�´�
POSITION DSB::FindHotspot( LPVOID pVoid )
{
	POSITION pos = m_lHotspot.GetHeadPosition();
	for (; pos; m_lHotspot.GetNext(pos) )
		if ( m_lHotspot.GetAt(pos).pVoid == pVoid )
			return pos;
	return 0;
}

// �� ������ Update �ϰ� Invalidate �Ѵ�
void DSB::UpdateHotspot( POSITION pos1, POSITION pos2 )
{
	CRect rcBox; GetRect( &rcBox );
	CRect rc( 0, 0, 0, 0 );

	if ( pos1 ) {
		// 10000, 10000 ���� �簢�� ��ǥ
		CRect rc10000 = m_lHotspot.GetAt(pos1).rc;
		// �̰��� DP ��ǥ�� �ٲ۴�
		rc |= CRect(rcBox.left + rcBox.Width() * rc10000.left/10000,
					rcBox.top + rcBox.Height() * rc10000.top/10000,
					rcBox.left + rcBox.Width() * rc10000.right/10000 + 3,
					rcBox.top + rcBox.Height() * rc10000.bottom/10000 + 3 );
	}
	if ( pos2 ) {
		// 10000, 10000 ���� �簢�� ��ǥ
		CRect rc10000 = m_lHotspot.GetAt(pos2).rc;
		// �̰��� DP ��ǥ�� �ٲ۴�
		rc |= CRect(rcBox.left + rcBox.Width() * rc10000.left/10000,
					rcBox.top + rcBox.Height() * rc10000.top/10000,
					rcBox.left + rcBox.Width() * rc10000.right/10000 + 3,
					rcBox.top + rcBox.Height() * rc10000.bottom/10000 + 3 );
	}

	if ( !rc.IsRectEmpty() ) m_pBoard->UpdateDSB(&rc);
}

// �� ������ ���� DP �簢�� ��ǥ�� ����Ѵ�
void DSB::GetRect( LPRECT prc )
{
	CRect rcClient; m_pBoard->GetClientRect( &rcClient );

	// ��ġ�� CBoard �� ������ ��� �ϰ�
	// ũ��� (����ũ��DSB������ �𸣹Ƿ�) �ڽ��� ������ ��� �Ѵ�
	int nMyUnit = GetDSBUnit();
	int nBoardUnit = m_pBoard->GetDSBUnit();

	prc->left = -4 + rcClient.left + rcClient.Width()/2
		+ m_rcDSB.left*nBoardUnit - m_rcDSB.right*nMyUnit/2 + m_pntOffset.x;
	prc->top = -4 + rcClient.top + rcClient.Height()/2
		+ m_rcDSB.top*nBoardUnit - m_rcDSB.bottom*nMyUnit/2 + m_pntOffset.y;
	prc->right = 4 + prc->left + m_rcDSB.right*nMyUnit;
	prc->bottom = 4 + prc->top + m_rcDSB.bottom*nMyUnit;
}

// ������� DSB ��ǥ�� ������ ����̽� ��ǥ�� ��ȯ�Ѵ�
POINT DSB::DSBtoDP( POINT p ) const
{
	int nMyUnit = GetDSBUnit();
	int nBoardUnit = m_pBoard->GetDSBUnit();
	CRect rcClient; m_pBoard->GetClientRect(&rcClient);

	POINT r;

	r.x = rcClient.left + rcClient.Width()/2
		+ m_rcDSB.left*nBoardUnit - m_rcDSB.right*nMyUnit/2
		+ p.x*nMyUnit + m_pntOffset.x;
	r.y = rcClient.top + rcClient.Height()/2
		+ m_rcDSB.top*nBoardUnit - m_rcDSB.bottom*nMyUnit/2
		+ p.y*nMyUnit + m_pntOffset.y;

	return r;
}

RECT DSB::DSBtoDP( const RECT& rc ) const
{
	CPoint p1( rc.left, rc.top );
	CPoint p2( rc.right, rc.bottom );
	return CRect( DSBtoDP(p1), DSBtoDP(p2) );
}

// ��ǥ�� �̵���Ų��
void DSB::Offset( int x, int y )
{
	// ���� �� �Լ��� ������ ���� ȿ���� ���� �Լ���
	m_pntOffset += CPoint(x,y);

	// �׷��� offset ���� Ŀ���� �Ǹ� DSB ���� ��ǥ��
	// ��߳��� �ǹǷ� offset ���� ����( DSBUnit �� �Ѵ� �� )
	// �� ���� m_rcDSB �� �ݿ��� �־�� �Ѵ�

	int xOverflowDSBs = m_pntOffset.x / m_pBoard->GetDSBUnit();
	int yOverflowDSBs = m_pntOffset.y / m_pBoard->GetDSBUnit();

	m_rcDSB.left += xOverflowDSBs;
	m_rcDSB.top += yOverflowDSBs;

	m_pntOffset.x -= xOverflowDSBs * m_pBoard->GetDSBUnit();
	m_pntOffset.y -= yOverflowDSBs * m_pBoard->GetDSBUnit();

	OnMove();
}

// ���� ��Ʈ�� ��´�
CFont* DSB::GetFont( int nSize )
{
	return m_pBoard->GetFont( m_bFixed ? -nSize-1 : nSize );
}

int DSB::GetDSBUnit() const
{
	// ���� ũ��� 14 !! -> ���� ��������
	return m_bFixed ? 14 : m_pBoard->GetDSBUnit();
}

// �� ��Ʈ���� ȭ�鿡���� ũ�⸦ ���Ѵ� (������ ��Ʈ ���)
CSize DSB::GetTextExtent( int nSize, LPCTSTR s )
{
	CClientDC dc(*m_pBoard);
	dc.SelectObject( GetFont(nSize) );
	return dc.GetTextExtent( s, _tcslen(s) );
}

// CBoard �� ȣ���ϴ� �׸��� �Լ� - ��µ� shade-box �� �� �Լ���
// �������̵� �Ͽ� �����ؾ� �Ѵ�
// ���̽� �Լ��� ������ ��ü�� ���� ���� ������ �׸���
void DSB::OnDraw(
	CDC* pDC,
	bool bBrushOrigin )
{
	CRect rc; GetRect(&rc);
	int x = rc.left, y = rc.top, cx = rc.Width(), cy = rc.Height();

	// ��� ĥ�ϱ�
	FillBack( pDC, &rc, m_pBmpMan, Mo()->nDSBShadeMode, bBrushOrigin );

	// ������ ���� X ��ư
	if ( m_nTimeOut < 0 ) {
		pDC->SelectStockObject( WHITE_PEN );
		pDC->MoveTo( x+cx-5, y+15 );
		pDC->LineTo( x+cx-15-1, y+5-1 );
		pDC->MoveTo( x+cx-15, y+15 );
		pDC->LineTo( x+cx-5+1, y+5-1 );
		pDC->MoveTo( x+cx-5, y+15+1 );
		pDC->LineTo( x+cx-15-1, y+5+1-1 );
		pDC->MoveTo( x+cx-15, y+15+1 );
		pDC->LineTo( x+cx-5+1, y+5+1-1 );
	}

	// ������ �ִ� �׵θ�
	CPen pnGray( PS_SOLID, 1, RGB(128,128,128) );
	pDC->SelectObject( &pnGray );
	pDC->MoveTo( x+cx-1, y );
	pDC->LineTo( x, y );
	pDC->LineTo( x, y+cy-1 );

	CPen pnDGray( PS_SOLID, 1, RGB(64,64,64) );
	pDC->SelectObject( &pnDGray );
	pDC->MoveTo( x, y+cy-1 );
	pDC->LineTo( x+cx-1, y+cy-1 );
	pDC->LineTo( x+cx-1, y );

	pDC->SelectStockObject( WHITE_PEN );
	pDC->MoveTo( x+cx-2, y+1 );
	pDC->LineTo( x+1, y+1 );
	pDC->LineTo( x+1, y+cy-2 );

	pDC->SelectStockObject( BLACK_PEN );
	pDC->MoveTo( x+1, y+cy-2 );
	pDC->LineTo( x+cx-2, y+cy-2 );
	pDC->LineTo( x+cx-2, y+1 );

	// �� ���̵��� �׸���
	POSITION pos = m_lHotspot.GetHeadPosition();
	for (; pos; m_lHotspot.GetNext(pos) ) {
		HOTSPOT& hs = m_lHotspot.GetAt(pos);

		// 10000, 10000 ��ǥ���� DP ��ǥ�� ��ȯ
		int xDraw = x + cx * hs.rc.left/10000;
		int yDraw = y + cy * hs.rc.top/10000;

		// �׸���
		if ( !hs.idBitmap && !hs.s ) ;
		else if ( !hs.idBitmap )
			PutText( pDC, hs.s, xDraw, yDraw, false,
				pos == m_posSel ? *hs.pColSel : *hs.pCol,
				pos == m_posSel ? *hs.pDecoSel : *hs.pDeco );
		else
			PutBitmap( pDC, hs.idBitmap, hs.s,
				xDraw, yDraw, false,
				pos == m_posSel ? *hs.pColSel : *hs.pCol,
				pos == m_posSel ? *hs.pDecoSel : *hs.pDeco );
	}
}

// ���ڸ� ����
RECT DSB::PutText(
	CDC* pDC,
	LPCTSTR s,				// �� ����
	int x, int y,			// ��ġ
	bool bDSB,				// x, y �� DSB ��ǥ�ΰ�
	COLORREF col,			// ����
	int deco,				// ���� ��� & ũ��
	bool bCheckOnly )
{
	bool bCenter = bDSB && x < 0;
	bool bMiddle = bDSB && y < 0;

	int nUnit = GetDSBUnit();

	// � �۾��� DSB ��ü�� Ŭ���ȴ�
	CRect rc; GetRect(&rc);

	// DSB ��ǥ�� �� ��ǥ�� ��ȯ�Ѵ�
	if ( bDSB ) {
		CPoint pnt = DSBtoDP( CPoint(x,y) );
		if ( bCenter ) x = rc.left + rc.Width()/2 - 15/2;
		else x = pnt.x;
		if ( bMiddle ) y = rc.top + rc.Height()/2 - nUnit/2;
		else y = pnt.y;
	}

	CFont* pFont = GetFont( (int) (deco&3) );
	pDC->SelectObject( pFont );

	// ���ϵ� �簢�� (����)
	CRect ret( CPoint(x,y), pDC->GetTextExtent( s ) );
	if ( bCenter ) ret.OffsetRect( -ret.Width()/2, 0 );

	// Ŭ�� ������ ����� �ȱ׸���
	{
		CRect rg(ret);
		rg.InflateRect( 3, 3, 3, 3 );
		CRect rcClip;
		pDC->GetClipBox( &rcClip );
		if ( ( rg & rcClip ).IsRectEmpty() )
			return ret;
	}

	if ( !bCheckOnly ) {	// ������ �׷��� �Ѵ�

		pDC->SetTextAlign(
			( bCenter ? TA_CENTER : TA_LEFT ) | TA_TOP );
		pDC->SetBkMode( TRANSPARENT );

		if ( ( deco & TD_OPAQUE ) || Mo()->bDSBOpaque ) {	// ������
			CRect rcFill = ret & rc;
			pDC->FillSolidRect( &rcFill, RGB(0,0,0) );
		}

		// �׸��� ȿ��
		if ( deco & TD_SHADE ) {
			pDC->SetTextColor( RGB(0,0,0) );
			pDC->ExtTextOut( x+1, y+1, ETO_CLIPPED, &rc, s, NULL );
			if ( deco & TD_OUTLINE )
				pDC->ExtTextOut( x+2, y+2, ETO_CLIPPED,
						&rc, s, NULL ); // ���� �β��� �׸���
		}
		// �׵θ� ȿ��
		if ( deco & TD_OUTLINE ) {
			pDC->SetTextColor( RGB(
				GetRValue(col)*3/5, GetGValue(col)*3/5, GetBValue(col)*3/5 ) );
			pDC->ExtTextOut( x+1, y, ETO_CLIPPED, &rc, s, NULL );
			pDC->ExtTextOut( x-1, y, ETO_CLIPPED, &rc, s, NULL );
			pDC->ExtTextOut( x, y+1, ETO_CLIPPED, &rc, s, NULL );
			pDC->ExtTextOut( x, y-1, ETO_CLIPPED, &rc, s, NULL );
		}

		// �۾� ����
		pDC->SetTextColor( col );
		pDC->ExtTextOut( x, y, ETO_CLIPPED, &rc, s, NULL );
	}

	return ret;
}

// ��Ʈ���� �׸���
RECT DSB::PutBitmap(
	CDC* pDC,
	UINT id,				// �׸� ��Ʈ��
	LPCTSTR s,				// ��Ʈ�� ������ ���� ����
	int x, int y,			// ��ġ
	bool bDSB,				// x, y �� DSB ��ǥ�ΰ�
	COLORREF col,			// ����
	int deco,				// ���� ��� & ũ��
	bool bCheckOnly )
{
	ASSERT( id );

	bool bCenter = bDSB && x < 0;
	bool bMiddle = bDSB && y < 0;

	CRect rc; GetRect(&rc);

	int nUnit = GetDSBUnit();
	CSize szRealBitmap = GetBitmapSize(id);
	CSize szBitmap( szRealBitmap.cx + 3, szRealBitmap.cy + 3 );

	CFont* pFont = GetFont( (int) (deco&3) );
	pDC->SelectObject( pFont );

	CSize szText;
	if ( s ) szText = pDC->GetTextExtent( s, _tcslen(s) );
	else szText.cx = szText.cy = 0;

	// ���ϵ� �簢�� ( �ϴ� ũ�⸸ �����Ѵ� )
	CRect rcRet( 0, 0, szBitmap.cx + szText.cx,
		max( szText.cy, szBitmap.cy ) );

	// DSB ��ǥ�� ��츦 ����ؼ� x, y �� DSB ��ǥ�� �̸� ����� �д�
	CPoint pnt = DSBtoDP( CPoint(x,y) );

	// �簢���� ������ ��ġ�� �ű��
	if ( bCenter ) rcRet.OffsetRect( rc.left + rc.Width()/2-rcRet.Width()/2 - 15/2, 0 );
	else if ( bDSB ) rcRet.OffsetRect( pnt.x, 0 );
	else rcRet.OffsetRect( x, 0 );
	if ( bMiddle ) rcRet.OffsetRect( 0, rc.top + rc.Height()/2 - (rcRet.Height()+1)/2 );
	else if ( bDSB ) rcRet.OffsetRect( 0, pnt.y );
	else rcRet.OffsetRect( 0, y );

	// �ؽ�Ʈ�� �׸��� ( ���ϵ� �簢���� ��´�
	if ( s && !bCheckOnly ) {
		PutText( pDC, s,
			rcRet.left+szBitmap.cx, rcRet.top + (szBitmap.cy-nUnit)/2,
			false, col, deco, false );
	}

	if ( !bCheckOnly ) {		// ������ �׷��� �Ѵ�

		CRect rcBitmap( CPoint( rcRet.left+2, rcRet.top+2 ),
						GetBitmapSize(id) );
		if ( ( rcBitmap & rc ) == rcBitmap ) {

			DrawBitmap( pDC, id, rcRet.left+2, rcRet.top+2 );

			if ( deco & TD_SUNKEN ) {
				pDC->SelectStockObject( BLACK_PEN );
				pDC->MoveTo( rcRet.left+szBitmap.cx, rcRet.top );
				pDC->LineTo( rcRet.left, rcRet.top );
				pDC->LineTo( rcRet.left, rcRet.top+szBitmap.cy);
				pDC->SelectStockObject( WHITE_PEN );
				pDC->LineTo( rcRet.left+szBitmap.cx, rcRet.top+szBitmap.cy );
				pDC->LineTo( rcRet.left+szBitmap.cx, rcRet.top );
			}
			if ( deco & TD_OUTLINE ) {
				CPen pen( PS_SOLID, 1, col );
				CPen* penOld = pDC->SelectObject( &pen );
				pDC->SelectStockObject( NULL_BRUSH );
				pDC->Rectangle(
					rcRet.left, rcRet.top,
					rcRet.right+1, rcRet.bottom+1 );
				pDC->SelectObject( penOld );
			}
		}
	}

	return rcRet;
}

// ParseString ���� ����ϴ� ���� �Լ�
// TCHAR �����Ͱ� ����Ű�� ������ ���� ���ڸ� �����Ѵ�
// ����, �Ǵ� ������ 0 ����
static const TCHAR* next_tchar( const TCHAR* p )
{
	if ( !p || !*p ) return 0;

#ifdef _UNICODE
	return p+1;
#else
	// TCHAR is char

	int nType;
	size_t nOffset = 0;

	nType = _mbsbtype( (const unsigned char*)p, nOffset );
	if ( nType == _MBC_SINGLE ) return p+1;
	else if ( nType != _MBC_LEAD ) return 0;
	else do {
		nType = _mbsbtype( (const unsigned char*)p, ++nOffset );
		if ( nType == _MBC_ILLEGAL ) return 0;
	} while ( nType == _MBC_TRAIL );

	return p+nOffset;
#endif
}

// �� ��Ʈ���� �� �ٷ� �ڸ���
// cLine     : �� ���� �뷫���� ���� (DSB ����)
// asLine    : 0 �� �ƴϸ� ���⿡ �� �κ� ��Ʈ���� �����Ѵ�
// s         : �ڸ� �� ��Ʈ��
// nMaxWidth : ������ �ʿ��� �� ������ �ִ� ���� ũ��
// ���ϰ� -1 �� ����, �׹ۿ���, ������ �ʿ��� ���� ��
int DSB::ParseString( LPTSTR* asLine, int cLine,
					  LPCTSTR s, size_t& nMaxWidth )
{
	CClientDC dc(*m_pBoard);
	dc.SelectObject( GetFont(0) );

	nMaxWidth = 0;
	int nMaxLineSize = cLine * m_pBoard->GetDSBUnit();

	int nCurLine = 0;	// ���� ����

	const TCHAR* pHead = s;	// ���� ������ ���� ������
	const TCHAR* pPrev = s;	// ���� ������
	const TCHAR* pCur;	// ���� ������

	do {
		for(;;) {
			pCur = next_tchar(pPrev);

			if ( !pCur || !*pCur ) break;
			if ( dc.GetTextExtent( pHead, int(pCur-pHead) ).cx
				> nMaxLineSize ) break;

			pPrev = pCur;
		}

		if ( pCur && *pCur ) {
			// ���� �ȳ�����
			size_t chars = size_t( pCur - pHead );
			size_t len = chars * sizeof(TCHAR);
			if ( nMaxWidth < len+1 ) nMaxWidth = chars+1;
			if ( asLine ) {
				memcpy( asLine[nCurLine], pHead, len );
				asLine[nCurLine][chars] = _T('\0');
			}
			nCurLine++;
			pPrev = pHead = pCur;
		}
		else {
			size_t chars = _tcslen( pHead );
			if ( nMaxWidth < chars+1 ) nMaxWidth = chars+1;
			if ( asLine ) {
				strcpy( asLine[nCurLine], pHead );
			}
			nCurLine++;
		}
	} while ( pCur && *pCur );

	return nCurLine;
}
