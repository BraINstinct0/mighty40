// Mighty.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Mighty.h"

#include "MightyToolTip.h"
#include "InfoBar.h"
#include "BmpMan.h"
#include "Board.h"
#include "BoardWrap.h"
#include "MainFrm.h"

#include "DSB.h"
#include "DEtc.h"

#include "DOption.h"
#include "DScoreBoard.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ��ƿ��Ƽ��

// ��Ʈ���� ũ�⸦ ��´�
CSize GetBitmapSize( UINT idBitmap )
{
	// ���� ���Ǵ� IDB_SA �� IDB_SPADE �� �̸� ������ �д�
	static CSize s_sz_IDB_SA(0,0);
	static CSize s_sz_IDB_SPADE(0,0);

	if ( idBitmap == IDB_SA && s_sz_IDB_SA.cy )
		return s_sz_IDB_SA;
	if ( idBitmap == IDB_SPADE && s_sz_IDB_SPADE.cy )
		return s_sz_IDB_SPADE;

	CBitmap bm;
	BITMAP bmInfo;
	VERIFY( bm.LoadBitmap( idBitmap ) );
	VERIFY( bm.GetBitmap( &bmInfo ) );

	if ( idBitmap == IDB_SA && !s_sz_IDB_SA.cx )
		s_sz_IDB_SA.cx = bmInfo.bmWidth,
		s_sz_IDB_SA.cy = bmInfo.bmHeight;
	if ( idBitmap == IDB_SPADE && !s_sz_IDB_SPADE.cx )
		s_sz_IDB_SPADE.cx = bmInfo.bmWidth,
		s_sz_IDB_SPADE.cy = bmInfo.bmHeight;

	VERIFY( bm.DeleteObject() );

	return CSize( bmInfo.bmWidth, bmInfo.bmHeight );
}

// ��Ʈ���� �׸���
void DrawBitmap( CDC* pDC, UINT idBitmap,
		int xTarget, int yTarget,
		int cxTarget, int cyTarget,
		int xSource, int ySource,
		int cxSource, int cySource )
{
	CDC dc;
	CBitmap bm, *pbmOld;
	BITMAP bmInfo;
	VERIFY( dc.CreateCompatibleDC( pDC ) );
	VERIFY( bm.LoadBitmap( idBitmap ) );
	VERIFY( bm.GetBitmap( &bmInfo ) );
	pbmOld = dc.SelectObject( &bm );

	pDC->SetTextColor( RGB(0,0,0) );
	pDC->SetBkColor( RGB(255,255,255) );
	VERIFY(
		pDC->StretchBlt( xTarget, yTarget,
			( cxTarget == -1 ? bmInfo.bmWidth : cxTarget ),
			( cyTarget == -1 ? bmInfo.bmHeight : cyTarget ),
			&dc,
			( xSource == -1 ? 0 : xSource ),
			( ySource == -1 ? 0 : ySource ),
			( cxSource == -1 ? bmInfo.bmWidth : cxSource ),
			( cySource == -1 ? bmInfo.bmHeight : cySource ),
			SRCCOPY )
	);

	dc.SelectObject( pbmOld );
	VERIFY( bm.DeleteObject() );
}

// ���带 �����Ѵ�
// ( bStop �� ���̸� ������ �Ҹ��� ���߰� ���� )
void PlaySound( UINT idWave, BOOL bStop )
{
	if ( Mo()->bUseSound )
		PlaySound( MAKEINTRESOURCE(idWave), AfxGetApp()->m_hInstance,
			SND_ASYNC|SND_NODEFAULT|SND_NOWAIT|SND_RESOURCE
			| (!bStop?SND_NOSTOP:0) );
}

/////////////////////////////////////////////////////////////////////////////
// CMightyApp

BEGIN_MESSAGE_MAP(CMightyApp, CWinApp)
	//{{AFX_MSG_MAP(CMightyApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_APP_OPTION, OnAppOption)
	ON_UPDATE_COMMAND_UI(ID_APP_SCOREBOARD, OnUpdateAppScoreboard)
	ON_COMMAND(ID_APP_SCOREBOARD, OnAppScoreboard)
	ON_COMMAND(ID_HELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMightyApp construction

CMightyApp::CMightyApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	m_pInfoBar = 0;
	m_pToolTip = new CMightyToolTip;
	m_bInBoss = false;
	m_pScoreBoard = 0;
}

