// MFSM.cpp: implementation of the CMFSM class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mighty.h"

#include "Play.h"
#include "Player.h"
#include "MFSM.h"

#include "SocketBag.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CMFSM::CMFSM( LPCTSTR sRule, CPlayer* _apPlayers[], CSocketBag* pSockBag )
{
	int i;

	m_uid = -1;

	m_bTerminate = false;
	m_bTerminated = true;
	m_bTermRequest = false;

	m_pSockBag = pSockBag;

	m_pServerThread = 0;

	m_bAIThreadTerminate = false;
	m_bAIThreadTerminated = true;
	m_nAIThreadRequest = -1;
	m_pAIThread = 0;

	pRule->Decode( sRule );
	ASSERT( pRule->nPlayerNum <= MAX_PLAYERS );
	nPlayers = pRule->nPlayerNum;

	for ( i = 0; i < MAX_PLAYERS; i++ )
		m_apePlayer[i] = 0;

	for ( i = 0; i < pRule->nPlayerNum; i++ )
		m_apePlayer[i] = new CEvent( FALSE, TRUE );	// manual event
	ResetEvents();

	for ( i = 0; i < pRule->nPlayerNum; i++ ) {
		apPlayers[i] = apAllPlayers[i] = _apPlayers[i];
		apPlayers[i]->SetCurrentMFSM( this );
		apPlayers[i]->Reset();
	}
	for ( i = pRule->nPlayerNum; i < MAX_PLAYERS; i++ )
		apPlayers[i] = apAllPlayers[i] = 0;

	InitStageData( 0 );
}

CMFSM::~CMFSM()
{
	// �����带 �����Ѵ�
	m_bTerminate = true;
	m_eNotify.SetEvent();

	while ( !m_bTerminated ) Sleep(100);
	TRACE("MFSM Service Terminated\n");

	// AI ������ ����
	m_bAIThreadTerminate = true;
	m_eAIThread.SetEvent();

	while ( !m_bAIThreadTerminated ) Sleep(100);
	TRACE("MFSM AIThread Terminated\n");

	// �÷��̾� �ڵ� ����
	for ( int i = 0; i < MAX_PLAYERS; i++ )
		delete apAllPlayers[i];

	for ( int j = 0; j < pRule->nPlayerNum; j++ )
		delete m_apePlayer[j];

	POSITION pos = m_lChatMsg.GetHeadPosition();
	while (pos) delete m_lChatMsg.GetNext(pos).pMsg;

	delete m_pSockBag;
}


// �÷��̾ ǥ���ϴ� ID, UID, Num ���� ��ȯ�� �Ѵ�
// �ش� �÷��̾ �������� �ʴ� ��� -1 �� �����Ѵ� (Num �� �����ϴ� �Լ��� ���)
long CMFSM::GetPlayerUIDFromID( long id ) { return id < 0 ? -1 : (id+m_uid)%pRule->nPlayerNum; }
long CMFSM::GetPlayerIDFromUID( long uid ) { return uid < 0 ? -1 : (uid-m_uid+pRule->nPlayerNum)%pRule->nPlayerNum; }
long CMFSM::GetPlayerIDFromNum( long num ) { return num < 0 || num >= nPlayers ? -1 : apPlayers[num]->GetID(); }
long CMFSM::GetPlayerNumFromID( long id ) { for ( int i = 0; i < nPlayers; i++ ) if ( apPlayers[i]->GetID() == id ) return i; return -1; }
long CMFSM::GetPlayerNumFromUID( long uid ) { return GetPlayerNumFromID( GetPlayerIDFromUID(uid) ); }
long CMFSM::GetPlayerUIDFromNum( long num ) { return GetPlayerUIDFromID( GetPlayerIDFromNum(num) ); }

