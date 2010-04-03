// Board_Layout.cpp : CBoard �� ���̾ƿ��� ���� �Լ��� ����
//

#include "stdafx.h"
#include "Mighty.h"

#include "MFSM.h"
#include "BmpMan.h"
#include "DSB.h"
#include "Board.h"
#include "BoardWrap.h"
#include "Play.h"
#include "Player.h"

#include "InfoBar.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// �÷��̾��� �տ� �� ī�带 �׸���
void CBoard::DrawHand(
	CDC* pDC,
	int nPlayer, int nPlayers,			// �÷��̾�/�÷��̾��
	const CCardList* pList,				// �÷��̾��� ī��
	int x, int y, int cx, int cy,		// ����
	int xOff, int yOff,					// ������ �׷������� �ɼ� ��ȭ
	bool bReveal,						// �ո��� ���ϰ��ΰ�
	int nMouseOver,						// ���콺�� ��ġ�ϴ� ī�� �ε���
	bool* abSelected,					// ���� ���õ� ī���
	bool bScore )						// �տ� �� ī�尡 �ƴ϶� ���� ī�带 �׸���
{
	// ����� ��忡���� ��� ī�带 ���δ�
#ifdef _DEBUG
	bReveal = true;
#endif

	if ( !m_pMFSM ) return;

	// ���� �簢�� ( �� ������ ������ �׸� ī�带 ����ȭ �Ѵ� )
	CRect rcClip( x, y, x+cx, y+cy );

	// ī�� ũ��
	int nCardWidth = m_szCard.cx;
	int nCardHeight = m_szCard.cy;

	// ��ü ī�� ��
	int nCards = pList->GetCount();

	// ���� ����

	POSITION pos = pList->GetHeadPosition();
	for ( int index = 0; pos; index++ ) {
		// ���� ī��
		CCard c = pList->GetNext(pos);
		if ( !bReveal ) c = CCard(0/*back*/);

		// ���� ī���� �簢��
		CRect rcCard = CalcRect(
			nPlayers, (bScore?CR_SCORE:CR_HAND), nPlayer, nCardWidth, nCardHeight,
			index, nCards );

		BOOL bOverlapped = !( rcClip & rcCard ).IsRectEmpty();

		// �Ʒ��� �÷��̾�(���)�� ���ؼ��� nMouseOver �� abSelected
		// ���ڸ� ����Ͽ� Ư���� ó���� �Ѵ�
		if ( !bScore ) {
			// ���� ����� ī�忡 ���� ó��
			if ( abSelected && abSelected[index] ) // ���� �鸲
				rcCard.OffsetRect( 0,
					-nCardHeight/CARD_SHIFT_RATIO_OF_SELECTION );
			else if ( index == nMouseOver ) // ��¦ �鸲
				rcCard.OffsetRect( 0,
					-nCardHeight/CARD_SHIFT_RATIO_OF_MOUSE_OVER );
		}

		if ( !bOverlapped
			&& ( rcClip & rcCard ).IsRectEmpty() ) continue;

		// ī�� �簢���� ī�带 �׸���
		m_bm.DrawCard( pDC, c, rcCard + CPoint(xOff,yOff) );
	}
}

