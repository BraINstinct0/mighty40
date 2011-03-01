// Board_DSB.cpp : DSB-related CBoard function bodies
//

#include "stdafx.h"
#include "Mighty.h"
#include "DSB.h"
#include "DEtc.h"
#include "DElection.h"
#include "Play.h"
#include "Player.h"
#include "MFSM.h"
#include "BmpMan.h"
#include "Board.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// �۲� ( 0 small 1 medium 2 big, �����̸� -nSize-1 �� Fixed )
CFont* CBoard::GetFont( int nSize )
{
	switch ( nSize ) {
	case 1: return &m_fntMiddle;
	case 2: return &m_fntBig;
	case -1: return &m_fntFixedSmall;
	case -2: return &m_fntFixedMiddle;
	case -3: return &m_fntFixedBig;
	default: return &m_fntSmall;
	}
}

// �� ��Ʈ���� ȭ�鿡���� ũ�⸦ ���Ѵ� (������ ��Ʈ ���)
CSize CBoard::GetTextExtent( int nSize, LPCTSTR s )
{
	CClientDC dc(this);
	dc.SelectObject( GetFont(nSize) );
	return dc.GetTextExtent( s, _tcslen(s) );
}

// DSB ���� ����ϴ� ���� ���� ũ�� (�ּ���Ʈ�� ������)
int CBoard::GetDSBUnit()
{
	// ���� ��Ʈ�� ����� ���� ���� ���
	if ( !m_fntSmall.GetSafeHandle() ) return 0;

	LOGFONT lf;
	m_fntSmall.GetLogFont(&lf);
	if ( lf.lfHeight >= -5 ) return max( lf.lfHeight, 5 );
	else return -lf.lfHeight;
}

void CBoard::AddDSB( DSB* pDSB )
{
	ASSERT( GetSafeHwnd() );

	// �� ���� ������� �� ������ ����
	// Below �� �� ���� �Ϲݺ��� �� ������ ����
	POSITION pos = m_lpDSB.GetHeadPosition();
	for ( ; pos; m_lpDSB.GetNext(pos) ) {

		DSB* pCur = m_lpDSB.GetAt(pos);
		int i = pDSB->IsModal() ? 2 : pDSB->IsBelow() ? 0 : 1;
		int j = pCur->IsModal() ? 2 : pCur->IsBelow() ? 0 : 1;

		if ( i >= j ) {
			m_lpDSB.InsertBefore( pos, pDSB );
			break;
		}
	}
	if ( !pos ) m_lpDSB.AddTail(pDSB);

	if ( pDSB->IsModal() ) m_lpModalDSB.AddHead( pDSB );

	pDSB->OnFocus( true );
	pDSB->SetBmpMan( &m_bm );
	pDSB->OnInit();
	pDSB->OnMove();

	SetDSBTimer( pDSB );

	// �簢���� ������ �ٽ� �׸���
	RECT rc;
	pDSB->GetRect( &rc );
	UpdateDSB( &rc );
}

// DSB �� Ÿ�̸Ӹ� ����
void CBoard::SetDSBTimer( DSB* pDSB )
{
	// Ÿ�̸� ���̵�� tiDSB + DSB ID �� ����Ѵ�
	UINT nIDEvent = UINT( (int)tiDSB + pDSB->GetID() );

	if ( !pDSB->IsPermanent() )
		if ( !SetTimer( nIDEvent, pDSB->GetTimeOut(), NULL ) ) {
			// Ÿ�̸Ӹ� ���� �� ���� ���
			// �� �ð� ��ŭ ���� �� �� ���ش� !
			ASSERT(0);
			Sleep( pDSB->GetTimeOut() );
			pDSB->Destroy();
		}
}