// ���� �÷��̾ ��������, apPlayers[] �� ��迭�Ѵ�
void CMFSM::RebuildPlayerArray()
{
	ASSERT( pRule->nPlayerNum >= 6 && nDeadID[0] != -1 && nDeadID[pRule->nPlayerNum-6] != -1 );

	for ( int i = 0, j = 0; i < pRule->nPlayerNum; i++ )
		if ( nDeadID[0] != i && nDeadID[pRule->nPlayerNum-6] != i ) {
			apPlayers[j] = apAllPlayers[i];
			apPlayers[j]->SetPlayerNum(j);
			// �ٸ� ������ �������Ѵ�
			if ( nMaster == i ) nMaster = j;
			if ( nBeginer == i ) nBeginer = j;
			if ( nFriend == i ) nFriend = j;
			if ( nCurrentPlayer == i ) nCurrentPlayer = j;
			j++;
		}
		else apAllPlayers[i]->SetPlayerNum(-1);

	ASSERT( nPlayers >= 6 );

	apPlayers[5] = 0;
	apPlayers[6] = 0;
	nPlayers = 5;	// 5 ���̶�� Ȯ���� ����
}

// ��� �÷��̾��� ī�带 �ɼǿ� ���� �����Ѵ�
void CMFSM::SortPlayerHand( bool bLeftKiruda, bool bLeftAce )
{
	if ( Mo()->bNoSort ) return;

	// ī�带 �����Ѵ�
	for ( int i = 0; i < pRule->nPlayerNum; i++ )
		apAllPlayers[i]->GetHand()->Sort(
			bLeftKiruda, bLeftAce );
}

// �÷��̾��� �̸����� �缳���Ѵ�
void CMFSM::SetPlayerNames( LPCTSTR asName[] )
{
	for ( int i = 0; i < pRule->nPlayerNum; i++ )
		apAllPlayers[i]->SetName( asName[i] );
}

// �� ī��(pCard)�� �� �÷��̾�(nPlayerID)��
// �ٸ� �÷��̾ ���� �� �ִ°�
// �׷� �� �ִٸ� true, ���� �� ���� �ٸ� ī�带 ã�ƾ�
// �Ѵٸ�(��ٸ�) false �� �����Ѵ�
bool CMFSM::KillTest( int nPlayerID, CCard* pCard, int* pDeadID )
{
	bool bKilled = false;
	for ( int i = 0; i < pRule->nPlayerNum; i++ ) {
		if ( i == nPlayerID ) continue;
		else if ( apAllPlayers[i]->GetHand()->Find( *pCard ) ) {
			bKilled = true;
			*pDeadID = i;
			break;
		}
	}
	return bKilled;
}

