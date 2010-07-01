// Mighty.h : main header file for the MIGHTY application
//

#if !defined(AFX_MIGHTY_H__6433F782_C1B5_11D2_97F2_000000000000__INCLUDED_)
#define AFX_MIGHTY_H__6433F782_C1B5_11D2_97F2_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

// ������ �ʴ� �����
#define LAST_TURN_2MA 12
#define LAST_TURN 9
#define MAX_PLAYERS 7
#define MAX_SCORE_2MA 13
#define MAX_SCORE 20
#define HIGHSCORE_MAXLIMIT 25
#define DECK_SIZE 53
#define MIGHTY_VERSION 40

#define BASE_MONEY 100	// 1 �δ� �⺻ ���� ����
#define MONEY_UNIT 10	// BASE_MONEY 1 �� ���� ��

#define WM_BOARDWRAPPER		(WM_USER+10)
#define WM_UPDATESCOREBOARD	(WM_USER+11)
#define WM_TOOLTIPUPDATE	(WM_USER+12)
#define WM_INFOBARUPDATE	(WM_USER+13)
#define WM_CALLSOCKPROC		(WM_USER+14)

// �� ���� �⺻ ��
#define DEFAULT_TIP_CX	256
// ���� �׵θ� ����
#define TOOLTIP_BORDER_MARGIN 5

// �������� ���� �ð��� (nDelayOneTurn : 0 ~ 10)

// ���� �ܰ躰 �ɸ��� �ð� (0.2��)
#define DELAY_SUFFLESTEP		200
// �¾��� �� �� �����ֱ������ ���� �ð�
#define DELAY_SETUP_SUFFLING	( Mo()->nDelayOneTurn * 200 + 500 )
// ���̽��� ī�带 ��� �����ִ� �ð�
#define DELAY_DEALMISS			( Mo()->nDelayOneTurn * 200 * 2 + 500 )
// �缱�ڰ� ������ �� Ȯ�� �޽����� �����ֱ������ �ð�
#define DELAY_MASTERDECL		( Mo()->nDelayOneTurn * 200 * 2 + 500 )
// �����尡 ���� �� �� �˸� �޽����� �����ִ� �Ⱓ
#define DELAY_FRIENDCARD		( 4000 )
// ���� ������ �����ִ� �ð�
#define DELAY_ELECTIONEND		( Mo()->nDelayOneTurn * 200 * 2 + 2000 )
// Ż�� �޽����� �� ����� �����ֱ� ������ ��� �ð�
#define DELAY_KILL_AND_EFFECT	( Mo()->nDelayOneTurn * 200 * 2 + 500 )
// AI���� Ż�� �޽����� ������ �� ��� �ð�
#define DELAY_KILL				( 0 )
// AI���� ���� �޽����� ������ �� ��� �ð�
#define DELAY_GOAL				( Mo()->nDelayOneTurn * 200 * 1 + 500 )
// ���� DSB �� Ÿ�Ӿƿ�
#define DELAY_GOALDSBTIMEOUT	( Mo()->nDelayOneTurn * 200 * 2 + 500 )
// �⸶���� DSB �� Ÿ�Ӿƿ�
#define DELAY_GIVEUPDSBTIMEOUT	( Mo()->nDelayOneTurn * 200 * 2 + 500 )
// ��Ŀ/��Ŀ�� DSB �� �����ִ� �ð�
#define DELAY_JOKERDSB			( Mo()->nDelayOneTurn * 200 * 5 )

#include "resource.h"       // main symbols
#include "state.h"
#include "rule.h"
#include "option.h"
#include "card.h"
#include "msg.h"


class CInfoBar;
class CMightyToolTip;
class DScoreBoard;

/////////////////////////////////////////////////////////////////////////////
// CMightyApp:
// See Mighty.cpp for the implementation of this class
//

class CMightyApp : public CWinApp
{
public:
	CMightyApp();
	~CMightyApp();