// ��� ���� ī�带 �׸���
void CBoard::DrawCenter(
	CDC* pDC,
	int nPlayers,						// �÷��̾��
	const CCardList* pList,				// �׸� ī��
	int nBeginer,						// ���� �÷��̾�(�Ʒ�0���� �ð����)
	int x, int y, int cx, int cy,		// �׸� ����
	int xOff, int yOff )				// ������ �׷��� ���� �ɼ� ��ȭ
{
	if ( !m_pMFSM ) return;

	// Ŭ���� ����
	CRect rcClip( x, y, x+cx, y+cy );

	// ī�� ũ��
	int nCardWidth = m_szCard.cx;
	int nCardHeight = m_szCard.cy;

	// ���� ������ ���� ī�尡 �׷������°�
	// (�׷������ٸ� �� ī�嵵 �ݵ�� �׷����Ѵ�)
	bool bPrevDrawn = false;

	// ���� ����
	POSITION pos = pList->GetHeadPosition();
	for ( int i = 0; pos; i++ ) {
		CCard c = pList->GetNext(pos);

		int nPlayer = ( nBeginer + i + nPlayers ) % nPlayers;

		// �׸� �簢���� ������ �׷����� �簢��
		CRect rcCard = CalcRect( nPlayers, CR_CENTER, 0, nCardWidth, nCardHeight,
			nPlayer, 0, 0 );

		if ( bPrevDrawn || !( rcCard & rcClip ).IsRectEmpty() ) {

			bPrevDrawn = true;

			// ī�� �簢���� ī�带 �׸���
			m_bm.DrawCard( pDC, c, rcCard + CPoint(xOff,yOff) );
		}
	}
}

// �̸��� �׸���
void CBoard::DrawName(
	CDC* pDC,
	int nPlayers,						// �÷��̾��
	LPCTSTR sName,						// �̸�
	int nPlayer,						// �׸� �÷��̾�
	COLORREF col, COLORREF colShadow,	// ���� ����, �׸��� ����
	CFont* pFont )						// ��Ʈ (����Ʈ�� �ý��� ��Ʈ)
{
	CRect rcNameRect = CalcNameRect( nPlayers, nPlayer, pDC, sName, pFont );

	int x = rcNameRect.left;
	int y = rcNameRect.top;

	CFont* pfntOld = pDC->SelectObject( pFont );

	pDC->SetBkMode( TRANSPARENT );
	pDC->SetTextAlign( TA_TOP | TA_LEFT );

	pDC->SetTextColor( colShadow );
	pDC->TextOut( x+1, y, CString(sName) );
	pDC->TextOut( x-1, y, CString(sName) );
	pDC->TextOut( x, y+1, CString(sName) );
	pDC->TextOut( x, y-1, CString(sName) );

	pDC->SetTextColor( col );
	pDC->TextOut( x, y, CString(sName) );

	pDC->SelectObject( pfntOld );

	if ( m_nTurnRect == nPlayer ) {

		pDC->SelectStockObject( NULL_BRUSH );

		rcNameRect.InflateRect( 2, 2, 3, 3 );

		CPen penShad( PS_SOLID, 0, colShadow );
		pDC->SelectObject( &penShad );
		pDC->Rectangle( &rcNameRect );

		rcNameRect.OffsetRect( -1, -1 );

		CPen pen( PS_SOLID, 0, col );
		pDC->SelectObject( &pen );
		pDC->Rectangle( &rcNameRect );

		pDC->SelectStockObject( WHITE_PEN );
	}
}

