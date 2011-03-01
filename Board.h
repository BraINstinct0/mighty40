// Board.h : interface of the CBoard class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_BOARD_H__6433F788_C1B5_11D2_97F2_000000000000__INCLUDED_)
#define AFX_BOARD_H__6433F788_C1B5_11D2_97F2_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMFSM;
class DSB;
class DSay;
class CBoardWrap;


/////////////////////////////////////////////////////////////////////////////
// CBoard window
// ����Ƽ ������ ���� ������

class CBoard : public CWnd
{
public:
	// �⺻ �Լ�
	CBoard();
	virtual ~CBoard();

	// ���� ���
	CBoardWrap* GetWrapper()						{ return m_pWrap; }

protected:
	// dtor ���� ȣ��Ǹ�, �Ҵ�� �ڿ��� ��ȯ�Ѵ�
	void CleanUp();
	// ���� CBoard �� ������ Mighty FSM �� ��Ʈ�Ѵ�
	// �̰��� ��Ʈ �ϴ� ���� �������� �������� ������ ���۵ȴ�
	void SetMFSM( CMFSM* pMFSM );
	// ������ BmpManager �� ��´�
	CBmpMan* GetBmpMan()							{ return &m_bm; }
	// �÷��̾��� �̸��� �ɼǿ��� �ٽ� �ε��Ѵ�
	void ReloadPlayerNames();

protected:
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
	// bSelectionRegion : �� ���� ���̸� 0 �� �÷��̾��� ���� ����, ���� �������� �Բ� �����Ѵ�
	void UpdatePlayer( int nPlayer, int x, int y, int cx, int cy, bool bUpdateNow = false, bool bSelectionRegion = false );
	void UpdatePlayer( int nPlayer, LPRECT prc = 0, bool bUpdateNow = false, bool bSelectionRegion = false );

protected:
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

	// ī�带 ������ �ִϸ��̼� ��
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

	// ī�� ���� �ִϸ��̼� ��
	// ������ �̺�Ʈ�� ��Ʈ�Ѵ�
	// ������ IDW_SUFFLE �� �����Ѵ�
	void SuffleCards( int nCount /*���� ȸ��*/, CEvent* pEvent );

	// ������ �÷��̾ Deal-Miss ���·� �����ش�
	// nPlayerID �� -1 �� ��� �װ��� �����Ѵ�
	// ������ ȭ���� Update ������ �ʴ´�
	void SetDealMiss( int nPlayerID )				{ m_nDealMiss = nPlayerID; }
	int GetDealMiss() const							{ return m_nDealMiss; }

	// Privilege �� �ش� �ε����� ī�带 select/deselect �Ѵ�
	// ������ ȭ���� Update ������ �ʴ´�
	void SetSelection( int nIndex, bool bSelect = true ) { ASSERT(nIndex>=0); m_abSelected[nIndex] = bSelect; }
	bool GetSelection( int nIndex ) const				{ ASSERT(nIndex>=0); return m_abSelected[nIndex]; }
	// ��ü ����
	void SetSelection( bool bSelect = true )		{ for ( int i = 0; i < sizeof(m_abSelected)/sizeof(bool); i++ ) m_abSelected[i] = bSelect; }
	int GetSelectionCount() const					{ int n = 0; for ( int i = 0; i < sizeof(m_abSelected)/sizeof(bool); i++ ) n += m_abSelected[i] ? 1 : 0; return n; }

// DSB ( shade-box ) ���� �Լ�
protected:
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

// Internal Helpers
protected:
	// Ÿ�̸� id
	enum TIME_ID {
		tiDSB = 1000,			// DSB Ÿ�̸ӵ��� ���̽� ��
	};

protected:
	// ���� �ƿ� ���� �Լ���

	// ȭ����� �� �簢�� ��ǥ�� ��´�
	// nPlayers   : �÷��� �ϴ� �÷��̾��� �� ( 1 ~ 6 )
	// type       : CR_CENTER ��� CR_SCORE �������� CR_HAND �տ��� ī�� CR_HIDDEN ���°�
	// nPlayer    : � �÷��̾ ���� �����ΰ� (nType!=0�϶��� ���)
	// nCardWidth : ī���� ���� ũ��
	// nCardHeight: ī���� ���� ũ��
	// nIndex, nAll : �簢�� �ȿ� ��� �ִ� �� �������� ��ġ��
	//                ��ü(nAll)���� ���°(nInex)�ΰ��� ���� ������
	//                �����Ѵ�, nIndex == -1 �̸� ��ü �簢���� ����
	// pbVertical : 0 �� �ƴϸ� �� �簢���� ���η� �� �簢�������� �����Ѵ�
	// pbDirection: 0 �� �ƴϸ� ������ ���� ( true:������ �Ʒ�,���ʿ��� ������ false:�� �ݴ�)
	enum CR_TYPE { CR_CENTER, CR_SCORE, CR_HAND, CR_HIDDEN };
	CRect CalcRect( int nPlayers, CR_TYPE type, int nPlayer, 
		int nCardWidth, int nCardHeight,
		int nIndex = -1, int nAll = 0,
		bool* pbVertical = 0, bool* pbDirection = 0 ) const;

	// �� �÷��̾��� �̸� ��ġ�� ���Ѵ�
	CRect CalcNameRect( int nPlayers, int nPlayer,
		CDC* pDC, LPCTSTR sName, CFont* pFont ) const;

	// ���� ȭ�� ũ�⿡ ���� ������ ī�� �� ũ�⸦ ��´�
	CSize GetCardZoom( bool bUseZoom ) const;

	// ���� ȭ�� ũ�⿡ ���� ������ ��Ʈ���� ���� �����
	void CreateFont();

protected:
	// �׸��� ���� �Լ���

	// �׷��� �� Ư���� ī��鿡 ���� ī�� �̵���
	// (���õ� ī�峪 ���콺 ���� ī��� ���� �����)
	enum {
		CARD_SHIFT_RATIO_OF_MOUSE_OVER = 20,// ī���� 1/20
		CARD_SHIFT_RATIO_OF_SELECTION = 5	// ī���� 1/5
	};

	// ���, �տ� �� ī��, ����, ��� ���� ī�带 ��� �׸���
	void DrawBoard( CDC* pDC, LPCRECT prc );

	// DSB �� �׸���
	// Board �� ���� ��ǥ�迡 �׷�������
	// xOff �� yOff �� �׷��ִ� �ɼ��� ������ �� �ִ�
	void DrawDSB( CDC* pDC, bool bBrushOrigin,
		int x, int y, int cx, int cy,
		int xOff = 0, int yOff = 0 );

	// �÷��̾��� �տ� �� ī�带 �׸���
	void DrawHand(
		CDC* pDC,
		int nPlayer, int nPlayers,			// �÷��̾�/�÷��̾��
		const CCardList* pList,				// �÷��̾��� ī��
		int x, int y, int cx, int cy,		// ����
		int xOff, int yOff,					// ������ �׷������� �ɼ� ��ȭ
		bool bReveal,						// �ո��� ���ϰ��ΰ�
		int nMouseOver,						// ���콺�� ��ġ�ϴ� ī�� �ε���
		bool* abSelected = 0,				// ���� ���õ� ī���
		bool bScore = false );				// �տ� �� ī�尡 �ƴ϶� ���� ī�带 �׸���

	// ��� ���� ī�带 �׸���
	void DrawCenter(
		CDC* pDC,
		int nPlayers,						// �÷��̾��
		const CCardList* pList,				// �׸� ī��
		int nBeginer,						// ���� �÷��̾�(�Ʒ�0���� �ð����)
		int x, int y, int cx, int cy,		// �׸� ����
		int xOff, int yOff );				// ������ �׷��� ���� �ɼ� ��ȭ

