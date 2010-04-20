// State.cpp: implementation of the CState class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mighty.h"

#include "State.h"
#include "Card.h"
#include "Play.h"
#include "Player.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CState::CState() :
	pRule(new CRule),
	lDeck(*(new CCardList)),
	lCurrent(*(new CCardList)),
	lDead(*(new CCardList))
{}

CState::~CState()
{
	CCard::SetState(0);

	delete pRule;
	delete &lDeck;
	delete &lCurrent;
	delete &lDead;
}

// �� �� ������ �����͵��� �ʱ�ȭ �Ѵ� (������ ��)
void CState::InitStageData( int _nGameNum, int _nBeginer )
{
	CCard::SetState( this );

	state = msReady;

	nGameNum = _nGameNum;

	// �� ����
	lDeck.RemoveAll();
	lDeck.NewDeck( pRule->nPlayerNum,
		CCard(pRule->nJokercall), CCard(pRule->nAlterJokercall) );

	for ( int s = 0; s < 4; s++ )
		lDeck.Suffle();

	lCurrent.RemoveAll();

	memset( aanHistory, 0, sizeof( aanHistory ) );

	// ���� �� ���� ���� �ʾ����Ƿ� ��� �÷��̾ ���ӿ� ����
	nPlayers = pRule->nPlayerNum;
	for ( int i = 0; i < pRule->nPlayerNum; i++ ) {
		apAllPlayers[i]->Reset();
		apPlayers[i] = apAllPlayers[i];
		apPlayers[i]->SetPlayerNum(i);
		abGiveup[i] = false;
	}

	lDead.RemoveAll();

	nMaster = -1;
	nFriend = -1;
	nDeadID[0] = -1;
	nDeadID[1] = -1;

	bFriendRevealed = false;

	goal.nKiruda = 0;
	goal.nMinScore = 0;
	goal.nFriend = 0;
	acDrop[0] = acDrop[1] = acDrop[2] = 0;
	nThrownPoints = 0;

	bJokerUsed = false;

	for ( int u = 0; u < 4; u++ )
		anUsedCards[u] = anUsingCards[u] = 0;

	nTurn = -1;
	nBeginer = _nBeginer;
	bJokercallEffect = false;
	nJokerShape = 0;

	cCurrentCard = 0;
	nCurrentPlayer = nBeginer;
}

// ������ �� (lCurrent �� ������� ��) ���� ȣ���Ͽ� �����丮 ������ �����Ѵ�
void CState::WriteHistory()
{
	int* anScored = aanHistory[0];
	int* anAssist = aanHistory[1];
	int* anLost = aanHistory[2];
	int* anShare = aanHistory[3];

	// �ٴ� ī�� ����� �÷��̾� ���� ���ƾ� �Ѵ�
	ASSERT( lCurrent.GetCount() == nPlayers );

	int i;

	// ������ ( ���� ���� �� ���� (%) )
	anShare[nBeginer] += 10;

	// ���� �÷��̾� ��ȣ�� ������ ���� ����
	int nWinner; bool bWinnerDef;
	nWinner = GetWinner( &lCurrent, &bWinnerDef );

	// �� �÷��̾ ���� ������ �����Ѵ�
	POSITION posCurrent = lCurrent.GetHeadPosition();
	for ( i = 0; i < nPlayers; i++ ) {

		// ���� �÷��̾��� �ε�����? ���� ���̾��°��� �����ȴ�
		int nPlayer = ( nBeginer + i ) % nPlayers;
		// �� �÷��̾ �� ī��
		CCard cCard = lCurrent.GetNext(posCurrent);

		// �� �÷��̾��� ���ҿ� ����
		// ���� : ���� ( ���� ���� ī�� �� )
		// ���ڿ� ���� �� : ���� ( ���� ���� ��� �� ���� )
		// ���ڿ� �ٸ� �� : ���� ( �ٸ� ���� ��� �� ���� )
		// �� �����Ѵ�

		if ( nPlayer == nWinner ) {
			// ����
			if ( nPlayers == 2 ) 
				anScored[nPlayer]++;
			else 
				anScored[nPlayer] += lCurrent.GetPoint();
		}
		else if ( bWinnerDef && ( nPlayer == nMaster
								|| nPlayer == nFriend )
				|| !bWinnerDef && ( nPlayer != nMaster
								&& nPlayer != nFriend ) )
			// ���ڿ� ���� ��
			anAssist[nPlayer] += cCard.IsPoint() ? 1 : 0;
		else // ���ڿ� �ٸ� ��
			anLost[nPlayer] += cCard.IsPoint() ? 1 : 0;
	}
}

