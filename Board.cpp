// Board.cpp : implementation of the CBoard class
//

#include "stdafx.h"
#include "Mighty.h"
#include "BmpMan.h"
#include "DSB.h"
#include "DEtc.h"
#include "DElection.h"
#include "DStartUp.h"
#include "Play.h"
#include "Player.h"
#include "MFSM.h"
#include "Board.h"
#include "BoardWrap.h"
#include "InfoBar.h"
#include "DScoreBoard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CBoard

CBoard::CBoard()
{
	m_hArrow = 0;
	m_hHand = 0;

	m_szPic.cx = m_szPic.cy = 0;
	m_szCard.cx = m_szCard.cy = 0;

	m_pMFSM = 0;

	m_pWaitingSelectionEvent = 0;
	m_pWaitingSelectionCard = 0;

	m_nMouseOver = -1;
	for ( int i = 0; i < (LAST_TURN+1)*2; i++ )
		m_abSelected[i] = false;
	m_nDealMiss = -1;

	m_nTurnRect = -1;

	m_pWrap = 0;
	m_bDoNotSendMeAnyMoreMessage = false;

	m_hFlying = 0;
	m_hSuffling = 0;
	m_bFlyProcTerminated = false;
	m_bFlyProcTerminate = false;

	for ( int j = 0; j < MAX_PLAYERS; j++ ) {
		m_apSayDSB[j] = 0;
		m_apGoalDSB[j] = 0;
	}
	m_pMasterGoalDSB = 0;
	m_pCurrentElectionDSB = 0;

	m_bDragDSB = false;
	m_pDragDSB = 0;
	m_pntLastDSB.x = m_pntLastDSB.y = 0x80000000;
	m_pHLDSB = 0;

	// FlyProc �����带 ����
	VERIFY( AfxBeginThread( FlyProc, (LPVOID)this ) );
}

CBoard::~CBoard()
{
	CleanUp();
}

// dtor ���� ȣ��Ǹ�, �Ҵ�� �ڿ��� ��ȯ�Ѵ�
void CBoard::CleanUp()
{
	// Ȥ�� ����ϰ� �������� �𸣴� �̺�Ʈ���� ��Ʈ�� �ش�
	m_eFlyIt.SetEvent();
	if ( m_pWaitingSelectionEvent ) {
		m_pWaitingSelectionCard = 0;
		m_pWaitingSelectionEvent->SetEvent();
	}

	if ( m_pMFSM ) {

		HANDLE hMFSMThread = m_pMFSM->DuplicateThreadHandle();
		m_pMFSM->EventExit();

		if ( hMFSMThread ) {
			VERIFY( ::WaitForSingleObject( hMFSMThread, INFINITE )
					== WAIT_OBJECT_0 );
			VERIFY( ::CloseHandle( hMFSMThread ) );
		}
		m_pMFSM = 0;
	}

	// �����带 ���δ�
	if ( !m_bFlyProcTerminated ) {
		m_bFlyProcTerminate = true;
		m_eFlyIt.SetEvent();
		while ( !m_bFlyProcTerminated ) Sleep(100);
	}

	// ��� DSB �� �����Ѵ�
	ASSERT( m_lpDSB.IsEmpty() );
	while ( m_lpDSB.GetCount() > 0 )
		m_lpDSB.GetTail()->Destroy();

	// ������ �� ī�� ������ ���� �� ī�� ������ �����Ѵ�
	if ( m_hFlying )
		FlyCard( m_hFlying );
	if ( m_hSuffling )
		SuffleCards( m_hSuffling );

	// ��Ʈ�� ��ü�� �����Ѵ�
	if ( m_pic.GetSafeHandle() )
		m_pic.DeleteObject();
	if ( m_board.GetSafeHandle() )
		m_board.DeleteObject();

	// ��Ʈ ��ü�� �����Ѵ�
	if ( m_fntSmall.GetSafeHandle() )
		m_fntSmall.DeleteObject();
	if ( m_fntMiddle.GetSafeHandle() )
		m_fntMiddle.DeleteObject();
	if ( m_fntBig.GetSafeHandle() )
		m_fntBig.DeleteObject();
	if ( m_fntFixedSmall.GetSafeHandle() )
		m_fntFixedSmall.DeleteObject();
	if ( m_fntFixedMiddle.GetSafeHandle() )
		m_fntFixedMiddle.DeleteObject();
	if ( m_fntFixedBig.GetSafeHandle() )
		m_fntFixedBig.DeleteObject();

	// ���� ����
	delete m_pWrap; m_pWrap = 0;
}

