// PlayerHuman.cpp: implementation of the CPlayerHuman class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mighty.h"

#include "Play.h"
#include "player.h"
#include "PlayerHuman.h"

#include "BmpMan.h"
#include "Board.h"
#include "InfoBar.h"
#include "MFSM.h"

#include "BoardWrap.h"
#include "DSB.h"
#include "DEtc.h"
#include "DElection.h"
#include "DReport.h"
#include "DScoreBoard.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// CPlayerHuman

CPlayerHuman::CPlayerHuman(
		int nID,		// ���� ��ȣ ( 7���� �÷��̾��� ���� )
		LPCTSTR sName,	// �̸�
		CWnd* pCallback )// call-back window
	: CPlayer( nID, sName, pCallback )
{
	m_pBoard = (CBoardWrap*)((CBoard*)pCallback)->GetWrapper();

	// ä��â �ڵ鷯
	Ib()->SetChatHandler( (DWORD)(LPVOID)this, ChatProc );
}

CPlayerHuman::~CPlayerHuman()
{
	Ib()->SetChatHandler( (DWORD)(LPVOID)this, 0 );

	if ( m_pMFSM->GetState()->IsNetworkGame() ) {
		// ��Ʈ��ũ �����̾��ٸ�, ������ ���������� Update �Ѵ�
		Mo()->anPlayerState[0] = MAKELONG( m_recAll.wm, m_recAll.lm );
		Mo()->anPlayerState[1] = MAKELONG( m_recAll.wf, m_recAll.lf );
		Mo()->anPlayerState[2] = MAKELONG( m_recAll.wa, m_recAll.la );
	}
}

// ���� �Լ���

const CState* CPlayerHuman::GetState()
{
	return m_pMFSM->GetState();
}

// Ż����ų ī�带 ��õ�Ѵ�
int CPlayerHuman::GetRecommendedKillCard( const CCardList* plDead )
{
	// �տ� ���� �ְ� ���ٸ� ã�´�

	int nKiruda = GetState()->goal.nKiruda;
	int nFrom, nTo;

	if ( !nKiruda )	// nokiruda
		nFrom = SPADE, nTo = CLOVER;
	else nFrom = nKiruda, nTo = nKiruda;

	for ( int s = nFrom; s <= nTo; s++ )
		if ( !GetHand()->Find( CCard( s, ACE ) )
			&& !plDead->Find( CCard( s, ACE ) ) )
			return (int)CCard( s, ACE );
		else for ( int i = KING; i > 2; i-- )
			if ( !GetHand()->Find( CCard( s, i ) )
				&& !plDead->Find( CCard( s, i ) ) )
				return (int)CCard( s, i );
	ASSERT(0);
	return (int)CCard(nKiruda,2);	// never reached
}

static bool find_both( const CCardList* p1, const CCardList* p2, CCard c )
{	return p1->Find(c) || p2->Find(c); }

// ������ ī�带 ��õ�Ѵ�
int CPlayerHuman::GetRecommendedFriend( const CCardList* plDeck )
{
	int i;
	CCard c;
	const CCardList* plHand = GetHand();

	// ����, ����Ƽ !
	c = CCard::GetMighty();
	if ( !find_both( plHand, plDeck, c ) ) return c;

	// ����, ��Ŀ
	c = CCard::GetJoker();
	if ( GetState()->pRule->bJokerFriend
		&& !find_both( plHand, plDeck, c ) ) return c;

	// ��A
	c = CCard( CCard::GetKiruda(), ACE );
	if ( !find_both( plHand, plDeck, c ) ) return c;

	// �� K
	c = CCard( CCard::GetKiruda(), KING );
	if ( !find_both( plHand, plDeck, c ) ) return c;

	// �ٸ� ACE
	for ( i = SPADE; i <= CLOVER; i++ ) {
		c = CCard( i, ACE );
		if ( !find_both( plHand, plDeck, c ) ) return c;
	}

	// �� Q
	c = CCard( CCard::GetKiruda(), QUEEN );
	if ( !find_both( plHand, plDeck, c ) ) return c;

	return 1;	// �ʱ�
}

// p �� �ִ� ī�带 �信�� �����Ѵ�
void CPlayerHuman::SetCurrentSelectedCard( CCardList* p )
{
	m_pBoard->SetSelection( false );
	// ���� ī�� ����Ʈ�� ���鼭 �ٽ� ���� ī�� �迭�� ��Ʈ�Ѵ�
	POSITION pos = p->GetHeadPosition();
	while (pos) m_pBoard->SetSelection(
		GetHand()->IndexFromPOSITION(
			GetHand()->Find( p->GetNext(pos) ) ) );
}

