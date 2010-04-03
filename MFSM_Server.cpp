// MFSM_Server.cpp: implementation of the CMFSM Server Thread
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mighty.h"

#include "Play.h"
#include "Player.h"
#include "MFSM.h"
#include "MFSM_Notify.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// ���� ������

UINT CMFSM::Server()
{
	TRACE( "Server Began\n" );
	int i;

	try {

		nBeginer = GetPlayerIDFromUID( 0 );

// ���� ����
lblMightyBegin:

		// ���� state �� �ʱ�ȭ �Ѵ�
		InitStageData( nGameNum + 1, nBeginer );

		// Ŭ���̾�Ʈ�� ��� �������� ī�� �迭�� ���´�
		if ( !IsServer() ) GetDeckFromServer();

		// �� �÷��̾��� �ʱ�ȭ
		for ( i = 0; i < pRule->nPlayerNum; i++ ) {
			nCurrentPlayer = GetPlayerNumFromID(i);
			NOTIFY_TO( i, OnInit(EVENT) );
		}

		// ī�带 ���� �ش�
		// ����, ����, ����, ���� �� ����
		{
			int nCardsPerOnePlayer = pRule->nPlayerNum == 6 ? 8 : 10;
			int nAllCards = nCardsPerOnePlayer * nPlayers;
			for ( i = 0; i < nAllCards; i++ ) {

				nCurrentPlayer = (nBeginer+i)%nPlayers;
				cCurrentCard = lDeck.RemoveTail();
				apPlayers[nCurrentPlayer]->GetHand()->AddTail( cCurrentCard );
				int nHandCount = apPlayers[nCurrentPlayer]
									->GetHand()->GetCount();

				// ����, ����, ����, ���� �� �� �� �÷��̾�� �˸�
				bool bShow = nHandCount == 2 || nHandCount == 5
					|| nHandCount == 8 || nHandCount == 10;
				if ( bShow ) NOTIFY_ALL( OnDeal( -1, nCurrentPlayer, 1, cCurrentCard, EVENT ) );
			}
		}

		// ���Ÿ� �����Ѵ�
		state = msElection;

		// ���������� �� �÷��̾�� �˸���
		for ( i = 0; i < pRule->nPlayerNum; i++ ) {
			nCurrentPlayer = GetPlayerNumFromID(i);
			NOTIFY_TO( i, OnBegin( GetState(), EVENT ) );
		}

		nCurrentPlayer = nBeginer;

		for(;;) {

			NOTIFY_ALL( OnBeginThink( nCurrentPlayer, 0, EVENT ) );

			// ���ο� ��ǥġ�� ��´�
			CGoal goalNew = goal;
			NOTIFY_TO( nCurrentPlayer, OnElection( &goalNew, EVENT ) );

			// ����� �� ��ǥġ���� �˻��Ѵ�
			if ( !IsValidNewGoal( goalNew.nKiruda, goalNew.nMinScore ) ) {
				goalNew.nMinScore = 0;	// ������ ������ ����
				if ( !IsValidNewGoal( goalNew.nKiruda, goalNew.nMinScore ) ) {
					// �����ؼ��� �ȵǴ� ��Ȳ (���� ���� ���ϴ� ���)
					goalNew.nKiruda = 0;
					goalNew.nMinScore = pRule->nMinScore;
					ASSERT( IsValidNewGoal( goalNew.nKiruda, goalNew.nMinScore ) );
				}
			}

			// ��ο��� �˸���
			NOTIFY_ALL( OnElecting( nCurrentPlayer,
				goalNew.nKiruda, goalNew.nMinScore, EVENT ) );

			// �� ��ǥġ�μ� ���Ű� �����°�?
			bool bEnd, bDealMiss;
			long nNextID;
			bEnd = CanBeEndOfElection( bDealMiss, nNextID, goalNew );

			if ( bDealMiss ) {		// ���̽�
				state = msEnd; goto lblCleanup;
			}
			else if ( goalNew.nMinScore == 0 )	// ����
				abGiveup[nCurrentPlayer] = true;
			else goal = goalNew;				// �⸶

			if ( bEnd ) {		// �̰����� ��
				nMaster = nNextID;	// �缱��
				break;
			}
			else		// ���Ÿ� �����
				nCurrentPlayer = nNextID;
		}

		// �缱 Ȯ��
		// �ְ� �÷��̰� ���۵ȴ�
		state = msPrivilege;

		// ��(nBeginer), �����÷��̾�(nCurrentPlayer) ���
		// �ְ����� ����
		nCurrentPlayer = nBeginer = nMaster;

		NOTIFY_ALL( OnBeginThink( nMaster, 0, EVENT ) );

		// �ϴ� �տ� �� ī�带 �������ϰ� ��� �÷��̾�� �˸���
		if ( !Mo()->bNoSort )
		for ( i = 0; i < nPlayers; i++ )
			apPlayers[i]->GetHand()->Sort( Mo()->bLeftKiruda, Mo()->bLeftAce );
		NOTIFY_ALL( OnElected( nMaster, EVENT ) );

		// 6 �� ��� �� ���� �÷��̾ ���̴� ������ �߰��ȴ�
		if ( pRule->nPlayerNum == 6 ) {

			CCard cKill(0);
			bool bKilled = false;
			do {

				cKill = CCard(0);
				NOTIFY_TO( nMaster,
					OnKillOneFromSix( &cKill, &lDead, EVENT ) );

				if ( cKill == CCard(0) ) {	// �߸��� ī�� ��
					ASSERT(0); cKill = CCard(SPADE,ACE);
				}

				bKilled = KillTest( nMaster, &cKill, &nDeadID );
				if ( !bKilled ) lDead.AddTail( cKill );

				NOTIFY_ALL( OnKillOneFromSix( cKill, bKilled, EVENT ) );

			} while ( !bKilled );

			// ���� �÷��̾��� ī�带 ���� ������

			CCardList* plcDead = apAllPlayers[nDeadID]->GetHand();
			int nCards = plcDead->GetCount(); ASSERT( nCards == 8 );

			// ȸ���ܰ�
			for ( i = 0; i < nCards; i++ ) {

				nCurrentPlayer = (nBeginer+i)%nPlayers;
				cCurrentCard = plcDead->RemoveTail();
				lDeck.AddTail( cCurrentCard );
				NOTIFY_ALL( OnDeal( nDeadID, -1, 3, cCurrentCard, EVENT ) );
			}

			RebuildPlayerArray();

			// ��� ���� ī�带 ���� ������ �ϳ��� �÷��̾ ���δ�
			lDeck.Suffle();
			// Ŭ���̾�Ʈ�� ��� �������� �ٽ� ���� ī�� �迭�� ���´�
			if ( !IsServer() ) GetDeckFromServer();
			// �� �÷��̾�� �˸���
			NOTIFY_ALL( OnSuffledForDead(EVENT) );

			// ��ο��� �˸���
			NOTIFY_ALL( OnDeal( -1, -1, 0, 0, EVENT ) );

			nCards = 10;	// ���� 10 ���� ���� �ش�
			for ( i = 0; i < nCards; i++ ) {

				nCurrentPlayer = (nBeginer+i)%nPlayers;
				cCurrentCard = lDeck.RemoveTail();
				CCardList* pHand = apPlayers[nCurrentPlayer]->GetHand();
				pHand->AddTail( cCurrentCard );
				if ( !Mo()->bNoSort )
					pHand->Sort( Mo()->bLeftKiruda, Mo()->bLeftAce );

				NOTIFY_ALL( OnDeal( -1, nCurrentPlayer, 9, cCurrentCard, EVENT ) );
			}
		}	// if ( pRule->nPlayerNum == 6 ) �� ��

		// �ְ��� ������ ī�带 ������
		{
			nCurrentPlayer = nMaster;
			CCardList* pHand = apPlayers[nMaster]->GetHand();

			int nCards = lDeck.GetCount();
			for ( i = 0; i < nCards; i++ ) {

				cCurrentCard = lDeck.RemoveTail();
				pHand->AddTail( cCurrentCard );
				if ( !Mo()->bNoSort )
					pHand->Sort( Mo()->bLeftKiruda, Mo()->bLeftAce );

				NOTIFY_ALL( OnDeal( -1, nCurrentPlayer, 10, cCurrentCard, EVENT ) );
			}
		}

		// �ְ����� Ư�� ���¸� ó���ϵ��� �Ѵ�
		{
			NOTIFY_ALL( OnBeginThink( nMaster, 1, EVENT ) );

			CGoal goalNew = goal;
			CCard ac[3];
			NOTIFY_TO( nMaster, OnElected( &goalNew, ac, EVENT ) );
			NOTIFY_ALL( OnPrivilegeEnd( &goalNew, ac, EVENT ) );

			// ����� �ݿ��Ѵ�

			// ������ ī�带 ������
			if ( apPlayers[nMaster]->GetHand()->GetCount() > 10 ) {
				CCardList* pHand = apPlayers[nMaster]->GetHand();
				POSITION pos = pHand->GetTailPosition();
				for ( i = 0; i < 3; i++ ) {
					if ( !pHand->Find(ac[i]) ) { // �߸��� ī��
						ASSERT(0);
						bool bDup;
						do {
							ac[i] = pHand->GetPrev(pos);	// �ƹ� ī�峪 �� ������!!
							bDup = false;
							for ( int j = 0; j < i; j++ )
								if ( ac[j] == ac[i] ) bDup = true;
						} while (bDup);
					}
				}
				RemoveDroppedCards( ac, goalNew.nKiruda );
				ASSERT( pHand->GetCount() == 10
					&& lDeck.GetCount() == 3 );
			}

			// ��ǥ ����, ������

			if ( !IsValidNewGoal( goalNew.nKiruda, goalNew.nMinScore ) ) {
				ASSERT(0);
				goalNew.nKiruda = goal.nKiruda;
				goalNew.nMinScore = goal.nMinScore;
			}

			goalNew.nFriend = GetValidFriend( goalNew.nFriend );
			goal = goalNew;
			if ( goal.nFriend == 0 ) { nFriend = -1; bFriendRevealed = true; }
			else if ( goal.nFriend == 100 ) nFriend = -2;
			else if ( goal.nFriend < 0 ) { nFriend = -goal.nFriend-1; bFriendRevealed = true; }
			else {
				CCard c(goal.nFriend);
				for ( i = 0; i < nPlayers; i++ )
					if ( apPlayers[i]->GetHand()->Find(c) ) {
						nFriend = i; break;
					}
				if ( i >= nPlayers ) { nFriend = -1; ASSERT(0); bFriendRevealed = true; }
			}
		}

		// �������� ������ ����
		state = msTurn;

		// �̸� �˸���
		for ( i = 0; i < pRule->nPlayerNum; i++ ) {
			nCurrentPlayer = GetPlayerNumFromID(i);
			NOTIFY_TO_ID( i, i, OnElectionEnd( EVENT ) );
		}

		// ���� �߿��� ���� ���� !
		// 10 �� �ݺ���
		for ( nTurn = 0; nTurn <= LAST_TURN; nTurn++ ) {

			nJokerShape = 0;
			bJokercallEffect = false;

			// �� �÷��̾�� �ݺ�
			for ( int cnt = 0; cnt < nPlayers; cnt++ ) {

				nCurrentPlayer = ( nBeginer + cnt ) % nPlayers;
				cCurrentCard = 0;

				// ī�带 ����
				CCard c(0);
				int eff = -1;

				NOTIFY_ALL( OnBeginThink( nCurrentPlayer, 2, EVENT ) );

				NOTIFY_TO( nCurrentPlayer, OnTurn( &c, &eff, EVENT ) );

				CCardList* pCurHand = apPlayers[nCurrentPlayer]->GetHand();

				// ����ΰ� �ƴϸ� �ٸ� �ƹ� ī�峪 ���� �Ѵ� !
				CCardList clAv;
				pCurHand->GetAvList( &clAv, &lCurrent, nTurn,
										nJokerShape, bJokercallEffect );
				if ( !clAv.Find( c ) ) { c = clAv.GetHead(); eff = 0; }

				cCurrentCard = (int)c;

				// ��Ŀ/��Ŀ�ݿ� ���õ� ������ �����Ѵ�
				SetJokerState( c, eff );
				// �� ī�� �÷��� ��Ʈ�Ѵ�
				if ( !c.IsJoker() ) anUsingCards[c.GetShape()-SPADE]
					|= (1<<(c.GetPowerNum()-1));

				// ������ ī�带 ����
				POSITION pos = pCurHand->Find(c); ASSERT(pos);
				int nHandIndex = pCurHand->IndexFromPOSITION(pos);
				pCurHand->RemoveAt( pos );
				lCurrent.AddTail( c );

				bool bFriendIsJustRevealed = false;
				if ( 0 < goal.nFriend && goal.nFriend < 54
						&& c == CCard(goal.nFriend) ) {
					// �����尡 ������, ������ ������ ��� ���ش�
					ASSERT( nFriend >= 0 );
					bFriendIsJustRevealed = bFriendRevealed = true;
					lDeck.AddTail( apPlayers[nFriend]->GetScore() );
					apPlayers[nFriend]->GetScore()->RemoveAll();
				}

				// ��ο��� �˸���
				NOTIFY_ALL( OnTurn( c, eff, nHandIndex, EVENT ) );

				if ( bFriendIsJustRevealed )
					NOTIFY_ALL( OnFriendIsRevealed( nFriend, EVENT ) );
			}

			// �� ���� ����

			// ���� ī�� �÷��� ��Ʈ
			for ( i = 0; i < 4; i++ ) {
				anUsedCards[i] |= anUsingCards[i];
				anUsingCards[i] = 0;
			}

			// ���ڸ� �����Ѵ�
			int nWinner = GetWinner( &lCurrent );

			nCurrentPlayer = nWinner;	// ���� �÷��̾�� ������
			cCurrentCard = 0;
			CCardList* pWinnerScore = apPlayers[nWinner]->GetScore();

			// �ʱ� �������� ��� �� ���ڰ� �����尡 �ȴ�
			if ( nFriend == -2 ) {							ASSERT( nTurn == 0 );
				bFriendRevealed = true;
				if ( nWinner == nMaster ) nFriend = -1;	// ��������
				else nFriend = nWinner;
				NOTIFY_ALL( OnFriendIsRevealed( nFriend, EVENT ) );
			}

			// ����� �����丮�� ���
			WriteHistory();

			// ��ο��� �˸���
			NOTIFY_ALL( OnTurnEnd( EVENT ) );

			// ���� ī�带 ���, ó���ϰ� ���ڸ� ���� ������ �Ѵ�
			while (!lCurrent.IsEmpty()) {

				CCard c( cCurrentCard = (int)lCurrent.RemoveTail() );

				if ( c.IsPoint() && GetState()->nMaster != nWinner
					&& ( GetState()->nFriend != nWinner
						|| !GetState()->bFriendRevealed ) )
					pWinnerScore->AddTail(c);
				else lDeck.AddTail(c);

				NOTIFY_ALL( OnTurnEnding( nWinner, EVENT ) );
			}
			lCurrent.RemoveAll();
			nBeginer = nWinner;
		}

		// ���� ��
		state = msEnd;
		{
			// ���� ���θ� ����ϴ� �÷�
			bool abCont[MAX_PLAYERS];
			for ( i = 0; i < pRule->nPlayerNum; i++ ) abCont[i] = true;

			// ������ ������ Update �Ѵ�
			{
				bool bDefWin; CString as[2];	// dummy var
				int anMoney[MAX_PLAYERS];
				for ( i = 0; i < nPlayers; i++ )
					anMoney[i] = apPlayers[i]->GetMoney();

				GetReport( bDefWin, 0, 0, as, 0, anMoney );

				// ������ Update
				for ( i = 0; i < nPlayers; i++ ) {
					GAME_RECORD& grCur = apPlayers[i]->GetCurRecord();
					GAME_RECORD& grAll = apPlayers[i]->GetAllRecord();

					if ( bDefWin ) {
						if ( i == nMaster ) grCur.wm++, grAll.wm++;
						else if ( i == nFriend ) grCur.wf++, grAll.wf++;
						else grCur.la++, grAll.la++;
					} else {
						if ( i == nMaster ) grCur.lm++, grAll.lm++;
						else if ( i == nFriend ) grCur.lf++, grAll.lf++;
						else grCur.wa++, grAll.wa++;
					}
				}

				for ( i = 0; i < nPlayers; i++ )
					apPlayers[i]->SetMoney( anMoney[i] );

				NOTIFY_ALL( OnEnd( &abCont[PLAYERID], EVENT ) );
			}

			// ���Ḧ �ϰ� �Ǿ����� �˻��Ѵ�
			bool bCont = true;
			for ( i = 0; i < pRule->nPlayerNum; i++ )
				if ( !abCont[i] ) { bCont = false; break; }

			if ( !bCont )  { // ����
				for ( i = pRule->nPlayerNum - 1; i >= 0; i-- )
					apAllPlayers[i]->OnTerminate( m_sTermRequestReason );
				throw this;
			}

			// nBeginer �� ���� ���� ��Ʈ�Ѵ�
			// (���� ���� ���� �÷��̾���� ���ԵǹǷ�
			//  nBeginer �� PlayerID �� ��Ʈ�Ǿ�� �Ѵ�)
			if ( pRule->bFriendGetsBeginer && nFriend >= 0 )
				nBeginer = GetPlayerIDFromNum(nFriend);
			else nBeginer = GetPlayerIDFromNum(nMaster);
		}

lblCleanup:
		goto lblMightyBegin;

	}
	catch ( CMFSM* ) {
		TRACE("caught terminating exception\n");
	}

	m_bTerminated = true;
	TRACE( "Server Ended\n" );
	return 0;
}