void CBoard::OnDestroy() 
{
	CWnd ::OnDestroy();

	// TODO: Add your message handler code here

	// * ���� *

	// �� �κ��� ��Ƽ �������� ���� ���� ������ ���õȴ�
	// Wrapper �� m_bDoNotSendMeAnyMoreMessage (���� bNoMore ) ��
	// ��Ʈ �Ǵ� �������� Board �� �޽����� ������ �ʰ� �ǹǷ�,
	// �Ʒ��ʿ� �ִ� bNoMore �� ��Ʈ�Ǵ� ���� ������ �޽�����
	// ���� �ȴ� - �׷��� �� �Լ��� ���ϵǸ� PostNcDestroy�� ����
	// Board �� ���� �� ���̹Ƿ� bNoMore �� ��Ʈ�Ǵ� �� ��������
	// ���� �ִ� WM_BOARDWRAPPER �޽����� �������� ��� ������
	// �߻��Ѵ� - ���� PeekMessage �� �� �޽������� �����ϴ�
	// ����� ����Ͽ���

#ifdef _DEBUG
	Sleep(2000);	// �簢���븦 ������ ������ �߻��� Ȯ���� ���δ�
#endif

	// ��� DSB �� �����Ѵ�
	while ( m_lpDSB.GetCount() > 0 )
		m_lpDSB.GetTail()->Destroy();

	m_bDoNotSendMeAnyMoreMessage = true;
	m_bFlyProcTerminate = true;

	// ���� WM_BOARDWRAPPER ��û�� ��� ���ش�
	MSG msg;
	while ( ::PeekMessage( &msg, GetSafeHwnd(),
		WM_BOARDWRAPPER, WM_BOARDWRAPPER, PM_REMOVE ) ) {
		::TranslateMessage( &msg );
		::DispatchMessage( &msg );
	}
}

BEGIN_MESSAGE_MAP(CBoard,CWnd )
	//{{AFX_MSG_MAP(CBoard)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	ON_WM_TIMER()
	ON_WM_LBUTTONUP()
	ON_WM_DESTROY()
	ON_WM_CHAR()
	ON_UPDATE_COMMAND_UI(ID_GAME_EXIT, OnUpdateGameExit)
	ON_COMMAND(ID_GAME_EXIT, OnGameExit)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_BOARDWRAPPER, OnWrapper)
	ON_NOTIFY_EX( TTN_NEEDTEXT, 0, OnToolTipNeedText )
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBoard message handlers

BOOL CBoard::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.style |= WS_CLIPCHILDREN;
	cs.lpszClass = AfxRegisterWndClass(
		CS_HREDRAW|CS_VREDRAW, 
		NULL, NULL, NULL);

	return TRUE;
}

void CBoard::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	CRect rcClip; dc.GetClipBox( &rcClip );

	CDC dcMem; dcMem.CreateCompatibleDC( &dc );
	dcMem.SelectObject( &m_pic );

	// ���ۿ��� ������ �´�
	dc.BitBlt(
		rcClip.left, rcClip.top,
		rcClip.Width(), rcClip.Height(),
		&dcMem, rcClip.left, rcClip.top,
		SRCCOPY );
}

int CBoard::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd ::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO: Add your specialized creation code here

	// ���� ����
	m_pWrap = new CBoardWrap(this);

	CClientDC dc(this);

	// Ŀ�� �б�
	if ( !m_hArrow ) m_hArrow = AfxGetApp()->LoadStandardCursor( IDC_ARROW );
	if ( !m_hHand ) m_hHand = AfxGetApp()->LoadCursor( IDC_HAND );

	// ��Ʈ ����
	CreateFont();

	// ��� ī�� �׸� �б�
	VERIFY( m_bm.LoadAllCards( &dc, &m_fntSmall ) );

	// �޸� �׸� �б�
	if ( Mo()->nBackPicture != -1 )
		VERIFY( m_bm.LoadBackPicture( &dc, Mo()->nBackPicture ) );
	else {
		if ( !m_bm.LoadBackPicture( &dc, -1, Mo()->sBackPicturePath ) ) {
			AfxMessageBox( _T("�޸� �׸� ������ ���� �� �����ϴ�\n�ɼǿ��� Ȯ���� ������"),
				MB_OK|MB_ICONEXCLAMATION );
			Mo()->nBackPicture = 0;
			VERIFY( m_bm.LoadBackPicture( &dc, Mo()->nBackPicture ) );
		}
	}

	// ��� �׸� �б�
	if ( Mo()->bUseBackground ) {
		if ( !m_bm.LoadBackground( &dc, Mo()->sBackgroundPath ) ) {
			AfxMessageBox( _T("��� �׸� ������ ���� �� �����ϴ�\n�ɼǿ��� Ȯ���� ������"),
				MB_OK|MB_ICONEXCLAMATION );
			Mo()->bUseBackground = false;
		}
	}

	// ���� �����ϰ�
	VERIFY( EnableToolTips() );

	return 0;
}