// ���� �� ���¿��� ������ ���带 �����Ѵ�
void CPlayerHuman::PlayTurnSound()
{
	CCard c( GetState()->cCurrentCard );

	// ����Ƽ ����
	if ( c.IsMighty() )
		PlaySound( IDW_MIGHTY, true );
	// ��Ŀ�� ����
	else if ( c.IsJokercall()
		&& GetState()->nBeginer == GetState()->nCurrentPlayer ) {

		if ( GetState()->bJokercallEffect ) {
			// ��¥ ��Ŀ��
			(new DShortMessage(m_pBoard))->Create(
				0, Mo()->bUseTerm ? _T("��ī�� !") : _T("��Ŀ �� !"),
				true, false, DELAY_JOKERDSB );
			PlaySound( IDW_JOKERCALL, true );
		}
		else {
			// ��Ŀ�� �ƴ�
			(new DShortMessage(m_pBoard))->Create(
				0, Mo()->bUseTerm ? _T("���� �ƴ�") : _T("��Ŀ�� �ƴ�"),
				true, false, DELAY_JOKERDSB );
			PlaySound( IDW_CARD );
		}
	}
	// ���� ��Ŀ ����
	else if ( c.IsJoker() && GetState()->bJokercallEffect )
		PlaySound( IDW_KILLEDJOKER );	// ���� ��Ŀ�� �Ҹ� �켱������ ����
	// ��Ŀ ����
	else if ( c.IsJoker() ) {

		if ( GetState()->nBeginer == GetState()->nCurrentPlayer ) {
			// ���� ���
			static LPCTSTR s_asJokerShape[2][5] = {
				{ _T(" ���� !"), _T(" �ٸ��� !"), _T(" Ʈ�� !"), _T(" ���� !") },
				{ _T(" �����̵� !"), _T(" ���̾Ƹ�� !"), _T(" ��Ʈ !"), _T(" Ŭ�ι� !") }
			};

			(new DShortMessage(m_pBoard))->Create(
				IDB_SPADE + GetState()->nJokerShape - SPADE,
				s_asJokerShape[Mo()->bUseTerm?0:1][GetState()->nJokerShape-SPADE],
				true, true, DELAY_JOKERDSB );
		}
		PlaySound( IDW_JOKER, true );
	}
	// ��� (��)
	else if ( c.IsKiruda() && GetState()->lCurrent.GetCount() > 0
		&& !GetState()->lCurrent.GetHead().IsJoker()
		&& !GetState()->lCurrent.GetHead().IsKiruda() ) {
		PlaySound( IDW_KIRUDA, true );
	}
	// �Ϲ�
	else PlaySound( IDW_CARD );
}

// ��Ŀ, ��Ŀ���� ó���Ѵ� (��ҽ� true ����)
bool CPlayerHuman::ProcessSpecialCards( CCard c, int* eff )
{
	const CState* pState = GetState();

	if ( c.IsJoker() )	{	// ��Ŀ�� ���

		if ( pState->nCurrentPlayer != pState->nBeginer )
			// ���� �ƴϸ� ��� ����
			return false;

		static LPCTSTR s_asJokerShape[2][6] = {
			{ _T("������ !"), _T("���� !"), _T("�ٸ��� !"), _T("Ʈ�� !"), _T("���� !"), _T("���") },
			{ _T("����"), _T("�����̵�"), _T("���̾Ƹ��"), _T("��Ʈ"), _T("Ŭ�ι�"), _T("���") }
		};

		// ���� ���콺 ��ġ
		CPoint pnt; ::GetCursorPos(&pnt);
		((CWnd*)*m_pBoard)->ScreenToClient(&pnt);

		// ���� ���ڸ� ǥ��
		long nSel = 0;
		DSelect* pSelDSB = new DSelect(m_pBoard);
		pSelDSB->Create( pnt.x+5, pnt.y,
			s_asJokerShape[Mo()->bUseTerm?0:1], 6,
			&m_eTemp, &nSel );

		m_pMFSM->WaitEvent( &m_eTemp );

		m_pBoard->UpdatePlayer( 0 );

		if ( nSel == 5 || nSel == -1 ) return true;	// ���

		if ( nSel == 0 ) {
			nSel = GetState()->goal.nKiruda;
			if ( nSel == 0 ) return true;	// �����
		}

		*eff = nSel;

		return false;
	}
	else if ( c.IsJokercall() ) {	// ��Ŀ���� ���

		// �ϴ� ��Ŀ���� ������ ��Ȳ�ΰ��� ����

		if ( pState->nCurrentPlayer != pState->nBeginer )
			// ���� �ƴϸ� ��Ŀ�� �Ұ���
			return false;
		if ( pState->bJokerUsed )
			// ��Ŀ�� �̹� ���͹��� ���
			return false;
		if ( !pState->pRule->bInitJokercallEffect && pState->nTurn == 0 )
			// ù�� ��Ŀ�� ȿ�¾���
			return false;

		static LPCTSTR s_asJokercall[2][3] = {
			{ _T("��ī�� !"), _T("���ݾƴ�"), _T("���") },
			{ _T("��Ŀ �� !"), _T("��Ŀ �� �ƴ�"), _T("���") }
		};

		// ���� ���콺 ��ġ
		CPoint pnt; ::GetCursorPos(&pnt);
		((CWnd*)*m_pBoard)->ScreenToClient(&pnt);

		// ���� ���ڸ� ǥ��
		long nSel = 0;
		DSelect* pSelDSB = new DSelect(m_pBoard);
		pSelDSB->Create( pnt.x+5, pnt.y,
			s_asJokercall[Mo()->bUseTerm?0:1], 3,
			&m_eTemp, &nSel );

		m_pMFSM->WaitEvent( &m_eTemp );

		m_pBoard->UpdatePlayer( 0 );

		*eff = nSel == 0 ? 1 : 0;

		if ( nSel == 2 || nSel == -1 ) // ���
			return true;
		else return false;
	}
	else return false;	// �� ���� ī��
}

