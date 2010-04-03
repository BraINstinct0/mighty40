// BoardWrapper.h : Wrapper of CBoard
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_BOARDWRAP_H__6433F789_C1B5_11D2_97F2_000000000000__INCLUDED_)
#define AFX_BOARDWRAP_H__6433F789_C1B5_11D2_97F2_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMFSM;
class DSB;
class CBmpMan;

/////////////////////////////////////////////////////////////////////////////
// CBoardWrap
// CBoard�� Wrapper Window

class CBoardWrap
{
public:
	// �⺻ �Լ�
	CBoardWrap( CWnd* pWnd );
	virtual ~CBoardWrap();

	operator HWND()									{ return (HWND)*m_pBoard; }
	operator CWnd*()								{ return m_pBoard; }
	void GetClientRect( LPRECT prc ) const;

	// ���� ���
	CBoardWrap* GetWrapper();

public:
	// ���� CBoard �� ������ Mighty FSM �� ��Ʈ�Ѵ�
	// �̰��� ��Ʈ �ϴ� ���� �������� �������� ������ ���۵ȴ�
	void SetMFSM( CMFSM* pMFSM );
	// ������ BmpManager �� ��´�
	CBmpMan* GetBmpMan();
	// �÷��̾��� �̸��� �ɼǿ��� �ٽ� �ε��Ѵ�
	void ReloadPlayerNames();

public:
	// ���� ȭ���� DSB�� �����Ѵ�
	// ������ ������ ���۸� �ٽ� �׸� �� �� ������ ���� �����쵵
	// ��ȿȭ ��Ų��
	// bUpdateNow : ��� �����츦 Update �Ѵ�
	void UpdateDSB( int x, int y, int cx, int cy, bool bUpdateNow = false );
	void UpdateDSB( LPCRECT prc = 0, bool bUpdateNow = false );
	// �÷��̾ �ٴ� ������ �����Ѵ�
	// �� ���� DSB �鵵 �Բ� ���ŵȴ� ( UpdateDSB �� ���������� ȣ���Ѵ� )
	// nPlayer �� -1 �̸� �ٴ� ������ ����
	// nPlayer �� -2 �̸� ��� ������ ����
	// prc �� 0 �̸� �� �÷��̾��� ��� ������ ����
	//        �׷��� ������ �� �������� ����
	// bUpdateNow : ��� �����츦 Update �Ѵ�
	// bSelectionRegion : �� ���� ���̸� 0 �� �÷��̾��� ���� �������� �Բ� �����Ѵ�
	void UpdatePlayer( int nPlayer, int x, int y, int cx, int cy, bool bUpdateNow = false, bool bSelectionRegion = false );
	void UpdatePlayer( int nPlayer, LPRECT prc = 0, bool bUpdateNow = false, bool bSelectionRegion = false );

public:
	// Ư�� ȿ��

	// ���� �簢���� �ش� �÷��̾� ��ġ�� �̵� (-1 �� ����)
	void SetTurnRect( int nPlayer );

	// ������ �ð����� ������ �ִٰ� ������ �̺�Ʈ�� ��Ʈ�Ѵ�
	void WaitWhile( int nMillisecond, CEvent* pEvent );

	// �� �Լ��� ȣ���ϸ� ���콺 Ŭ��, �Ǵ� ä�� â����
	// ����ڰ� ī�带 ������ �� ���� ����ߴٰ�
	// ������ �̺�Ʈ�� ��Ʈ�Ѵ�
	void WaitSelection( CEvent* pEvent, long* pSelectedCard );
	// ���� ���¸� ����Ѵ�
	void CancelSelection();

	// ī�带 ������ ���ϸ��̼� ��
	// ������ �̺�Ʈ�� ��Ʈ�Ѵ�
	// (�׸��� ȿ���� �߾��� ���� �������� ����)
	void FlyCard(
		int nCard,			// ���� ī��
		int nSpeed,			// �ӵ� ( 0 : ǥ�� 1 : �ſ� ���� 2 : ���� )
		int nFrom,			// ���� ��ġ ( 0 ��� 1 �������� 2 �տ��� ī�� 3 ���°� 4 ���� �����!!)
		int nFromPlayer,	// ���� ��ġ ( �÷��̾� ��ȣ, nFrom!=0�϶��� ���  )
		int nFromIndex,		// �� ��° ī���ΰ� (-1�̸� �� ���)
		int nTo,			// �� ��ġ
		int nToPlayer,
		int nToIndex,
		CEvent* pEvent );

	// ī�� ���� ���ϸ��̼� ��
	// ������ �̺�Ʈ�� ��Ʈ�Ѵ�
	// ������ IDW_SUFFLE �� �����Ѵ�
	void SuffleCards( int nCount /*���� ȸ��*/, CEvent* pEvent );