// ��� �κ��� ���ϸ��̼�
// ũ��� (ī����)*2 X (ī�����)*1.5
// x, y �� ���ϸ��̼��� �� ��� ��ġ
// szCard �� ī�� ũ��
// nCount ī�� ��� (nStep==0 �϶��� ���)
// nStep
// 0 : ��� ī�尡 ���� ���
// 1 : ī�尡 �������� �и��Ǵ� ��
// 2 : ī�尡 �������� �и��Ǿ���
// 3 : ī�尡 ����� �������� ��
void CBoard::DrawCenterAnimation( CDC* pDC, bool bBrushOrigin,
								 CSize szCard, int x, int y,
								 int nStep, int nCardCount )
{
	if ( !m_pMFSM ) return;

	// ��� 0�� �׿��ٸ� �ƹ��͵� �׸��� �ʴ´�
	if ( !nStep && !nCardCount ) return;

	CSize szRealCard = GetBitmapSize( IDB_SA );

	// nCount �� ���� �׷����� ī���� �� ���� �����Ѵ�
	int nCount;
	if ( nCardCount == -1 ) nCardCount = 53;

	if ( nCardCount < 3 ) nCount = nCardCount;
	else if ( nCardCount < 10 ) nCount = nCardCount*2/3;
	else nCount = szCard.cx * (nCardCount-9) / 520 + 6;

	int nMaxCount = szCard.cx * (53-9) / 520 + 6;

	// 1 �� ������ Ư���� �׸��ڰ� �ʿ� !
	if ( nStep == 1 )
		m_bm.DrawShade( pDC,
			x + szCard.cx/4, y,
			szCard.cx/2, szCard.cy/2 + nCount*2,
			bBrushOrigin );

	// ���� ī�带 ���� �׸���
	for ( int i = 0; i < nCount; i++ ) {
		int a, b;

		// �� ���� Board.cpp �� CalcCardRectForFlyCard ������
		// ���Ǿ����Ƿ� ����
		b = y - szCard.cy/2 + nMaxCount*2 - i*2;

		if ( nStep == 0 ) a = x - szCard.cx/2;
		else if ( nStep == 2 && i % 2 ) a = x - szCard.cx, b = b + i*2 - i/2*2;
		else if ( nStep == 2 ) a = x, b = b + i*2 - i/2*2;
		else if ( nStep == 1 && i < nCount/2
				|| nStep == 3 && i % 2 ) a = x - szCard.cx*3/4;
		else a = x - szCard.cx/4;

		m_bm.DrawCard( pDC, 0, a, b, szCard.cx, szCard.cy );
	}
}

// �տ� �� ī��, ����, ��� ���� ī�带 ��� �׸���
void CBoard::DrawBoard( CDC* pDC, LPCRECT prc )
{
	const CState* pState = m_pMFSM ? m_pMFSM->GetState() : 0;

	// Ŭ�� �簢��
	CRect rcClip( *prc );

	// ���� �׸���
	CRect rcClient; GetClientRect( &rcClient );
	m_bm.DrawBackground( Mo()->bUseBackground,
		Mo()->bTile, Mo()->bExpand, (COLORREF)Mo()->nBackColor,
		pDC, rcClip, rcClient.Width(), rcClient.Height() );

	if ( !pState ) return;	// �׸����� �ƹ��͵� ����

	// ���� �÷��̾� ��
	int nPlayers = pState->nPlayers;

	// ������ ����� �÷��� �ϰ� �ִ°�
	bool bHuman = pState->apPlayers[0]->IsHuman();

	// ���� ī�尡 �׷����°�
	// ���� ī�尡 �׷����ٸ� �� �÷��̾��� �տ� �� ī�嵵
	// �Բ� �׷��� �Ѵ�
	bool bScoreDrawn[MAX_PLAYERS];

	// ������ ī��
	{
		for ( int i = 0; i < nPlayers; i++ ) {

			bool bReveal = !bHuman || !pState->pRule->bHideScore;

			CRect rcScore = CalcRect(
					nPlayers, CR_SCORE, i, m_szCard.cx, m_szCard.cy );

			CRect rcDraw;
			if ( rcDraw.IntersectRect( &rcScore, &rcClip ) ) {
				bScoreDrawn[i] = true;
				DrawHand( pDC, i, nPlayers,
					pState->apPlayers[i]->GetScore(),
					rcDraw.left, rcDraw.top, rcDraw.Width(), rcDraw.Height(),
					0, 0, bReveal, 0, 0, true );
			}
			else bScoreDrawn[i] = false;
		}
	}

	// ��� ����
	{
		CRect rcDraw;
		CRect rcCenter = CalcRect(
			nPlayers, CR_CENTER, 0, m_szCard.cx, m_szCard.cy );

		if ( rcDraw.IntersectRect( &rcCenter, &rcClip ) ) {

			// ���� ���� ���̸� �����
			// '�ٴ� ī�� ����Ʈ(lCurrent)'�� �׸���
			if ( pState && pState->state == msTurn )

				DrawCenter( pDC, nPlayers,
					&pState->lCurrent, pState->nBeginer,
					rcDraw.left, rcDraw.top, rcDraw.Width(), rcDraw.Height(),
					0, 0 );

			// �׷��� ������ ���� �׸���
			// (ī�带 ���� �߿��� �׸��� �ʴ´�)
			else if ( pState && !m_hSuffling ) {
				CRect rc; GetClientRect( &rc );
				DrawCenterAnimation(
					pDC, false, m_szCard,
					rc.left+rc.Width()/2, rc.top+rc.Height()/2,
					0, pState->lDeck.GetCount() );
			}
		}
	}

	// �տ� �� ī��
	{
		for ( int i = 0; i < nPlayers; i++ ) {

			bool bReveal = !bHuman || i == 0 || i == m_nDealMiss
				? true : false;
#ifdef _DEBUG
//bReveal = true;
#endif
			int nMouseOver = !bHuman || i != 0 ? -1 : m_nMouseOver;
			bool* abSelected =
				!bHuman || i != 0 || pState->state != msPrivilege
				? 0 : m_abSelected; 

			CRect rcHand = CalcRect(
					nPlayers, CR_HAND, i, m_szCard.cx, m_szCard.cy );

			// ī�尡 ���� ���� �ִ� ��츦 ���
			rcHand.top -= m_szCard.cy / CARD_SHIFT_RATIO_OF_SELECTION;

			if ( bScoreDrawn[i] || !( rcHand & rcClip ).IsRectEmpty() )

				DrawHand( pDC, i, nPlayers,
					pState->apPlayers[i]->GetHand(),
					rcHand.left, rcHand.top, rcHand.Width(), rcHand.Height(),
					0, 0, bReveal, nMouseOver, abSelected, false );
		}
	}

	// �̸�
	{
		for ( int i = 0; i < nPlayers; i++ ) {

			// �����ΰ�
			bool bDef = pState->nMaster == i
				|| pState->bFriendRevealed && pState->nFriend == i;
			DrawName( pDC, nPlayers, pState->apPlayers[i]->GetName(), i,
				bDef ? Mo()->nDefColor : Mo()->nAttColor,
				RGB(0,0,0), &m_fntSmall );
		}
	}
}