	COption* GetOption() { return &m_option; }
	CInfoBar* GetInfoBar() { return m_pInfoBar; }
	CMightyToolTip* GetToolTip() { return m_pToolTip; }
	DScoreBoard* GetScoreBoard() { return m_pScoreBoard; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMightyApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation

public:
	//{{AFX_MSG(CMightyApp)
	afx_msg void OnAppAbout();
	afx_msg void OnAppOption();
	afx_msg void OnUpdateAppScoreboard(CCmdUI* pCmdUI);
	afx_msg void OnAppScoreboard();
	afx_msg void OnHelp();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	COption m_option;
	CInfoBar* m_pInfoBar;
	CMightyToolTip* m_pToolTip;
	DScoreBoard* m_pScoreBoard;
	bool m_bInBoss;
};

// �ɼ��� ��´�
inline COption* Mo()
{
	return ((CMightyApp*)AfxGetApp())->GetOption();
}

// ���� �����츦 ��´�
inline CWnd* Mw()
{
	return AfxGetApp()->m_pMainWnd;
}
// ���� �������� ��´�
class CMainFrame;
inline CMainFrame* Mf()
{
	return (CMainFrame*)AfxGetApp()->m_pMainWnd;
}

// ���¸��븦 ��´�
inline CInfoBar* Ib()
{
	return ((CMightyApp*)AfxGetApp())->GetInfoBar();
}

// ���� �����츦 ��´�
inline CMightyToolTip* Tt()
{
	return ((CMightyApp*)AfxGetApp())->GetToolTip();
}

// �������� ��´�
inline DScoreBoard* Sb()
{
	return ((CMightyApp*)AfxGetApp())->GetScoreBoard();
}

// ���� �����Ѵ�
inline CString FormatMoney( int nMoney, int nPlayers )
{
	CString sRet;
	if ( Mo()->bMoneyAsSupport )
		sRet.Format( _T("%d.%d%%"), nMoney * 100 / BASE_MONEY / nPlayers,
					nMoney * 1000 / BASE_MONEY / nPlayers % 10 );
	else sRet.Format( _T("%d"), nMoney * MONEY_UNIT );
	return sRet;
}

// ��ƿ��Ƽ��

// ��Ʈ���� ũ�⸦ ��´�
CSize GetBitmapSize( UINT idBitmap );
// ��Ʈ���� �׸���
void DrawBitmap( CDC* pDC, UINT idBitmap,
		int xTarget, int yTarget,
		int cxTarget = -1, int cyTarget = -1,
		int xSource = -1, int ySource = -1,
		int cxSource = -1, int cySource = -1 );
// ���带 �����Ѵ�
// ( bStop �� ���̸� ������ �Ҹ��� ���߰� ���� )
void PlaySound( UINT idWave, BOOL bStop = FALSE );


// CCriticalSection �� VERIFY ����� �߰��Ѵ�
class CMightyCriticalSection
{
public:
	void Lock() const { VERIFY( ((CCriticalSection&)m_cs).Lock() ); }
	void Unlock() const { VERIFY( ((CCriticalSection&)m_cs).Unlock() ); }
private:
	CCriticalSection m_cs;
};
#define CCriticalSection CMightyCriticalSection

// �����͸� �ڵ����� delete �ϴ� Ŭ����
template<class TYPE>
struct CAutoDelete
{
	CAutoDelete( TYPE** ptr ) : pp(ptr) {}
	~CAutoDelete() { if ( pp ) delete *pp; }
	void Attach( TYPE** t ) { delete *pp; pp = t; }
	TYPE** Detach() { TYPE** t = pp; pp = 0; return t; }
	TYPE** pp;
};

#define AUTODELETE_MSG( PTR ) CAutoDelete<CMsg> _adm##PTR( &PTR )
#define AUTODELETE_MSG_EX( PTR, VALUE ) CAutoDelete<CMsg> VALUE( &PTR )

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MIGHTY_H__6433F782_C1B5_11D2_97F2_000000000000__INCLUDED_)
