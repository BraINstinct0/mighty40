// PlayerHuman.h: interface for the CPlayerHuman class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLAYERHUMAN_H__672332B3_DC38_11D2_983D_0000212035B8__INCLUDED_)
#define AFX_PLAYERHUMAN_H__672332B3_DC38_11D2_983D_0000212035B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CBoardWrap;

// ����� �Է��� ������� �ϴ� �÷��̾� ��ü
// ������ ���� ���� �Ѱ��� �� �ְ� �Ǹ�
// call-back �����쿡 WM_PLAYER �޽����� �����ش�
// �� �� LPARAM ���� ������ CMsg* ��
// �� ���� ����ڰ� ����(delete) �ؾ� �Ѵ�

class CPlayerHuman : public CPlayer
{
public:
	CPlayerHuman(
		int nID,		// ���� ��ȣ ( 6���� �÷��̾��� ���� )
		LPCTSTR sName,	// �̸�
		CWnd* pCallback );// call-back window
	virtual ~CPlayerHuman();

	// �� �÷��̾�� ����ΰ�
	virtual bool IsHuman() const					{ return true; }

	// CPlayer �������̽� ����

	// �� ���� ���۵� - OnBegin ��, ���� ���� �й���� ���� ����
	virtual void OnInit( CEvent* e );

	// ī�带 ���� �ְ� ���� ( nFrom ���� nTo �� (-1�� �߾�)
	// nCurrentCard �� �̵��Ͽ��� )
	// nMode  0 : ī�带 ������ �ʰ�, ���� ��ü ȭ�� ���� �ʿ�
	//        1 : ī�� ������
	//        2 �̻� : Mo()->bShowDealing �� ���� ���� �� ������ ��
	//        3 �̻� : �ӵ��� �ʹ� ������ �ϸ� �ȵ�
	//        9 �̻� : ī�� ����
	//        10 : �ְ��� Privilege �ܰ���, �ӵ��� ������, �Ҹ� ����
	virtual void OnDeal( int nFrom, int nTo, int nMode, int nCurrentCard, CEvent* e );

	// �ϳ��� ������ ���۵�
	// �� ���ӿ��� ���Ǵ� ���� ������ �˷��ش�
	// �� ���� ������ CCard::GetState() �ε� �� �� �ִ�
	// pState->nCurrentPlayer ���� �ٷ� �ڱ� �ڽ���
	// ��ȣ�̸�, �� ���� �ڽ��� �ν��ϴµ� ���ȴ�
	virtual void OnBegin( const CState* pState, CEvent* );

	// 6������ �缱�� ��� �� ����� �׿��� �Ѵ�
	// ���� ī�带 �����ϸ� �ȴ� - �� �� �Լ���
	// �ݺ������� ȣ��� �� �ִ� - �� ���
	// CCardList �� ���ݱ��� ������ ī���� ����Ʈ��
	// �����Ǿ� ȣ��ȴ�
	// 5�� �����ϸ� (�� ��� �˰����� �߸��Ǿ��ų�
	// ����� �� �� �����ϴ� ���) ���Ƿ� ������ 5�� ��
	// �ϳ��� �״´� !
	virtual void OnKillOneFromSix( CCard* pcCardToKill,
		CCardList* plcFailedCardsTillNow, CEvent* );

	// �ְ��� �ٸ� �÷��̾ ���δ�
	// bKilled : ���̸� ������ �׿���, �����̸� ��ٸ� ¤����
	virtual void OnKillOneFromSix( CCard cKill,
		bool bKilled, CEvent* e );

	// 7������ �缱�� ��� �� ����� �׿��� �Ѵ�
	// �� �Լ��� �� �� �ϳ��� ���̴� �Լ���,
	// OnKillOneFromSix�� ����.
	// 5�� �����ϸ� (�� ��� �˰����� �߸��Ǿ��ų�
	// ����� �� �� �����ϴ� ���) ���Ƿ� ������ 5�� ��
	// �ϳ��� �״´� !
	virtual void OnKillOneFromSeven( CCard* pcCardToKill,
		CCardList* plcFailedCardsTillNow, CEvent* );

	// �ְ��� �ٸ� �÷��̾ ���δ�
	// bKilled : ���̸� ������ �׿���, �����̸� ��ٸ� ¤����
	virtual void OnKillOneFromSeven( CCard cKill,
		bool bKilled, CEvent* e );

	// ������ ��´�
	// pNewGoal �� state.goal �� ��������� ������ ��ϵǾ� �ִ�
	// �н��Ϸ��� nMinScore �� 0���� ��Ʈ
	// Ư�� �� �̽��� ���ϴ� ��� ������ -1�� ��Ʈ
	// pNewGoal.nFriend �� ������� �ʴ´�
	virtual void OnElection( CGoal* pNewGoal, CEvent* );