	// �̸��� �׸���
	void DrawName(
		CDC* pDC,
		int nPlayers,						// �÷��̾��
		LPCTSTR sName,						// �̸�
		int nPlayer,						// �׸� �÷��̾�
		COLORREF col, COLORREF colShadow,	// ���� ����, �׸��� ����
		CFont* pFont );						// ��Ʈ (0 �̸� �ý��� ��Ʈ ���)

	// ��� �κ��� �ִϸ��̼�
	// ũ��� (ī����)*2 X (ī�����)*1.5
	// x, y �� �ִϸ��̼��� �� ��� ��ġ
	// szCard �� ī�� ũ��
	// nCount ī�� ��� (nStep==0 �϶��� ���)
	// nStep
	// 0 : ��� ī�尡 ���� ���
	// 1 : ī�尡 �������� �и��Ǵ� ��
	// 2 : ī�尡 �������� �и��Ǿ���
	// 3 : ī�尡 ����� �������� ��
	void DrawCenterAnimation( CDC* pDC, bool bBrushOrigin,
		CSize szCard,
		int x, int y, int nStep,
		int nCount = -1 );

	// ī�带 ������ - �� �Լ��� ���� FlyCard �� ������ ����
	// ���ϴ� �ɼ��� �ָ� ī�带 ������ �����ϸ鼭
	// �ڵ��� �����Ѵ� - �� �ڵ��� �ٽ�
	// �Լ��� ���ڷ� �����ϸ� ���� �ڵ���
	// �����ϸ鼭 ���ݾ� ī�� �����⸦ �����Ѵ�.
	// �����Ⱑ �������� 0 �� �����Ѵ�
	// �����찡 �������� ������ �ڿ��� �����ϰ� 0 �� �����Ѵ�
	volatile void* FlyCard(
		volatile void*& h,		// �ڵ�
		int nCard,				// ���� ī��
		const CRect& rcBegin,	// ���۵Ǵ� ī�� �簢��
		const CPoint& ptEnd,	// ������ ��ġ ( �簢���� �»�� )
		int nStates,			// ������� ������ �Ϸ��ϴ°�
		int nShadeMode );		// �׸��� ȿ�� (0:�Ʒ����� �Ʒ��� 1:������ �Ʒ���
								//              2:�Ʒ����� ���� 3:������ ����)
	volatile void* FlyCard( volatile void*&, int nStep = 1 );

	// ī�带 ���� �ִϸ��̼��� ���δ�
	// �� �Լ��� ���� SuffleCards �� ������ ��������
	// DrawCenterAnimation �Լ��� ����Ѵ�
	// FlyCard �� ���� ������� ȣ���Ѵ�
	// �����찡 �������� ������ �ڿ��� �����ϰ� 0 �� �����Ѵ�
	volatile void* SuffleCards(
		volatile void*& h,		// �ڵ�
		int nCount,				// ���� ȸ��
		SIZE szCard );			// ī�� ũ��
	volatile void* SuffleCards( volatile void*& );

	// FlyCard �����Լ�- �־��� ������ ���� �簢���̳� ���� �簢���� ����Ѵ�
	// ���� ���ڴ� FlyCard �� ����
	RECT CalcCardRectForFlyCard( int nType, int nPlayer, int nIndex );

protected:
	// ����� ��Ÿ �Լ���

