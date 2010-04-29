// Board_SE.cpp : Special Effecting implementation for CBoard
//

#include "stdafx.h"
#include "Mighty.h"
#include "BmpMan.h"
#include "Play.h"
#include "Player.h"
#include "MFSM.h"
#include "Board.h"
#include "BoardWrap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// �� �Լ��� ȣ���ϸ� ���콺 Ŭ��, �Ǵ� ä�� â����
// ����ڰ� ī�带 ������ �� ���� ����ߴٰ�
// ������ �̺�Ʈ�� ��Ʈ�Ѵ�
void CBoard::WaitSelection( CEvent* pEvent, long* pSelectedCard )
{
	ASSERT( !m_pWaitingSelectionEvent );

	m_pWaitingSelectionEvent = pEvent;
	m_pWaitingSelectionCard = pSelectedCard;

	// Ŀ��, ī���̵� ȿ���� �����ش�

	// ���� ��ǥ�� ��´�
	POINT point;
	GetCursorPos( &point );
	ScreenToClient( &point );

	// ī�� �̵�
	if ( CanSelect() ) {
		ShiftCard( point.x, point.y, true );
		SetCursor( m_hHand );
	}
}

// ���� ���¸� ����Ѵ�
void CBoard::CancelSelection()
{
	ASSERT( m_pWaitingSelectionEvent );
	m_pWaitingSelectionEvent = 0;
	m_pWaitingSelectionCard = 0;

	SetCursor( m_hArrow );

	// �ٽ� �÷��� ī�带 �����·�
	m_nMouseOver = -1;
	// ShiftCard( (int)0x80000000, (int)0x80000000, false );
}

// ���� �簢���� �ش� �÷��̾� ��ġ�� �̵� (-1 �� ����)
void CBoard::SetTurnRect( int nPlayer )
{
	if ( !m_pMFSM || !m_pMFSM->GetState() ) {
		m_nTurnRect = nPlayer;
		return;
	}

	int nTurnRect = m_nTurnRect;
	m_nTurnRect = nPlayer;

	if ( nTurnRect != -1	// ���� �簢���� ����
		&& nTurnRect < m_pMFSM->GetState()->nPlayers
		&& nTurnRect != m_nTurnRect ) {

		CRect rc;
		{
			CClientDC dc(this);
			rc = CalcNameRect( m_pMFSM->GetState()->nPlayers, nTurnRect,
					&dc, m_pMFSM->GetState()->apPlayers[nTurnRect]->GetName(),
					&m_fntSmall );
		}
		rc.InflateRect( 3, 3 );
		UpdatePlayer( nTurnRect, &rc );
	}
	if ( m_nTurnRect != -1
		&& m_nTurnRect < m_pMFSM->GetState()->nPlayers ) {

		CRect rc;
		{
			CClientDC dc(this);
			rc = CalcNameRect( m_pMFSM->GetState()->nPlayers, m_nTurnRect,
					&dc, m_pMFSM->GetState()->apPlayers[m_nTurnRect]->GetName(),
					&m_fntSmall );
		}
		rc.InflateRect( 3, 3 );
		UpdatePlayer( m_nTurnRect, &rc );
	}
}

// ������ �ð����� ������ �ִٰ� ������ �̺�Ʈ�� ��Ʈ�Ѵ�
void CBoard::WaitWhile( int nMillisecond, CEvent* pEvent )
{
	// Flying ť�� �߰�
	AddFlyingQueue( new CMsg( _T("lll"), opWaitWhile,
		(long)nMillisecond, (long)(LPVOID)pEvent ) );
}

// ī�带 ������ ���ϸ��̼� ��
// ������ �̺�Ʈ�� ��Ʈ�Ѵ�
// (�׸��� ȿ���� �߾��� ���� �������� ����)
void CBoard::FlyCard( int nCard, int nSpeed,
	int nFrom, int nFromPlayer, int nFromIndex,
	int nTo, int nToPlayer, int nToIndex, CEvent* pEvent )
{
	// Flying ť�� �߰�
	AddFlyingQueue( new CMsg( _T("llllllllll"), opFlyCard,
		(long)nCard, (long)nSpeed,
		(long)nFrom, (long)nFromPlayer, (long)nFromIndex,
		(long)nTo, (long)nToPlayer, (long)nToIndex,
		(long)(LPVOID)pEvent ) );
}

// ī�� ���� ���ϸ��̼� ��
// ������ �̺�Ʈ�� ��Ʈ�Ѵ�
// ������ IDW_SUFFLE �� �����Ѵ�
void CBoard::SuffleCards( int nCount /*���� ȸ��*/, CEvent* pEvent )
{
	// Flying ť�� �߰�
	AddFlyingQueue( new CMsg( _T("lll"), opSuffleCards,
		(long)nCount, (long)(LPVOID)pEvent ) );
}