// ���� goal �� ���ؼ�, ������ ���ٿ�
// �ּ� ������ ������ �����Ѱ��� �����Ѵ�
// nNewMinScore �� 0 �̸� ���� ���� ���θ�, -1 �̸�
// DealMiss ���� ���θ� ���´�
bool CState::IsValidNewGoal( int nNewKiruda, int nNewMinScore ) const
{
	// �� ��Ȳ�� �������ΰ�, Ư�� ��Ȳ�ΰ��� �Ǵ��Ѵ�
	bool bInElection = state == msElection ? true : false;
	bool bBlindKirudaChange =
		state == msPrivilege
		&& apPlayers[nCurrentPlayer]->GetHand()->GetCount() < 13;

	ASSERT( state == msElection || state == msPrivilege );

	// �� �̽� ���θ� �Ǵ�
	if ( nNewMinScore == -1 )
		// Ư�� ��Ȳ�̶�� �� �̽� �Ұ�
		if ( !bInElection )	return false;
		else if ( IsDealMiss( apPlayers[nCurrentPlayer]->GetHand() ) )
			return true;
		else return false;

	// ���� ���� ���θ� �Ǵ�
	if ( nNewMinScore == 0 ) {
		// Ư�� ��Ȳ�̶�� ���� �Ұ�
		if ( !bInElection )	return false;
		// ���̰�, ���� ������ �ȸ������ �����̸�, ����
		// ���⸶�� �볳���� �ʴ� ���
		else if ( goal.nMinScore == 0 && nCurrentPlayer == nBeginer
				&& !pRule->bBeginerPass ) return false;
		// �� �ܿ��� ������ �� ����
		else return true;
	}

	// �ּ����� ���� �˻�
	int nMinLimit;
	if ( pRule->bNoKirudaAdvantage && nNewKiruda == 0 )
		nMinLimit = pRule->nMinScore-1;
	else nMinLimit = pRule->nMinScore;
	if ( nNewMinScore < nMinLimit ) return false;

	// �ִ����� ���� �˻�
	int nMaxLimit;
	if ( pRule->nPlayerNum == 2 ) {
		if ( pRule->nMinScore > MAX_SCORE_2MA )
			nMaxLimit = pRule->nMinScore;
		else
			nMaxLimit = MAX_SCORE_2MA;
	}
	else {
		if ( pRule->bHighScore ) nMaxLimit = HIGHSCORE_MAXLIMIT;
		else nMaxLimit = MAX_SCORE;
	}
	if ( nMaxLimit < nNewMinScore ) return false;

	// ���� �⸶�� ���� ���� �˻�
	if ( bInElection ) {
		int temp = goal.nMinScore;
		if ( pRule->bNoKirudaAlways ) {		// ���� �׻� 1 ���� �θ��� ��Ģ�� ��
			if ( nNewKiruda == 0 ) temp--;					// �� ������ ����� ��� temp�� 1 ���ҽ�Ŵ.
			if ( goal.nKiruda == 0 && temp < 19 ) temp++;	// ���� ������ ����̸� ����� ��� temp�� 1 ������Ŵ. ��, ��� 19�� ��� ��� 20 �θ� �� ����
		}
		if ( nNewMinScore <= temp ) return false;
	}

	// ���� �߿��� ���̻��� ������ ����
	if ( bInElection ) return true;

	// ���ٸ� �ٲٴ� ���
	if ( goal.nKiruda != nNewKiruda ) {

		// �÷��� �ϴ� ����
		int nMinDiff;
		if ( !pRule->bRaise2ForKirudaChange
			|| bBlindKirudaChange )
			nMinDiff = 1;
		else nMinDiff = 2;
		if ( pRule->bNoKirudaAdvantage && ( nNewKiruda == 0 ) ) nMinDiff--;

		if ( min( goal.nMinScore + nMinDiff, nMaxLimit )
			<= min( nNewMinScore, nMaxLimit ) )
			return true;
		else return false;
	}

	if ( goal.nMinScore <= nNewMinScore )
		return true;
	else return false;
}

// ���ο� ������ �ĺ� ���� �ָ� ������ ��� �� ���� �״��,
// �ƴϸ� 0(��������)�� �����Ѵ�
int CState::GetValidFriend( int nFriend ) const
{
	// �ƿ� �������� ������ ���� ���
	if ( !pRule->bFriend ) return 0;

	// ��������, �ʱ� ������
	if ( nFriend == 0 || nFriend == 100 ) return nFriend;

	// �÷��̾�
	if ( nFriend < 0 ) {
		if ( -nFriend-1 < nPlayers
			&& -nFriend-1 != nMaster ) return nFriend;
		else { ASSERT(0); return 0; }
	}

	// ī��
	if ( !( 0 < nFriend && nFriend < 54 ) ) { ASSERT(0); return 0; }
	CCard c(nFriend);

	// ��Ŀ������ �Ұ�
	if ( c.IsJoker() && !pRule->bJokerFriend ) { ASSERT(0); return 0; }

	if ( apPlayers[nMaster]->GetHand()->Find(c)
		|| lDeck.Find(c) ) return 0;
	else return nFriend;
}

