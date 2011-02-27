// Player.h: interface for the CPlayer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLAYER_H__E21B2746_CFA8_11D2_9810_000000000000__INCLUDED_)
#define AFX_PLAYER_H__E21B2746_CFA8_11D2_9810_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMsg;
class CMFSM;

// ����
struct GAME_RECORD {
	int wm;			// win count as a master
	int lm;			// lost count as a master
	int wf;			// win count as a friend
	int lf;			// lost count as a friend
	int wa;			// win count as a attacker
	int la;			// lost count as a attacker
};


// �÷��̾��� ����Ŭ����
// �� �÷��̾ ���� ���� (�տ� �� ī��, �̸� ��) ��
// �����ϴ� Ŭ�����̸�, CPlay�� �����ϴ� �������̽��� �����Ѵ�
// (�߰����� �̺�Ʈ ������ ����ϱ� ������ �����Լ��� �ƴ�)

// ���� : �÷��̾�ID  : �� ȣ��Ʈ �������� �÷��̾��� ������ ID
//                      6���� ���� �÷��̾���� �Բ� �����ϸ� CState::apAllPlayers
//                      �������� �ε����̴�
//        �÷��̾��ȣ: ������ ������ �ϴ� �÷��̾��� ��ȣ
//                      CState::apPlayer �������� �ε�����, ����� �÷���
//                      �ϴ� ��� 0 ���� �׻� ����̴�
//        �÷��̾�UID : �� ȣ��Ʈ ������ �÷��̾�ID �� 0 �� �÷��̾
//                      ���� �������� ���� �÷��̾�ID

class CPlayer
{
public:
	CPlayer( int nID,		// ���� ��ȣ ( 7���� �÷��̾��� ���� )
		LPCTSTR sName,		// �̸�
		CWnd* pCallback );	// call-back window
	virtual ~CPlayer() {}

	// �� �Ǹ��� �ؾ� �ϴ� �ʱ�ȭ - �տ��� ī��, ����ī��,
	// ��ȣ ���� �ٽ� �ʱ�ȭ �Ѵ� (�÷��̾� ��ȣ�� ID�� �ʱ�ȭ)
	void Reset();

public:
	// �� �÷��̾�� ����ΰ�
	virtual bool IsHuman() const					{ return false; }
	// �� �÷��̾�� ��ǻ�� AI �ΰ�
	virtual bool IsComputer() const					{ return false; }
	// �� �÷��̾�� ��Ʈ��ũ�ΰ�
	virtual bool IsNetwork() const					{ return false; }

public:
	// ���� ��ȣ ( 7���� �÷��̾�+ 13���� �������� ���� ) �� ��´�
	int GetID() const								{ return m_nID; }
	void SetID( int nPlayer)						{ m_nID = nPlayer; }
	// �÷��̾� ��ȣ ( ���� �÷��� �ϴ� �÷��̾� �� ���� )
	int GetPlayerNum() const						{ return m_nNum; }
	void SetPlayerNum( int nPlayer )				{ m_nNum = nPlayer; }
	// MFSM
	void SetCurrentMFSM( CMFSM* pMFSM )				{ m_pMFSM = pMFSM; }
	// �̸�
	CString GetName() const							{ return m_sName; }
	void SetName( LPCTSTR sName )					{ m_sName = sName; }
	// �տ� �� ī��� ������ ī��
	CCardList* GetHand()							{ return &m_lcHand; }
	const CCardList* GetHand() const				{ return &m_lcHand; }
	CCardList* GetScore()							{ return &m_lcScore; }
	const CCardList* GetScore() const				{ return &m_lcScore; }
	// ��
	int GetMoney() const							{ return m_nMoney; }
	void SetMoney( int nMoney )						{ m_nPrevMoney = m_nMoney; m_nMoney = nMoney; }
	int GetPrevMoney() const						{ return m_nPrevMoney; }
	// ���� ����
	const GAME_RECORD& GetCurRecord() const			{ return m_recCur; }
	GAME_RECORD& GetCurRecord()						{ return m_recCur; }
	// ��� ����
	const GAME_RECORD& GetAllRecord() const			{ return m_recAll; }
	GAME_RECORD& GetAllRecord()						{ return m_recAll; }

public:
	// �������̽� ( CPlay Ȯ�� )

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
	virtual void OnKillOneFromSix( CCard* pcCardToKill,
		CCardList* plcFailedCardsTillNow, CEvent* );