// ī�� ������/���� ������ ���� �Լ�
UINT CBoard::FlyProc( LPVOID _pThis )
{
	TRACE("FlyProc Began\n");

	// ���� Ÿ�̸Ӹ� ���� ������ �ܰ�� ��Ʈ�Ѵ�
	bool bPeriodBegan = false;
	TIMECAPS tc;
	if ( ::timeGetDevCaps( &tc, sizeof(tc) ) == TIMERR_NOERROR
			&& ::timeBeginPeriod( tc.wPeriodMin ) == TIMERR_NOERROR )
		bPeriodBegan = true;


	CBoard* pThis = (CBoard*)_pThis;

	while ( !pThis->m_bFlyProcTerminate ) {

		// �̺�Ʈ�� ���� ���
		VERIFY( pThis->m_eFlyIt.Lock() );

		// ť�� �˻��Ͽ� ���ο� �۾��� �����Ѵ�

		CMsg* pMsg = 0;
		AUTODELETE_MSG(pMsg);

		pThis->m_csFlying.Lock();
			if ( pThis->m_lFlyingQueue.GetCount() > 0 )
				pMsg = pThis->m_lFlyingQueue.RemoveHead();
		pThis->m_csFlying.Unlock();

		if ( !pMsg ) continue;	// �ƹ� �ϵ� ����

		long nOp; VERIFY( pMsg->PumpLong(nOp) );
		switch ( (OPERATION)nOp ) {

		case opWaitWhile:	// ���
		{
			// ���ڸ� ��´�
			long nMilliseconds, dummy;
			CEvent* pEvent;

			VERIFY( pMsg->PumpLong(nMilliseconds) );
			VERIFY( pMsg->PumpLong(dummy) );
			pEvent = (CEvent*)(LPVOID)dummy;

			// �� �ð���ŭ ��¥�� ����Ѵ�
			Sleep( nMilliseconds );

			pEvent->SetEvent();

		} break;

		case opFlyCard:		// ī�� ������
		{
			// ���ڸ� ��´�
			long nCard, nSpeed, nFrom, nFromPlayer, nFromIndex,
				nTo, nToPlayer, nToIndex, dummy;
			CEvent* pEvent;

			VERIFY( pMsg->PumpLong(nCard) );
			VERIFY( pMsg->PumpLong(nSpeed) );
			VERIFY( pMsg->PumpLong(nFrom) );
			VERIFY( pMsg->PumpLong(nFromPlayer) );
			VERIFY( pMsg->PumpLong(nFromIndex) );
			VERIFY( pMsg->PumpLong(nTo) );
			VERIFY( pMsg->PumpLong(nToPlayer) );
			VERIFY( pMsg->PumpLong(nToIndex) );
			VERIFY( pMsg->PumpLong(dummy) );
			pEvent = (CEvent*)(LPVOID)dummy;

			if ( nSpeed == 1 || Mo()->bSlowMachine )
				pThis->DoFlyCard2( nCard, nSpeed, nFrom, nFromPlayer, nFromIndex,
					nTo, nToPlayer, nToIndex );
			else pThis->DoFlyCard( nCard, nSpeed, nFrom, nFromPlayer, nFromIndex,
					nTo, nToPlayer, nToIndex );

			pEvent->SetEvent();

		} break;

		case opSuffleCards:	// ī�� ����
		{
			// ���ڸ� ��´�
			long nCount, dummy;
			CEvent* pEvent;

			VERIFY( pMsg->PumpLong(nCount) );
			VERIFY( pMsg->PumpLong(dummy) );
			pEvent = (CEvent*)(LPVOID)dummy;

			pThis->DoSuffleCards( nCount );

			pEvent->SetEvent();

		} break;

		default:			// �� �ܿ��� ����
			ASSERT(0);
			break;
		}
	}

	// Ÿ�̸Ӹ� ����ġ
	if ( bPeriodBegan )
		VERIFY( ::timeEndPeriod( tc.wPeriodMin ) == TIMERR_NOERROR );

	TRACE("FlyProc Ended\n");
	pThis->m_bFlyProcTerminated = true;
	return 0;
}


// ���� ī�带 ���´� ( FlyProc ������ ȣ��� )
void CBoard::DoSuffleCards( int nCount )
{
	ASSERT( !m_hSuffling );

	// �� �ܰ踶�� �ɸ��� �ð�
	int nInterval = DELAY_SUFFLESTEP;

	// ī�� ���� ����
	SuffleCards( m_hSuffling, nCount, m_szCard );

	while ( !m_bFlyProcTerminate && m_hSuffling ) {
		SuffleCards( m_hSuffling );
		Sleep( nInterval );
	}
}

// ���� ī�带 ������ ( FlyProc ������ ȣ��� )

