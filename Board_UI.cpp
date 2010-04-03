// Board_UI.cpp : CBoard �� ����� �������̽� ���� �κ��� ����
//

#include "stdafx.h"
#include "Mighty.h"
#include "BmpMan.h"
#include "Board.h"
#include "Play.h"
#include "Player.h"
#include "MFSM.h"
#include "DSB.h"
#include "DScoreBoard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// OnLButtonDown �� ���ؼ� DSB ó���� �� �ִ� �Լ�
bool CBoard::CookDSBForLButtonDown( POINT point )
{
	POSITION pos = m_lpDSB.GetHeadPosition();
	for (; pos; m_lpDSB.GetNext(pos) ) {
		DSB* pDSB = m_lpDSB.GetAt(pos);

		// ��� �����̸� ��� DSB �� �˻��Ѵ�
		if ( !m_lpModalDSB.IsEmpty()
			&& pDSB != m_lpModalDSB.GetHead() ) continue;

		CRect rc; pDSB->GetRect( &rc );
		if ( rc.PtInRect( point ) )
			// DSB �� Ŭ���Ǿ��°�

			if ( pDSB->DispatchHotspot( point.x, point.y, false) )
				// Ŭ���� ó����
				return true;

			else {
				// ��Ŀ�� �̵�
				if ( m_pDragDSB && m_pDragDSB != pDSB )
					m_pDragDSB->OnFocus( false );

				// �巡���� ���� !
				m_bDragDSB = true; m_pDragDSB = pDSB;
				m_pntLastDSB = point;
				SetCapture();
				RECT rcClient; GetClientRect(&rcClient);
				ClientToScreen( &rcClient );
				ClipCursor( &rcClient );

				// �ڵ����� ������� �ʰ� �����
				m_pDragDSB->SetPermanent();
				// �� ������ �̵�
				m_lpDSB.RemoveAt(pos);
				m_lpDSB.AddHead( pDSB );
				pDSB->OnFocus( true );
				UpdateDSB( rc );
				return true;
			}
	}

	// ��� ���¸� ������ �۾��� �����Ѵ�
	if ( !m_lpModalDSB.IsEmpty() ) return true;

	// LButtonDown ó���� ���
	return false;
}

// OnLButtonDown �� ���ؼ� ī�� ���� ó���� �� �ִ� �Լ�
bool CBoard::CookSelectionForLButtonDown( POINT point )
{
	ASSERT( m_pWaitingSelectionEvent );
	ASSERT( m_pWaitingSelectionCard );

	if ( !m_pMFSM ) return false;

	if ( !CanSelect() )
		// ���� �Ұ����ϸ� ��ȿ
		return false;

	POSITION pos = CardFromPoint( point.x, point.y );
	if ( pos ) {
		const CState* pState = m_pMFSM->GetState();
		*m_pWaitingSelectionCard =
			pState->apPlayers[0]->GetHand()->GetAt(pos);
		m_pWaitingSelectionEvent->SetEvent();
	}
	// ã�� ���ߴٸ� ������ ���� Ŭ���� ���̴�
	// �� �Ҹ��� ����
	else if ( Mo()->bUseSound )
		MessageBeep( MB_OK );

	return true;
}