// DSB �� �׸���
// Board �� ���� ��ǥ�迡 �׷�������
// xOff �� yOff �� �׷��ִ� �ɼ��� ������ �� �ִ�
void CBoard::DrawDSB( CDC* pDC, bool bBrushOrigin,
						int x, int y, int cx, int cy,
						int xOff, int yOff )
{
	CRect rcClipBox( x, y, x+cx, y+cy );

	// ������ DSB ���� ���ٷ� �׷� �ش�
	// ������ DSB �� z order �� �� ����̹Ƿ�
	POSITION pos = m_lpDSB.GetTailPosition();
	while (pos) {
		DSB* pDSB = m_lpDSB.GetPrev(pos);
		CRect rc; pDSB->GetRect( &rc );

		if ( !( rcClipBox & rc ).IsRectEmpty() ) {

			// pDSB �� �ɼ��� ��� ���� �׸� �ɼ����� �����
			// DSB �� ���������� �ڽ��� ������ ���� ���� ��ǥ�� ����ϱ� �����̴�
			pDSB->Offset( xOff, yOff );
			pDSB->OnDraw( pDC, bBrushOrigin );
			pDSB->Offset( -xOff, -yOff );
		}
	}
}

// �÷��̾ �ٴ� ����, ��� ����, �̸� ���� �����Ѵ�
// �� ���� DSB �鵵 �Բ� ���ŵȴ�
// nPlayer �� -1 �̸� �ٴ� ������ ����
// nPlayer �� -2 �̸� ��� ������ ����
// prc �� 0 �̸� �� �÷��̾��� ��� ������ ����
//        �׷��� ������ �� �������� ����
// bUpdateNow : ��� �����츦 Update �Ѵ�
// bSelectionRegion : �� ���� ���̸� 0 �� �÷��̾��� ���� ����, ���� �������� �Բ� �����Ѵ�
void CBoard::UpdatePlayer( int nPlayer, int x, int y, int cx, int cy, bool bUpdateNow, bool bSelectionRegion )
{
	CRect rc( x, y, x+cx, y+cy );
	UpdatePlayer( nPlayer, &rc, bUpdateNow, bSelectionRegion );
}