	// ������ ��´�
	// pNewGoal �� state.goal �� ��������� ������ ��ϵǾ� �ִ�
	// �н��Ϸ��� nMinScore �� 0���� ��Ʈ
	// Ư�� �� �̽��� ���ϴ� ��� ������ -1�� ��Ʈ
	// pNewGoal.nFriend �� ������� �ʴ´�
	virtual void OnElection( CGoal* pNewGoal, CEvent* );

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

	// �� ���� �������� �˸���
	// ���� state.plCurrent ���� �״�� �����ִ�
	virtual void OnTurnEnd( CEvent* );

public:
	// �������̽� ( CMFSM �� ȣ���ϴ� Notify )

	// �� ���� ���۵� - OnBegin ��, ���� ���� �й���� ���� ����
	virtual void OnInit( CEvent* e )				{ e->SetEvent(); }
	// 2������ ī�带 ����
	virtual void OnSelect2MA( int* selecting, CCardList* plCard, CEvent* e )	{ selecting, plCard, e->SetEvent(); }
	// ī�带 ���� �ְ� ���� ( nFrom ���� nTo �� (-1�� �߾�)
	// nCurrentCard �� �̵��Ͽ��� )
	// nMode  0 : ī�带 ������ �ʰ�, ���� ��ü ȭ�� ���� �ʿ�
	//        1 : ī�� ������
	//        2 �̻� : Mo()->bShowDealing �� ���� ���� �� ������ ��
	//        3 �̻� : �ӵ��� �ʹ� ������ �ϸ� �ȵ�
	//        9 �̻� : ī�� ����
	//        10 : �ְ��� Privilege �ܰ���, �ӵ��� ������, �Ҹ� ����
	virtual void OnDeal( int nFrom, int nTo, int nMode, int nCard, CEvent* e )	{ nFrom, nTo, nMode, nCard, e->SetEvent(); }
	// �ٸ� ����� ���� ����� ��´�
	virtual void OnElecting( int nPlayerID, int nKiruda,
		int nMinScore, CEvent* e )					{ nPlayerID, nKiruda, nMinScore, e->SetEvent(); }
	// �ְ��� ��������
	virtual void OnElected( int nPlayerID, CEvent* e )	{ nPlayerID, e->SetEvent(); }
	// ī�带 �����.
	virtual void OnSelect2MA( int* selecting, CEvent *e )	{ selecting, e->SetEvent(); }
	// �ְ��� �ٸ� �÷��̾ ���δ�
	// bKilled : ���̸� ������ �׿���, �����̸� ��ٸ� ¤����
	virtual void OnKillOneFromSix( CCard cKill,
		bool bKilled, CEvent* e )					{ cKill, bKilled, e->SetEvent(); }
	// �÷��̾ ���� �� ī�带 ������
	virtual void OnSuffledForDead( CEvent* e )		{ e->SetEvent(); }
	// Ư�� ��尡 ������ ���� ����� ����Ǿ���
	virtual void OnPrivilegeEnd( CGoal* pNewGoal, CCard acDrop[3], CEvent* e ) { pNewGoal, acDrop, e->SetEvent(); }
	// ī�带 ������ �˷��ش�
	// ��� ������ State �� �ִ�
	// nHandIndex �� �� ī�尡 �տ��� ī���� ���° �ε����� ī�忴�°�
	virtual void OnTurn( CCard c, int eff, int nHandIndex, CEvent* e )	{ c, eff, nHandIndex, e->SetEvent(); }
	// ���� ����Ǳ� ����, �� ī�带 ȸ���ϴ� ������ �ܰ�
	// cCurrentCard �� ���� ���ư��� ī��
	virtual void OnTurnEnding( int nWinner, CEvent* e )		{ nWinner, e->SetEvent(); }
	// �����尡 ���� ����
	virtual void OnFriendIsRevealed( int nPlayer, CEvent* e )	{ nPlayer, e->SetEvent(); }
	// ���� ���� ( *pbCont �� false �̸� ���� ���� )
	virtual void OnEnd( bool* pbCont, CEvent* e )			{ *pbCont = true; e->SetEvent(); }
	// nPlayer �� ���� ���� ( ������ �����ɸ��� �۾� ���� )
	// nMode : 0 ���� ������  1 Ư�Ǹ�� ó����  2 �� ī�� ������
	virtual void OnBeginThink( int nPlayer, int nMode, CEvent* e )	{ nPlayer, nMode, e->SetEvent(); }
	// ���α׷� ����
	// �� �Լ��� �ϳ��� �÷��̾ disconnect �� ��,
	// �Ǵ� OnEnd ���� �Ѹ��� �÷��̾�� false ��
	// �������� �� ȣ��ȴ� - Human �� ��� ������ �����Ѵ�
	// sReason : ����Ǵ� ���� (0 �̸� ��� ����)
	virtual void OnTerminate( LPCTSTR sReason )				{ sReason; }
	// ä�� �޽��� (bSource : ä��â�� �ҽ�)
	virtual void OnChat( int nPlayerID, LPCTSTR sMsg,
						bool bSource )						{ nPlayerID, sMsg, bSource; }

protected:
	int m_nID;
	int m_nNum;
	int m_nMoney;
	int m_nPrevMoney;
	GAME_RECORD m_recCur;
	GAME_RECORD m_recAll;
	CString m_sName;
	CWnd* m_pwndCallback;
	CCardList m_lcHand;
	CCardList m_lcScore;
	CMFSM* m_pMFSM;
	CPlay* m_pPlay;
};

