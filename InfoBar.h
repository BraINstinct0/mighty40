// InfoBar.h: interface for the CInfoBar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INFOBAR_H__BE37E2C6_C23D_11D2_97F3_000000000000__INCLUDED_)
#define AFX_INFOBAR_H__BE37E2C6_C23D_11D2_97F3_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// �Ʒ����� ���¹�
// ��Ƽ �÷��� ������ �� ä��â���ε� ���ȴ�

// ���� ���
// ����, ��ǥ����, ������ �ؽ�Ʈ�� ��Ʈ �� �� �ִ�

// ���¹� ���
// ���� ǥ�õ� �ؽ�Ʈ�� ���� ���� �ٸ� ǥ���� �� �ִ�

// ä��â ��� (ShowChat(true)�Լ��� ä��â ���� �ٲ۴�)
// �Էµ� �޽�����, ä�� �޽��� �ڵ鷯�� ���޵ȴ�
// (SetChatHandler �Լ�)

class CInfoBar : public CStatusBar  
{
public:
	CInfoBar();
	virtual ~CInfoBar();
	BOOL Create( CFrameWnd* pParent );
public:
	// ����� ���� �Լ���

	// �ʱ�ȭ
	void Reset()									{ SafeSendMessage( WM_INFOBARUPDATE, 0, 0 ); }

	// ����, ��ǥ����, ������ �ؽ�Ʈ ��Ʈ
	void SetKiruda( int nKiruda )					{ SafeSendMessage( WM_INFOBARUPDATE, 1, (LPARAM)nKiruda ); }
	void SetMinScore( int nMinScore, int nDefaultMinScore ) { SafeSendMessage( WM_INFOBARUPDATE, 2, (LPARAM)MAKELONG((WORD)nMinScore,(WORD)nDefaultMinScore) ); }
	void SetFriendText( LPCTSTR sFriendText )		{ SafeSendMessage( WM_INFOBARUPDATE, 3, (LPARAM)sFriendText ); }

	// ���� ����� ǥ�� : 100 ���� nPercentage ��ŭ��
	//                    �����Ǿ� ǥ�õȴ�
	void SetProgress( int nPercentage )				{ SafeSendMessage( WM_INFOBARUPDATE, 4, (LPARAM)nPercentage ); }
	// ǥ�õǴ� �޽���
	void SetText( LPCTSTR sText )					{ SafeSendMessage( WM_INFOBARUPDATE, 5, (LPARAM)sText ); }

	// ä�� ��Ʈ���� ǥ���ϰų� �����
	void ShowChat( bool bShow = true )				{ SafeSendMessage( WM_INFOBARUPDATE, 6, bShow ? 1 : 0 ); }
	// ä��â�� �Է� ��Ŀ���� �ش� ( ������ true )
	bool SetFocusToChat()							{ return SafeSendMessage( WM_INFOBARUPDATE, 7, 0 ) ? true : false; }

	// ä�� �޽��� �ڵ鷯�� ��Ʈ
	// ����(pfnProc==0) �ϱ� ���ؼ��� ������ dwUser ����
	// ��ġ�ؾ� �Ѵ�
	void SetChatHandler( DWORD dwUser,
		void (*pfnProc)( LPCTSTR sMsg, DWORD dwUser ) ) { CHATHANDLERINFO chi = { dwUser, pfnProc }; SafeSendMessage( WM_INFOBARUPDATE, 8, (LPARAM)(LPCVOID)&chi ); }

protected:
	// �������̵�
	virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );
	afx_msg void OnSize( UINT nType, int cx, int cy );
	virtual BOOL OnCommand( WPARAM wParam, LPARAM lParam );
	virtual INT_PTR OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
	virtual BOOL PreTranslateMessage( MSG *pMsg );
	afx_msg BOOL OnToolTipNeedTextW( UINT id, NMHDR * pNMHDR, LRESULT * pResult );
	afx_msg BOOL OnToolTipNeedTextA( UINT id, NMHDR * pNMHDR, LRESULT * pResult );
	LRESULT SafeSendMessage( UINT u, WPARAM w, LPARAM l ) { return GetSafeHwnd() ? SendMessage( u, w, l ) : 0; }
	LRESULT OnInfoBarUpdate( WPARAM, LPARAM );
	afx_msg LRESULT OnSetText( WPARAM, LPARAM lParam );
	DECLARE_MESSAGE_MAP()

protected:
	// ������ �ؽ�Ʈ ���� �׸���
	void DrawMainPane( CDC* pDC );

protected:
	// ����
	int m_nKiruda;
	// ��ǥ����, �⺻����
	int m_nMinScore;
	int m_nDefaultMinScore;
	// �����
	int m_nPercentage;
	// ������ �ؽ�Ʈ
	CString m_sFriend;
	// ä��â
	CEdit m_cChat;
	// ä�� ����ΰ�
	bool m_bChat;
	// ���� ����, ��ǥ����, ������ �ؽ�Ʈ�� ������ ���� ��Ʈ���� �����Ѵ�
	CString GetGoalString() const;

	// ä�� �ڵ鷯 ����
	struct CHATHANDLERINFO {
		DWORD dwUser;
		void (*pfnProc)( LPCTSTR sMsg, DWORD dwUser );
	} m_handler;

	struct CChatBuffer;
	CChatBuffer* m_pChatBuffer;
};

#endif // !defined(AFX_INFOBAR_H__BE37E2C6_C23D_11D2_97F3_000000000000__INCLUDED_)