void CBoard::UpdatePlayer( int nPlayer, LPRECT prc, bool bUpdateNow, bool bSelectionRegion )
{
	// MFSM ���� ��Ʈ�Ǿ� �־�� �Ѵ�
	// �׷��� ���� ��� nPlayer == -2 (��ü) ���� �����ϴ�
	const CState* pState = m_pMFSM ? m_pMFSM->GetState() : 0;

	ASSERT( pState || nPlayer == -2 );
	if ( !pState && nPlayer != -2 ) return;

	CRect rc;
	{
		CClientDC dcClient(this);
		CDC dcMem; dcMem.CreateCompatibleDC( &dcClient );
		dcMem.SelectObject( &m_board );

		if ( prc == NULL ) {

			if ( nPlayer == -2 )
				// nPlayer == -2 �̸� ��ü �簢��
				rc.SetRect( 0, 0, m_szPic.cx, m_szPic.cy );

			else if ( nPlayer == -1 )
				// nPlayer == -1 �̸� ��� �簢��
				rc = CalcRect(
					pState->nPlayers, CR_CENTER, 0,
					m_szCard.cx, m_szCard.cy );

			else {
				// Ư�� �÷��̾�

				// �տ� �� ī��
				rc = CalcRect(
						pState->nPlayers, CR_HAND, nPlayer,
						m_szCard.cx, m_szCard.cy );
				if ( nPlayer == 0 )	// 0�� �÷��̾��� ī�� �̵��� ���
					if ( bSelectionRegion )
						rc.top -= m_szCard.cy/CARD_SHIFT_RATIO_OF_SELECTION;
					else rc.top -= m_szCard.cy/CARD_SHIFT_RATIO_OF_MOUSE_OVER;

				// ���� ī��
				if ( bSelectionRegion )
					rc |= CalcRect( pState->nPlayers, CR_SCORE, nPlayer,
									m_szCard.cx, m_szCard.cy );
			}
			prc = &rc;
		}

		// ���� �׸���
		dcMem.IntersectClipRect( prc );
		DrawBoard( &dcMem, prc );
	}
	// DSB �׸���
	UpdateDSB( prc, bUpdateNow );
}

// ���� ȭ���� �����Ѵ�
// ������ ������ ���۸� �ٽ� �׸� �� �� ������ ���� �����쵵
// ��ȿȭ ��Ų��
void CBoard::UpdateDSB( int x, int y, int cx, int cy, bool bUpdateNow )
{
	CRect rc( x, y, x+cx, y+cy );
	UpdateDSB( &rc, bUpdateNow );
}