	// �ٸ� ����� ���� ����� ��´�
	void OnElecting( int nPlayerID, int nKiruda,
		int nMinScore, CEvent* e );

	// �ְ��� ��������
	virtual void OnElected( int nPlayerID, CEvent* e );

	// �缱�� ���
	// pNewGoal �� state.goal �� ��������� ������ ��ϵǾ� �ִ�
	// pNewGoal �� ���ο� ������ �����ϰ� (nFriend����)
	// acDrop �� ���� �� ���� ī�带 �����Ѵ�
	// * ���� * �� �Լ� ���Ŀ� �Ѹ��� �׾ �÷��̾���
	// ��ȣ(pState->nCurrentPlayer)�� ����Ǿ��� �� �ִ�
	virtual void OnElected( CGoal* pNewGoal, CCard acDrop[3], CEvent* );

	// ���Ű� ������ �� ������ ���۵Ǿ����� �˸���
	// * ���� * �� �Լ� ���Ŀ� �Ѹ��� �׾ �÷��̾���
	// ��ȣ(pState->nCurrentPlayer)�� ����Ǿ��� �� �ִ�
	virtual void OnElectionEnd( CEvent* );

	// ī�带 ����
	// pc �� �� ī��, pe �� pc�� ��Ŀ�ΰ�� ī�� ���,
	// pc�� ��Ŀ���ΰ�� ���� ��Ŀ�� ����(0:��Ŀ�� �ƴ�)
	// �� �ܿ��� ����
	virtual void OnTurn( CCard* pc, int* pe, CEvent* );

	// ���� ����Ǳ� ����, �� ī�带 ȸ���ϴ� ������ �ܰ�
	// cCurrentCard �� ���� ���ư��� ī��
	virtual void OnTurnEnding( int nWinner, CEvent* e );

	// �� ���� �������� �˸���
	// ���� state.plCurrent ���� �״�� �����ִ�
	virtual void OnTurnEnd( CEvent* );

	// ī�带 ������ �˷��ش�
	// ��� ������ State �� �ִ�
	// nHandIndex �� �� ī�尡 �տ��� ī���� ���° �ε����� ī�忴�°�
	virtual void OnTurn( CCard c, int eff, int nHandIndex, CEvent* e );

	// �����尡 ���� ����
	virtual void OnFriendIsRevealed( int nPlayer, CEvent* e );

	// ���� ���� ( *pbCont �� false �̸� ���� ���� )
	virtual void OnEnd( bool* pbCont, CEvent* e );

	// nPlayer �� ���� ���� ( ������ �����ɸ��� �۾� ���� )
	// nMode : 0 ���� ������  1 Ư�Ǹ�� ó����  2 �� ī�� ������
	virtual void OnBeginThink( int nPlayer, int nMode, CEvent* e );

	// ���α׷� ����
	// �� �Լ��� �ϳ��� �÷��̾ disconnect �� ��,
	// �Ǵ� OnEnd ���� �Ѹ��� �÷��̾�� false ��
	// �������� �� ȣ��ȴ� - Human �� ��� ������ �����Ѵ�
	// sReason : ����Ǵ� ���� (0 �̸� ��� ����)
	virtual void OnTerminate( LPCTSTR sReason );

	// ä�� �޽��� (bSource : ä��â�� �ҽ�)
	virtual void OnChat( int nPlayerID, LPCTSTR sMsg, bool bSource );

protected:
	// ���� ��� & �Լ�

	// ���� CBoardWrap
	CBoardWrap* m_pBoard;
	// ���������� ����ϴ� �̺�Ʈ ��ü
	CEvent m_eTemp;
	// ���� DGoal �� ������ goal ��ü
	CGoal m_goal;
	// ���� Board �� ���õ� ī�带 �����ϴ� ����Ʈ
	CCardList m_clTemp;

	// ���� ���
	const CState* GetState();
	// Ż����ų ī�带 ��õ�Ѵ�
	int GetRecommendedKillCard( const CCardList* plDead );
	// ������ ī�带 ��õ�Ѵ�
	int GetRecommendedFriend( const CCardList* plDeck );
	// p �� �ִ� ī�带 �信�� �����Ѵ�
	void SetCurrentSelectedCard( CCardList* p );
	// ���� �� ���¿��� ������ ���带 �����Ѵ�
	void PlayTurnSound();
	// ��Ŀ, ��Ŀ���� ó���Ѵ� (��ҽ� true ����)
	bool ProcessSpecialCards( CCard c, int* eff );

	// ä��â �ڵ鷯
	static void ChatProc( LPCTSTR s, DWORD dwUser );
};

#endif // !defined(AFX_PLAYERHUMAN_H__672332B3_DC38_11D2_983D_0000212035B8__INCLUDED_)