// �־��� ī���а� ���̽��ΰ� �����Ѵ�
bool CState::IsDealMiss( const CCardList* pl ) const
{
	bool bHasMighty = pl->Find( CCard::GetMighty() ) ? true : false;
	bool bHasJoker = pl->Find( CCard::GetJoker() ) ? true : false;
	int nPoints = pl->GetPoint();

	// 10 �� ��
	if ( pRule->bDM_Only10 && nPoints == 1 ) {
		POSITION pos10 = pl->GetHeadPosition();
		while (pos10) if ( pl->GetNext(pos10).GetNum() == 10 ) return true;
	}

	// �ֲٴ� J �� ��
	if ( pRule->bDM_OneEyedJack && nPoints == 1 ) {
		POSITION posJ = pl->GetHeadPosition();
		while (posJ) if ( pl->GetNext(posJ).IsOneEyedJack() ) return true;
	}

	// ��Ŀ -1��
	if ( pRule->bDM_JokerIsReversePoint && bHasJoker ) nPoints--;

	// ����Ƽ 0��
	if ( pRule->bDM_OnlyMighty && bHasMighty ) nPoints--;

	if ( pRule->bDM_Duplicate ) {
		// 10 �� ��
		if ( pRule->bDM_Only10 && nPoints == 1 ) {
			POSITION pos10 = pl->GetHeadPosition();
			while (pos10) if ( pl->GetNext(pos10).GetNum() == 10 ) return true;
		}
		// �ֲٴ� J �� ��
		if ( pRule->bDM_OneEyedJack && nPoints == 1 ) {
			POSITION posJ = pl->GetHeadPosition();
			while (posJ) if ( pl->GetNext(posJ).IsOneEyedJack() ) return true;
		}
	}

	// ���� �������� ���� ī���� ���� ����� ����

	// ����ī�尡 ���� �� ���̽�
	if ( pRule->bDM_NoPoint) {
		if ( pRule->bDM_OnlyOne && nPoints <= 1 ) return true;
		if ( nPoints <= 0 ) return true;
	}

	// ��� ����ī���϶� ���̽�
	if ( pRule->bDM_AllPoint && nPoints >= pl->GetCount() ) return true;

	// �� �ܿ��� ���̽��� �ƴ�
	return false;
}

// �־��� ī�� ����Ʈ�� ���� ���� �÷��̾� ��ȣ�� �˾Ƴ���
// ���� ���� ���� state ������ �����Ѵ�
int CState::GetWinner( const CCardList* pList,
	bool* pbWinnerIsDef,
	int _nJokerShape, int _bJokercallEffect,
	int _nPlayers, int _nTurn,
	int _nBeginer, int _nMaster, int _nFriend ) const
{
	POSITION posWin = pList->GetWinner(
		_nTurn == -1 ? nTurn : _nTurn,
		_nJokerShape == -1 ? nJokerShape : _nJokerShape,
		_bJokercallEffect == -1 ? bJokercallEffect : (_bJokercallEffect?true:false) );
	int nRet = ( (_nBeginer == -1 ? nBeginer : _nBeginer)
				+ pList->IndexFromPOSITION( posWin ) )
		% ( _nPlayers == -1 ? nPlayers : _nPlayers );

	if ( pbWinnerIsDef )
		*pbWinnerIsDef = 
			nRet == ( _nMaster == -1 ? nMaster : _nMaster )
			|| nRet == ( _nFriend == -1 ? nFriend : _nFriend );

	return nRet;
}

// �ش� �÷��̾ �����ΰ��� �Ǵ��Ѵ�
bool CState::IsDefender( int nPlayer ) const
{
	return nPlayer == nMaster || nPlayer == nFriend;
}

// ���� ��Ŀ���� ��Ŀ�� ȿ���� �� �� �ִ����� �˻��Ѵ�
bool CState::IsEffectiveJokercall() const
{
	if ( nTurn == 0 && !pRule->bInitJokercallEffect ) return false;
	if ( bJokerUsed ) return false;
	if ( lCurrent.GetCount() > 0 ) return false;
	return true;
}

// �� ������ ��Ʈ��ũ�� ������ �����ΰ�
bool CState::IsNetworkGame() const
{
	for ( int i = 0; i < nPlayers; i++ )
		if ( apPlayers[i]->IsNetwork() ) return true;
	return false;
}