// ä��â �ڵ鷯
void CPlayerHuman::ChatProc( LPCTSTR s, DWORD dwUser )
{
	CPlayerHuman* pThis = (CPlayerHuman*)(LPVOID)dwUser;
	CMsg* pMsg = new CMsg( _T("lls"), CMsg::mmChat,
		pThis->m_pMFSM->GetPlayerUIDFromID( pThis->GetID() ), s );
	pThis->m_pMFSM->EventChat( pMsg, true );
}


// CPlayer �������̽� ����

// �� ���� ���۵� - OnBegin ��, ���� ���� �й���� ���� ����
void CPlayerHuman::OnInit( CEvent* e )
{
	// ���� �ʱ�ȭ
	m_clTemp.RemoveAll();

	// Board �� �ʱ�ȭ
	m_pBoard->SetDealMiss(-1);
	m_pBoard->SetSelection(false);
	m_pBoard->SetElectionDSB( 0 );
	m_pBoard->SetTurnRect( -1 );
	Ib()->Reset();

	// �������� ����
	Sb()->Update(GetState());

	// ¦! �Ҹ��� ����
	PlaySound( IDW_CARDSETUP, true );
	m_pBoard->UpdatePlayer( -2 );

	// ���� ��ٸ��� (DELAY_SETUP_SUFFLING)
	if ( Mo()->bShowDealing )
		Sleep( DELAY_SETUP_SUFFLING );

	// ī�带 ���� �����Ѵ�
	// �� ���̸� e �� ��Ʈ�� ���̴�
	if ( Mo()->bShowDealing )
		m_pBoard->SuffleCards( 2, e );
	else e->SetEvent();
}

// ī�带 ���� �ְ� ���� ( nFrom ���� nTo �� (-1�� �߾�)
// nCurrentCard �� �̵��Ͽ��� )
// nMode  0 : ī�带 ������ �ʰ�, ���� ��ü ȭ�� ���� �ʿ�
//        1 : ī�� ������
//        2 �̻� : Mo()->bShowDealing �� ���� ���� �� ������ ��
//        3 �̻� : �ӵ��� �ʹ� ������ �ϸ� �ȵ�
//        9 �̻� : ī�� ����
//        10 : �ְ��� Privilege �ܰ���, �ӵ��� ������, �Ҹ� ����
void CPlayerHuman::OnDeal( int nFrom, int nTo, int nMode, int nCard, CEvent* e )
{
	if ( nMode == 0 ) {
		m_pBoard->SetTurnRect( -1 );
		m_pBoard->UpdatePlayer( -2 );
		e->SetEvent();
		return;
	}

	// �ҽ�(nFrom)�� �׸��� �ٽ� �����Ѵ�
	m_pBoard->UpdatePlayer( nFrom, 0, true );

	if ( nMode == 10 )
		PlaySound( IDW_GETCARD, true );

	if ( Mo()->bShowDealing ) {
		// ī�带 ������ �ϸ�

		int nSpeed =  nMode == 10 ? 0 : nMode >= 3  ? 2 : 1;

		int nFlyingCard = 0;
		// ���� ī�� �ߴ� �����̸� �ְ��� ���� ������ ī�带 �����ش�
		if ( nMode == 10 && GetState()->pRule->bAttScoreThrownPoints )
			nFlyingCard = nCard;

		// ī�带 ������
		m_pBoard->FlyCard( nFlyingCard, nSpeed,
			nFrom == -1 ? 4 : 2, nFrom, -1,
			nTo == -1 ? 4 : 2, nTo, -1, &m_eTemp );

		// �� ���������� ��ٸ���
		m_pMFSM->WaitEvent( &m_eTemp );
	}

	bool bUpdateSelectionRegion = false;
	if ( nMode >= 9 && nTo == 0 && GetState()->nMaster == 0
			&& GetState()->apPlayers[0]->IsHuman() ) {
		m_clTemp.AddTail(nCard);
		SetCurrentSelectedCard( &m_clTemp );
		bUpdateSelectionRegion = true;
	}

	// ī�尡 ������ ���������Ƿ� ������(to)�� �ٽ� �׸���
	Sleep(0);
	m_pBoard->UpdatePlayer( nTo, 0, false, bUpdateSelectionRegion );
	e->SetEvent();
}