void CBoard::UpdateDSB( LPCRECT prc, bool bUpdateNow )
{
	{
		CClientDC dcClient(this);
		CDC dcPic; dcPic.CreateCompatibleDC( &dcClient );
		dcPic.SelectObject( &m_pic );

		CRect rc;
		if ( prc == NULL ) {
			rc.SetRect( 0, 0, m_szPic.cx, m_szPic.cy );
			prc = &rc;
		}

		dcPic.IntersectClipRect( prc );

		// ���� ���� ������ ����
		{
			CDC dcBoard; dcBoard.CreateCompatibleDC( &dcClient );
			dcBoard.SelectObject( &m_board );

			dcPic.BitBlt( prc->left, prc->top,
				prc->right-prc->left, prc->bottom-prc->top,
				&dcBoard, prc->left, prc->top, SRCCOPY );
		}

		m_csFlying.Lock();

		// ī�带 ���� ���̸� �� �׸��� �Բ� �׸���
		if ( m_pMFSM && m_hSuffling ) {

			SUFFLECARDS_INFO* pInfo = (SUFFLECARDS_INFO*)m_hSuffling;

			// ���� �׸���
			CRect rcClient; GetClientRect( &rcClient );
			CRect rcDraw(	pInfo->xCenter - pInfo->sz.cx,
							pInfo->yCenter - pInfo->sz.cy,
							pInfo->xCenter + pInfo->sz.cx,
							pInfo->yCenter + pInfo->sz.cy );
			m_bm.DrawBackground( Mo()->bUseBackground,
				Mo()->bTile, Mo()->bExpand, (COLORREF)Mo()->nBackColor,
				&dcPic, &rcDraw, rcClient.Width(), rcClient.Height() );
			// ���� �׸� �׸���
			DrawCenterAnimation( &dcPic,
				pInfo->bBrushOrigin, m_szCard,
				pInfo->xCenter, pInfo->yCenter,
				pInfo->nCurStep );
		}
		// ī�带 ������ ���̸� �� ī�嵵 �Բ� �׸���
		if ( m_pMFSM && m_hFlying ) {

			FLYCARD_INFO* pInfo = (FLYCARD_INFO*)m_hFlying;

			// �׸���
			if ( pInfo->nCurState <= pInfo->nStates )
				m_bm.DrawShade( &dcPic,
					pInfo->x + pInfo->nCurShadeDepth,
					pInfo->y + pInfo->nCurShadeDepth,
					pInfo->rcBegin.Width(), pInfo->rcBegin.Height(), false );

			// ī��
			m_bm.DrawCard( &dcPic, pInfo->nCard,
				pInfo->x, pInfo->y,
				pInfo->rcBegin.Width(), pInfo->rcBegin.Height() );
		}

		m_csFlying.Unlock();

		// DSB �׸���
		DrawDSB( &dcPic, false, prc->left, prc->top,
			prc->right - prc->left, prc->bottom - prc->top);
	}

	InvalidateRect( prc, FALSE );
	if ( bUpdateNow ) {
		UpdateWindow();
		GdiFlush();
	}
}

// ī�带 ������ !
// ���ϴ� �ɼ��� �ָ� ī�带 ������ �����ϸ鼭
// �ڵ��� �����Ѵ� - �� �ڵ��� �ٽ�
// �Լ��� ���ڷ� �����ϸ� ���� �ڵ���
// �����ϸ鼭 ���ݾ� ī�� �����⸦ �����Ѵ�.
// �����Ⱑ �������� 0 �� �����Ѵ�
// �����찡 �������� ������ �ڿ��� �����ϰ� 0 �� �����Ѵ�
volatile void* CBoard::FlyCard(
	volatile void*& h,		// �ڵ�
	int nCard,				// ���� ī��
	const CRect& rcBegin,	// ���۵Ǵ� ī�� �簢��
	const CPoint& ptEnd,	// ������ ��ġ ( �簢���� �»�� )
	int nStates,			// ������� ������ �Ϸ��ϴ°�
	int nShadeMode )		// �׸��� ���
{
#define SHADE_OFFSET 10		// �׸��� �ɼ�
	ASSERT( nStates > 0 || rcBegin.TopLeft() == ptEnd );

	// ���ο� ���� ������ �����
	FLYCARD_INFO* pInfo = new FLYCARD_INFO;
	pInfo->nCard = nCard;
	pInfo->rcBegin = rcBegin;
	pInfo->ptEnd = ptEnd;
	pInfo->nStates = nStates;
	pInfo->nShadeMode = nShadeMode;
	pInfo->nCurState = -1;
	pInfo->nCurShadeDepth = pInfo->nShadeMode % 2 ?
		SHADE_OFFSET * pInfo->rcBegin.Height()/m_szCard.cy : 0;
	pInfo->x = pInfo->xLast = rcBegin.left;
	pInfo->y = pInfo->yLast = rcBegin.top;
	pInfo->sz.cx = rcBegin.Width()+SHADE_OFFSET;
	pInfo->sz.cy = rcBegin.Height()+SHADE_OFFSET;

	// ���� ũ�� ������ ������
	Mf()->LockSizing();

	m_csFlying.Lock();
		h = (void*)pInfo;
	m_csFlying.Unlock();

	return pInfo;
}