void CBoard::OnSize(UINT nType, int cx, int cy) 
{
	CWnd ::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here

	// ������ ī�� ũ�⸦ �̸� ����ؼ� ������ �ִ´�
	m_szCard = GetCardZoom( Mo()->bZoomCard );

	// ��Ʈ�� �ٽ� �����
	CreateFont();

	// ���� �ִ� ���۰� ����� ũ�ٸ� ���� ���� �ʿ䰡 ����
	if ( !m_pic.GetSafeHandle() ||
		 !m_board.GetSafeHandle() ||
			m_szPic.cx < cx || m_szPic.cy < cy ) {

		CClientDC dc(this);

		if ( m_pic.GetSafeHandle() )
			m_pic.DeleteObject();
		m_pic.CreateCompatibleBitmap( &dc, cx, cy );

		if ( m_board.GetSafeHandle() )
			m_board.DeleteObject();
		m_board.CreateCompatibleBitmap( &dc, cx, cy );

		m_szPic.cx = cx; m_szPic.cy = cy;
	}

	// ��� DSB �� OnMove �� ȣ���Ѵ�
	POSITION pos = m_lpDSB.GetHeadPosition();
	while (pos) m_lpDSB.GetNext(pos)->OnMove();

	// �׸��� �׸���
	UpdatePlayer( -2 );
}

void CBoard::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default

	// DSB �� �˻��Ѵ�
	if ( CookDSBForLButtonDown( point ) ) ;	// �ٸ� ó���� �ǳʶ�

	// ī�带 �������̴�
	else if ( m_pWaitingSelectionEvent
		&& CookSelectionForLButtonDown( point ) ) ;

	else ;

	CWnd ::OnLButtonDown(nFlags, point);
}

void CBoard::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default

	// DSB �巡�� ���̸� �ϴ� �巡�׸� ����Ѵ�
	if ( m_bDragDSB ) {

		CRect  rc; m_pDragDSB->GetRect( &rc );

		// DSB �� �巡��
		m_pDragDSB->Offset( point - m_pntLastDSB );

		// ���� �簢���� ���ο� �簢���� �������� invalidate
		CRect rcUpdate = rc;
		m_pDragDSB->GetRect( &rc );
		rcUpdate.SetRect(
			min( rcUpdate.left, rc.left ), min( rcUpdate.top, rc.top ),
			max( rcUpdate.right, rc.right ), max( rcUpdate.bottom, rc.bottom ) );
		UpdateDSB( &rcUpdate, true );
		m_pntLastDSB = point;
	}

	// ī�带 �����ϱ� ���� ����ϴ� ���
	// ���콺 �Ʒ��ʿ� �ִ� ī�带 ���ݾ� �÷��ش�
	// (��� ���°� �ƴ� ��쿡��)
	else if ( m_lpModalDSB.IsEmpty() && CanSelect() )
		ShiftCard( point.x, point.y, true );

	// ī�尡 ���� �ö� �ִ� ��� �����ش�
	else if ( m_pMFSM && m_nMouseOver != -1 )
		ShiftCard( 0x80000000, 0x80000000, true );

	CWnd ::OnMouseMove(nFlags, point);
}

void CBoard::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default

	if ( m_bDragDSB ) {	// DSB �� �巡����
		m_bDragDSB = false;
		m_pDragDSB = 0;
		ReleaseCapture();
		ClipCursor( 0 );
	}

	CWnd ::OnLButtonUp(nFlags, point);
}

