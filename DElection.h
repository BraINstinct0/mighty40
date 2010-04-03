// DElection.h: interface for the all election-related DSB class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DELECTION_H__B24183A6_F55C_11D2_9894_000000000000__INCLUDED_)
#define AFX_DELECTION_H__B24183A6_F55C_11D2_9894_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMFSM;

// ���ſ� ���õ� DSB �� ����


/////////////////////////////////////////////////////////////////////////////
// ���� ���� DSB
// �� DSB �� �缱�ڸ� ���� ���� ���Ǹ� �缱�ڰ�
// ������ ������ ���� ���ȴ�

class DGoal  : public DSB
{
public:
	DGoal( CBoardWrap* pBoard );
	virtual ~DGoal();

	// bPrivilege : �缱�ڰ� ���� �����ϴ� ���ΰ�?
	// pMFSM      : ��� �÷��̾��� �տ� �� ī���
	//              ��ǥ ���� validity �� �׽�Ʈ �ϱ� ���� �����Ѵ�
	void Create( bool bPrivilege,
		const CState* pState, CEvent* pEvent, CGoal* pResult );

protected:
	// �ʱ�ȭ ( ������ �� ȣ��� )
	virtual void OnInit();

	// CBoard �� ȣ���ϴ� �׸��� �Լ� - ��µ� shade-box �� �� �Լ���
	// �������̵� �Ͽ� �����ؾ� �Ѵ�
	// ���̽� �Լ��� ������ ��ü�� ���� ���� ������ �׸���
	virtual void OnDraw(
		CDC* pDC,
		bool bBrushOrigin );

	// �� ������ Ŭ���� �� �Ҹ��� �Լ�
	// LPVOID ���� 0 �̸� ���� (X�ڸ� Ŭ��) ��û
	virtual void OnClick( LPVOID pVoid );

	// Ȯ�� �Ҹ��� �ȳ���
	virtual void OnClickSound() {}

protected:
	// ���� ������ & �Լ�

	// Privilege ����ΰ�
	bool m_bPrivilege;
	// State
	const CState* m_pState;
	// �÷��̾��� �տ� �� ī��
	const CCardList* m_plHand;
	// ��ǥ���� �簢�� ��ġ
	CRect m_rcMinScore;
	// shape �簢���� ǥ�� �Ӽ� �迭
	int m_aatdShape[5][2];
	// ���� ȭ��ǥ�� ǥ�� �Ӽ�, ǥ�� ���� �迭
	// [0][0] = [��ȭ��ǥ][����]
	int m_aatdArrow[2][2];
	COLORREF m_aacolArrow[2][2];
	// Ȯ�� ��ư�� ���� & �Ӽ�
	COLORREF m_colConfirm;
	int m_tdConfirm, m_tdConfirmSel;
	// ���� �ּ� ����
	int m_nMinScore;
	// ���� ǥ�õǴ� ��ǥ ���� ��Ʈ��
	CString m_sMinScore;
	// ������ ���õ� ����
	int m_nKiruda;
	// �⸶ ���� / ���̽�
	bool m_bDealMiss;

	// �־��� ������� ���ٸ� �ٲ۴�
	void ChangeKiruda( int nShape );
	// �־��� ������ �ּ� ������ �ٲ۴�
	// (valid �� ������ ��������)
	void ChangeMinScore( int nMinScore );

	// Toggler �� ���� �÷�
	volatile bool m_bTerminateToggler;
	volatile bool m_bTogglerTerminated;
	CEvent m_ev;	// �̺�Ʈ

	// Board �� Selection �� ������ ������
	// ī�� ������ ����ϴ� ���Ҹ��� �Ѵ�
	static UINT TogglerProc( LPVOID _pThis );
};


/////////////////////////////////////////////////////////////////////////////
// ī�� ���̱� DSB