volatile void* CBoard::FlyCard( volatile void*& h, int nStep )
{
	ASSERT(h);
	if ( !h ) return 0;

	// ������ �� �浹�� ���ϱ� ���� ���ȸ� �Ѵ�
	CBoardWrap* pWrap = GetWrapper();

	m_csFlying.Lock();

	FLYCARD_INFO* pInfo = (FLYCARD_INFO*)h;

	if ( !GetSafeHwnd() ) {
		// �����찡 ���ڱ� ������� !
		delete pInfo;
		h = 0;
		m_csFlying.Unlock();
		return h;
	}

	CSize szRealCard = GetBitmapSize( IDB_SA );

	pInfo->nCurState = min( pInfo->nCurState + nStep,
							pInfo->nStates + 1 );

	// �׸��� ����
	if ( pInfo->nShadeMode == 1 )		// ������ �Ʒ���
		pInfo->nCurShadeDepth =
			SHADE_OFFSET*(pInfo->nStates-pInfo->nCurState)
			*pInfo->rcBegin.Height()/pInfo->nStates/m_szCard.cy;
	else if ( pInfo->nShadeMode == 2 )	// �Ʒ����� ����
		pInfo->nCurShadeDepth =
			SHADE_OFFSET*pInfo->nCurState
			*pInfo->rcBegin.Height()/pInfo->nStates/m_szCard.cy;
	else if ( pInfo->nShadeMode == 3 )	// ������ ����
		pInfo->nCurShadeDepth = SHADE_OFFSET
			*pInfo->rcBegin.Height()/m_szCard.cy;
	else pInfo->nCurShadeDepth = 0;

	pInfo->xLast = pInfo->x;
	pInfo->yLast = pInfo->y;

	// �̹��� ���� ��ǥ�� ���
	pInfo->x = pInfo->rcBegin.left
			+ (pInfo->ptEnd.x-pInfo->rcBegin.left)
				* pInfo->nCurState / ( pInfo->nStates + 1 );
	pInfo->y = pInfo->rcBegin.top
			+ (pInfo->ptEnd.y-pInfo->rcBegin.top)
				* pInfo->nCurState / ( pInfo->nStates + 1 );

	// �׸� �簢�� (���� �簢���� ���� �簢���� ������)
	CRect rcDraw(
		min( pInfo->xLast, pInfo->x ),
		min( pInfo->yLast, pInfo->y ),
		max( pInfo->xLast, pInfo->x )+pInfo->sz.cx,
		max( pInfo->yLast, pInfo->y )+pInfo->sz.cy );

	// ī�� �����Ⱑ ����Ǿ���
	if ( pInfo->nCurState > pInfo->nStates ) {

		m_csFlying.Unlock();

		// ���������� �ѹ� �׷��ش� ( update �� ��� ������ �ʴ´� )
		pWrap->UpdateDSB( &rcDraw );

		// �ڿ��� ��ȯ
		m_csFlying.Lock();
			h = 0;
			delete pInfo;
		m_csFlying.Unlock();

		// ���� ũ�� ���� ������ ����
		Mf()->UnlockSizing();

		return 0;
	}

	m_csFlying.Unlock();

	// �׸��� !!
	pWrap->UpdateDSB( &rcDraw, true );

	ASSERT( pInfo->nStates > 0 );

	return h;
}