	// �÷��̾��ȣ�� �����Ѵ� - �Ʒ��� �÷��̾����
	// �ð� �������� 0 ~ 7(v4.0) �� ǥ�� ���(���밪)��
	// ���� �÷��̾� ��ȣ(����)�� �����ϰų� ����ȯ �Ѵ�
	int MapAbsToLogic( int nAbsPlayer ) const;
	int MapLogicToAbs( int nLogicalPlayer ) const;
	// ����� �� ī�� �߿��� ������ �ε����� �ִ� ī����
	// ���� �簢�� ��ġ�� ���� ( ����Ʈ �� ī�� ��ġ���� ��� )
	bool CalcRealCardRect( LPRECT prc, int index ) const;
	// ����� �� ī�� �߿��� ������ ����Ʈ�� �ִ�
	// ī���� POSITION �� ����
	POSITION CardFromPoint( int x, int y ) const;
	// ������ ��ġ�� ī�带 ��� �ø���
	// (m_abSelected �� m_nMouseOver ���� �ٲ� �� Update �Ѵ�)
	// bMouseOver �� true �̸� m_nMouseOver �� �����ϰ�
	// ������ �÷����� ī��� �����ش�
	// false �̸� m_abSelected �� ���� ����ϰ�
	// ����� �ݿ��Ѵ�
	void ShiftCard( int x, int y, bool bMouseOver );
	// ���� Ŀ�� �Ʒ����� ī�带 �� �� �ִ°�
	bool CanSelect() const;
	// OnLButtonDown �� ���ؼ� DSB ó���� �� �ִ� �Լ�
	bool CookDSBForLButtonDown( POINT point );
	// OnLButtonDown �� ���ؼ� ī�� ���� ó���� �� �ִ� �Լ�
	bool CookSelectionForLButtonDown( POINT point );

// Internal Members
protected:
	HCURSOR			m_hArrow;	// ȭ��ǥ Ŀ��
	HCURSOR			m_hHand;	// �ո�� Ŀ��

	CFont			m_fntSmall,	// ���� ��Ʈ
					m_fntMiddle,
					m_fntBig,
					m_fntFixedSmall,
					m_fntFixedMiddle,
					m_fntFixedBig;

	CBitmap			m_pic;		// ���� ������ ����� �׷��� �ִ� ��Ʈ�� ����
	CBitmap			m_board;	// ���, ��, �ٴ��� �׷��� �ִ� ��Ʈ��
	CSize			m_szPic;	// �� �׸����� ũ��

	CBmpMan			m_bm;		// ��Ʈ�� �Ŵ���

	CSize			m_szCard;	// ���� ī�� ũ��

	CMFSM*			m_pMFSM;	// ���� Mighty FSM

	CEvent*			m_pWaitingSelectionEvent;	// NULL�� �ƴϸ� Ŭ�� ��� �̺�Ʈ ������
	long*			m_pWaitingSelectionCard;	// Ŭ�� ��� ���¿��� Ŭ���� ī�带 ������ ������

	int		m_nMouseOver;		// ���� ���콺 �Ʒ��� �־ ���� ����� ī�� (-1�� ���°��)
	bool	m_abSelected[LAST_TURN_2MA+2];	// privilege �� ���õ� ī�� (���� 14���� ���)
	int		m_nDealMiss;		// �� �̽� �� �÷��̾� (�� �÷��̾��� ī�� ������ reveal)

	int		m_nTurnRect;		// ���� �簢���� ��ġ

	CBoardWrap*	m_pWrap;		// Wrapper ��ü
public:	bool m_bDoNotSendMeAnyMoreMessage;	// Wrapper ���� �����Ѵ�

protected:
	// ī�� ������ ���� ���
	// * ī�� �������� �ؼ� �׻� FlyCard ���� �����ִ� ���� �ƴϰ�,
	//   ī�� ����, �����ð� ����ϱ� �� �ð� ������ �ΰ� �ð� ȿ����
	//   �־�� �ϴ� ���� ����� �����Ѵ�

	CCriticalSection	m_csFlying;			// ī�� ������/���� ť�� ���� CS

	CList<CMsg*,CMsg*>	m_lFlyingQueue;		// ī�� ������/���� ť
	volatile void*		m_hFlying;			// ���� ������ �ڵ�
	volatile void*		m_hSuffling;		// ���� ���� �ڵ�
	CEvent				m_eFlyIt;			// ī�� ������/���� �����尡 ����ϴ� �̺�Ʈ
	volatile bool		m_bFlyProcTerminated;
	volatile bool		m_bFlyProcTerminate;