class DKill  : public DSB
{
public:
	DKill( CBoardWrap* pBoard ) : DSB(pBoard) {}
	virtual ~DKill() {}

	// pcToKill : �� DSB �� ��� ī�带 ������ ��� (���-�� ��������)
	// plDead   : ���ݱ��� ��ٸ� ¤�� ī���
	// plHand   : �÷��̾��� �տ� �� ī��
	void Create( CEvent* pEvent, CCard* pcToKill,
		const CCardList* plDead, const CCardList* plHand );

protected:
	// �ʱ�ȭ ( ������ �� ȣ��� )
	virtual void OnInit();

	// CBoard �� ȣ���ϴ� �׸��� �Լ� - ��µ� shade-box �� �� �Լ���
	// �������̵� �Ͽ� �����ؾ� �Ѵ�
	// ���̽� �Լ��� ������ ��ü�� ���� ���� ������ �׸���
	virtual void OnDraw(
		CDC* pDC, bool bBrushOrigin );

	// �� ������ Ŭ���� �� �Ҹ��� �Լ�
	// LPVOID ���� 0 �̸� ���� (X�ڸ� Ŭ��) ��û
	virtual void OnClick( LPVOID pVoid );

	// Ȯ�� �Ҹ��� �ȳ���
	virtual void OnClickSound() {}

protected:
	// ���� ������ & �Լ�

	// ���� ���õ� ī��
	CCard* m_pcToKill;
	// ���ݱ����� ī���
	const CCardList* m_plDead;
	// �տ� �� ī���
	const CCardList* m_plHand;
	// ���� ���õ� ���
	int m_nShape;
	// ���õ� ����� �Ӽ�
	int m_aatdShape[4][2];
	// m_nShape �� Ư�� ������ ��Ʈ�ϸ鼭 m_aatdShape ��
	// �� �����Ѵ�
	void SetCurShape( int nShape );
	// m_pcToKill ���� �����ϰ� �� ����� ȭ�鿡 ������Ʈ �Ѵ�
	void SetCurCard( CCard c );
	// ���� ī�带 �׸� ������ �����Ѵ�
	void CalcCurCardRect( LPRECT pRc );
};


/////////////////////////////////////////////////////////////////////////////
// �缱�� ��ǥ DSB

class DMasterDecl  : public DSB
{
public:
	DMasterDecl( CBoardWrap* pBoard ) : DSB(pBoard) {}
	virtual ~DMasterDecl() {}

	// bTemp       : true �̸� �� DSB �� ������ �缱�� ��ǥ DSB
	//               false �̸� �� DSB �� ���� Ȯ�� DSB
	// bHuman      : ����� �缱���ΰ�
	// bToKill     : �ٸ� �÷��̾ �׿��� �ϴ°�
	// bUseFriend  : ������ ������ �ִ°�
	// sMasterName : �缱�� �̸�
	// goal        : ����
	void Create( bool bTemp, bool bHuman, bool bToKill,
		bool bUseFriend, int nDefaultMinScore, const CGoal& goal,
		LPCTSTR sMasterName, LPCTSTR sFriendName, int nTimeOut );

protected:
	// CBoard �� ȣ���ϴ� �׸��� �Լ� - ��µ� shade-box �� �� �Լ���
	// �������̵� �Ͽ� �����ؾ� �Ѵ�
	// ���̽� �Լ��� ������ ��ü�� ���� ���� ������ �׸���
	virtual void OnDraw(
		CDC* pDC,
		bool bBrushOrigin );

	// �ƹ����� Ŭ���ص� �ٷ� ��������� �Ѵ�
	virtual bool DispatchHotspot( int x, int y, bool bCheckOnly );

protected:
	// ���� ������ & �Լ�

	bool m_bTemp;
	bool m_bHuman;
	bool m_bToKill;
	bool m_bUseFriend;
	int m_nDefaultMinScore;
	CString m_sMasterName;
	CString m_sFriendName;
	CGoal m_goal;
};