// ����� ���� (�Ǵ� Speed == 1 (�ְ�ӵ�)�϶�)
void CBoard::DoFlyCard2(
	int nCard,			// ���� ī��
	int nSpeed,			// �ӵ� ( 0 : ǥ�� 1 : �ſ� ���� 2 : ���� )
	int nFrom,			// ���� ��ġ ( 0 ��� 1 �������� 2 �տ��� ī�� 3 ���°� 4 �� �����!! )
	int nFromPlayer,	// ���� ��ġ ( �÷��̾� ��ȣ, nFrom!=0�϶��� ���  )
	int nFromIndex,		// �� ��° ī���ΰ� (-1�̸� �� ���)
	int nTo,			// �� ��ġ
	int nToPlayer,
	int nToIndex )
{
	ASSERT( !m_hFlying );

	// ���ۿ��� ������ �� ������ ī�� ����
	CRect rcBegin = CalcCardRectForFlyCard(
						nFrom, nFromPlayer, nFromIndex );
	CRect rcEnd = CalcCardRectForFlyCard(
						nTo, nToPlayer, nToIndex );

	// ���� ��� ( ����� ���� )
	int nShadeMode = ( nTo == 0 || nTo == 1 || nTo == 4 ? 0 : 2 )
		| ( nFrom == 0 || nFrom == 1 || nFrom == 4 ? 0 : 1 );

	// �����Ӱ� ����
	int nFlyingInterval = 0;

	// �� ������ ��
	int nStates = 2;

	// ������ ����
	FlyCard( m_hFlying, nCard, rcBegin, rcEnd.TopLeft(), nStates, nShadeMode );

	while ( !m_bFlyProcTerminate && m_hFlying ) {
		Sleep( nFlyingInterval );
		FlyCard( m_hFlying );
	}

	nSpeed;	//unused
}

void CBoard::DoFlyCard(
	int nCard,			// ���� ī��
	int nSpeed,			// �ӵ� ( 0 : ǥ�� 1 : �ſ� ���� 2 : ���� )
	int nFrom,			// ���� ��ġ ( 0 ��� 1 �������� 2 �տ��� ī�� 3 ���°� 4 �� �����!! )
	int nFromPlayer,	// ���� ��ġ ( �÷��̾� ��ȣ, nFrom!=0�϶��� ���  )
	int nFromIndex,		// �� ��° ī���ΰ� (-1�̸� �� ���)
	int nTo,			// �� ��ġ
	int nToPlayer,
	int nToIndex )
{
	ASSERT( !m_hFlying );
	ASSERT( nSpeed == 0 || nSpeed == 2 );

	// ���ۿ��� ������ �� ������ ī�� ����
	CRect rcBegin = CalcCardRectForFlyCard(
						nFrom, nFromPlayer, nFromIndex );
	CRect rcEnd = CalcCardRectForFlyCard(
						nTo, nToPlayer, nToIndex );

	// ���� ��� ( ����� ���� )
	int nShadeMode = ( nTo == 0 || nTo == 1 || nTo == 4 ? 0 : 2 )
		| ( nFrom == 0 || nFrom == 1 || nFrom == 4 ? 0 : 1 );

	// ��ü ������ �ð� (ms)
	long tTotalTime = 20 + (10-Mo()->nCardSpeed) * 100;
	if ( nSpeed == 2 ) tTotalTime = tTotalTime * 2/5;	// 40%

	// �� ������ ��
	int nStates;
	if ( nSpeed == 2 ) nStates = 2 + Mo()->nCardMotion*2;
	else nStates = 5 + Mo()->nCardMotion*5;

	// ���� ������
	int nState = 0;
	// �������
	long tBuffer = 5;	// 5 millisec
	// ���۽ð�
	long tBegin = 0;
	// ���� �ð��̾�� �ϴ� ��
	long tCurrentLogical = 0;
	// ��¥ ���� �ð�
	long tCurrentReal = 0;
	// ����
	long tDiff = 0;
	// �� ���� ������ ��
	int nFramesToSkip = 0;

	// ������ ����
	FlyCard( m_hFlying, nCard, rcBegin, rcEnd.TopLeft(), nStates, nShadeMode );

	while ( !m_bFlyProcTerminate && m_hFlying ) {

		if ( nState == 0 ) {
			// ù �������� �׳� �׸���
			FlyCard( m_hFlying );
			nState++;
			tBegin = (long)::timeGetTime();
			continue;
		}

		tCurrentLogical = tBegin
			+ tTotalTime * ( nState - 1 ) / ( nStates - 1 );
		tCurrentReal = (long)::timeGetTime();
		tDiff = tCurrentLogical - tCurrentReal;

		if ( tDiff >= tBuffer ) {	// �ʹ� ������ !
			Sleep( 0 );
			continue;
		}
		else if ( -tBuffer < tDiff ) {	// �� �ϰ� �ִ�
			nFramesToSkip = 0;
		}
		else {	// �ʹ� ������ (��ŵ)
			nFramesToSkip = -tDiff * nStates / tTotalTime;
		}

		FlyCard( m_hFlying, nFramesToSkip + 1 );

		nState += ( nFramesToSkip + 1 );
		Sleep(0);
	}
}