// ������ ��ġ�� ī�带 ��� �ø���
// (m_abSelected �� m_nMouseOver ���� �ٲ� �� Update �Ѵ�)
// bMouseOver �� true �̸� m_nMouseOver �� �����ϰ�
// ������ �÷����� ī��� �����ش�
// false �̸� m_abSelected �� ���� ����ϰ�
// ����� �ݿ��Ѵ�
void CBoard::ShiftCard( int x, int y, bool bMouseOver )
{
	ASSERT( m_pMFSM );
	if ( !m_pMFSM ) return;

	const CState* pState = m_pMFSM->GetState();

	// �÷��̾ ����� ��쿡�� �ش�ȴ�
	if ( !pState->apPlayers[0]->IsHuman() ) return;
	// �ڱ� ���� ��쿡�� �ش�ȴ�
	if ( pState->state != msTurn
		|| pState->nCurrentPlayer != 0 ) return;

	// �տ� �� ī��
	CCardList* pHand = pState->apPlayers[0]->GetHand();

	// �÷��� ī���� ��ġ�� �ε���
	int index = -1;
	if ( bMouseOver ) {
		POSITION pos = CardFromPoint( x, y );
		index = pos ? pHand->IndexFromPOSITION( pos ) : -1;
	}

	if ( bMouseOver && m_nMouseOver == index )
		// �̹� �÷��� �ִ� ī���̱� ������ �׳� ����
		return;

	CRect rcUpdate(0,0,0,0);

	if ( m_nMouseOver >= 0 ) {	// �ٸ� ī�尡 �÷��� �־���
		CRect rc = CalcRect( pState->nPlayers,
						CR_HAND, 0, m_szCard.cx, m_szCard.cy,
						m_nMouseOver, pHand->GetCount() );
		rc.top -= m_szCard.cy/CARD_SHIFT_RATIO_OF_SELECTION;

		rcUpdate |= rc;	// ���� ������ update �Ѵ�
	}

	m_nMouseOver = index;

	if ( m_nMouseOver >= 0 ) {	// ���ο� ī�带 �ø���
		CRect rc = CalcRect( pState->nPlayers,
							CR_HAND, 0, m_szCard.cx, m_szCard.cy,
							m_nMouseOver, pHand->GetCount() );
			rc.top -= m_szCard.cy/CARD_SHIFT_RATIO_OF_SELECTION;
			rcUpdate |= rc;
	}
	UpdatePlayer( 0, &rcUpdate, true );
}

// ���� Ŀ�� �Ʒ����� ī�带 �� �� �ִ°� !
bool CBoard::CanSelect() const
{
	const CState* pState = m_pMFSM ? m_pMFSM->GetState() : 0;

	// ī�� �������̰� ������ ���۵Ǿ���� �ϰ�
	// �÷��̾ ����̾�� ��
	if ( !m_pWaitingSelectionEvent
		|| !pState || !pState->apPlayers[0]->IsHuman() )
		return false;

	// Ŀ���� Ŭ���̾�Ʈ ��ġ
	POINT pntCursor;
	GetCursorPos( &pntCursor );
	ScreenToClient( &pntCursor );

	// �տ� �� ī��
	CCardList* pHand = pState->apPlayers[0]->GetHand();
	// �÷��� ī���� ��ġ�� �ε���
	POSITION pos = CardFromPoint( pntCursor.x, pntCursor.y );

	// ī�� ���� �־�� �Ѵ�
	if ( !pos ) return false;

	// ���� Ư�� ����(3���� ������)��� ��� ī�带 ���� ����
	if ( pState->state == msPrivilege ) return true;

	// �׷��� ������ �� �� �ִ� ī�常 ���� ����
	CCardList lAv;
	pHand->GetAvList( &lAv, &pState->lCurrent, pState->nTurn,
		pState->nJokerShape, pState->bJokercallEffect );
	if ( !lAv.Find( pHand->GetAt(pos) ) )
		return false;
	else return true;
}