// �ϳ��� ������ ���۵�
// �� ���ӿ��� ���Ǵ� ���� ������ �˷��ش�
// �� ���� ������ CCard::GetState() �ε� �� �� �ִ�
// pState->nCurrentPlayer ���� �ٷ� �ڱ� �ڽ���
// ��ȣ�̸�, �� ���� �ڽ��� �ν��ϴµ� ���ȴ�
void CPlayerHuman::OnBegin( const CState* pState, CEvent* e )
{
	// �ݾ�! �ϴ� ���� �Ҹ��� ����
	PlaySound( IDW_BEGIN, true );

	// ī�带 �����ϰ� �̸��� �ٽ� �׸��� ���ؼ� ��ü ȭ���� �����Ѵ� !
	m_pMFSM->SortPlayerHand( Mo()->bLeftKiruda, Mo()->bLeftAce );
	m_pBoard->UpdatePlayer( -2 );

	// �����Ѵٴ� DSB �� ����
	// ���� ȸ���� �޽��� �ڽ��� ����Ѵ�
	CString sMsg;
	if ( GetState()->nGameNum == 1 ) sMsg = _T("�ʴ� ����");
	else sMsg.Format( _T("�� %d �� �뼱"), GetState()->nGameNum );

	DShortMessage* pDSB = new DShortMessage( m_pBoard );
	pDSB->Create( 0, sMsg, true, false, -1 );
	m_pBoard->SetElectionDSB( pDSB );

	CPlayer::OnBegin( pState, e );
}

// 6������ �缱�� ��� �� ����� �׿��� �Ѵ�
// ���� ī�带 �����ϸ� �ȴ� - �� �� �Լ���
// �ݺ������� ȣ��� �� �ִ� - �� ���
// CCardList �� ���ݱ��� ������ ī���� ����Ʈ��
// �����Ǿ� ȣ��ȴ�
// 5�� �����ϸ� (�� ��� �˰����� �߸��Ǿ��ų�
// ����� �� �� �����ϴ� ���) ���Ƿ� ������ 5�� ��
// �ϳ��� �״´� !
void CPlayerHuman::OnKillOneFromSix( CCard* pcCardToKill,
	CCardList* plcFailedCardsTillNow, CEvent* e )
{
	// ���̱� DSB �� ǥ��
	DKill* pDSB = new DKill(m_pBoard);
	*(CCard*)pcCardToKill =
		GetRecommendedKillCard(
			(CCardList*)plcFailedCardsTillNow );
	pDSB->Create( e, (CCard*)pcCardToKill,
		(CCardList*)plcFailedCardsTillNow, GetHand() );
}

