// DConnect.h: interface for the DConnect class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DCONNECT_H__2BCC6B66_8ECD_11D3_9A8E_000000000000__INCLUDED_)
#define AFX_DCONNECT_H__2BCC6B66_8ECD_11D3_9A8E_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CPlayerSocket;
class DConnectPopup;

class DConnecting;

// ����â���� ����ϴ� �޺��ڽ��� ����Ʈ�� Subclassing �ϴ� Ŭ����
struct DConnectingComboEdit : public CEdit
{
	DConnecting* m_pParent;
	afx_msg void OnChar( UINT nChar, UINT nRepCnt, UINT nFlags );
	void SubclassCombobox( CComboBox& box );
	DECLARE_MESSAGE_MAP()
};

// ���� â
class DConnecting : public DSB
{
public:
	DConnecting( CBoardWrap* pBoard ) : DSB(pBoard), m_pSocket(0) {}
	virtual ~DConnecting();

	void Create();

public:
	CPlayerSocket* DetachSocket()					{ CPlayerSocket* pRet = m_pSocket; m_pSocket = 0; return pRet; }

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

	virtual void Destroy();

protected:
	CPlayerSocket* m_pSocket;
	// ok ��ư ����
	COLORREF m_colOk;
	// �޺�
	CComboBox m_combo;
	DConnectingComboEdit m_edit;	// m_combo �� Edit â
	// ���� ����(�����ڷ�'��' ������ ������ ����)
	bool m_spectator;
	// ���� �ڵ鷯
	static void SockProc( CPlayerSocket* pSocket, CMsg* pMsg,
							DWORD dwUser1, DWORD dwUser2 );
	void SockProc( CMsg* pMsg );
	void Fail( LPCTSTR sMsg );

	friend DConnectingComboEdit;
};


// ���� �޽����� ����� �Ŀ�, pToKill �� ���ְ� /* DStartUp �� ����Ѵ� */
// DConnect�� ����ϴ� ������ �ٲپ��� (v4.0: 2011.1.7)
class DConnectFail : public DMessageBox
{
public:
	DConnectFail( CBoardWrap* pBoard )
		: DMessageBox( pBoard ) {}
	void Create( LPCTSTR s1, LPCTSTR s2, LPCTSTR s3, DSB* pToKill ) {
		m_pToKill = pToKill;
		SetModal();
		LPCTSTR s[3] = { s1, s2, s3 };
		DMessageBox::Create( true, s3 ? 3 : s2 ? 2 : 1, s );
		::MessageBeep( MB_ICONEXCLAMATION );
	}
	virtual void OnClick( LPVOID ) {
		if ( m_pToKill ) m_pToKill->Destroy();
		(new DConnecting(m_pBoard))->Create();
		Destroy();
	}
protected:
	DSB* m_pToKill;
};

// �ٸ� �÷��̾��� ������ ��ٸ��� ���� â
// �Ǵ� ������ �����Ͽ� ���� ���¸� ���� Ŭ���̾�Ʈ â
class DConnect : public DSB, public CZSocket  
{
public:
	DConnect( CBoardWrap* pBoard );
	virtual ~DConnect();

	// pServerSocket �� 0 �̸� �� DSB �� ������ ����DSB
	// 0 �� �ƴϸ�, pServerSocket �� ������ �ϴ�
	// Ŭ���̾�Ʈ DSB
	void Create( CPlayerSocket* pServerSocket, long players, bool spectatorOnly );

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

protected:

	// ���� ����
	bool m_bServer;
	CPlayerSocket* m_pServerSocket;
	// �ڽ��� UID
	long m_uid;