int CBoard::OnToolHitTest( CPoint point, TOOLINFO* pTI ) const
{
	__declspec(thread) static int s_cLastTooltipCard = 0;

	if ( !m_pMFSM || !Mo()->bCardTip
		|| !Mo()->bUseHintInNetwork
			&& m_pMFSM->GetState()->IsNetworkGame() )
		return -1;

	const CState* pState = m_pMFSM->GetState();

	if ( pState->state != msTurn ) return -1;

	if ( pTI ) {

		pTI->cbSize = sizeof(TOOLINFO);
		pTI->uFlags = TTF_TRANSPARENT;
		pTI->hwnd = GetSafeHwnd();
		pTI->hinst = 0;
		pTI->lpszText = LPSTR_TEXTCALLBACK;
	}

	// �÷��̾��� �տ� �� ī�� ��
	POSITION pos =
		!pState->apPlayers[0]->IsHuman() ? 0
		: CardFromPoint( point.x, point.y );
	if ( pos ) {

		const CCardList* pHand = pState->apPlayers[0]->GetHand();
		CCard c( pHand->GetAt(pos) );

		if ( c != CCard(s_cLastTooltipCard) ) {
			s_cLastTooltipCard = c;
			return -1;
		}

		int nIndex = pHand->IndexFromPOSITION(pos);

		if ( pTI ) {
			pTI->uId = (UINT)(int)c;
			pTI->rect = CalcRect( pState->nPlayers, CR_HAND, 0,
								m_szCard.cx, m_szCard.cy,
								nIndex, pHand->GetCount() );
			if ( nIndex+1 < pHand->GetCount() )
				pTI->rect.right = CalcRect( pState->nPlayers, CR_HAND, 0,
								m_szCard.cx, m_szCard.cy,
								nIndex+1, pHand->GetCount() ).left;
		}
		return 1;
	}

	// �� �÷��̾��� �̸� & ���� ��
	int i;
	for ( i = 0; i < pState->nPlayers; i++ ) {

		CRect rc = CalcRect( pState->nPlayers, CR_HAND, i,
						m_szCard.cx, m_szCard.cy )
				| CalcRect( pState->nPlayers, CR_SCORE, i,
						m_szCard.cx, m_szCard.cy );

		if ( rc.PtInRect(point) ) {

			if ( s_cLastTooltipCard != 0 ) {
				s_cLastTooltipCard = 0; return -1;
			}
			if ( pTI ) {
				pTI->rect = rc;
				pTI->uId = 100 + i;
			}
			s_cLastTooltipCard = 0;
			return 1;
		}
	}

	// ��� ī��
	int nCount = pState->lCurrent.GetCount();
	pos = pState->lCurrent.GetTailPosition();
	for ( i = nCount-1; i >= 0; i-- ) {

		CCard c = pState->lCurrent.GetPrev(pos);

		int n = ( pState->nBeginer + i ) % pState->nPlayers;
		CRect rc = CalcRect( pState->nPlayers, CR_CENTER, 0,
						m_szCard.cx, m_szCard.cy,
						n, pState->nPlayers );
		if ( rc.PtInRect(point) ) {

			if ( c != CCard(s_cLastTooltipCard) ) {
				s_cLastTooltipCard = c;
				return -1;
			}
			if ( pTI ) {
				pTI->uId = (UINT)(int)c;
				pTI->rect = rc;
			}
			return 1;
		}
	}
	
	return -1;
}

BOOL CBoard::OnToolTipNeedText( UINT /*id*/, NMHDR * pNMHDR, LRESULT* /*pResult*/ )
{
	__declspec(thread) static TCHAR s_sBuff[64];

	if ( !m_pMFSM ) return FALSE;

    TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pNMHDR;

	const CState* pState = m_pMFSM->GetState();
	int id = (int)pNMHDR->idFrom;
	CString s;

	if ( 100 <= id && id < 150 )		// �÷��̾� �̸�
		s.Format( _T("%s %d �� ����"),
			pState->apPlayers[id-100]->GetName(),
			pState->apPlayers[id-100]->GetScore()->GetCount() );

	else if ( 1 <= id && id <= 53 )	{	// ī��
		CCard c((int)id);
		s = m_pMFSM->GetHelpString( c );
	}

	else return FALSE;

	strcpy_s( s_sBuff, s );
	pTTT->lpszText=s_sBuff;
	return TRUE;
}

// �÷��̾��� �̸��� �ɼǿ��� �ٽ� �ε��Ѵ�
void CBoard::ReloadPlayerNames()
{
	// ��Ʈ��ũ ���� ���̶�� �������� �ʴ´�
	if ( m_pMFSM && m_pMFSM->GetState()->IsNetworkGame() ) return;

	if ( m_pMFSM ) {
		// ������

		LPCTSTR asNames[MAX_PLAYERS];
		for ( int i = 0; i < MAX_PLAYERS; i++ )
			asNames[i] = Mo()->aPlayer[i].sName;
		m_pMFSM->SetPlayerNames( asNames );

		Sb()->Update( m_pMFSM->GetState() );
	}
	else Sb()->Update();
}