// �ְ��� �ٸ� �÷��̾ ���δ�
// bKilled : ���̸� ������ �׿���, �����̸� ��ٸ� ¤����
void CPlayerHuman::OnKillOneFromSix( CCard cKill,
	bool bKilled, CEvent* e )
{
	const CState* pState = m_pMFSM->GetState();

	// �ϴ� ��Ŀ�� �Ҹ��� ����
	PlaySound( IDW_JOKERCALL, true );

	// DSB �� ǥ��
	DDeadDecl* pDecl = new DDeadDecl( m_pBoard );
	pDecl->Create(
		pState->apAllPlayers[pState->nMaster]->GetName(),
		(int)cKill, false, -1 );
	m_pBoard->SetElectionDSB( pDecl );

	// ���� !!
	if ( !bKilled ) {

		// ��� ����
		Sleep( DELAY_KILL_AND_EFFECT );

		// �Ƿ� �Ҹ��� ����
		PlaySound( IDW_GIVEUP, true );

		// ���� DSB �� ǥ��
		DDeadDecl* pDecl = new DDeadDecl( m_pBoard );
		pDecl->Create(
			pState->apAllPlayers[pState->nMaster]->GetName(),
			(int)cKill, true, -1 );
		m_pBoard->SetElectionDSB( pDecl );

		// �Ǵٽ� ����
		Sleep( DELAY_KILL_AND_EFFECT );
	}
	e->SetEvent();
}
	// 7������ �缱�� ��� �� ����� �׿��� �Ѵ�
	// �� �Լ��� �� �� �ϳ��� ���̴� �Լ���,
	// OnKillOneFromSix�� ����.
	// 5�� �����ϸ� (�� ��� �˰����� �߸��Ǿ��ų�
	// ����� �� �� �����ϴ� ���) ���Ƿ� ������ 5�� ��
	// �ϳ��� �״´� !
void CPlayerHuman::OnKillOneFromSeven( CCard* pcCardToKill,
	CCardList* plcFailedCardsTillNow, CEvent* e )
{
	// ���̱� DSB �� ǥ��
	DKill* pDSB = new DKill(m_pBoard);
	*(CCard*)pcCardToKill =
		GetRecommendedKillCard(
			(CCardList*)plcFailedCardsTillNow );
	pDSB->Create( e, (CCard*)pcCardToKill,
		(CCardList*)plcFailedCardsTillNow, GetHand() );
}

// 7������ �ְ��� �ٸ� �÷��̾ ���δ�
// bKilled : ���̸� ������ �׿���, �����̸� ��ٸ� ¤����
void CPlayerHuman::OnKillOneFromSeven( CCard cKill,
	bool bKilled, CEvent* e )
{
	const CState* pState = m_pMFSM->GetState();

	// �ϴ� ��Ŀ�� �Ҹ��� ����
	PlaySound( IDW_JOKERCALL, true );

	// DSB �� ǥ��
	DDeadDecl* pDecl = new DDeadDecl( m_pBoard );
	pDecl->Create(
		pState->apAllPlayers[pState->nMaster]->GetName(),
		(int)cKill, false, -1 );
	m_pBoard->SetElectionDSB( pDecl );

	// ���� !!
	if ( !bKilled ) {

		// ��� ����
		Sleep( DELAY_KILL_AND_EFFECT );

		// �Ƿ� �Ҹ��� ����
		PlaySound( IDW_GIVEUP, true );

		// ���� DSB �� ǥ��
		DDeadDecl* pDecl = new DDeadDecl( m_pBoard );
		pDecl->Create(
			pState->apAllPlayers[pState->nMaster]->GetName(),
			(int)cKill, true, -1 );
		m_pBoard->SetElectionDSB( pDecl );

		// �Ǵٽ� ����
		Sleep( DELAY_KILL_AND_EFFECT );
	}
	e->SetEvent();
}

// ������ ��´�
// pNewGoal �� state.goal �� ��������� ������ ��ϵǾ� �ִ�
// �н��Ϸ��� nMinScore �� 0���� ��Ʈ
// Ư�� �� �̽��� ���ϴ� ��� ������ -1�� ��Ʈ
// pNewGoal.nFriend �� ������� �ʴ´�
void CPlayerHuman::OnElection( CGoal* pNewGoal, CEvent* e )
{
	DGoal* pDSB = new DGoal(m_pBoard);
	pDSB->Create( false, m_pMFSM->GetState(), e, pNewGoal );
}

// �ٸ� ����� ���� ����� ��´�
void CPlayerHuman::OnElecting( int nPlayerID, int nKiruda,
	int nMinScore, CEvent* e )
{
	m_goal.nKiruda = nKiruda;
	m_goal.nMinScore = nMinScore;
	m_goal.nFriend = 0;
	m_pBoard->FloatGoalDSB( nPlayerID, &m_goal );

	bool bDealMiss;
	long nNextID;

	m_pMFSM->CanBeEndOfElection( bDealMiss, nNextID, m_goal );

	if ( bDealMiss ) {	// ���̽�

		PlaySound( IDW_CARDSET );

		if ( nMinScore == -1 ) {	// ���� ���̽�
			m_pBoard->SetDealMiss(nPlayerID);
			m_pBoard->UpdatePlayer( nPlayerID, 0 );
		}
		else { // ��ü ���̽�
			DShortMessage* pSM = new DShortMessage(m_pBoard);
			pSM->Create( 0, _T("�� �̽� !"), true, true, DELAY_DEALMISS );
			m_pBoard->SetElectionDSB( pSM );
		}
		Sleep(DELAY_DEALMISS);
	}

	e->SetEvent();
}

