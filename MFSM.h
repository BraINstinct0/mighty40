// MFSM.h: interface for the CMFSM class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MFSM_H__FDD28386_D3C4_11D2_981F_000000000000__INCLUDED_)
#define AFX_MFSM_H__FDD28386_D3C4_11D2_981F_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CPlayer;
class CSocketBag;
struct CPlay;

// Mighty Final State Machine
// ����Ƽ�� ���� ��ü�� �����ϴ� �ſ� �߻�ȭ �� ��ƾ
// Init �� ȣ���ϸ� �������� �����带 ����� ����ȴ�

class CMFSM  : private CState
{
public:
	// * ���� *
	// �÷��̾���� �� MFSM �� �ı��Ǵ� ���� �ڵ�������
	// �� MFSM �� delete �Ѵ�
	CMFSM( LPCTSTR sRule, CPlayer* _apPlayers[], CSocketBag* pSockBag = 0 );

public:
	// ���񽺸� �����Ѵ�
	// ���ο� �����尡 ��ٷ� ���۵ȴ�
	// uid : �� MFSM �� uid ( 0 �̸� �� MFSM �� �ݵ�� �����̴� )
	void Init( long uid );
	// ���� �������� �ڵ��� Duplicate �Ѵ� (Wait �ϱ� ����)
	HANDLE DuplicateThreadHandle();

	// ���� ���¸� ��´�
	const CState* GetState() const					{ return (const CState*)this; }
	// ���������� ��´�
	CSocketBag* GetSockBag()						{ return m_pSockBag; }
	// �����ΰ�?
	bool IsServer() const							{ return m_uid == 0; }

public:
	// �÷��̾ ǥ���ϴ� ID, UID, Num ���� ��ȯ�� �Ѵ�
	// �ش� �÷��̾ �������� �ʴ� ��� -1 �� �����Ѵ� (Num �� �����ϴ� �Լ��� ���)
	long GetPlayerUIDFromID( long id );
	long GetPlayerIDFromUID( long uid );
	long GetPlayerIDFromNum( long num );
	long GetPlayerNumFromID( long id );
	long GetPlayerNumFromUID( long uid );
	long GetPlayerUIDFromNum( long num );

public:
	// ��ƿ��Ƽ �Լ���

	// ��� �÷��̾��� ī�带 �ɼǿ� ���� �����Ѵ�
	void SortPlayerHand( bool bLeftKiruda, bool bLeftAce );

	// �÷��̾��� �̸����� �缳���Ѵ�
	void SetPlayerNames( LPCTSTR asName[] );

	// �� ī��(pCard)�� �� �÷��̾�(nPlayerID)��
	// �ٸ� �÷��̾ ���� �� �ִ°�
	// �׷� �� �ִٸ� true, ���� �� ���� �ٸ� ī�带 ã�ƾ�
	// �Ѵٸ�(��ٸ�) false �� �����Ѵ�
	// pDeadID �� ���� �÷��̾ ����
	bool KillTest( int nPlayerID, CCard* pCard, int* pDeadID );

	// (������) �־��� goalNew �� ���Ÿ� �����ų ��
	// �ִ°��� �Ǵ��Ѵ� - ������ goal, abGiveUp, IsDealMiss() ��
	// ���������� ����Ѵ�
	// ���ϰ��� - bDealMiss : �� �̽��� ����Ǿ��°��� �Ǵ�
	//            nNextID : �缱�� ID �Ǵ� ���� �⸶�� ID
	// ���Ÿ� ��� �ؾ� �ϴ���(false), �� �̽��� �缱 Ȯ������
	// ����Ǵ���(true)�� �����Ѵ�
	bool CanBeEndOfElection( bool& bDealMiss, long& nNextID,
		const CGoal& goalNew );

	// �� ī�忡 ���� ���� ��Ʈ���� �����Ѵ�
	CString GetHelpString( CCard c ) const;

	// ���� ����ÿ� �������� �� ��� ����Ʈ�� ��´�
	// �迭 ���ϰ��� �迭 ũ��� �÷��̸� �� �ο� ( �ִ� 5 )
	void GetReport(
		// ���ϰ�
		bool& bDefWin,			// ���(����)���� �¸��ߴ°� ?
		int* pnDefPointed,		// ���� ����
		int anContrib[],		// ���嵵 (�� ���� �� �ۼ�Ʈ ������ Ȱ���� �ߴ°�)
		CString asCalcMethod[2],// [0] ��� �����, �⺻ �׼��� ��� �� [1] ��Ÿ ������Ģ
		CString asExpr[],		// ���� ��� ��
		// ��-���-��������
		int anMoney[]			// ȣ��ÿ��� ���� ������ ��, ���ϵ� ���� ���� ��
	) const;

public:
	// �̺�Ʈ �߻��� �˸�

	// ���� �޽���
	void EventExit( LPCTSTR sMsg = 0 );
	// ä�� �޽��� ( bSource �� ���̸�, ä��â���� ������� �޽��� )
	void EventChat( CMsg* pMsg, bool bSource );

public:
	// ���� CPlayer �� �θ��� �Լ���

	// ������ ��Ų �۾��� �Ϸ�Ǿ����� MFSM ���� �˸���
	void Notify( int nPlayerID );
	// �ش� �̺�Ʈ�� Lock(=Wait)�Ѵ� - ���� ���ǽ� CMFSM �ڽ���
	// throw �ȴ�
	void WaitEvent( CEvent* pEvent );