	// ȣ��Ʈ �ּ�
	CString m_sAddress;
	// ���Ǵ� ��Ģ
	CString m_sRule;
	CRule m_rule;
	// ���� ���ӵǾ� �ִ� ������� ����
	struct PLAYERINFO {
		CString sName;	// �̸�
		CString sInfo;	// ����(���), �Ǵ� AI��(��ǻ��)
		long dfa[4];	// ���� & ��
		bool bComputer;	// ��ǻ���ΰ�?
		CPlayerSocket* pSocket;
	}
	// v4.0: ������ ���� 20����� (2011.1.7)
	m_aInfo[MAX_CONNECTION];
	// v4.0: ������ �� (2011.1.7)
	int m_nSpectators;
	TCHAR m_specstr[10];
	// ä��â ����
	TCHAR (*m_asChatData)[256];
	COLORREF *m_acolChatData;
	DWORD m_nChatDataBegin;	// (���� ��ġ)
	DWORD m_nChatDataEnd;
	// ä��â �簢��
	CRect m_rcChat;
	// ���� ��ũ�� ����
	COLORREF m_acolMark[MAX_PLAYERS]; // v4.0: ���� ������ٰ� �� �� ƨ��� ���� ���� (2010.4.6)
	// �˾� �޴�
	DConnectPopup* m_pPopup;
	// ������Ʈ (�̻������ ������ ���ϰ� ����)
	CList<CString,CString&> m_lBlackList;

protected:
	// ��ƿ��Ƽ (����)

	// m_acolMark �� update ��
	void UpdateMarks();
	// ȸ���Ұ����� ������ �߻����� �� ȣ��
	void Fail( LPCTSTR sMsg1, LPCTSTR sMsg2 = 0 );

	// ���۹�ư�� �ֽ��̿� ���
	void RegisterOk();
	// ����� �ٴ� ��ȣ�� ���
	void RegisterMarks();
	// ��Ģ�� ���
	void RegisterRule();
	// ������ ��ư�� ���
	void RegisterSpec();

	// ä�� �޽����� ����(new)
	CMsg* CreateChatMsg( long uid, LPCTSTR sMsg );
	// uid ��ġ�� �÷��̾ ��ǻ�ͷ� ġȯ
	void SetComputer( long uid, int money );

	// ������ ���� (�����μ�)
	bool BeginServer();
	// ������ ���� (Ŭ���̾�Ʈ�μ�)
	bool BeginClient();

protected:
	// �������� ȣ���ϴ� ��ƿ��Ƽ��

	// ���� ������ �÷��̾�鿡�� �޽����� ����
	void SendToAll( CMsg* pMsg, long uidExcept = -1 );
	// ���� m_aInfo ��Ȳ�� ����ϴ� CMsg ��ü�� ����(new)
	// ��, mmPrepare �޽���
	CMsg* CreateStateMsg();
	// m_aInfo �� i ��° �÷��̾ ���� CMsg ��ü�� ����(new)
	// ��, mmChanged �޽���
	CMsg* CreatePlayerInfoMsg( long uid );
	// mmInit �޽����� ����(new) : v4.0: players�� �������� �ٲ� (2010.5.23)
	CMsg* CreateInitMsg( long players );
	// mmUID �޽����� ����(new)
	CMsg* CreateUIDMsg( long uid );
	// Ư�� �÷��̾ ���� ���� ȣ���� ����
	// (Computer �� �ٲٰ� �ٸ� �÷��̾�� ����)
	// ( bAccessDenied �� ���̸�, �� �÷��̾��
	// �߹濡 ���� �����Ǵ°��� )
	void FailedForPlayer( long uid, bool bAccessDenied = false );
	// �÷��̾ �ϳ� �߰�
	// ���ϵǴ� ���� uid, -1 �̸� ������� ����, -2 �̸� ���� �̸� ����, -3 �̸� ��� �ź�, -4 �̸� ������ �ź�
	// pMsg : mmNewPlayer �޽���
	long AddPlayer( CMsg* pMsg, CPlayerSocket* pSocket = 0 );
	// �÷��̾ ���� ( bAccessDenied �� ���̸�, �� �÷��̾��
	// �߹濡 ���� �����Ǵ°��� )
	void RemovePlayer( long uid, bool bAccessDenied = false );

	// ���� ��û
	virtual void OnAccept( int nErr );