// (������) �־��� goalNew �� ���Ÿ� �����ų ��
// �ִ°��� �Ǵ��Ѵ� - ������ goal, abGiveUp, IsDealMiss() ��
// ���������� ����Ѵ�
// ���ϰ��� - bDealMiss : �� �̽��� ����Ǿ��°��� �Ǵ�
//            nNextID : �缱�� ID �Ǵ� ���� �⸶�� ID
// ���Ÿ� ��� �ؾ� �ϴ���(false), �� �̽��� �缱 Ȯ������
// ����Ǵ���(true)�� �����Ѵ�
bool CMFSM::CanBeEndOfElection( bool& bDealMiss, long& nNextID,
	const CGoal& goalNew )
{
	bDealMiss = false;
	nNextID = -1;

	if ( goalNew.nMinScore == -1 ) {	// ���̽�
		bDealMiss = true;
		return true;
	}
	else if ( goalNew.nMinScore == 0 ) {	// ����
		// ���� �÷��̾ ã�� ����
		long i, nID = -1;
		for ( i = 1; i < pRule->nPlayerNum; i++ ) {
			nID = ( nCurrentPlayer + i ) % pRule->nPlayerNum;
			if ( !abGiveup[nID] ) break;
		}
		if ( i == pRule->nPlayerNum ) {	// ��� ���� (���̽�)
			bDealMiss = true;
			return true;
		}
		else {	// nID �� �������� ���� ���� �÷��̾�
			nNextID = nID;
			// �� �÷��̾� ���� ��� �׾����� ����
			for ( i = i+1; i < pRule->nPlayerNum; i++ ) {
				nID = ( nCurrentPlayer + i ) % pRule->nPlayerNum;
				if ( !abGiveup[nID] ) return false;
			}
			// ��� �׾���
			// �� ���� �������� ���� ���൵ ������ ���¶��
			if ( goal.nMinScore == 0 ) return false;
			// �� �ܿ��� ����
			else return true;
		}
	}
	else {		// �⸶
		ASSERT( goalNew.nMinScore > goal.nMinScore
			|| ( pRule->bNoKirudaAlways && goalNew.nMinScore >= goal.nMinScore ));

		// Ǯ�� �÷ȴٸ� ��� �缱
		if ( pRule->bHighScore && goalNew.nMinScore >= HIGHSCORE_MAXLIMIT
					|| goalNew.nMinScore >= MAX_SCORE 
			|| pRule->nPlayerNum == 2 && goalNew.nMinScore >= MAX_SCORE_2MA ) {
			nNextID = nCurrentPlayer;
			return true;
		}

		// ���� �÷��̾ ã�� ����
		long i, nID = -1;
		for ( i = 1; i < pRule->nPlayerNum; i++ ) {
			nID = ( nCurrentPlayer + i ) % pRule->nPlayerNum;
			if ( !abGiveup[nID] ) break;
		}
		if ( i == pRule->nPlayerNum ) {	// ��� ���� (�缱)
			nNextID = nCurrentPlayer;
			return true;
		}
		else {	// nID �� ���� �÷��̾�
			nNextID = nID;
			return false;
		}
	}
}

// �� ī�忡 ���� ���� ��Ʈ���� �����Ѵ�
CString CMFSM::GetHelpString( CCard c ) const
{
	static const LPCTSTR s_aNumber[] = {
		_T("2 "), _T("3 "), _T("4 "), _T("5 "),
		_T("6 "), _T("7 "), _T("8 "), _T("9 "), _T("10 "),
		_T("J "), _T("Q "), _T("K "), _T("A ") };
	static const LPCTSTR s_aHighest[] = {
		_T(" (����ī)"), _T(" (�ְ���)") };

	if ( (int)c == 0 ) return CString();

	bool b = Mo()->bUseTerm;
	int t = b ? 0 : 1;
	CString s = c.GetString(b);

	// Ư�� ī���
	if ( c.IsMighty() || c.IsJoker() ) return s;

	// �� ��翡�� �� ī�庸�� ���� ī���� ���� ���� (����Ƽ ����)
	CString sHigh;
	int nHighers = 0;
	for ( int i = 12; i > c.GetPowerNum()-1; i-- ) {
		long flag = anUsedCards[c.GetShape()-1];
					//| anUsingCards[c.GetShape()-1];
		if ( !( flag & ( 1 << i ) )
				&& !CCard(c.GetShape(),i==12?ACE:i+2).IsMighty() ) {
			sHigh += s_aNumber[i];
			nHighers++;
		}
	}

	if ( nHighers > 0 ) return s + _T(" ( ") + sHigh + _T(')');
	else return s + s_aHighest[t];
}