// �ְ��� ��������
void CPlayerHuman::OnElected( int nPlayerID, CEvent* e )
{
	const CState* pState = GetState();

	Ib()->SetText( pState->apAllPlayers[nPlayerID]->GetName()
		+ _T(" ���� �缱�Ǿ����ϴ�") );

	// ����Ƽ, ��Ŀ���� �ٲ�������� �𸣱� ������
	// ���̴� ī�带 �����Ѵ�
	m_pBoard->UpdatePlayer( -2 );

	Sleep( DELAY_MASTERDECL );

	// §~ �Ҹ��� ����
	PlaySound( IDW_NOTIFY, true );

	// DSB �� �ϳ� ����

	DMasterDecl* pDecl = new DMasterDecl( m_pBoard );
	pDecl->Create(
		true,			// bTemp
		nPlayerID == 0,	// bHuman
		pState->pRule->nPlayerNum == 6,	// bToKill
		pState->pRule->bFriend,			// bUseFriend
		pState->pRule->nMinScore,		// nDefaultMinScore
		pState->goal,	// goal
		pState->apAllPlayers[nPlayerID]->GetName(),	// sMasterName
		0,	// sFriendName
		-1 );	// nTimeOut
	m_pBoard->SetElectionDSB( pDecl );
	// ������ ���ִ� ��ǥ ���ڴ� �����
	m_pBoard->FloatGoalDSB( -1, 0 );

	Ib()->SetKiruda( pState->goal.nKiruda );
	Ib()->SetMinScore( pState->goal.nMinScore, pState->pRule->nMinScore );
	Ib()->SetFriendText( _T("-����-") );

	Sleep( DELAY_MASTERDECL );

	e->SetEvent();
}

// �缱�� ���
// pNewGoal �� state.goal �� ��������� ������ ��ϵǾ� �ִ�
// pNewGoal �� ���ο� ������ �����ϰ� (nFriend����)
// acDrop �� ���� �� ���� ī�带 �����Ѵ�
// * ���� * �� �Լ� ���Ŀ� �Ѹ��� �׾ �÷��̾���
// ��ȣ(pState->nCurrentPlayer)�� ����Ǿ��� �� �ִ�
void CPlayerHuman::OnElected( CGoal* pNewGoal, CCard acDrop[3], CEvent* e )
{
	// ��ǥ ���� & ����ī�� ����
	DGoal* pGoalDSB = new DGoal(m_pBoard);
	pGoalDSB->Create( true, m_pMFSM->GetState(), &m_eTemp, pNewGoal );
	m_pBoard->SetElectionDSB( pGoalDSB );

	m_pMFSM->WaitEvent( &m_eTemp );

	// ���� ī�带 �˾ƿ´�
	int i, j;
	POSITION pos = GetHand()->GetHeadPosition();
	for ( i = j = 0; pos; i++ ) {
		CCard c = GetHand()->GetNext(pos);
		if ( m_pBoard->GetSelection(i) )
			acDrop[j++] = c;
	}
	ASSERT(j==3);

	// ȭ���� ������Ʈ ( �Ҹ��� �Բ� )
	PlaySound( IDW_CARDSET, true );
	m_pBoard->SetSelection( false );
	m_pMFSM->RemoveDroppedCards( acDrop, pNewGoal->nKiruda );
	m_pBoard->UpdatePlayer( -2 );
	Ib()->SetKiruda( pNewGoal->nKiruda );
	Ib()->SetMinScore( pNewGoal->nMinScore, GetState()->pRule->nMinScore );

	// �����带 ���Ѵ�
	if ( m_pMFSM->GetState()->pRule->bFriend ) {
		// ������ ������ ��������

		DFriend* pFriendDSB = new DFriend(m_pBoard);
		// ��õ ������� �Բ�
		pNewGoal->nFriend = GetRecommendedFriend( &GetState()->lDeck );
		pFriendDSB->Create( e, &pNewGoal->nFriend, GetState() );
	}
	else {	// ������ �������� �̺�Ʈ�� ��Ʈ
		pNewGoal->nFriend = 0;
		e->SetEvent();
	}
}