/////////////////////////////////////////////////////////////////////////////
// Ż���� ��ǥ �޽��� DSB

class DDeadDecl  : public DSB
{
public:
	DDeadDecl( CBoardWrap* pBoard ) : DSB(pBoard) {}
	virtual ~DDeadDecl() {}

	// sMaster  : �缱�� �̸�
	// nCard    : ȭ�鿡 ǥ�õ� ī��
	// bFail    : �� ī��� Ż�� ���� ī���ΰ� (�޽����� ���� �޶���)
	void Create( LPCTSTR sMaster, int nCard,
		bool bFail, int nTimeOut );

protected:
	// CBoard �� ȣ���ϴ� �׸��� �Լ� - ��µ� shade-box �� �� �Լ���
	// �������̵� �Ͽ� �����ؾ� �Ѵ�
	// ���̽� �Լ��� ������ ��ü�� ���� ���� ������ �׸���
	virtual void OnDraw(
		CDC* pDC, bool bBrushOrigin );

protected:
	// ���� ������ & �Լ�

	CString m_sMaster;
	int m_nCard;
	bool m_bFail;
};


/////////////////////////////////////////////////////////////////////////////
// ������ DSB
// �� DSB �� ���� Board �� ���� ���� ī��� ��ȣ �ۿ��Ѵ�
// ��, ���ϳ��� �����尡 ī�� ���� �̺�Ʈ�� �����ϸ�,
// Board �� ���� ī�� ������Ʈ�� �����Ѵ�

class DFriend  : public DSB
{
public:
	DFriend( CBoardWrap* pBoard ) : DSB(pBoard) {}
	virtual ~DFriend() {}

	// pnFriend : ����� (�ǹ̴� CGoal::nFriend ����) (���-�� ��������)
	void Create( CEvent* pEvent, int* pnFriend, const CState* pState );

protected:
	// �ʱ�ȭ ( ������ �� ȣ��� )
	virtual void OnInit();

	// CBoard �� ȣ���ϴ� �׸��� �Լ� - ��µ� shade-box �� �� �Լ���
	// �������̵� �Ͽ� �����ؾ� �Ѵ�
	// ���̽� �Լ��� ������ ��ü�� ���� ���� ������ �׸���
	virtual void OnDraw(
		CDC* pDC, bool bBrushOrigin );

	// �� ������ Ŭ���� �� �Ҹ��� �Լ�
	// LPVOID ���� 0 �̸� ���� (X�ڸ� Ŭ��) ��û
	virtual void OnClick( LPVOID pVoid );

	// Ȯ�� �Ҹ��� �ȳ���
	virtual void OnClickSound() {}

protected:
	// ���� ������ & �Լ�

	// ���� ���õ� ������
	int* m_pnFriend;
	// �տ� �� ī���
	CCardList m_lHand;
	// ���� ����
	const CState* m_pState;
	// ���� ���õ� ���
	int m_nShape;
	// ���õ� ����� �Ӽ�
	int m_aatdShape[4][2];
	// ���õ� ������ ī���� ����, �Ӽ�
	COLORREF m_acolFriend[6+MAX_PLAYERS];
	int m_atdFriend[6+MAX_PLAYERS];
	int m_atdFriendSel[6+MAX_PLAYERS];

	// m_nShape �� Ư�� ������ ��Ʈ�ϸ鼭 m_aatdShape ��
	// �� �����Ѵ�
	void SetCurShape( int nShape );
	// m_pnFriend ���� �����ϰ� �� ����� ȭ�鿡 ������Ʈ �Ѵ�
	void SetCurFriend( int nFriend );
	// ���� ī�带 �׸� ������ �����Ѵ�
	void CalcCurCardRect( LPRECT pRc );
};


#endif // !defined(AFX_DELECTION_H__B24183A6_F55C_11D2_9894_000000000000__INCLUDED_)