	// ������ �÷��̾ Deal-Miss ���·� �����ش�
	// nPlayerID �� -1 �� ��� �װ��� �����Ѵ�
	// ������ ȭ���� Update ������ �ʴ´�
	void SetDealMiss( int nPlayerID );
	int GetDealMiss();

	// Privilege �� �ش� �ε����� ī�带 select/deselect �Ѵ�
	// ������ ȭ���� Update ������ �ʴ´�
	void SetSelection( int nIndex, bool bSelect = true );
	bool GetSelection( int nIndex );
	// ��ü ����
	void SetSelection( bool bSelect = true );
	int GetSelectionCount();

// DSB ( shade-box ) ���� �Լ�
public:
	void AddDSB( DSB* pDSB );		// DSB ����Ʈ�� �߰�
	void SetDSBTimer( DSB* pDSB );	// DSB �� Ÿ�̸Ӹ� �缳�� (DSB���� Ÿ�̸ӷ�)
	bool RemoveDSB( DSB* pDSB );	// DSB ����Ʈ���� ����
	int GetDSBUnit();				// DSB ���� ����ϴ� ���� ���� ũ�� (�ּ���Ʈ�� ������)
	// �۲� ( 0 small 1 medium 2 big, �����̸� -nSize-1 �� Fixed )
	CFont* GetFont( int nSize );
	// �� ��Ʈ���� ȭ�鿡���� ũ�⸦ ���Ѵ� (������ ��Ʈ ���)
	CSize GetTextExtent( int nSize, LPCTSTR s );
	// DSB �� ����ϱ� ���� �뷫���� �÷��̾� ��ġ�� ���
	// ���������� �� �÷��̾��� '��������'�� �����Ѵ�
	// ( nPlayerNum �� -1 �̸� ��ü Ŭ���̾�Ʈ�� �� ��� )
	// pbVert ���� ������ ���η� �䰡 ���θ� �����Ѵ�
	RECT CalcPlayerExtent( int nPlayerNum, bool* pbVert = 0 );
	// �ش� �÷��̾� �ڸ��� ä�� DSB �� ����
	// nPlayerID : �÷��̾� ID
	// sMsg    : �޽���
	void FloatSayDSB( int nPlayerID, LPCTSTR sMsg );
	// �ش� �÷��̾� �ڸ��� ���� DSB �� ����
	// nPlayer : �÷��̾� ��ȣ ( -1 �̸� ������ DSB �� ���� )
	// goal    : �� �÷��̾��� ����
	// goal.nMinScore �� 0 �̸� �⸶ ����, -1 �̸� ���̽�
	void FloatGoalDSB( int nPlayerID, const CGoal* pGoal );
	// ������ DSB �� ���� DSB �� �����Ѵ�
	// �ѹ��� �ϳ��� ���� DSB �� ȭ�鿡 ǥ�õǹǷ�
	// ������ ���� DSB �� ���ŵȴ�
	// pDSB �� 0 �̸� ������ ���� DSB �� ���ŵȴ�
	void SetElectionDSB( DSB* pDSB );

protected:
	friend class CBoard;
	CWnd* m_pBoard;

	// �Լ� �̸�
	enum WRAPPER_FUNC {
		wfGetWrapper, wfSetMFSM, wfGetBmpMan, wfReloadPlayerNames,
		wfUpdateDSB_1, wfUpdateDSB_2,
		wfUpdatePlayer_1, wfUpdatePlayer_2, wfSetTurnRect, wfWaitWhile,
		wfWaitSelection, wfCancelSelection, wfFlyCard,wfSuffleCards,
		wfSetDealMiss, wfGetDealMiss, wfSetSelection_1, wfSetSelection_2,
		wfGetSelection, wfGetSelectionCount, wfAddDSB, wfSetDSBTimer,
		wfRemoveDSB, wfGetDSBUnit, wfGetFont, wfGetTextExtent,
		wfCalcPlayerExtent, wfFloatSayDSB, wfFloatGoalDSB, wfSetElectionDSB
	};

	// �������� �Լ� ���� ����ü
	union WRAPPER_ARG {

		struct { void* p; bool b; } PB;
		struct { int i4[4]; bool b; } I4B;
		struct { int i5[5]; bool b2[2]; } I5B2;
		struct { int i; void* p; bool b2[2]; } IPB2;
		struct { int i; void* p; } IP;
		struct { void* p2[2]; } P2;
		struct { int i8[8]; void* p; } I8P;
		struct { int i; } I;
		struct { int i; bool b; } IB;
		struct { bool b; } B;
		struct { void* p; } P;
		struct { void* p2[2]; int i; } P2I;
	};
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BOARDWRAP_H__6433F789_C1B5_11D2_97F2_000000000000__INCLUDED_)