// ���Ű� ������ �� ������ ���۵Ǿ����� �˸���
// * ���� * �� �Լ� ���Ŀ� �Ѹ��� �׾ �÷��̾���
// ��ȣ(pState->nCurrentPlayer)�� ����Ǿ��� �� �ִ�
void CPlayerHuman::OnElectionEnd( CEvent* e )
{
	// ���� ����
	PlaySound( IDW_NOTIFY, true );
	m_pBoard->UpdatePlayer( -2 );
	DMasterDecl* pDecl = new DMasterDecl(m_pBoard);
	pDecl->Create( false,	// bTemp
		GetState()->nDeadID != 0 && GetState()->nMaster == 0,
		GetState()->pRule->nPlayerNum == 6,
		GetState()->pRule->bFriend,
		GetState()->pRule->nMinScore,
		GetState()->goal,
		GetState()->apPlayers[GetState()->nMaster]->GetName(),
		GetState()->nFriend >= 0 ?
			GetState()->apPlayers[GetState()->nFriend]->GetName()
			: _T(""),
		DELAY_ELECTIONEND );
	m_pBoard->SetElectionDSB( pDecl );

	// ���� �ٿ� ���� ������ ����Ѵ�

	Ib()->Reset();
	Ib()->SetKiruda( GetState()->goal.nKiruda );
	Ib()->SetMinScore( GetState()->goal.nMinScore, GetState()->pRule->nMinScore );

	int nFriend = GetState()->goal.nFriend;
	CString sFriend;
	if ( nFriend == 0 ) sFriend = _T("��");
	else if ( nFriend == 100 ) sFriend = _T("�ʱ�");
	else if ( nFriend < 0 ) sFriend = GetState()->apPlayers[GetState()->nFriend]->GetName();
	else sFriend = CCard(nFriend).GetString( Mo()->bUseTerm );
	sFriend += _T(" ������");
	Ib()->SetFriendText( sFriend );

	e->SetEvent();
}

// ī�带 ����
// pc �� �� ī��, pe �� pc�� ��Ŀ�ΰ�� ī�� ���,
// pc�� ��Ŀ���ΰ�� ���� ��Ŀ�� ����(0:��Ŀ�� �ƴ�)
// �� �ܿ��� ����
void CPlayerHuman::OnTurn( CCard* pc, int* pe, CEvent* e )
{
	*pe = 0;
	long nCard = int(*pc);

	do {
		m_pBoard->WaitSelection( &m_eTemp, &nCard );
		m_pMFSM->WaitEvent( &m_eTemp );

		*pc = CCard(nCard);

		m_pBoard->CancelSelection();

		// ��Ŀ�� ��Ŀ���̸� Ư���� ó���� �� �Ѵ�
	} while ( ProcessSpecialCards( *pc, pe ) );

	e->SetEvent();
}

// ī�带 ������ �˷��ش�
// ��� ������ State �� �ִ�
// nHandIndex �� �� ī�尡 �տ��� ī���� ���° �ε����� ī�忴�°�
void CPlayerHuman::OnTurn( CCard, int, int nHandIndex, CEvent* e )
{
	// ī�带 ������
	m_pBoard->FlyCard( GetState()->cCurrentCard, 0,
		2, GetState()->nCurrentPlayer,
			GetState()->nCurrentPlayer ? -1 : nHandIndex,
		0, -1, GetState()->nCurrentPlayer,
		&m_eTemp );

	// �÷��̾� ����
	m_pBoard->UpdatePlayer( GetState()->nCurrentPlayer );

	m_pMFSM->WaitEvent( &m_eTemp );

	// ������ ! �Ҹ��� ����Ѵ�
	PlayTurnSound();

	// �ٽ� ��� ������ ����
	m_pBoard->UpdatePlayer( -1 );

	e->SetEvent();
}

