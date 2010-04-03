#if !defined(AFX_DSCOREBOARD_H__409EEE93_51F6_11D3_99D3_000000000000__INCLUDED_)
#define AFX_DSCOREBOARD_H__409EEE93_51F6_11D3_99D3_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DScoreBoard.h : header file
//

struct CState;

/////////////////////////////////////////////////////////////////////////////
// DScoreBoard dialog

class DScoreBoard : public CDialog
{
// Construction
public:
	DScoreBoard(CWnd* pParent = NULL);   // standard constructor

	// ���ھ� ���带 ���̰ų� �����
	// �����찡 �������� �ʾҴٸ� �����Ѵ�
	void Show( bool bShow = true );

	// ���� ���ھ Update �Ѵ�
	void Update( const CState* pState = 0 );

	// �־��� ��Ʈ�� �迭�� ���� �� ��Ʈ���� �����ϴ�
	// ���� �����Ѵ�
	int CalcMaxWidth( CString as[], int nCount, CDC* pDC );

	// ��ũ�ѹٸ� ����
	void RecalcScrollBar();

	bool m_bShowing;
	int m_nPos;
	int m_nPosMax;
	int m_nCategories;
	CString m_aasData[10 /*ALLCATEGORIES*/ ][MAX_PLAYERS+1];

// Dialog Data
	//{{AFX_DATA(DScoreBoard)
	enum { IDD = IDD_SCOREBOARD };
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DScoreBoard)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void raw_Update( const CState* pState );

	// Generated message map functions
	//{{AFX_MSG(DScoreBoard)
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	afx_msg void OnPaint();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	//}}AFX_MSG
	afx_msg LRESULT OnUpdateScoreBoard( WPARAM, LPARAM );
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DSCOREBOARD_H__409EEE93_51F6_11D3_99D3_000000000000__INCLUDED_)