CMightyApp::~CMightyApp()
{
	delete m_pToolTip;
	delete m_pScoreBoard;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CMightyApp object

CMightyApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CMightyApp initialization

BOOL CMightyApp::InitInstance()
{
	WSADATA wd;
	if ( WSAStartup( MAKEWORD( 1, 1 ), &wd ) != 0 ) {
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifndef _AFXDLL
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
#ifdef _DEBUG
	SetRegistryKey(_T("MightyDebug"));
#else
	SetRegistryKey(_T("Mighty"));
#endif
	Mo()->Load();

	// To create the main window, this code creates a new frame window
	// object and then sets it as the application's main window object.

	CMainFrame* pFrame = new CMainFrame;
	m_pMainWnd = pFrame;

	// create and load the frame with its resources

	pFrame->LoadFrame(IDR_MAINFRAME,
		WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL,
		NULL);

	// The one and only window has been initialized, so show and update it.
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();

	// �� ���� ��Ʈ�� ��Ҹ� ��� �����Ѵ�
	m_pInfoBar = (CInfoBar*)m_pMainWnd->GetDlgItem( AFX_IDW_STATUS_BAR );
	m_pScoreBoard = new DScoreBoard;
	m_pScoreBoard->Show(Mo()->bLastScoreboard);

	pFrame->GetView()->GetWrapper()->SetMFSM(0);

	// ó������ �� ���̶��, �ɼ��� �����ش�
	if ( Mo()->bInit ) {
		Mo()->bInit = false;
		PostMessage( pFrame->GetSafeHwnd(), WM_COMMAND, ID_APP_OPTION, 0 );
	}

	return TRUE;
}

int CMightyApp::ExitInstance() 
{
	VERIFY( WSACleanup() == 0 );

	// �ɼ� ����
	Mo()->Save();
	// �ٸ� �����尡 �������� ����Ѵ�
#ifndef _DEBUG
	Sleep(1000);
#endif
	return CWinApp::ExitInstance();
}

/////////////////////////////////////////////////////////////////////////////
// CMightyApp message handlers

// App command to run the dialog
void CMightyApp::OnAppAbout()
{
	(new DAbout(Mf()->GetView()->GetWrapper()))->Create();
}

void CMightyApp::OnAppOption() 
{
	// TODO: Add your command handler code here
	DOption opt;
	if ( opt.DoModal() == IDOK ) {

		// ȭ���� �ٽ� Update �Ѵ�
		CBoardWrap* pWrap = Mf()->GetView()->GetWrapper();
		pWrap->ReloadPlayerNames();
		pWrap->UpdatePlayer(-2);
	}
}

void CMightyApp::OnHelp() 
{
	// TODO: Add your command handler code here
	::WinHelp( 0, _T("Mighty.hlp"), HELP_CONTENTS, 0 );
}

void CMightyApp::OnUpdateAppScoreboard(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( Mo()->bLastScoreboard ? TRUE : FALSE );
}

void CMightyApp::OnAppScoreboard() 
{
	// TODO: Add your command handler code here
	if ( Mo()->bLastScoreboard ) m_pScoreBoard->Show(false);
	else m_pScoreBoard->Show(true);
}

BOOL CMightyApp::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class

	if ( ( pMsg->message == WM_LBUTTONDOWN || pMsg->message == WM_RBUTTONDOWN )
			&& m_pToolTip->GetSafeHwnd() ) {
		// ���� ��
		m_pToolTip->PostMessage( WM_CLOSE );
		return FALSE;
	}
	// ����Ű ó��
	else if ( pMsg && pMsg->message == WM_KEYDOWN
		&& pMsg->wParam == VK_PAUSE && !m_bInBoss ) {

		m_bInBoss = true;

		int nSelector;

		if ( Mo()->nBossType == 0 )		// ����
			nSelector = rand()%4;
		else nSelector = Mo()->nBossType - 1;

		CWnd* pWnd = AfxGetMainWnd();
		if ( !pWnd ) return FALSE;

		pWnd->ShowWindow( SW_MINIMIZE );

		if ( nSelector < 2 ) {
			pWnd->SetWindowText(_T("�� ��ǻ��"));
			pWnd->SetIcon( LoadIcon( IDI_MYCOM ), TRUE );
			pWnd->MessageBox(
				( nSelector % 2 ? _T("msimg32.dll��(��) ���� �� �����ϴ�: ���� �����Դϴ�.\n\n�����̳� ��� ���α׷��� �������ΰ� �����ϴ�.")
				: _T("���� ������ ���� ���� �����Դϴ�.\n���� ȭ���� ������ ���� �ִ� â�� ������ ǥ���Ͻʽÿ�.") ),
				_T("�� ��ǻ��"), MB_OK|MB_ICONSTOP|MB_SETFOREGROUND );
		}
		else {
			pWnd->SetWindowText(_T("Microsoft Internet Explorer"));
			pWnd->SetIcon( LoadIcon( IDI_IEX ), TRUE );
			if ( nSelector % 2 )
				pWnd->MessageBox(
					_T("�ٿ�ε� �Ϸ�"),
					_T("util.zip(www.microsoft.com)"),
					MB_OK|MB_ICONINFORMATION|MB_SETFOREGROUND );
			else
				pWnd->MessageBox(
					_T("http://www.microsoft.com ����Ʈ�� �� �� �����ϴ�.\n\n������ ������ �� �����ϴ�."),
					_T("Microsoft Internet Explorer"),
					MB_OK|MB_ICONSTOP|MB_SETFOREGROUND );
		}
		CString sTitle; sTitle.LoadString( AFX_IDS_APP_TITLE );
		pWnd->SetIcon( LoadIcon( IDR_MAINFRAME ), TRUE );
		pWnd->SetWindowText( sTitle );
		pWnd->ShowWindow( SW_RESTORE );

		m_bInBoss = false;

		return FALSE;
	}

	return CWinApp::PreTranslateMessage(pMsg);
}