// DSB �� ����
bool CBoard::RemoveDSB( DSB* pDSB )
{
	// �ش� dsb �� ã�´�
	POSITION pos = m_lpDSB.GetHeadPosition();
	for ( ; pos; m_lpDSB.GetNext(pos) )
		if ( m_lpDSB.GetAt(pos)->GetID() == pDSB->GetID() ) {

			m_lpDSB.GetAt(pos)->OnFocus(false);
			m_lpDSB.RemoveAt(pos);	// �ϴ� �簢���� ������ �Ѵ�

			// �� DSB �� ����Ű�� �����͵��� ��� �����Ѵ�

			// ���϶���Ʈ �Ǿ�����
			if ( m_pHLDSB == pDSB )
				m_pHLDSB = 0;
			// �巡�� ���̾���
			if ( m_pDragDSB == pDSB ) {
				m_pDragDSB = 0;
				m_bDragDSB = false;
				ReleaseCapture();
				ClipCursor( 0 );
			}
			// ä��, ���� DSB ����
			for ( int t = 0; t < MAX_PLAYERS; t++ )
				if ( m_apSayDSB[t] == pDSB ) {
					m_apSayDSB[t] = 0; break; }
				else if ( m_apGoalDSB[t] == pDSB ) {
					m_apGoalDSB[t] = 0; break; }
			// �缱�Ƿ��� ���� DSB ����
			if ( m_pMasterGoalDSB == pDSB )
				m_pMasterGoalDSB = 0;
			// ���� DSB ����
			if ( m_pCurrentElectionDSB == pDSB )
				m_pCurrentElectionDSB = 0;
			// ��� DSB ����
			POSITION posModal = m_lpModalDSB.GetHeadPosition();
			for ( ; posModal; m_lpModalDSB.GetNext(posModal) )
				if ( m_lpModalDSB.GetAt(posModal) == pDSB ) {
					m_lpModalDSB.RemoveAt(posModal); break; }

			if ( GetSafeHwnd() ) {
				RECT rc;
				pDSB->GetRect( &rc );
				// Ȥ�� �𸣴� Ÿ�̸� id �� ���ش�
				KillTimer( UINT( (int)tiDSB + pDSB->GetID() ) );
				// ������Ʈ
				UpdateDSB( &rc, true );
			}
			return true;
		}

	ASSERT(0);
	return false;
}

// DSB �� ����ϱ� ���� �뷫���� �÷��̾� ��ġ�� ���
// ���������� �� �÷��̾��� '��������'�� �����Ѵ�
// ( nPlayerNum �� -1 �̸� ��ü Ŭ���̾�Ʈ�� �� ��� )
// pbVert ���� ������ ���η� �䰡 ���θ� �����Ѵ�
RECT CBoard::CalcPlayerExtent( int nPlayerNum, bool* pbVert )
{
	// �� �÷��̾��� ���� �������� ī�� ������ ������ ����
	// ��ü Ŭ���̾�Ʈ ������ �������� �����Ѵ� !

	const CState* pState = m_pMFSM ? m_pMFSM->GetState() : 0;
	bool bVert = false;
	if ( pbVert ) *pbVert = bVert;	// �ӽ÷� Ŭ����

	CRect rcClient;
	{
		GetClientRect( &rcClient );
		if ( !pState || nPlayerNum < 0 ) return rcClient;
	}

	CRect rcScore = CalcRect( pState->nPlayers, CR_SCORE, nPlayerNum,
					m_szCard.cx, m_szCard.cy, -1, 0, &bVert, 0 );
	if ( nPlayerNum >= pState->nPlayers ) return rcScore;
	CRect rcHand = CalcRect( pState->nPlayers, CR_HAND, nPlayerNum,
					m_szCard.cx, m_szCard.cy );

	if ( pbVert ) *pbVert = bVert;

	// ���� �������� ī�� ������ ����
	if ( bVert ) {
		if ( rcHand.left < rcScore.left ) rcScore.left = rcHand.right;
		else rcScore.right = rcHand.left;
	} else {
		if ( rcHand.top < rcScore.top ) rcScore.top = rcHand.bottom;
		else rcScore.bottom = rcHand.top;
	}
	// Ŭ���̾�Ʈ�� Ŭ��
	rcScore &= rcClient;

	return rcScore;
}

// �ش� �÷��̾� �ڸ��� ä�� DSB �� ����
// nPlayerID : �÷��̾� ID
// sMsg    : �޽���
void CBoard::FloatSayDSB( int nPlayerID, LPCTSTR sMsg )
{
	int nPlayer = -1;	// �÷��̾� ��ȣ
	const CState* pState = 0;
	if ( m_pMFSM ) {
		pState = m_pMFSM->GetState();
		nPlayer = m_pMFSM->GetPlayerNumFromID( nPlayerID );
	}
	if ( !pState ) return;

	CRect rcRange = CalcPlayerExtent( nPlayer );

	int x = rcRange.left + rcRange.Width()/2;
	int y = rcRange.top + rcRange.Height()/2;

	int nUnit = GetDSBUnit();
	CRect rcClient; GetClientRect( &rcClient );
	int cxDSB = rcClient.Width()*2/5 / nUnit - 2;

	DSay* pDSB = new DSay(m_pWrap);
	pDSB->Create( m_apSayDSB[nPlayerID],
		x, y, cxDSB,
		pState->apAllPlayers[nPlayerID]->GetName(),
		sMsg, Mo()->nChatDSBTimeOut * 2000,
		nPlayerID == 0 ? true : false );

	m_apSayDSB[nPlayerID] = pDSB;
}