BOOL CBoard::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*message*/) 
{
	// TODO: Add your message handler code here and/or call default

	// DSB �� �˻��ؼ� �� ���� ���� Ŀ���� �ִٸ�
	// Ŀ���� �� ������� �ٲ۴�

	POINT point;	// ���� ��ġ
	GetCursorPos( &point );
	ScreenToClient( &point );

	POSITION pos = m_lpDSB.GetHeadPosition();
	for (; pos; m_lpDSB.GetNext(pos) ) {
		DSB* pDSB = m_lpDSB.GetAt(pos);

		// ��� �����̸� ��� DSB �� �˻��Ѵ�
		if ( !m_lpModalDSB.IsEmpty()
			&& pDSB != m_lpModalDSB.GetHead() ) continue;

		CRect rc; pDSB->GetRect( &rc );
		if ( rc.PtInRect( point ) ) {
			// pDSB ���� Ŀ���� ����
			if ( pDSB->DispatchHotspot( point.x, point.y, true) ) {
				// �� ���� ���� ����
				if ( m_pHLDSB && m_pHLDSB != pDSB )
					// ���϶���Ʈ ���̾��� DSB �� ��� ���·�
					m_pHLDSB->DispatchHotspot( 0x80000000, 0x80000000, true );
				SetCursor( m_hHand );
				m_pHLDSB = pDSB;
				return TRUE;
			}
			break;
		}
	}

	if ( m_pHLDSB )	// ���϶���Ʈ ���̾��� DSB �� ��� ���·�
		// ���� �� ��ǥ�� �� ������ ���� �ʰ���?
		m_pHLDSB->DispatchHotspot( 0x80000000, 0x80000000, true );
	if ( pos )
		m_pHLDSB = m_lpDSB.GetAt(pos);
	else m_pHLDSB = 0;

	// ��� ���°� �ƴϰ� ī�带 ���� ��
	if ( m_lpModalDSB.IsEmpty() && CanSelect() )
		SetCursor( m_hHand );
	// �� ���� ���
	else SetCursor( m_hArrow );

	return TRUE;
}

void CBoard::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default

	if ( nIDEvent >= (int)tiDSB ) {
		// DSB ��ȿ ����

		// �ش� DSB �� ã�´�
		DSB* pDSB = 0;
		POSITION pos = m_lpDSB.GetHeadPosition();
		while (pos) {
			DSB* p = m_lpDSB.GetNext(pos);
			if ( p->GetID() + tiDSB == (int)nIDEvent ) {
				pDSB = p;
				break;
			}
		}
		ASSERT( pDSB );

		// �� ó�� (Ÿ�̸Ӹ� ���̰� DSB�� ����)
		VERIFY( KillTimer( nIDEvent ) );
		if ( pDSB && !pDSB->IsPermanent() )
			pDSB->Destroy();
	}

	CWnd ::OnTimer(nIDEvent);
}

void CBoard::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	if ( nChar == (UINT)'\x001b' ) {
		// ESC �� ������
		if ( !m_lpModalDSB.IsEmpty() )
			m_lpModalDSB.GetHead()->OnESC();
		if ( !m_lpDSB.IsEmpty() )
			m_lpDSB.GetHead()->OnESC();
	}
	else if ( nChar == (UINT)'\x000d' ) {
		// Enter �� ������
		if ( !m_lpModalDSB.IsEmpty() )
			m_lpModalDSB.GetHead()->OnEnter();
		if ( !m_lpDSB.IsEmpty() )
			m_lpDSB.GetHead()->OnEnter();
	}
	CWnd ::OnChar(nChar, nRepCnt, nFlags);
}


// ���� Mighty FSM �� ��Ʈ�Ѵ�
// �̰��� ��Ʈ �ϴ� ���� �������� �������� ������ ���۵ȴ�
void CBoard::SetMFSM( CMFSM* pMFSM )
{
	m_nTurnRect = -1;	// ���� �簢��, �����Ѵ�

	// ���� �� �ִ� ��� DSB ���� ����
	POSITION pos = m_lpDSB.GetHeadPosition();
	while (pos) m_lpDSB.GetNext(pos)->Destroy();
	ASSERT( m_lpDSB.IsEmpty() );

	// ���� ��� ���µ� �����Ѵ�
	if ( m_pWaitingSelectionEvent )
		CancelSelection();

	m_pMFSM = pMFSM;

	if ( !m_pMFSM ) {
		// MFSM �� ����� ����� ���� DSB �� ����
		// �ʱ� DSB
		DStartUp* pDSB = new DStartUp(GetWrapper());
		pDSB->Create( 0 );
		Sb()->Update();
	}

	UpdatePlayer( -2, 0 );
}

void CBoard::OnUpdateGameExit(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_pMFSM ? TRUE : FALSE );
}

void CBoard::OnGameExit() 
{
	if ( m_pMFSM ) {
		m_pMFSM->EventExit();
		m_pMFSM = 0;
	}
}