// ���� ����Ǳ� ����, �� ī�带 ȸ���ϴ� ������ �ܰ�
// cCurrentCard �� ���� ���ư��� ī��
void CPlayerHuman::OnTurnEnding( int nWinner, CEvent* e )
{
	// ���¹� ����
	if ( GetState()->apPlayers[nWinner]->GetHand()->GetCount() == 0 ) {
		m_pBoard->SetTurnRect( -1 );
		Ib()->SetText( _T("���� ����") );
	}
	else {
		m_pBoard->SetTurnRect( nWinner );
		Ib()->SetText( GetState()->apPlayers[nWinner]->GetName()
		+ _T(" ���� �̹� ���� �̰���ϴ�") );
	}

	CCard c(GetState()->cCurrentCard);

	if ( c.IsPoint() && GetState()->nMaster != nWinner
		&& ( GetState()->nFriend != nWinner
			|| !GetState()->bFriendRevealed ) ) {
		// ����ī���̰� �ߴ��̸� ���� ��������

		m_pBoard->FlyCard( (int)c, 2,
			0, -1,
			( GetState()->nBeginer + GetState()->lCurrent.GetCount() )
					% GetState()->nPlayers,
			1, nWinner, -1,
			&m_eTemp );

		m_pBoard->UpdatePlayer( -1 );

		m_pMFSM->WaitEvent( &m_eTemp );

		PlaySound( IDW_SCORE, true );
		m_pBoard->UpdatePlayer( nWinner, 0, false, true );
	}
	else {	// ���� ī�尡 �ƴ� ���

		m_pBoard->FlyCard( (int)c, 2,
			0, -1,
			( GetState()->nBeginer + GetState()->lCurrent.GetCount() )
					% GetState()->nPlayers,
			3, nWinner, -1,
			&m_eTemp );

		m_pBoard->UpdatePlayer( -1 );

		m_pMFSM->WaitEvent( &m_eTemp );
	}

	e->SetEvent();
}

// �� ���� �������� �˸���
// ���� state.plCurrent ���� �״�� �����ִ�
void CPlayerHuman::OnTurnEnd( CEvent* e )
{
	Sleep( Mo()->nDelayOneTurn * 200 );
	e->SetEvent();
}

// �����尡 ���� ����
void CPlayerHuman::OnFriendIsRevealed( int nPlayer, CEvent* e )
{
	// �����尡 ������������ ������

	// �̸� ������ �����ϰ� ������ ǥ��
	m_pBoard->UpdatePlayer( -2 );
	if ( nPlayer >= 0 ) {

		(new DShortMessage(m_pBoard))->Create(
			0, _T("������"), true, false, DELAY_FRIENDCARD, nPlayer );
	}
	else {

		(new DShortMessage(m_pBoard))->Create(
			0, _T("�ְ��� �ʱ��� �Ǿ ��������� �����մϴ�"), true, false, DELAY_FRIENDCARD,
			GetState()->nMaster );
	}
	Ib()->SetText(_T("�����尡 ���� �����ϴ�"));

	e->SetEvent();
}

// ���� ���� ( *pbCont �� false �̸� ���� ���� )
void CPlayerHuman::OnEnd( bool* pbCont, CEvent* e )
{
	Sb()->Update(GetState());

	// ����Ʈ ��ȭ���ڸ� ǥ���Ѵ�
	(new DReport(m_pBoard))->Create( m_pMFSM, pbCont, e );
}

// nPlayer �� ���� ���� ( ������ �����ɸ��� �۾� ���� )
// nMode : 0 ���� ������  1 Ư�Ǹ�� ó����  2 �� ī�� ������
void CPlayerHuman::OnBeginThink( int nPlayer, int nMode, CEvent* e )
{
	LPCTSTR asMsg[] = { _T(" ���� ���� ���� ��"),
		_T(" ���� ���� �������ϰ� �ֽ��ϴ�"), _T(" �� �����Դϴ�") };
	m_pBoard->SetTurnRect( nPlayer );
	Ib()->SetText( m_pMFSM->GetState()->apPlayers[nPlayer]->GetName()
					+ asMsg[nMode] );
	e->SetEvent();
}

// ���α׷� ����
// �� �Լ��� �ϳ��� �÷��̾ disconnect �� ��,
// �Ǵ� OnEnd ���� �Ѹ��� �÷��̾�� false ��
// �������� �� ȣ��ȴ� - Human �� ��� ������ �����Ѵ�
// sReason : ����Ǵ� ���� (0 �̸� ��� ����)
void CPlayerHuman::OnTerminate( LPCTSTR sReason )
{
	if ( sReason && *sReason ) {
		// ���� ������ ǥ���ϴ� ��ȭ ���ڸ� ���
		DMessageBox* pBox = new DMessageBox(m_pBoard);
		pBox->SetAction( &m_eTemp );
		pBox->SetModal();
		pBox->Create( true, 1, &sReason );
		if ( Mo()->bUseSound ) ::MessageBeep( MB_ICONEXCLAMATION );

		m_pMFSM->WaitEvent( &m_eTemp );
	}

	m_pBoard->SetMFSM(0);
}

// ä�� �޽��� (bSource : ä��â�� �ҽ�)
void CPlayerHuman::OnChat( int nPlayerID, LPCTSTR sMsg, bool bSource )
{
	bSource;	// unused
	m_pBoard->FloatSayDSB( nPlayerID, sMsg );
}