	// �� �Լ��� ���α׷� ���谡 �߸��Ǿ� �ʿ��� ���� �Լ���
	// �÷��̾ ȣ���ϸ� �ְ��� ī�� �� acDrop �� �ش��ϴ�
	// �� ���� ī�带 ������ �߰� ���� �����
	// �����Ѵ� - MFSM �� ������ ȣ���� ������
	// CPlayerHuman �� ��� ���� ī�� ���ð� ������ ���� ���̿�
	// ȭ�� Update �� �ʿ��ϱ� ������ �� �Լ��� �ʿ��ϴ�
	void RemoveDroppedCards( CCard acDrop[3], int nNewKiruda );

	// Ư�� CPlayerMAI �� �θ��� �Լ���

	// AI �����忡��, �ڽ��� �Լ��� ȣ���϶�� ��û�Ѵ�
	// nType : 0 OnElection / 1 OnKillOneFromSix / 2 OnTurn
	void RequestCallAIThread( CPlay* pPlay, int nType,
				CEvent* pEvent, LPVOID arg1 = 0, LPVOID arg2 = 0 );

protected:
	// 0�� �÷��̾��� ���������� ID
	long m_uid;

	// �����忡�� ����� �˸��� �̺�Ʈ
	CEvent m_eNotify;
	// ��ǵ�
	volatile bool m_bTerminate;	// �����϶�
	// ���� �����尡 �� ���� ��Ʈ�����μ� ����
	volatile bool m_bTerminated;
	// soft �� ���� ��û
	volatile bool m_bTermRequest;
	CString m_sTermRequestReason;
	// ä�� �޽��� ��û
	CCriticalSection m_csChatMsg;
	struct CHATITEM { CMsg* pMsg; bool bSource; };
	CList<CHATITEM,CHATITEM&> m_lChatMsg;
	// ������ ������
	static UINT ServiceProc( LPVOID pThis );
	// ���� ������
	UINT Server();
	CWinThread* m_pServerThread;

	// ���ϵ�
	CSocketBag* m_pSockBag;
	// ���� �̺�Ʈ�� ����ϱ� ���� �̺�Ʈ ��ü
	CEvent m_eSock;

	// �÷��̾�鿡 ���� ��� �̺�Ʈ ( ���� �̺�Ʈ���� )
	CEvent* m_apePlayer[MAX_CONNECTION];
	// �� �÷��̾ ���� ��û �Լ��� ȣ���ؾ� �ϴ��� ����
	bool m_abWaitingPlayerRedo[MAX_CONNECTION];
	// ���� ������� �÷��̾ ���� �̺�Ʈ�� ����
	CEvent* m_apeWaitingPlayer[MAX_CONNECTION+1];	// �̺�Ʈ(m_eNotify ����)
	int m_nWaitingPlayer;						// ����
	// �̺�Ʈ�� �����ϰ�, m_apeWaitingPlayer ���� �ʱ�ȭ �Ѵ�
	void ResetEvents();
	// �̺�Ʈ�� ����ϸ�, ���ܸ� ó���ϴ� �Լ�
	// ������� �÷��̾��� �̺�Ʈ�� ����, Set �Ǹ� m_apeWaitingPlayer
	// ���� �� �̺�Ʈ�� �����Ѵ�
	// �� �Լ��� �ſ� �߿��ϸ� �ڼ��� ���� ���� ������ ����
	// ���ϰ��� ��� event �� ��Ʈ�� �÷��̾��� PlayerID �Ǵ� -1 (����)
	int Wait();

	// AI ������ ����
	void InvokeAIThread();
	CWinThread* m_pAIThread;	// �� ������
	static UINT AIThread( LPVOID pVoid );
	UINT AIThread();
	CEvent m_eAIThread;
	volatile bool m_bAIThreadTerminate;
	volatile bool m_bAIThreadTerminated;
	volatile int m_nAIThreadRequest;
	LPVOID m_apArgs[2];
	CEvent* m_peAIThreadRequestedEvent;
	CPlay* m_pAIThreadTargetPlay;
	CCriticalSection m_csAIThread;
	void CallAIThread( CPlay* pPlay, int nType,
				CEvent* pEvent, LPVOID arg1, LPVOID arg2 );

	// ��Ÿ ���� �Լ�

	// �Ҹ���
	virtual ~CMFSM();

	// ���� �÷��̾ ��������, apPlayers[] �� ��迭�Ѵ�
	void RebuildPlayerArray();
	// m_eNotify �̺�Ʈ�� ó��
	void ProcessNotifyEvents();
	// ä�� �޽����� ó��
	void ProcessChatMessage();
	// ��Ŀ���� ������ ��Ȳ���� �����Ͽ� �׷��ٸ� �־���
	// ���ڿ� ���� bIsJokercall �� ��Ʈ�Ѵ�
	// �� ��Ŀ�� ��� nJokerShape �� ��Ʈ�Ѵ�
	void SetJokerState( CCard c, int nEffect );
	// �ڸ��� ���´� ( �ڸ� ���� �ɼ��� ���� ��쿡�� : 2011.2.27 )
	void SuffleSeat( int& nBeginer );
	// �������� �ڸ��� ������ ���� ��ġ�� ���Ѵ�
	void GetSeatFromServer( int& nBeginer );
	// �������� ���� ���´�
	void GetDeckFromServer();
};

#endif // !defined(AFX_MFSM_H__FDD28386_D3C4_11D2_981F_000000000000__INCLUDED_)