// �� �Լ��� ���α׷� ���谡 �߸��Ǿ� �ʿ��� ���� �Լ���
// �÷��̾ ȣ���ϸ� �ְ��� ī�� �� acDrop �� �ش��ϴ�
// �� ���� ī�带 ������ �߰� ���� �����
// �����Ѵ� - MFSM �� ������ ȣ���� ������
// CPlayerHuman �� ��� ���� ī�� ���ð� ������ ���� ���̿�
// ȭ�� Update �� �ʿ��ϱ� ������ �� �Լ��� �ʿ��ϴ�
void CMFSM::RemoveDroppedCards( CCard acDrop[3], int nNewKiruda )
{
	// ���Ѹ�ŭ Ȯ���� �����Ѵ�
	ASSERT( state == msPrivilege );
	ASSERT( apPlayers[nMaster]->GetHand()->GetCount() == 13 || (nPlayers == 2 && apPlayers[nMaster]->GetHand()->GetCount() == 14 ) );	//v4.0
	ASSERT( lDeck.GetCount() == 0 );

	CCardList* pHand = apPlayers[nMaster]->GetHand();

	for ( int i = 0; i < ( nPlayers == 2 ? 1 : 3 ); i++ ) {

		POSITION pos = pHand->Find(acDrop[i]);
		ASSERT(pos);
		if ( pos ) {
			lDeck.AddTail(acDrop[i]);
			pHand->RemoveAt( pos );
		}
	}
	nThrownPoints = lDeck.GetPoint();

	goal.nKiruda = nNewKiruda;
	SortPlayerHand( Mo()->bLeftKiruda, Mo()->bLeftAce );
}

// ��Ŀ���� ������ ��Ȳ���� �����Ͽ� �׷��ٸ� �־���
// ���ڿ� ���� bIsJokercall �� ��Ʈ�Ѵ�
// �� ��Ŀ�� ��� nJokerShape �� ��Ʈ�Ѵ�
void CMFSM::SetJokerState( CCard c, int nEffect )
{
	if ( c.IsJokercall() ) {

		if ( IsEffectiveJokercall() )
			bJokercallEffect = nEffect ? true : false;
	}
	else if ( c.IsJoker() ) {

		bJokerUsed = true;

		if ( nCurrentPlayer != nBeginer ) return;
		nJokerShape = SPADE <= nEffect && nEffect <= CLOVER
			? nEffect : SPADE;
	}
}

