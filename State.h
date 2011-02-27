// State.h: interface for the CState class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STATE_H__BE37E2CA_C23D_11D2_97F3_000000000000__INCLUDED_)
#define AFX_STATE_H__BE37E2CA_C23D_11D2_97F3_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#ifndef MAX_PLAYERS
#	define MAX_PLAYERS		7
#endif
#ifndef MAX_CONNECTION
#	define MAX_CONNECTION	20
#endif

#ifndef MIGHTY_EXPORT
#	ifdef _WINDLL
#		define MIGHTY_EXPORT __declspec(dllimport)
#	else
#		define MIGHTY_EXPORT __declspec(dllexport)
#	endif
#endif

struct CRule;
class CPlayer;
class CCard;
class CCardList;


// ����Ƽ ������ ���� ���� ( FSM ���� ��� )
enum MIGHTY_STATE {

	msReady,					// ��� �����Ͱ� ��������� �� ���� �ʱ�ȭ ��
	msDeal2MA,					// 2������ ī�带 ����
	msElection,					// ���Ű� ���۵�
	msPrivilege,				// �ְ��� ������ 3���� ���� ( ������ ���� ���� �ٲٴ� ��Ȳ ���� )
	msTurn,						// ī�带 ���� ��
	msEnd,						// ���� ��, ������ ���ǰ� ��µ�
};


// �ְ��� ������ ���� ��ǥ

struct CGoal
{
	// ���� ( ������϶� 0 )
	int nKiruda;
	// �ּҵ��� ( �� ���� 0�̸� ���� ������ ��������� ���� ���̴� )
	int nMinScore;
	// ������ ( <0 -(�÷��̾��ȣ)-1, <54&&>0 ī��, ==0 ��������, == 100 �ʱ� )
	int nFriend;
};


// ������ ���� ���¸� ��Ÿ���� ����ü

struct MIGHTY_EXPORT CState  
{
	// ���� ����

	MIGHTY_STATE state;


	// ���� ����

	// ���Ǵ� ��Ģ
	CRule* pRule;
	// ���ݱ����� ���� ȸ��
	int nGameNum;
	// ��� �÷��̾�� ( �ִ� 20��=MAX_CONNECTION )
	CPlayer* apAllPlayers[MAX_CONNECTION];
	// ��
	CCardList& lDeck;
	// �ٴ�ī���
	CCardList& lCurrent;
	// �����丮 ( ����, ����, ����, ������ )
	int aanHistory[4][MAX_PLAYERS];
	// �ڸ� ���� �ɼǿ� �ʿ��� ������ ��ġ ���� ( v4.0 : 2011.2.27 )
	int changed[MAX_PLAYERS];

	// �� ����

	// ������ ������ �ϴ� �÷��̾�� ( �ִ� 7�� )
	CPlayer* apPlayers[MAX_CONNECTION];
	// ������ ������ �ϴ� �÷��̾���� ��
	int nPlayers;
	// �� �÷��̾��� ���� ��� ����
	bool abGiveup[MAX_PLAYERS];
	// 6,7 ������ �״� ī�� �ҷ��� �� ������ ī���
	CCardList& lDead;
	// �ְ� ( �Ǵ� �ְ� ������ )
	int nMaster;
	// ������ ( ������ -1, �ʱ��������̰� ���� �ȵ����� -2 )
	int nFriend;
	// 6,7 �� Ż����ID ( ���� �ƹ��� Ż�� �ȵ����� -1 )
	int nDeadID[MAX_PLAYERS-5];
	// �����尡 �������°�
	bool bFriendRevealed;
	// ���� ��ǥ
	CGoal goal;
	// ���� �� ���� ī��
	int acDrop[3];
	// ������ ���� ��� �ִ� ���� ī�� ��
	int nThrownPoints;
	// ��Ŀ�� ���Դ°� ( �� �� �ٴڿ� �ִ� ī�� ���� )
	bool bJokerUsed;
	// ���ݱ��� ������ ī��� �÷� ( �� �� �ٴڿ� �ִ� ī�� ���� )
	// SPADE~CLOVER ���� 0 �� ��Ʈ(TWO)���� 11(KING), 12�� ��Ʈ(ACE) ����
	long anUsedCards[4];
	// �ٴڿ� �ִ� ī�� �÷� (anUsedCards �� OR ����)
	long anUsingCards[4];

	// �� ����

	// ���� �� ( 0 ~ LAST_TURN ), -1 �̸� ������
	int nTurn;
	// ���� �÷��̾�
	int nBeginer;
	// ��Ŀ�� ȿ����
	bool bJokercallEffect;
	// ��Ŀ ���
	int nJokerShape;

	// ī�� ����

	// ���� ������ ī�� ( �Ͽ��� ���ų� ���� �� )
	int cCurrentCard;
	// ���� �÷��̾�
	int nCurrentPlayer;


	// ���� �Լ���

	// ������/�Ҹ���
	CState();
	virtual ~CState();
	// �� �� ������ �����͵��� �ʱ�ȭ �Ѵ� (������ ��)
	// ��, pRule, apPlayers[] �� �ʱ�ȭ ���� �ʴ´� (�̸� �������־�� ��)
	void InitStageData(	int _nGameNum, int _nBeginer = 0 );
	// �ڸ��� ���� ���� ��ġ�� ���Ѵ�
	void SuffleSeat( int& nBeginer );
	// ������ �� (lCurrent �� ������� ��) ���� ȣ���Ͽ� �����丮 ������ �����Ѵ�
	void WriteHistory();

	// ���� goal �� ���ؼ�, ������ ���ٿ�
	// �ּ� ������ ������ �����Ѱ��� �����Ѵ�
	// nNewMinScore �� 0 �̸� ���� ���� ���θ�, -1 �̸�
	// DealMiss ���� ���θ� ���´�
	bool IsValidNewGoal( int nNewKiruda, int nNewMinScore ) const;

	// ���ο� ������ �ĺ� ���� �ָ� ������ ��� �� ���� �״��,
	// �ƴϸ� 0(��������)�� �����Ѵ�
	int GetValidFriend( int nFriend ) const;

	// �־��� ī���а� ���̽��ΰ� �����Ѵ�
	bool IsDealMiss( const CCardList* pl ) const;

	// �ش� �÷��̾ �����ΰ��� �Ǵ��Ѵ�
	bool IsDefender( int nPlayer ) const;

	// ���� ��Ŀ���� ��Ŀ�� ȿ���� �� �� �ִ����� �˻��Ѵ�
	bool IsEffectiveJokercall() const;

	// �־��� ī�� ����Ʈ�� ���� ���� �÷��̾� ��ȣ�� �˾Ƴ���
	// ���� ���� ���� state ������ �����Ѵ�
	// pbWinnerIsDef : ���ڰ� �����ΰ� ���θ� ����
	int GetWinner( const CCardList* pList,
		bool* pbWinnerIsDef = 0,
		int nJokerShape = -1, int bJokercallEffect = -1,
		int nPlayers = -1, int nTurn = -1,
		int nBeginer = -1, int nMaster = -1, int nFriend = -1 ) const;

	// �� ������ ��Ʈ��ũ�� ������ �����ΰ�
	bool IsNetworkGame() const;
};

#endif // !defined(AFX_STATE_H__BE37E2CA_C23D_11D2_97F3_000000000000__INCLUDED_)