// �ش� �÷��̾� �ڸ��� ���� DSB �� ����
// nPlayer : �÷��̾� ��ȣ
// goal    : �� �÷��̾��� ����
// goal.nMinScore �� 0 �̸� �⸶ ����, -1 �̸� ���̽�
void CBoard::FloatGoalDSB( int nPlayer, const CGoal* pGoal )
{
	static const UINT arcBitmap[] = {
		IDB_NOTHING, IDB_SPADE, IDB_DIAMOND, IDB_HEART, IDB_CLOVER };

	if ( !m_pMFSM ) return;

	if ( nPlayer == -1 ) {	// ������ DSB �� ����
		for ( int i = 0; i < MAX_PLAYERS; i++ )
			if ( m_apGoalDSB[i] ) m_apGoalDSB[i]->Destroy();
		m_pMasterGoalDSB = 0;
		return;
	}

	// ���ο� ������ ��Ÿ���� ������ �缱 DSB �� Ÿ�Ӿƿ� DSB �� �����
	if ( pGoal->nMinScore != 0 && m_pMasterGoalDSB ) {
		m_pMasterGoalDSB->SetTimeout( DELAY_GOALDSBTIMEOUT );
		m_pMasterGoalDSB = 0;
	}

	const CState* pState = m_pMFSM ? m_pMFSM->GetState() : 0;
	CString sName = pState && nPlayer >= 0
			? (LPCTSTR)pState->apAllPlayers[nPlayer]->GetName()
			: _T("");

	DShortMessage* pDSB = new DShortMessage(m_pWrap);

	if ( pGoal->nMinScore < 0 ) {	// ���̽�

		PlaySound( IDW_CARDSETUP, true );
		pDSB->Create( 0, _T("�� �̽� !"),
			true, true, DELAY_DEALMISS, nPlayer, sName );
		if ( m_apGoalDSB[nPlayer] ) m_apGoalDSB[nPlayer]->Destroy();
		m_apGoalDSB[nPlayer] = pDSB;
	}
	else if ( pGoal->nMinScore == 0 ) {	// �⸶ ����

		PlaySound( IDW_GIVEUP, true );
		pDSB->Create( IDB_STOP, _T(" �⸶ ����"),
			true, true, DELAY_GIVEUPDSBTIMEOUT, nPlayer, sName );
		if ( m_apGoalDSB[nPlayer] ) m_apGoalDSB[nPlayer]->Destroy();
		m_apGoalDSB[nPlayer] = pDSB;
	}
	else {

		// �⺻ �ּ�����
		const CState* pState = m_pMFSM ? m_pMFSM->GetState() : 0;
		int nDefaultMinScore = pState ? pState->pRule->nMinScore : 0;

		PlaySound( IDW_GOAL, true );
		pDSB->Create(
			arcBitmap[pGoal->nKiruda],
			CString(_T(" ")) + CCard::GetGoalString(
				Mo()->bUseTerm, pGoal->nKiruda,
				pGoal->nMinScore ),
			true, true, -1, nPlayer, sName );

		if ( m_apGoalDSB[nPlayer] ) m_apGoalDSB[nPlayer]->Destroy();
		m_apGoalDSB[nPlayer] = pDSB;
		m_pMasterGoalDSB = pDSB;
	}
}

// ������ DSB �� ���� DSB �� �����Ѵ�
// �ѹ��� �ϳ��� ���� DSB �� ȭ�鿡 ǥ�õǹǷ�
// ������ ���� DSB �� ���ŵȴ�
// pDSB �� 0 �̸� ������ ���� DSB �� ���ŵȴ�
void CBoard::SetElectionDSB( DSB* pDSB )
{
	if ( m_pCurrentElectionDSB )
		m_pCurrentElectionDSB->Destroy();
	ASSERT( !m_pCurrentElectionDSB );
	m_pCurrentElectionDSB = pDSB;
}