// ���� ����ÿ� �������� �� ��� ����Ʈ�� ��´�
// �迭 ���ϰ��� �迭 ũ��� �÷��̸� �� �ο� ( �ִ� 5 )
void CMFSM::GetReport(
	// ���ϰ�
	bool& bDefWin,			// ���(����)���� �¸��ߴ°� ?
	int* pnDefPointed,		// ���� ����
	int anContrib[],		// ���嵵 (�� ���� �� �ۼ�Ʈ ������ Ȱ���� �ߴ°�)
	CString asCalcMethod[2],// [0] ��� �����, �⺻ �׼��� ��� �� [1] ��Ÿ ������Ģ
	CString asExpr[],		// ���� ��� ��
	// ��-���-��������
	int anMoney[]			// ȣ��ÿ��� ���� ������ ��, ���ϵ� ���� ���� ��
) const
{
#ifdef _DEBUG	// �� ���� ���� �����Ѵ�
	int nAllDebugSum = 0;
	for ( int debug = 0; debug < nPlayers; debug++ )
		nAllDebugSum += anMoney[debug];
#endif

	const int* anScored = aanHistory[0];
	const int* anAssist = aanHistory[1];
	const int* anTurn = aanHistory[3];

	int i;

	int nDefSum = 0, nAttSum = 0;			// ����, ������ ��
	int nDefPointed = 0, nAttPointed = 0;	// �������� ��

	// ���嵵�� ����ϱ� ���ؼ� ������ ����, ������ �հ�
	// �ߴ��� ����, ������ ���� ����� �д�
	for ( i = 0; i < nPlayers; i++ ) {
		if ( i == nFriend || i == nMaster )
			nDefSum += anScored[i] + anAssist[i],
			nDefPointed += anScored[i];
		else nAttSum += anScored[i] + anAssist[i],
			nAttPointed += anScored[i];
	}

	// ������ ī���� ������ �������� �ִ°� ó��
	if ( pRule->bAttScoreThrownPoints )
		nAttPointed += nThrownPoints;
	else if ( pRule->nPlayerNum != 2 ) nDefPointed += nThrownPoints; //2�������� �������� �ʴ´� (v4.0 : 2010.4.18)

	// ���嵵 ���
	// �� ���� �� ��� ������ ��� ������ �� �߿���
	// �� �÷��̾ �� ������ ������ ��
	for ( i = 0; i < nPlayers; i++ ) {

		if ( i == nFriend || i == nMaster ) {
			if ( anContrib )
			anContrib[i] = nDefSum == 0 ? 0
				: ( anScored[i] + anAssist[i] ) * 100 / nDefSum;
		}
		else {
			if ( anContrib )
			anContrib[i] = nAttSum == 0 ? 0
				: ( anScored[i] + anAssist[i] ) * 100 / nAttSum;
		}
	}

	// ������ ������ ����
	if ( pnDefPointed ) *pnDefPointed = nDefPointed;

	// ������ �� �� ���θ� ����
	// ������ ��ǥ �������� ������ ��
	if ( pRule->nPlayerNum == 2 ) bDefWin = nDefPointed >= min( goal.nMinScore, MAX_SCORE_2MA );
	else bDefWin = nDefPointed >= min( goal.nMinScore, MAX_SCORE );

	/////////////////////////////////////////////////////////////////////////
	// ������ ����Ѵ�

	// ��(ǥ����), ��(������), (����)��(��)
	int m = goal.nMinScore,
		b = pRule->nMinScore,
		s = nDefPointed;

	if ( goal.nKiruda==0 &&
		( ( pRule->bNoKirudaAdvantage && m == b ) || pRule->bNoKirudaAlways ) )
		b--;	// ����ٷ� 1 ���� �θ� �� �ִ� ��� �⺻�� 1 ���ҽ�Ŵ

	CString& sCalcMethod = asCalcMethod[0];
	CString& sResult = asCalcMethod[1];

	// ���� �̵��� ���� ���Ѵ�
	int nMoved;		// �̵��� �⺻�� (���� �ƴ�)
	{
		//������ �¸��� ���
		if( bDefWin ) {
			// ���躸�� 2.0 ( (��-��)*2+(��-��) )
			if ( pRule->bS_Use20 ) {
				sCalcMethod.Format( _T("((%d-%d)x2+(%d-%d))"), m,b,s,m );
				nMoved = (m-b)*2+(s-m);
			}
			// ���躸�� 4.0 ( (��-��)*1.5+(��-��) )
			else if ( pRule->bS_Use40 ) {
				sCalcMethod.Format( _T("((%d-%d)x1.5+(%d-%d))"), m,b,s,m );
				nMoved = (m-b)*3/2+(s-m);
			}
			// �θ��°� �� ( (��-��+1)*2 )
			else if ( pRule->bS_Call ) {
				sCalcMethod.Format( _T("((%d-%d+1)x2"), m,b );
				nMoved = (m-b+1)*2;
			}
			// Eye for an Eye ( (��-��) )
			else if ( pRule->bS_Efe ) {
				sCalcMethod.Format( _T("(%d-%d)"), s,m );
				nMoved = (s-m);
			}
			// Modified Eye for an Eye ( (��-��)+1 )
			else if ( pRule->bS_MEfe ) {
				sCalcMethod.Format( _T("(%d-%d+1)"), s,m );
				nMoved = (s-m)+1;
			}
			// Base Ten ( (��-10) )
			else if ( pRule->bS_Base10 ) {
				sCalcMethod.Format( _T("(%d-10)"), s );
				nMoved = (s-10);
			}
			// Base Thirteen ( (��-13) )
			else if ( pRule->bS_Base13 ) {
				sCalcMethod.Format( _T("(%d-13)"), s );
				nMoved = (s-13);
			}
			// Base Min ( (��-��) )
			else if ( pRule->bS_BaseM ) {
				sCalcMethod.Format( _T("(%d-%d)"), s,b );
				nMoved = (s-b);
			}
			else ASSERT(0);
		}
		else {			// ���� ��
			// Eye for an Eye ( (��-��) )
			if ( pRule->bSS_Efe ) {
				sCalcMethod.Format( _T("(%d-%d)"), m,s );
				nMoved = (m-s);
			}
			// Tooth for an Tooth ( (��-��) or (��-13)+(13-��)*2 )
			else if ( pRule->bSS_Tft ) {
				if ( s >= b ) {
					sCalcMethod.Format( _T("(%d-%d)"), m,s );
					nMoved = (m-s);
				}
				else {
					sCalcMethod.Format( _T("(%d-%d)+(%d-%d)x2"), m,b,b,s );
					nMoved = (m-b+(b-s)*2);
				}
			}
			else ASSERT(0);
		}

		// 2�� ��Ģ
		if ( nMoved < 0 ) nMoved = 0;				// �̱� ��� �ּ��� 0���� ȹ���ϰ� ��

		if ( bDefWin && nDefPointed >= MAX_SCORE 
			|| pRule->nPlayerNum == 2 && nDefPointed >= MAX_SCORE_2MA ) {	// ��
			if ( pRule->bS_StaticRun ) {
				sCalcMethod.Format( _T("%d(����)"), 20 );	// ������
				nMoved = 20;
			}
			else if ( pRule->bS_DoubleForDeclaredRun ) {
				if ( goal.nMinScore >= MAX_SCORE || pRule->nPlayerNum == 2 && goal.nMinScore >= MAX_SCORE_2MA ) {
					sResult += _T(" x2(��)");	// ��
					nMoved *= 2;
				}
			} else if ( pRule->bS_DoubleForRun ) {
				sResult += _T(" x2(��)");		// ����� ��
				nMoved *= 2;
			}
		}
		if ( !bDefWin && pRule->bS_DoubleForReverseRun ) { // �鷱 �˻�
			if( pRule->bS_AGoalReverse && nAttPointed >= goal.nMinScore ) {
				sResult += _T(" x2(�鷱)");
				nMoved *= 2;
			}
			else if( pRule->bS_A11Reverse && nAttPointed >= 11 ) {
				sResult += _T(" x2(�鷱)");
				nMoved *= 2;
			}
			else if( pRule->bS_AMReverse && nAttPointed >= pRule->nMinScore) {
				sResult += _T(" x2(�鷱)");
				nMoved *= 2;
			}
		}
		if ( pRule->bS_DoubleForNoKiruda			// �����
			&& bDefWin && goal.nKiruda == 0 ) {
			sResult += _T(" x2(���)");
			nMoved *= 2;
		}
		if ( pRule->bS_DoubleForNoFriend && pRule->bFriend	// ������ ������ �ִ� ��� ���� 2�谡 ����
				&& bDefWin && goal.nFriend == 0 ) {
			sResult += _T(" x2(����)");
			nMoved *= 2;
		}
	}

	CString sMoved; sMoved.Format(_T(" = [%d]"), nMoved );
	sResult += sMoved;

	// ���� ���� �ӽ� �����ϴ� ��
	int nMoneySum = 0;

	int nAllMoney = BASE_MONEY * pRule->nPlayerNum;
	CString sMultiply;
	if ( Mo()->bMoneyAsSupport ) {
		int nMilli = 10000/nAllMoney%10;
		if ( !nMilli )
			sMultiply.Format( _T("%d.%dx"), 100/nAllMoney, 1000/nAllMoney%10 );
		else sMultiply.Format( _T("%d.%d%dx"), 100/nAllMoney,
			1000/nAllMoney%10, nMilli );
	}
	else sMultiply.Format( _T("%dx"), MONEY_UNIT );

	// �ߴ��� �ְų� ������ ���� ����ϰ�
	// ��� ���� ���ϰ��� ��Ʈ�Ѵ�
	for ( i = 0; i < nPlayers; i++ )
		if ( i != nMaster && i != nFriend ) {

			if ( asExpr )
				asExpr[i].Format(_T("%s %c %s[�⺻]"),
					::FormatMoney( anMoney[i], pRule->nPlayerNum ),
					bDefWin ? _T('-') : _T('+'),
					sMultiply );

			if ( bDefWin ) {
				anMoney[i] -= nMoved;
				nMoneySum += nMoved;
			}
			else {
				anMoney[i] += nMoved;
				nMoneySum -= nMoved;
			}
		}

	// ���� nMoneySum �� ó���Ѵ�
	// ���� �����ŭ ������(�����) ������� �ְ��� ��������
	// ������ŭ ������(�ߴ��) ��������

	// ������� �ְ��� �� �� � ������ �ϴ°�
	int nMasterShare, nFriendShare;
	{
		if ( !pRule->bFriend
			|| !goal.nFriend )	// �����尡 ���� ���
			nMasterShare = nPlayers - 1, nFriendShare = 0;	// 1:0
		else if ( nPlayers-2 < 2 )	// �ߴ��� ���纸�� ���� ���
			nMasterShare = 1, nFriendShare = 1;	// 1:1
		else	// �������� ���
			// �����尡 1 ��ġ�� ���� �ְ��� �������� �� ���´�
			nMasterShare = nPlayers - 3,
			nFriendShare = 1;
	}

	// ������ ���ϰ��� ��Ʈ

	// �ְ�
	if ( asExpr )
		asExpr[nMaster].Format(_T("%s %c %s[�⺻]"),
			::FormatMoney( anMoney[nMaster], pRule->nPlayerNum ),
			bDefWin ? _T('+') : _T('-'),
			sMultiply );

	if ( nMasterShare > 1 ) {
		CString s; s.Format( _T("x%d��"), nMasterShare );
		if ( asExpr ) asExpr[nMaster] += s;
	}

	anMoney[nMaster] +=
		nMoneySum * nMasterShare / (nMasterShare+nFriendShare);

	nMoneySum -=
		nMoneySum * nMasterShare / (nMasterShare+nFriendShare);

	// ������
	if ( pRule->bFriend && goal.nFriend ) {
		if ( asExpr )
			asExpr[nFriend].Format(_T("%s %c %s[�⺻]"),
				::FormatMoney( anMoney[nFriend], pRule->nPlayerNum ),
				bDefWin ? _T('+') : _T('-'),
				sMultiply );
		anMoney[nFriend] += nMoneySum;
		nMoneySum = 0;
	}

	// ������ �� ���� �����Ѵ�
	int nInsuff = 0;
	for ( i = 0; i < nPlayers; i++ )
		if ( anMoney[i] < 0 ) {
			nInsuff += anMoney[i];
			anMoney[i] = 0;
		}

	if ( nInsuff < 0 ) {	// ���� ���ڸ�

		if ( bDefWin )	// ������ �̰����� �ְ��� ���޴´�
			anMoney[nMaster] += nInsuff;
		else {			// �ߴ��� �̰����� ���ݾ� ���޴´�
			int nAttCount =	// �ߴ���
				nPlayers - 1 /*�ְ�*/
				- ( ( pRule->bFriend && goal.nFriend )
						? 1 : 0 ); /*������*/
			int nInsuffShare = nInsuff / nAttCount;

			for ( int i = 0; i < nPlayers; i++ )
				if ( i != nMaster && i != nFriend ) {
					// �ߴ��̸� ���� ���� ����
					nAttCount--;
					if ( nAttCount > 0 ) {
						anMoney[i] += nInsuffShare;
						nInsuff -= nInsuffShare;
					}
					else {
						anMoney[i] += nInsuff;
						nInsuff = 0;
					}
				}
			ASSERT( nInsuff == 0 );
		}
	}

#ifdef _DEBUG	// �� ���� �հ踦 ���Ѵ�
	int nAllDebugSum2 = 0;
	for ( debug = 0; debug < nPlayers; debug++ )
		nAllDebugSum2 += anMoney[debug];
	ASSERT( nAllDebugSum == nAllDebugSum2 );
#endif
}