// ī�带 ���� ���ϸ��̼��� ���δ�
// �� �Լ��� ���� SuffleCards �� ������ ��������
// DrawCenterAnimation �Լ��� ����Ѵ�
// FlyCard �� ���� ������� ȣ���Ѵ�
// �����찡 �������� ������ �ڿ��� �����ϰ� 0 �� �����Ѵ�
volatile void* CBoard::SuffleCards(
	volatile void*& h,		// �ڵ�
	int nCount,				// ���� ȸ��
	SIZE szCard )			// ī�� ũ��
{
	// ȭ�� �簢��
	CRect rcClient;
	GetClientRect( &rcClient );

	SUFFLECARDS_INFO* pInfo = new SUFFLECARDS_INFO;
	pInfo->nCount = nCount + 1;
	pInfo->sz.cx = szCard.cx * 2;
	pInfo->sz.cy = szCard.cy * 3/2;
	pInfo->nCurCount = 0;
	pInfo->nCurStep = 3;
	pInfo->xCenter = rcClient.left + rcClient.Width()/2;
	pInfo->yCenter = rcClient.top + rcClient.Height()/2;
	pInfo->bBrushOrigin = ( pInfo->xCenter + pInfo->sz.cx/2
							+ pInfo->yCenter + pInfo->sz.cy/2 )
							% 2 ? false : true;

	// ���� ũ�� ������ ����
	Mf()->LockSizing();

	// ���� �ٿ� ����ٸ� �׸��� �����Ѵ�
	Ib()->SetText( _T("ī�带 ���� ���Դϴ�...") );
	Ib()->SetProgress( 0 );

	m_csFlying.Lock();
		h = pInfo;
	m_csFlying.Unlock();

	return pInfo;
}

volatile void* CBoard::SuffleCards( volatile void*& h )
{
	ASSERT(h);
	if ( !h ) return 0;

	// ������ �� �浹�� ���ϱ� ���� ���ȸ� �Ѵ�
	CBoardWrap* pWrap = GetWrapper();

	m_csFlying.Lock();

	SUFFLECARDS_INFO* pInfo = (SUFFLECARDS_INFO*)h;

	if ( ( ++pInfo->nCurStep %= 4 ) == 0 )
		pInfo->nCurCount++;

	if ( !GetSafeHwnd() ) {
		// �����찡 ���ڱ� ������� !
		delete pInfo;
		h = 0;
		m_csFlying.Unlock();
		return 0;
	}

	// �׸� �簢��
	CRect rcDraw(
		CPoint(	pInfo->xCenter - pInfo->sz.cx/2,
				pInfo->yCenter - pInfo->sz.cy/2 ),
		CSize(	pInfo->sz.cx, pInfo->sz.cy ) );

	m_csFlying.Unlock();

	// �׸��� !!
	pWrap->UpdateDSB( &rcDraw, true );

	if ( pInfo->nCurCount >= pInfo->nCount ) {
		// ������ ȸ����ŭ ���� ���� ��ĥ ���� �Ǿ���

		// �ڿ��� ��ȯ
		m_csFlying.Lock();
			h = 0;
			delete pInfo;
		m_csFlying.Unlock();

		// ���� ũ�� ���� ������ ����
		Mf()->UnlockSizing();
		// ���¹� ����ġ
		Ib()->Reset();

		return 0;
	}
	else {
		ASSERT( pInfo->nCount > 0 );

		// �Ҹ� ���
		if ( pInfo->nCurStep == 3 )
			PlaySound( IDW_SUFFLE, true );

		// ���¹� ���
		Ib()->SetProgress(
			( (pInfo->nCurCount-1)*4 + pInfo->nCurStep )
				* 100 / ( (pInfo->nCount-1)*4 ) );

		return h;
	}
}