inline CPlayer::CPlayer( int nID, LPCTSTR sName, CWnd* pCallback )
{
	m_nID = nID;
	m_nNum = -1;
	m_nMoney = 0;
	m_nPrevMoney = 0;
	memset( &m_recCur, 0, sizeof(GAME_RECORD) );
	memset( &m_recAll, 0, sizeof(GAME_RECORD) );
	m_sName = sName;
	m_pwndCallback = pCallback;
	m_pMFSM = 0;
	m_pPlay = 0;
}

// �� �Ǹ��� �ؾ� �ϴ� �ʱ�ȭ - �տ��� ī��, ����ī��,
// ��ȣ ���� �ٽ� �ʱ�ȭ �Ѵ� (�÷��̾� ��ȣ�� ID�� �ʱ�ȭ)
inline void CPlayer::Reset()
{
	m_nNum = GetID();
	m_lcHand.RemoveAll();
	m_lcScore.RemoveAll();
}

inline void CPlayer::OnBegin( const CState* pState, CEvent* e )
{	if ( m_pPlay ) m_pPlay->OnBegin(pState);
	e->SetEvent(); }
inline void CPlayer::OnKillOneFromSix( CCard* pcCardToKill,
	CCardList* plcFailedCardsTillNow, CEvent* e )
{	if ( m_pPlay ) m_pPlay->OnKillOneFromSix(pcCardToKill,plcFailedCardsTillNow);
	e->SetEvent(); }
inline void CPlayer::OnElection( CGoal* pNewGoal, CEvent* e )
{	if ( m_pPlay ) m_pPlay->OnElection(pNewGoal);
	e->SetEvent(); }
inline void CPlayer::OnElected( CGoal* pNewGoal, CCard acDrop[3], CEvent* e )
{	if ( m_pPlay ) m_pPlay->OnElected(pNewGoal,acDrop);
	e->SetEvent(); }
inline void CPlayer::OnElectionEnd( CEvent* e )
{	if ( m_pPlay ) m_pPlay->OnElectionEnd();
	e->SetEvent(); }
inline void CPlayer::OnTurn( CCard* pc, int* pe, CEvent* e )
{	if ( m_pPlay ) m_pPlay->OnTurn(pc,pe);
	e->SetEvent(); }
inline void CPlayer::OnTurnEnd( CEvent* e )
{	if ( m_pPlay ) m_pPlay->OnTurnEnd();
	e->SetEvent(); }

#endif // !defined(AFX_PLAYER_H__E21B2746_CFA8_11D2_9810_000000000000__INCLUDED_)