	// ���� �ڵ鷯
	static void ServerSockProc( CPlayerSocket* pSocket, CMsg* pMsg,
								DWORD dwUser1, DWORD dwUser2 );
	void ServerSockProc( long uid, CMsg* pMsg, CPlayerSocket* pSocket );

protected:
	// Ŭ���̾�Ʈ���� ȣ���ϴ� ��ƿ��Ƽ��

	// mmNewPlayer �޽����� ����
	CMsg* CreateNewPlayerMsg( long players, bool spectatorOnly );
	// mmUID �޽����� �����Ͽ� Update
	bool ReceiveUIDMsg( CMsg* pMsg );
	// mmPrepare �޽����� ����
	bool ReceiveStateMsg( CMsg* pMsg );
	// mmChanged �޽����� ����
	bool ReceivePlayerInfoMsg( CMsg* pMsg );
	// mmChat �޽����� ����
	bool ReceiveChatMsg( CMsg* pMsg );

	// ���� �ڵ鷯
	static void ClientSockProc( CPlayerSocket* pSocket, CMsg* pMsg,
								DWORD dwUser1, DWORD dwUSer2 );
	void ClientSockProc( CMsg* pMsg );

protected:
	// ä��â ����

	// ä�� ȭ�鿡 sMsg �� �׸��� (uid==-1 �̸� ���� �޽���)
	void Chat( LPCTSTR sMsg, long uid = -1, bool bUpdate = true );

	// ä��â �ڵ鷯
	static void ChatProc( LPCTSTR sMsg, DWORD dwUser );
	void ChatProc( LPCTSTR sMsg );

public:
	// DConnectPopup �� ȣ���ϴ� �Լ�

	// �÷��̾ �پ��ִ� ���� ��ũ ��ġ�� ��´�
	CPoint GetMarkPos( long uid ) const
	{	return DSBtoDP( CPoint( 1, ( m_rule.nPlayerNum == 6 ? 5 : m_rule.nPlayerNum == 7 ? 4 : 7 ) + uid*3 + 1 ) ); }
	// ������ ��ư ��ġ�� ��´�
	CPoint GetSpecPos( ) const
	{	return DSBtoDP( CPoint( 25, m_rule.nPlayerNum >= 6 ? 9 : 11 ) ); }
	// m_pPopup �� Ŭ�����Ѵ�
	void ClearPopup() { m_pPopup = 0; }
	// OnClick �� ȣ���Ѵ�
	void Click( LPVOID pVoid ) { OnClick( pVoid ); }
};


// DConnect ���� ������ ���� �޴�
// uid�� -1�� ���� ������ ��ư�� ���� ����̴�.
class DConnectPopup : public DSelect
{
public:
	DConnectPopup( DConnect* pOuter, CBoardWrap* pBoard )
		: DSelect(pBoard), m_pOuter(pOuter) {}
	virtual ~DConnectPopup()
	{	// �� 213 : uid==1 && result==3
		m_pOuter->Click( (LPVOID)( 200 + m_uid*10 + m_nResult ) );
		m_pOuter->ClearPopup();
	}

	void Create( long uid )
	{
		m_uid = uid;
		SetModal();
		if ( uid == -1 || uid == -2 ) {
			m_uid = -1;
			CPoint pt = m_pOuter->GetSpecPos();
			DSelect::Create( pt.x, pt.y, const_cast<LPCTSTR*>(uid == -1 ? s_atText1 : s_atText2), 3, 0, &m_nResult );
		}
		else {
			CPoint pt = m_pOuter->GetMarkPos( uid );
			DSelect::Create( pt.x, pt.y, s_asText, 3, 0, &m_nResult );
		}
	}

protected:
	DConnect* m_pOuter;
	long m_nResult;
	long m_uid;
	static LPCTSTR s_asText[];

public:
	static LPTSTR s_atText1[];
	static LPTSTR s_atText2[];
};

#endif // !defined(AFX_DCONNECT_H__2BCC6B66_8ECD_11D3_9A8E_000000000000__INCLUDED_)