	static UINT	FlyProc( LPVOID pThis );	// ī�� ������/���� ������ ���� �Լ�
	enum OPERATION { opWaitWhile, opFlyCard, opSuffleCards };

	// Flyingť�� �� �޽����� �߰��ϰ� Flying�̺�Ʈ�� ��Ʈ�Ѵ�
	void AddFlyingQueue( CMsg* pMsg )		{ m_csFlying.Lock(); m_lFlyingQueue.AddTail(pMsg); m_csFlying.Unlock(); m_eFlyIt.SetEvent(); }
	// FlyProc ���� ȣ��Ǵ� ���� ���� �Լ���
	void DoFlyCard( int, int, int, int, int, int, int, int );
	void DoFlyCard2( int, int, int, int, int, int, int, int );
	void DoSuffleCards( int );

protected:
	// DSB ���� ���

	CList<DSB*,DSB*> m_lpDSB;	// DSB ����Ʈ

	DSay*	m_apSayDSB[MAX_PLAYERS];	// ȭ�鿡 ǥ�õǰ� �ִ� ä�� ����
	DSB*	m_apGoalDSB[MAX_PLAYERS];	// ȭ�鿡 ǥ�õǰ� �ִ� ���� ����
	DSB*	m_pMasterGoalDSB;			// m_apGoalDSB �� �缱�Ƿ� �ϴ� ���� ����
	DSB*	m_pCurrentElectionDSB;		// ȭ�鿡 ǥ�õǰ� �ִ� ���� ���� ����

	bool		m_bDragDSB;				// DSB �� �巡�� �ϴ� ���ΰ�
	DSB*		m_pDragDSB;				// �巡�� �ϰ� �ִ� DSB
	CPoint		m_pntLastDSB;			// �巡�� �ϴ� DSB �� ���� ��ǥ
	DSB*		m_pHLDSB;				// Highlighted DSB
	CList<DSB*,DSB*> m_lpModalDSB;		// Modal ������ DSB

protected:
	// ������ FlyCard �������� ���Ǵ�, �ڵ� ����ü
	struct FLYCARD_INFO {
		int nCard;				// ���� ī��
		CRect rcBegin;			// ���۵Ǵ� ī�� �簢��
		CPoint ptEnd;			// ������ ��ġ ( �簢���� �»�� )
		int nStates;			// ������� ������ �Ϸ��ϴ°�
		int nShadeMode;			// �׸��� ���
		int nCurShadeDepth;		// ������ �׸��� ����
		int nCurState;			// ���� ���° �ܰ��ΰ�
		int x;					// ���� x ��ġ
		int y;					// ���� y ��ġ
		int xLast;				// ���� x ��ġ
		int yLast;				// ���� y ��ġ
		CSize sz;				// ���� ũ�� (��Ʈ���� ���� ũ��)
	};
	// ������ SuffleCards �������� ���Ǵ�, �ڵ� ����ü
	struct SUFFLECARDS_INFO {
		int nCount;				// �� ���� ȸ��
		int nCurCount;			// ���ݱ��� ���� ȸ��
		int nCurStep;			// ���� �׸� �ܰ� ( 0-4)
		int xCenter;
		int yCenter;			// ȭ�� �߽� ��ǥ
		bool bBrushOrigin;		// �귯�� ����
		CSize sz;				// ȭ�鿡 ������ ũ��
	};

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBoard)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL
	virtual int OnToolHitTest( CPoint point, TOOLINFO* pTI ) const;

	// Generated message map functions
protected:
	//{{AFX_MSG(CBoard)
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnUpdateGameExit(CCmdUI* pCmdUI);
	afx_msg void OnGameExit();
	//}}AFX_MSG
	afx_msg LRESULT OnWrapper( WPARAM wParam, LPARAM lParam );
	afx_msg BOOL OnToolTipNeedText( UINT id, NMHDR * pNMHDR, LRESULT * pResult );
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BOARD_H__6433F788_C1B5_11D2_97F2_000000000000__INCLUDED_)
