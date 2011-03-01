// DConnect.cpp: implementation of the DConnect class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mighty.h"

#include "BoardWrap.h"
#include "DSB.h"
#include "DEtc.h"
#include "DStartUp.h"

#include "MFSM.h"
#include "ZSocket.h"
#include "PlayerSocket.h"
#include "SocketBag.h"

#include "DConnect.h"
#include "DRule.h"
#include "Option.h"
#include "InfoBar.h"


#include "Play.h"
#include "Player.h"
#include "PlayerHuman.h"
#include "PlayerNetwork.h"
#include "PlayerMai.h"
#include "PlayerDummy.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// ä�� ���� ���μ�
#define CHAT_LINES  6

// ��ƿ��Ƽ��

static CString format_score( int d, int f, int a )
{
	CString sRet;
	sRet.Format( _T("%d/%d %d/%d %d/%d"),
		LOWORD(d), HIWORD(d), LOWORD(f), HIWORD(f), LOWORD(a), HIWORD(a) );
	return sRet;
}

static CString get_name( LPCTSTR sPath )
{
	if ( !sPath || !*sPath ) return CString(_T("<�⺻ AI>"));
	LPCTSTR sSlash = _tcsrchr( sPath, _T('\\') );
	if ( !sSlash ) return CString(sPath);
	else return CString( sSlash + 1 );
}

static void get_host( CString& s )
{
	CZInetAddr addr;
	if ( !CZInetAddr::GetHostAddr( &addr )
		|| !addr.GetAddr( &s ) ) {
		s = _T("Unknown");
	}
}

// �޺��ڽ����� �ּҸ� �Ľ��� �´�
static void parse_address( CComboBox& combo, CString& sAddr, UINT& uPort )
{
	CString sFull;
	combo.GetWindowText( sFull );

	LPCTSTR sColon = _tcschr( sFull, _T(':') );
	if ( !sColon ) {
		// : �� �����Ƿ� sFull �� �� sAddr �̴�
		sAddr = sFull;
		uPort = (UINT) Mo()->nPort;
	}
	else {
		// : ������ �����Ѵ�
		_tcsncpy( sAddr.GetBuffer( sColon - sFull ), sFull,
					sColon - sFull );
		sAddr.ReleaseBuffer( sColon - sFull );
		uPort = (UINT) _ttoi( sColon + 1 );
	}
}

static CString create_entermsg( LPCTSTR sName, long d, long f, long a, bool isSpectator )
{
	int w = (int)LOWORD(d)+(int)LOWORD(f)+(int)LOWORD(a);
	int l = (int)HIWORD(d)+(int)HIWORD(f)+(int)HIWORD(a);

	CString sEnterMsg;
	if(isSpectator)
		sEnterMsg.Format( _T("%s ���� �����ڷ� �����Ͽ����ϴ�"), sName);
	else
	{
		sEnterMsg.Format( _T("%s ���� �����Ͽ����ϴ� - ")
			_T("�ְ�:%d��%d�� ������:%d��%d�� �ߴ�:%d��%d�� �·�:%d%%"),
			sName, LOWORD(d), HIWORD(d),
			LOWORD(f), HIWORD(f), LOWORD(a), HIWORD(a),
			(w+l) == 0 ? 0 : w*100/(w+l) );
	}
	return sEnterMsg;
}

static void update_addrlist( LPCTSTR sAddr )
{
	int i;
	// sAddr �� ���� �ּҰ� �ֳ� ������ ��, ������ �� �ּҸ� ���ش�
	// ������ �� ���� �ּҸ� ���ش�
	for ( i = ADDRESSBOOK_SIZE - 1; i >= 0; i-- )
		if ( Mo()->asAddressBook[i] == sAddr ) break;

	if ( i < 0 ) i = ADDRESSBOOK_SIZE - 1;

	for ( ; i > 0; i-- )
		Mo()->asAddressBook[i] = Mo()->asAddressBook[i-1];

	Mo()->asAddressBook[0] = sAddr;
}


LPCTSTR DConnectPopup::s_asText[] = {
	_T("���"), _T("�� �÷��̾ �߹�"), _T("�� ���ӿ��� ���������� �߹�")
};

LPTSTR DConnectPopup::s_atText1[] = {
	_T("���"), _T("������ ���� �ź�"), _T("������ ���� �߹�")
};

LPTSTR DConnectPopup::s_atText2[] = {
	_T("���"), _T("������ ���� ���"), _T("������ ���� �߹�")
};

/////////////////////////////////////////////////////////////////////////////
// DConnecting

BEGIN_MESSAGE_MAP( DConnectingComboEdit, CEdit )
	ON_WM_CHAR()
END_MESSAGE_MAP()

// Combo ���� ���Ͱ� �������� OnClick ���� ó��
void DConnectingComboEdit::OnChar( UINT nChar, UINT nRepCnt, UINT nFlags )
{
	if ( (TCHAR)nChar  == _T('\x0d') )
		m_pParent->OnClick( (LPVOID)0xffffffff );
	else if ( (TCHAR)nChar == _T('\x1b') )
		m_pParent->OnClick( (LPVOID)0 );
	else CEdit::OnChar( nChar, nRepCnt, nFlags );
}

void DConnectingComboEdit::SubclassCombobox( CComboBox& box )
{
	HWND hWnd = ::GetWindow( box.GetSafeHwnd(), GW_CHILD );
	while ( hWnd ) {
		if ( ::SendMessage( hWnd, WM_GETDLGCODE, 0, 0 ) & DLGC_WANTCHARS ) {
			VERIFY( SubclassWindow( hWnd ) );
			break;
		}
		else ::GetWindow( box.GetSafeHwnd(), GW_HWNDNEXT );
	}
}

DConnecting::~DConnecting()
{
	m_combo.DestroyWindow();
	delete m_pSocket;
}

void DConnecting::Create()
{
	// ���� �޺����ڸ� �����Ѵ�
	m_combo.Create( WS_CHILD | WS_VISIBLE
		| CBS_AUTOHSCROLL | CBS_DROPDOWN | CBS_HASSTRINGS,
		CRect(0,0,0,0), *m_pBoard, 0 );
	m_combo.SetFocus();

	m_edit.m_pParent = this;
	m_edit.SubclassCombobox( m_combo );

	// �޺� �ڽ���, �ּ� �����丮�� �����Ѵ� (�ֱٰͺ���)
	for ( int i = 0; i < ADDRESSBOOK_SIZE; i++ )
		if ( !Mo()->asAddressBook[i].IsEmpty() )
			m_combo.AddString( Mo()->asAddressBook[i] );
	m_combo.SetCurSel( 0 );

	m_colOk = s_colCyan;

	SetFixed();
	SetModal();
	DSB::Create( 0, 0, 20, 13, -1 );
}

void DConnecting::Destroy()
{
	m_combo.ShowWindow( SW_HIDE );
	DSB::Destroy();
}

// �ʱ�ȭ ( ������ �� ȣ��� )
void DConnecting::OnInit()
{
	DSB::OnInit();

	// v4.0���� �߰�(2010.1.7)
	m_spectator = false;
	RegisterHotspot( 3, 9, -1, -1, true, 0, _T("���� ����: �÷��̾�"),
		&s_colWhite, &s_tdShade, &s_colWhite, &s_tdOutline,
		(LPVOID)1 );
	RegisterHotspot( 4, 11, -1, -1, true, 0, _T("  Ȯ��  "),
		&m_colOk, &s_tdShade, &m_colOk, &s_tdOutline,
		(LPVOID)0xffffffff );
	RegisterHotspot( 12, 11, -1, -1, true, 0, _T("  ���  "),
		&s_colCyan, &s_tdShade, &s_colCyan, &s_tdOutline,
		(LPVOID)0 );
}

// CBoard �� ȣ���ϴ� �׸��� �Լ� - ��µ� shade-box �� �� �Լ���
// �������̵� �Ͽ� �����ؾ� �Ѵ�
// ���̽� �Լ��� ������ ��ü�� ���� ���� ������ �׸���
void DConnecting::OnDraw(
	CDC* pDC,
	bool bBrushOrigin )
{
	DSB::OnDraw( pDC, bBrushOrigin );

	CRect rcExam;

	if ( !m_pSocket ) {	// �������� �ƴ� ��

		// ����
		PutText( pDC, _T("������ ������ �ּҸ� �Է��ϼ���"), 2, 1, true,
			s_colWhite, s_tdShade );
		rcExam = PutText( pDC, _T("����: �ּ�(:��Ʈ��ȣ)"), 2, 3, true,
			s_colWhite, s_tdShade );
		PutText( pDC, _T("��1: 123.45.123.45"), 2, 4, true,
			s_colWhite, s_tdShade );
		PutText( pDC, _T("��2: www.snu.ac.kr:4111"), 2, 5, true,
			s_colWhite, s_tdShade );
	}
	else {	// ������

		rcExam = PutText( pDC, _T("�Ʒ� �ּҷ� ������..."), 2, 3, true,
			s_colWhite, s_tdShade );
	}

	// �޺� �ڽ��� ������ ��ġ�� �ű��
	CRect rcLast;
	m_combo.GetWindowRect( &rcLast );
	m_combo.GetParent()->ScreenToClient( &rcLast );

	CRect rc; GetRect(&rc);
	CRect rcCombo( CPoint( rc.left + rc.Width()/8,
						rcExam.bottom + GetDSBUnit()*3 ),
					CSize( rc.Width()*3/4, rc.Height() ) );

	if ( rcLast.TopLeft() != rcCombo.TopLeft() ) {
		m_combo.SetWindowPos( 0, rcCombo.left, rcCombo.top,
			rcCombo.Width(), rcCombo.Height(),
			SWP_NOZORDER | SWP_NOREDRAW | SWP_SHOWWINDOW | SWP_NOACTIVATE );
	}
}

// �� ������ Ŭ���� �� �Ҹ��� �Լ�
// LPVOID ���� 0 �̸� ���� (X�ڸ� Ŭ��) ��û
void DConnecting::OnClick( LPVOID pVoid )
{
	switch ( (int)pVoid ) {

	case 0: {
		(new DStartUp(m_pBoard))->Create( 1 );
		if ( m_pSocket ) {
			m_pSocket->ClearTrigger();
			delete m_pSocket; m_pSocket = 0;
		}
		Destroy();
		break;
	}

	case 0xffffffff: {	// ���� !

		// �ּ�
		CString sAddr; UINT uPort;
		parse_address( m_combo, sAddr, uPort );
		CZInetAddr addr( sAddr, uPort );

		// ���ο� ������ ����
		if ( m_pSocket ) {
			ASSERT(0);
			m_pSocket->ClearTrigger();
			delete m_pSocket;
		}
		m_pSocket = new CPlayerSocket();

		if ( !m_pSocket->CreateClientSocket(addr) ) {
			// ���� ����
			delete m_pSocket; m_pSocket = 0;
			(new DConnectFail(m_pBoard))->Create(
				_T("������ �����Ͽ����ϴ� !"), 0, 0, 0 );
			Destroy();
		}
		else {
			// ���� �õ��� (DSB �� Disable �Ѵ�)
			m_combo.EnableWindow( FALSE );
			Ib()->SetFocusToChat();
			m_colOk = s_colGray;	// ok ��ư�� ȸ������

			CRect rc; GetRect( &rc );
			m_pBoard->UpdateDSB( &rc );

			// �ڵ鷯�� ��ġ
			m_pSocket->SetTrigger( (DWORD)(LPVOID)this, 0, SockProc );
		}

		break;
	}
	case 1: {	// ���� ���� ����
		m_spectator = !m_spectator;
		DeleteHotspot( (LPVOID)1 );
		if(m_spectator)
			RegisterHotspot( 3, 9, -1, -1, true, 0, _T("���� ����: ������"),
				&s_colYellow, &s_tdShade, &s_colYellow, &s_tdOutline,
				(LPVOID)1 );
		else RegisterHotspot( 3, 9, -1, -1, true, 0, _T("���� ����: �÷��̾�"),
				&s_colWhite, &s_tdShade, &s_colWhite, &s_tdOutline,
				(LPVOID)1 );

		CRect rc; GetRect( &rc );
		m_pBoard->UpdateDSB( &rc );
	}
	}
}

// ���� �ڵ鷯
void DConnecting::SockProc( CPlayerSocket* pSocket, CMsg* pMsg,
						DWORD dwUser1, DWORD dwUser2 )
{
	ASSERT( pSocket == ((DConnecting*)(LPVOID)dwUser1)->m_pSocket );
	pSocket, dwUser2;	// unused
	((DConnecting*)(LPVOID)dwUser1)->SockProc( pMsg );
}

void DConnecting::Fail( LPCTSTR sMsg )
{
	if ( m_pSocket ) {
		m_pSocket->ClearTrigger();
		delete m_pSocket; m_pSocket = 0;
	}

	(new DConnectFail(m_pBoard))->Create(
		sMsg ? sMsg : _T("������ ����Ǿ����ϴ�"), 0, 0, 0 );
	Destroy();
}

void DConnecting::SockProc( CMsg* pMsg )
{
	AUTODELETE_MSG(pMsg);

	if ( pMsg->GetType() == CMsg::mmConnected ) {
		// �ܰ� 1 : ���� ���
		long nErr;
		if ( !pMsg->PumpLong( nErr )
			|| !pMsg->PumpLong( nErr )
			|| nErr != 0 )
			Fail( _T("������ �����Ͽ����ϴ� !") );
		else {
			CString sAddr; m_combo.GetWindowText( sAddr );
			update_addrlist( sAddr );	// �ּҷϿ� ���

			m_pSocket->SetTrigger( (DWORD)(LPVOID)this, 0, SockProc );
		}
	}
	else if ( pMsg->GetType() == CMsg::mmInit ) {
		// �ܰ� 2 : ������ ������ ����
		// ���� DConnect DSB �� ��� �� �ִ�
		long nVer, nPlayers;
		if ( !pMsg->PumpLong( nVer )
			|| !pMsg->PumpLong( nVer )
			|| !pMsg->PumpLong( nPlayers ) 
			|| nVer != MIGHTY_VERSION )
			Fail( _T("�������� ������ �ٸ��ϴ� !") );
		else {
			(new DConnect(m_pBoard))->Create( DetachSocket(), nPlayers, m_spectator );
			Destroy();
		}
	}
	else if ( pMsg->GetType() == CMsg::mmDisconnected ) {
		// �����κ����� ���� ����
		Fail( _T("�����κ��� ������ ���������ϴ� !") );
	}
	else if ( pMsg->GetType() == CMsg::mmError ) {
		// �����κ����� ���� �޽���
		CString sMsg;
		long lDummy;
		if ( pMsg->PumpLong( lDummy )
			&& pMsg->PumpString( sMsg ) )
			Fail( sMsg );
		else Fail( _T("�����κ��� �����ڵ尡 ���ŵǾ����ϴ� !") );
	}
	else {
		// �𸣴� �̻��� �޽���
		ASSERT(0);
		m_pSocket->SetTrigger( (DWORD)(LPVOID)this, 0, SockProc );
	}
}


/////////////////////////////////////////////////////////////////////////////
// DConnect

DConnect::DConnect( CBoardWrap* pBoard ) : DSB(pBoard)
{
	m_asChatData = new TCHAR[CHAT_LINES][256];
	m_acolChatData = new COLORREF[CHAT_LINES];

	m_bServer = true;
	m_pServerSocket = 0;
	m_uid = 0;

	for ( int i = 0; i < MAX_CONNECTION; i++ ) {
		m_aInfo[i].bComputer = true;
		m_aInfo[i].pSocket = 0;
	}
	UpdateMarks();

	m_nChatDataBegin = 0;
	m_nChatDataEnd = 0;

	// ä��â �簢�� ( 31 x CHAT_LINES )
	m_rcChat.SetRect( 1, 26, 32, 26+CHAT_LINES );

	m_pPopup = 0;
}

DConnect::~DConnect()
{
	// ä��â �ڵ鷯�� Ŭ����
	Ib()->SetChatHandler( (DWORD)(LPVOID)this, 0 );
	if ( m_pPopup )
		m_pPopup->Destroy();
	ASSERT( !m_pPopup );	// popup �� ������ ������

	if ( m_pServerSocket ) {
		// �ڵ鷯�� ���� ������ �ݴ´�
		m_pServerSocket->ClearTrigger();
		delete m_pServerSocket; m_pServerSocket = 0;
	}
	m_pServerSocket = 0;

	for ( int i = 0; i < MAX_CONNECTION; i++ )
		if ( m_aInfo[i].pSocket ) {
			m_aInfo[i].pSocket->ClearTrigger();
			delete m_aInfo[i].pSocket;
		}

	delete[] m_asChatData;
	delete[] m_acolChatData;
}

void DConnect::Create( CPlayerSocket* pServerSocket, long players, bool spectatorOnly )
{
	// �ϴ� ������ �ϰ� ����, �� �����߿� ������ ���� ������
	// Fail() �� ȣ���Ѵ� (Fail()�� ������ ���¸� �����ϹǷ�)
	bool bFailed = false;
	m_nSpectators = 0;

	UINT uPort;

	if ( pServerSocket ) {
		m_pServerSocket = pServerSocket;
		m_bServer = false;
	}
	else m_bServer = true;

	if ( m_bServer ) {

		// ���� ������ ����
		CZInetAddr addr( Mo()->sAddress, Mo()->nPort );
		if ( !CreateServerSocket(addr) )
			bFailed = true;

		// �� ȣ��Ʈ�� �ּҸ� ��´�
		uPort = (UINT)Mo()->nPort;
		CString sAddr;
		get_host( sAddr );
		m_sAddress.Format( _T("%s:%u"), sAddr, uPort );

		// ��Ģ ��Ʈ��
		if ( Mo()->nPreset == 0 ) m_sRule = _T("���������");
		else { CRule r; m_sRule = r.Preset( Mo()->nPreset ); }
		m_rule.Decode( Mo()->rule.Encode() );

		// ��� ���� ( ó������ �ڽŸ� ���� ��� ��ǻ�͵� )

		m_aInfo[0].bComputer = false;
		m_aInfo[0].sName = Mo()->aPlayer[0].sName;
		m_aInfo[0].sInfo = format_score(
			Mo()->anPlayerState[m_rule.nPlayerNum-2][0], Mo()->anPlayerState[m_rule.nPlayerNum-2][1],
			Mo()->anPlayerState[m_rule.nPlayerNum-2][2] );
		
		m_aInfo[0].pSocket = 0;

		m_aInfo[0].dfa[0] = Mo()->anPlayerState[m_rule.nPlayerNum-2][0];
		m_aInfo[0].dfa[1] = Mo()->anPlayerState[m_rule.nPlayerNum-2][1];
		m_aInfo[0].dfa[2] = Mo()->anPlayerState[m_rule.nPlayerNum-2][2];
		m_aInfo[0].dfa[3] = BASE_MONEY;

		for ( int i = 1; i < MAX_CONNECTION; i++ )
			SetComputer( i, BASE_MONEY );

		// ����޽����� ä��â���� ����Ѵ�
		CString sEnterMsg = create_entermsg( m_aInfo[0].sName,
			Mo()->anPlayerState[m_rule.nPlayerNum-2][0], 
			Mo()->anPlayerState[m_rule.nPlayerNum-2][1], 
			Mo()->anPlayerState[m_rule.nPlayerNum-2][2],
			false );
		Chat( sEnterMsg, -1, false );

		UpdateMarks();
	}
	else {	// !m_bServer

		// ���ӵ� ȣ��Ʈ�� �ּҸ� ��´�
		CZInetAddr addr = m_pServerSocket->GetPeerName();

		CString sAddr( _T("Unknown") );
		uPort = 0;
		VERIFY( addr.GetAddr( &sAddr, &uPort ) );

		m_sAddress.Format( _T("%s:%u"), sAddr, uPort );

		// ȣ��Ʈ����, mmNewPlayer �޽����� ������
		CMsg* pNewPlayerMsg = CreateNewPlayerMsg( players, spectatorOnly );
		AUTODELETE_MSG(pNewPlayerMsg);
		if ( !m_pServerSocket->SendMsg( pNewPlayerMsg ) )
			// ���� !
			bFailed = true;
	}

	// ä��â �ڵ鷯�� ��Ʈ
	Ib()->SetChatHandler( (DWORD)(LPVOID)this, ChatProc );

	DSB::Create( 0, 0, 33, 33, -1 );


	if ( bFailed ) {
		if ( m_bServer ) {
			CString sPort; sPort.Format( _T("%u"), uPort );
			Fail( _T("������ ���� ��Ʈ�� ���ε� �� �� �������ϴ� !"), sPort );
		}
		else {
			Fail( _T("������ �޽����� ���� �� �����ϴ� !") );
		}
	}
	else {	// ����, client handler �� ��Ʈ�Ѵ�
		if ( !m_bServer ) {
			m_pServerSocket->SetTrigger( (DWORD)(LPVOID)this, 0, ClientSockProc );
		}
	}
}

// �ʱ�ȭ ( ������ �� ȣ��� )
void DConnect::OnInit()
{
	DSB::OnInit();

	if ( m_bServer ) {
		// ������ hotspot ���� ���
		RegisterOk();
		RegisterMarks();
		RegisterRule();
		RegisterSpec();
	}

	RegisterHotspot(
		27, 23, -1, -1, true, 0,
		_T("���"),
		&s_colCyan, &s_tdMidium, &s_colCyan, &s_tdMidiumOutline,
		(LPVOID)0 );
}

// CBoard �� ȣ���ϴ� �׸��� �Լ� - ��µ� shade-box �� �� �Լ���
// �������̵� �Ͽ� �����ؾ� �Ѵ�
// ���̽� �Լ��� ������ ��ü�� ���� ���� ������ �׸���
void DConnect::OnDraw(
	CDC* pDC,
	bool bBrushOrigin )
{
	DSB::OnDraw( pDC, bBrushOrigin );

	// �ּ�
	PutText( pDC, m_sAddress,
			-1, 1, true, s_colYellow, s_tdMidiumOutline );
	// ������
	int y = m_rule.nPlayerNum >= 6 ? 5 :7;
	PutText( pDC, _T("������ :"),
			2, y - 2, true, s_colWhite, s_tdShade );
	for ( int i = 0; i < m_rule.nPlayerNum; i++ ) {

		// ��ǻ�ʹ� ���, ����� �����
		COLORREF col = m_aInfo[i].bComputer ? s_colWhite : s_colYellow;

		// '��'�� �׵θ�, �������� �׸���
		int deco = i == m_uid ? s_tdOutline : s_tdShade;
		PutText( pDC, m_aInfo[i].sName,
			3, y+i*3, true, col, deco );
		PutText( pDC, m_aInfo[i].sInfo, 4, y+i*3+1, true,
			s_colGray, s_tdNormal );
	}

	// ��Ģ
	PutText( pDC, _T("���� ��Ģ : "),
			25, y - 2, true, s_colWhite, s_tdShade );

	// ������
	if ( !m_bServer )
		PutText( pDC, _T("������ : "),
				25, y + 6, true, s_colWhite, s_tdShade );
	
	wsprintf(m_specstr, "%d ��", m_nSpectators);

	PutText( pDC, m_specstr,
			25, y + 8, true, s_colWhite, s_tdShade );

	// ä��â
	int nChatLines = (int)( m_nChatDataEnd - m_nChatDataBegin );
	for ( int t = 0; t < nChatLines; t++ ) {
		int nIndex = ( m_nChatDataBegin + t ) % CHAT_LINES;
		PutText( pDC, m_asChatData[nIndex],
			m_rcChat.left, m_rcChat.top+t, true,
			m_acolChatData[nIndex], s_tdNormal );
	}

	CRect rcBound;
	CPen pnGray( PS_SOLID, 0, s_colGray );
	pDC->SelectStockObject( NULL_BRUSH );
	pDC->SelectObject( &pnGray );

	// �׵θ� (ä��â)
	rcBound = DSBtoDP( m_rcChat );
	rcBound.InflateRect( GetDSBUnit()/2, GetDSBUnit()/2 );
	pDC->RoundRect( &rcBound,
		CPoint( GetDSBUnit(), GetDSBUnit() ) );

	// �׵θ� (������)
	rcBound = DSBtoDP( CRect( 1, y, 24, 25 ) );
	rcBound.InflateRect( GetDSBUnit()/2, GetDSBUnit()/2, 0, GetDSBUnit()/3 );
	pDC->RoundRect( &rcBound,
		CPoint( GetDSBUnit(), GetDSBUnit() ) );

	pDC->SelectStockObject( WHITE_PEN );

	// �׵θ� (Ȯ��)
	if ( m_bServer ) {

		rcBound = DSBtoDP( CRect( 25, 20, 32, 22 ) );
		rcBound.InflateRect( GetDSBUnit()/2, GetDSBUnit()/3, GetDSBUnit()/2, -GetDSBUnit()/4 );
		pDC->RoundRect( &rcBound,
			CPoint( GetDSBUnit(), GetDSBUnit() ) );
	}

	// �׵θ� (���)
	rcBound = DSBtoDP( CRect( 25, 23, 32, 25 ) );
	rcBound.InflateRect( GetDSBUnit()/2, GetDSBUnit()/3, GetDSBUnit()/2, -GetDSBUnit()/4 );
	pDC->RoundRect( &rcBound,
		CPoint( GetDSBUnit(), GetDSBUnit() ) );
}

// �� ������ Ŭ���� �� �Ҹ��� �Լ�
// LPVOID ���� 0 �̸� ���� (X�ڸ� Ŭ��) ��û
void DConnect::OnClick( LPVOID pVoid )
{
	switch ( (int)pVoid ) {

	case 0: {
		(new DStartUp(m_pBoard))->Create( 1 );
		Destroy();
		break;
	}

	case 500: {	// ��Ģ�� ������
		// read-only, auto-delete
		DRule* pDRule = new DRule( m_rule.Encode(), true, true );
		pDRule->Create(
			*m_pBoard,
			WS_VISIBLE | WS_DLGFRAME
			| WS_CLIPCHILDREN | WS_CLIPSIBLINGS
			| WS_SYSMENU | WS_CAPTION );
		pDRule->SetIcon( AfxGetApp()->LoadIcon( IDR_MAINFRAME ), TRUE );
		break;
	}

	case 600: {	// ��Ģ ����
		CString rulename = CRule::AttemptSaveRule( m_rule.Encode(), m_sRule );
		if ( rulename != "" )
			DeleteHotspot( (LPVOID)600 );
		break;
	}
	case 0xffffffff: {	// ���� !
		if ( m_bServer ) {
			CMsg msgBegin( _T("l"), CMsg::mmBeginGame );
			SendToAll( &msgBegin );	// ��ο��� �˸�
			VERIFY( BeginServer() );
		}
		break;
	}

	default: {
		if ( (int)pVoid >= 100 && (int)pVoid < 100 + MAX_PLAYERS ) {
			// �÷��̾���� ���� ��ũ
			long uid = (int)pVoid - 100;
			ASSERT( !m_pPopup );
			if ( uid != 0 )
				(m_pPopup = new DConnectPopup(this,m_pBoard))
								->Create( uid );
		}
		else if ( (int)pVoid >= 200 && (int)pVoid < 300 ) {
			// DConnectPopup �� ������ �� ���ϰ�
			long uid = ((int)pVoid - 200 ) / 10;
			long result = ((int)pVoid - 200 ) % 10;

			// ����� �ƴϸ�, �����Ѵ�
			if ( m_aInfo[uid].bComputer ) break;

			if ( result == 1 ) 	// �Ͻ� �߹�
				FailedForPlayer( uid, true );
			else if ( result == 2 ) {	// ���� �߹�
				// ������Ʈ�� �ִ´�
				m_lBlackList.AddTail( m_aInfo[uid].sName );
				FailedForPlayer( uid, true );
			}
		}
		else if ( (int)pVoid == 1000 ) {	// ������ ����
			(m_pPopup = new DConnectPopup(this,m_pBoard))
							->Create( Mo()->bObserver ? -1 : -2 );
		}
		else if ( (int)pVoid >= 190 && (int)pVoid < 200 ) {
			int result = (int)pVoid - 190;
			if ( result == 1 ) {	// ������ ����/���
				Mo()->bObserver = !Mo()->bObserver;
			}
			else if ( result == 2 ) {	// ������ ���� �߹�
				int i;
				for ( i = m_rule.nPlayerNum; i < MAX_CONNECTION; i++ ) {
					if ( !m_aInfo[i].bComputer )
						FailedForPlayer ( i, true );
				}
			}
		}
		break;
	}
	}
}

// m_acolMark �� update ��
void DConnect::UpdateMarks()
{
	for ( int i = 0; i < MAX_PLAYERS; i++ )
		if ( !m_bServer || m_aInfo[i].bComputer )
			m_acolMark[i] = s_colGray;
		else m_acolMark[i] = s_colCyan;
}

// ȸ���Ұ����� ������ �߻����� �� ȣ��
void DConnect::Fail( LPCTSTR sMsg1, LPCTSTR sMsg2 )
{
	// �޽��� ���ڸ� ����
	// �� ���ڴ� �ڵ����� this �� ���δ�
	(new DConnectFail(m_pBoard))->Create(
		_T("���� !!"), sMsg1, sMsg2, this );
}

// ���۹�ư�� �ֽ��̿� ���
void DConnect::RegisterOk()
{
	RegisterHotspot(
		27, 20, -1, -1, true, 0,
		_T("����"),
		&s_colCyan, &s_tdMidium, &s_colCyan, &s_tdMidiumOutline,
		(LPVOID)0xffffffff );
}

// ����� �ٴ� ��ȣ�� ���
void DConnect::RegisterMarks()
{
	int y = m_rule.nPlayerNum >= 6 ? 5 : 7;
	for ( int i = 0; i < m_rule.nPlayerNum; i++ )
		RegisterHotspot(
			1, y+i*3, -1, -1, true, 0, _T("��"),
			&m_acolMark[i], &s_tdNormal,
			&m_acolMark[i], &s_tdOutline, (LPVOID)( i + 100 ) );
}

// ��Ģ�� ���
void DConnect::RegisterRule()
{
	RegisterHotspot(
		25, 7 - ( m_rule.nPlayerNum >= 6 ? 2 : 0 ), -1, -1, true, 0,
		m_sRule,
		&s_colCyan, &s_tdShade, &s_colCyan, &s_tdOutline, (LPVOID)500 );

	if ( CRule::RuleExists( m_rule.Encode() ) == "" )
		RegisterHotspot(
			25, 9 - ( m_rule.nPlayerNum >= 6 ? 2 : 0 ), -1, -1, true, 0,
			_T("��Ģ ����"),
			&s_colYellow, &s_tdShade, &s_colYellow, &s_tdOutline, (LPVOID)600 );
}

void DConnect::RegisterSpec()
{
	RegisterHotspot(
		25, m_rule.nPlayerNum >= 6 ? 11 : 13, -1, -1, true, 0, _T("������ : "),
		&s_colCyan, &s_tdShade,
		&s_colCyan, &s_tdOutline, (LPVOID)( 1000 ) );
}

// ä�� �޽����� ����(new)
CMsg* DConnect::CreateChatMsg( long uid, LPCTSTR sMsg )
{
	return new CMsg( _T("lls"), CMsg::mmChat, uid, sMsg );
}

// uid ��ġ�� �÷��̾ ��ǻ�ͷ� ġȯ
void DConnect::SetComputer( long uid, int money )
{
	if ( m_aInfo[uid].pSocket ) {
		m_aInfo[uid].pSocket->ClearTrigger();
		delete m_aInfo[uid].pSocket;
	}

	if(!m_aInfo[uid].bComputer)
	{
		m_aInfo[uid].bComputer = true;
		if(m_nSpectators > 0) m_nSpectators--;
	}
	if(uid < MAX_PLAYERS)
	{
		m_aInfo[uid].sName = Mo()->aPlayer[uid].sName + _T(" (��ǻ��)");
		m_aInfo[uid].sInfo = get_name( Mo()->aPlayer[uid].sAIDLL );
	}
	else
	{
		m_aInfo[uid].sName = _T("���� Dummy");
		m_aInfo[uid].sInfo = get_name( Mo()->aPlayer[0].sAIDLL );
	}
	m_aInfo[uid].pSocket = 0;

	m_aInfo[uid].dfa[0] = 0;
	m_aInfo[uid].dfa[1] = 0;
	m_aInfo[uid].dfa[2] = 0;
	m_aInfo[uid].dfa[3] = money;
}

// ���� ������ �÷��̾�鿡�� �޽����� ����
void DConnect::SendToAll( CMsg* pMsg, long uidExcept )
{
	for ( int i = 1; i < MAX_CONNECTION; i++ )
		if ( uidExcept != i && !m_aInfo[i].bComputer ) {
			// ����̸� ����
			if ( m_aInfo[i].pSocket )
				if ( !m_aInfo[i].pSocket->SendMsg( pMsg ) )
					FailedForPlayer(i);
		}
}

// ���� m_aInfo ��Ȳ�� ����ϴ� CMsg ��ü�� ����(new)
// ��, mmPrepare �޽���
CMsg* DConnect::CreateStateMsg()
{
	int i;
	CString sFormat = _T("lss");

	CMsg* k = new CMsg( sFormat,
		CMsg::mmPrepare, m_rule.Encode(), m_sRule );
	for(i = 0; i < MAX_CONNECTION; i++)
	{
		k->PushString(m_aInfo[i].sName);
		k->PushString(m_aInfo[i].sInfo);
		k->PushLong(m_aInfo[i].bComputer ? 1 : 0);
		k->PushLong(m_aInfo[i].dfa[0]);
		k->PushLong(m_aInfo[i].dfa[1]);
		k->PushLong(m_aInfo[i].dfa[2]);
		k->PushLong(m_aInfo[i].dfa[3]);
	}
	return k;
}

// m_aInfo �� i ��° �÷��̾ ���� CMsg ��ü�� ����(new)
// ��, mmChanged �޽���
CMsg* DConnect::CreatePlayerInfoMsg( long uid )
{
	return new CMsg( _T("llssl"),
		CMsg::mmChanged, uid,
		m_aInfo[uid].sName, m_aInfo[uid].sInfo, m_aInfo[uid].bComputer ? 1 : 0 );
}

// mmInit �޽����� ����(new)
CMsg* DConnect::CreateInitMsg( long players )
{
	return new CMsg( _T("lll"), CMsg::mmInit, MIGHTY_VERSION, players );
}

// mmUID �޽����� ����(new)
CMsg* DConnect::CreateUIDMsg( long uid )
{
	return new CMsg( _T("ll"), CMsg::mmUID, uid );
}

// Ư�� �÷��̾ ���� ���� ȣ���� ����
// (Computer �� �ٲٰ� �ٸ� �÷��̾�� ����)
// ( bAccessDenied �� ���̸�, �� �÷��̾��
// �߹濡 ���� �����Ǵ°��� )
void DConnect::FailedForPlayer( long uid, bool bAccessDenied )
{
	RemovePlayer( uid, bAccessDenied );
	CMsg* pMsg = CreatePlayerInfoMsg( uid );
	AUTODELETE_MSG(pMsg);
	SendToAll( pMsg );
}

// �÷��̾ �ϳ� �߰�
// ���ϵǴ� ���� uid, -1 �̸� ������� ����, -2 �̸� ���� �̸� ����,
// -3 �̸� ��� �ź�, -4 �̸� ������ �ź�
// pMsg : mmNewPlayer �޽���
long DConnect::AddPlayer( CMsg* pMsg, CPlayerSocket* pSocket )
{
	int i;
	// �̸�, ������ �����Ѵ�
	// spectatorOnly: �����ڸ� �ϰ� ���� ������ ���Դ� (2011.1.7)
	CString sName; long spectatorOnly; long r[3]; long p;

	long m;
	VERIFY( pMsg->PumpLong( m ) && m == CMsg::mmNewPlayer );

	if ( !pMsg->PumpString( sName )
		|| !pMsg->PumpLong( spectatorOnly )
		|| !pMsg->PumpLong( r[0] ) || !pMsg->PumpLong( r[1] ) || !pMsg->PumpLong( r[2] )
		|| !pMsg->PumpLong( p ) ) return -3;


	// ������Ʈ�� �ִ� �ι��ΰ� �����Ѵ�
	POSITION pos = m_lBlackList.GetHeadPosition();
	while (pos)
		if ( m_lBlackList.GetNext(pos) == sName ) return -3;

	// ������ �ź��ε� �����ڷ� ���Դ�
	if ( !Mo()->bObserver && spectatorOnly )
		return -4;
	
	// ���ڸ��� ã�´�
	long uid;
	for ( uid = 0; uid < MAX_CONNECTION; uid++ )
	{
		if ( m_aInfo[uid].bComputer )
		{
			if ( !(spectatorOnly && uid < m_rule.nPlayerNum) )
				break;
		}
		// ���� �г��� �Ұ��ε� ���� �г����� �����ϸ�
		else if ( !Mo()->bSameName && m_aInfo[uid].sName == sName ) return -2;
	}
	// ���� ������ ��� 20��, �Ұ����� ��� �÷��̾� ������ ���� ��� �ڸ��� ��á�ٴ� ǥ��
	if ( uid >= 20 || (( !Mo()->bObserver) && uid >= m_rule.nPlayerNum ) )
		return -1;

	m_aInfo[uid].sName = sName;
	m_aInfo[uid].sInfo = format_score( r[0], r[1], r[2] );
	m_aInfo[uid].bComputer = false;
	m_aInfo[uid].pSocket = pSocket;
	for ( i = 0; i < 3; i++ )
		m_aInfo[uid].dfa[i] = r[i];
	m_aInfo[uid].dfa[3] = BASE_MONEY;

	// ����޽����� ä��â���� ����Ѵ�
	CString sEnterMsg = create_entermsg(
							m_aInfo[uid].sName, r[0], r[1], r[2], uid >= m_rule.nPlayerNum );

	CMsg* pEnterMsg = CreateChatMsg( -1, sEnterMsg );
	AUTODELETE_MSG(pEnterMsg);

	SendToAll( pEnterMsg );

	Chat( sEnterMsg, -1, false );
	
	if(uid >= m_rule.nPlayerNum)
		m_nSpectators++;

	// ȭ���� Update
	UpdateMarks();
	CRect rc; GetRect( &rc );
	m_pBoard->UpdateDSB( &rc );

	return uid;
}

// �÷��̾ ����
void DConnect::RemovePlayer( long uid, bool bAccessDenied )
{
	ASSERT( !m_aInfo[uid].bComputer );

	if ( bAccessDenied ) {	// �߹� ���� �޽����� ������
		CMsg msgOut( _T("ls"), CMsg::mmError,
					_T("���ӿ��� �߹���߽��ϴ� !!") );
		m_aInfo[uid].pSocket->SendMsg( &msgOut );
	}

	CString sName = m_aInfo[uid].sName;

	SetComputer( uid, BASE_MONEY);

	// ����޽����� ä��â���� ����Ѵ�
	CString sOutMsg;
	sOutMsg.Format(
		!bAccessDenied ? _T("%s ���� �����Ͽ����ϴ�")
		: _T("%s ���� ���ӿ��� �߹���߽��ϴ� !!"), sName );

	CMsg* pChatMsg = CreateChatMsg( -1, sOutMsg );
	AUTODELETE_MSG(pChatMsg);
	SendToAll( pChatMsg );

	Chat( sOutMsg, -1, false );

	// ȭ���� Update
	UpdateMarks();
	CRect rc; GetRect( &rc );
	m_pBoard->UpdateDSB( &rc );
}

// ���� ��û
void DConnect::OnAccept( int nErr )
{
	// ���ж�� ? (�����Ѵ�)
	if ( nErr ) { ASSERT(0); return; }

	// ������ �޴´�
	CPlayerSocket* pSocket = new CPlayerSocket();
	if ( !Accept( *pSocket ) ) {
		ASSERT(0);
		delete pSocket;
		return;
	}

	// mmInit ���� �����ϰ�, �ڵ鷯�� ��Ʈ
	CMsg* pMsg = CreateInitMsg( m_rule.nPlayerNum );
	AUTODELETE_MSG(pMsg);

	if ( !pSocket->SendMsg( pMsg ) ) {
		ASSERT(0);
		delete pSocket;
		return;
	}

	// 0 �� uid (anonymous) �� �ڵ鷯�� ��Ʈ
	pSocket->SetTrigger( (DWORD)(LPVOID)this, 0, ServerSockProc );
}

// ���� �ڵ鷯
void DConnect::ServerSockProc( CPlayerSocket* pSocket, CMsg* pMsg,
								DWORD dwUser1, DWORD dwUser2 )
{
	((DConnect*)(LPVOID)dwUser1)->ServerSockProc(
		(long)dwUser2, pMsg, pSocket );
}

void DConnect::ServerSockProc( long uid, CMsg* pMsg, CPlayerSocket* pSocket )
{
	AUTODELETE_MSG(pMsg);

	if ( uid == 0 ) {
		// Anonymous Socket
		// ���� uid ��ũ ���� ���Ӹ� �Ǿ� �ִ� ����

		if ( pMsg->GetType() == CMsg::mmDisconnected ) {
			// ������ ����� - �� ���Ͽ� ���ؼ��� �ش´�
			delete pSocket;
			return;
		}
		else if ( pMsg->GetType() == CMsg::mmNewPlayer ) {
			// ��� ��û
			uid = AddPlayer( pMsg, pSocket );

			if ( uid < 0 ) {
				CMsg msgErr( _T("ls"), CMsg::mmError,
					uid == -1 ? _T("�̹� �ο��� �� á���ϴ�") : 
					uid == -2 ? _T("���� �̸��� �����մϴ�") :
					uid == -3 ? _T("������ �źεǾ����ϴ�") :
					uid == -4 ? _T("�����ڸ� ������� �ʽ��ϴ�")
					: _T("������ �ҷ��մϴ�") );
				VERIFY( pSocket->SendMsg( &msgErr ) );
				delete pSocket;
				return;
			}
			else {
				// ��� ���� !

				// �ٸ�����鿡�� �˸�
				CMsg* pInfoMsg = CreatePlayerInfoMsg( uid );
				AUTODELETE_MSG(pInfoMsg);

				SendToAll( pInfoMsg, uid );	// �� ����� ����

				// �� ������Դ� ����, UID �� ��ü �����͸� �ش�

				CMsg* pUIDMsg = CreateUIDMsg( uid );
				CMsg* pAllInfo = CreateStateMsg();

				AUTODELETE_MSG(pUIDMsg);
				AUTODELETE_MSG(pAllInfo);

				if ( !pSocket->SendMsg( pUIDMsg )
					|| !pSocket->SendMsg( pAllInfo ) )
					FailedForPlayer( uid );
				else	// �������� uid �� �ڵ鷯�� ����
					pSocket->SetTrigger(
						(DWORD)(LPVOID)this, uid, ServerSockProc );

				return;	// SetTrigger �� uid 0 ���� �缳������ �ʵ���
			}
		}
	}
	else {	// uid �� 1 ~ nPlayerNum ����
			// m_aInfo �� ��ϵ� �÷��̾ ���� �޽���

		if ( pMsg->GetType() == CMsg::mmDisconnected ) {
			// ���� ����
			FailedForPlayer( uid );
			return;
		}
		else if ( pMsg->GetType() == CMsg::mmChat ) {
			// ä�� �޽���
			if ( !ReceiveChatMsg( pMsg ) ) {
				FailedForPlayer( uid );
				return;
			}
			else SendToAll( pMsg );
		}
	}

	// �� uid �� ���ؼ� �޽����� �� ���� �� �ֵ���
	// �ڵ鷯�� �缳��
	pSocket->SetTrigger( (DWORD)(LPVOID)this, uid, ServerSockProc );
}

// mmNewPlayer �޽����� ����
CMsg* DConnect::CreateNewPlayerMsg( long players, bool spectatorOnly )
{
	return new CMsg( _T("lslllll"), CMsg::mmNewPlayer,
		Mo()->aPlayer[0].sName,
		spectatorOnly ? 1 : 0,
		Mo()->anPlayerState[players-2][0], Mo()->anPlayerState[players-2][1], Mo()->anPlayerState[players-2][2],
		0 // BASE_MONEY �Ǵ� �ڱ⵷?
	);
}

static long lDummy;

// mmUID �޽����� �����Ͽ� Update
bool DConnect::ReceiveUIDMsg( CMsg* pMsg )
{
	if ( !pMsg->PumpLong( lDummy )
		|| !pMsg->PumpLong( m_uid ) ) {
		ASSERT(0);
		return false;
	}
	else return true;
}

// mmPrepare �޽����� ����
bool DConnect::ReceiveStateMsg( CMsg* pMsg )
{
	int i;
	CString sPreset;
	CString sRule;

	if ( !pMsg->PumpLong( lDummy )
		|| !pMsg->PumpString( sRule )
		|| !pMsg->PumpString( sPreset ) ) return false;

	// ��Ģ
	m_rule.Decode( sRule );
	m_sRule = sPreset;

	// ��Ģ ������ ��ϵ��� �ʾҴٸ� ����Ѵ�
	if ( !FindHotspot( (LPVOID)500 ) ) RegisterRule();

	// ��� ����
	long bComputer;
	m_nSpectators = 0;
	for ( i = 0; i < MAX_CONNECTION; i++ ) {
		if ( !pMsg->PumpString( m_aInfo[i].sName )
			|| !pMsg->PumpString( m_aInfo[i].sInfo )
			|| !pMsg->PumpLong( bComputer )
			|| !pMsg->PumpLong( m_aInfo[i].dfa[0] )
			|| !pMsg->PumpLong( m_aInfo[i].dfa[1] )
			|| !pMsg->PumpLong( m_aInfo[i].dfa[2] )
			|| !pMsg->PumpLong( m_aInfo[i].dfa[3] ) ) return false;
		m_aInfo[i].bComputer = !!bComputer;
		if(i >= m_rule.nPlayerNum && !bComputer)
			m_nSpectators++;
	}

	// Mark ���� ��ϵ��� �ʾҴٸ� ����Ѵ�
	if ( !FindHotspot( (LPVOID)100 ) ) RegisterMarks();
	UpdateMarks();

	CRect rc; GetRect(&rc);
	m_pBoard->UpdateDSB( &rc );

	return true;
}

// mmChanged �޽����� ����
bool DConnect::ReceivePlayerInfoMsg( CMsg* pMsg )
{
	long uid, bComputer;

	if ( !pMsg->PumpLong( lDummy )
		|| !pMsg->PumpLong( uid )
		|| !pMsg->PumpString( m_aInfo[uid].sName )
		|| !pMsg->PumpString( m_aInfo[uid].sInfo )
		|| !pMsg->PumpLong( bComputer ) ) return false;

	if(uid >= m_rule.nPlayerNum)
	{
		if(!m_aInfo[uid].bComputer) m_nSpectators--;
		if(!bComputer) m_nSpectators++;
	}
	m_aInfo[uid].bComputer = !!bComputer;

	UpdateMarks();

	CRect rc; GetRect(&rc);
	m_pBoard->UpdateDSB( &rc );

	return true;
}

// mmChat �޽����� ����
bool DConnect::ReceiveChatMsg( CMsg* pMsg )
{
	long uid;
	CString sMsg;
	if ( !pMsg->PumpLong( uid )
		|| !pMsg->PumpLong( uid )
		|| !pMsg->PumpString( sMsg ) ) return false;
	Chat( sMsg, uid );
	return true;
}

// ���� �ڵ鷯
void DConnect::ClientSockProc( CPlayerSocket* pSocket, CMsg* pMsg,
							DWORD dwUser1, DWORD dwUSer2 )
{	((DConnect*)(LPVOID)dwUser1)->ClientSockProc( pMsg ); }

void DConnect::ClientSockProc( CMsg* pMsg )
{
	AUTODELETE_MSG(pMsg);

	if ( pMsg->GetType() == CMsg::mmDisconnected ) {
		// ���� ����
		Fail( _T("�����κ��� ������ ���������ϴ� !") );
		return;
	}
	else if ( pMsg->GetType() == CMsg::mmUID ) {
		// UID ���� ��´�
		if ( !ReceiveUIDMsg( pMsg ) )
			// Close �� ȣ���ϸ�, ������ Trigger �޽�������
			// mmDisconnected �� �� ���̴�
			m_pServerSocket->Close();
	}
	else if ( pMsg->GetType() == CMsg::mmPrepare ) {
		// �غ� (��� ���¸� ����)
		if ( !ReceiveStateMsg( pMsg ) )
			m_pServerSocket->Close();
	}
	else if ( pMsg->GetType() == CMsg::mmChanged ) {
		// �Ϻ� ���� ����
		if ( !ReceivePlayerInfoMsg( pMsg ) )
			m_pServerSocket->Close();
	}
	else if ( pMsg->GetType() == CMsg::mmBeginGame ) {
		// ���� ���� !
		m_pServerSocket->ClearTrigger();
		VERIFY( BeginClient() );
		return;
	}
	else if ( pMsg->GetType() == CMsg::mmChat ) {
		// ä�� �޽���
		if ( !ReceiveChatMsg( pMsg ) )
			m_pServerSocket->Close();
	}
	else if ( pMsg->GetType() == CMsg::mmError ) {
		// ���� �޽���
		m_pServerSocket->Close();
		CString sMsg;
		if ( pMsg->PumpLong( lDummy )
			&& pMsg->PumpString( sMsg ) )
			Fail( sMsg );
		else Fail( _T("�����κ��� �����ڵ尡 ���ŵǾ����ϴ� !") );
		return;
	}
	else {
		// �𸣴� �޽���
		ASSERT(0);
	}

	m_pServerSocket->SetTrigger( (DWORD)(LPVOID)this, 0, ClientSockProc );
}

// ä��â �ڵ鷯
void DConnect::ChatProc( LPCTSTR sMsg, DWORD dwUser )
{	((DConnect*)(LPVOID)dwUser)->ChatProc( sMsg ); }

void DConnect::ChatProc( LPCTSTR sMsg )
{
	CMsg* pChatMsg = CreateChatMsg( m_uid, sMsg );
	AUTODELETE_MSG(pChatMsg);

	if ( m_bServer ) { 	// ������� ��ο��� ������
		SendToAll( pChatMsg );
		// �ڽſ��Դ� �Ⱥ������Ƿ� ���� �׸���
		Chat( sMsg, m_uid );
	}
	else {	// Ŭ���̾�Ʈ���, mmChat �޽����� �޾Ƹ��ɰ��̴�
		m_pServerSocket->SendMsg( pChatMsg );
	}
}

// ä�� ȭ�鿡 sMsg �� �׸��� (uid==-1 �̸� ���� �޽���)
void DConnect::Chat( LPCTSTR sMsg, long uid, bool bUpdate )
{
	int i;

	CString sFullMsg;
	if ( uid == -1 )
		sFullMsg = CString(_T("## ")) + sMsg + _T(" ##");
	else sFullMsg = m_aInfo[uid].sName + _T(": ") + sMsg;

	// �����ٷ� �Ѿ�⵵ �ϹǷ�, Parse �Ͽ� ���� �ʿ��� ���� ���� ���Ѵ�
	size_t nMaxWidth;	// ������ �ִ� ��
	int n = ParseString( 0, m_rcChat.Width()-1, sFullMsg, nMaxWidth );

	LPTSTR* asMsg = new LPTSTR[n];
	for ( i = 0; i < n; i++ ) asMsg[i] = new TCHAR[nMaxWidth];

	ParseString( asMsg, m_rcChat.Width()-1, sFullMsg, nMaxWidth );

	// �� ������ �߰��Ѵ�
	for ( i = 0; i < n; i++ ) {

		strcpy( m_asChatData[m_nChatDataEnd % CHAT_LINES], asMsg[i] );
		m_acolChatData[m_nChatDataEnd % CHAT_LINES] =
			uid == -1 ? s_colCyan
			: uid == m_uid ? s_colYellow
			: s_colWhite;
		m_nChatDataEnd++;
	}

	if ( m_nChatDataEnd - m_nChatDataBegin > CHAT_LINES )
		m_nChatDataBegin = m_nChatDataEnd - CHAT_LINES;

	for ( i = 0; i < n; i++ ) delete[] asMsg[i];
	delete[] asMsg;

	if ( bUpdate ) {
		CRect rc = DSBtoDP( m_rcChat );
		m_pBoard->UpdateDSB( &rc );
	}
}

// ������ ���� (�����μ�)
bool DConnect::BeginServer()
{
	int i;
	ASSERT( m_uid == 0 );

	// �� �ҽ��� DStartUp.cpp ����

	int nPlayers = m_rule.nPlayerNum;

	CPlayer* apPlayers[MAX_CONNECTION];

	// Human Player ����
	apPlayers[0] = new CPlayerHuman(
		0, m_aInfo[0].sName, *m_pBoard );

	// Network Players / AI Players ����
	bool bFailed = false;
	CString sFailedDLL;
	for ( i = 1; i < MAX_CONNECTION; i++ ) {

		if ( m_aInfo[i].bComputer ) {	// AI Player
			if ( i < nPlayers ) {
				CPlayerMai* pPlayer = new CPlayerMai(
					i, m_aInfo[i].sName, *m_pBoard,
					Mo()->aPlayer[i].sAIDLL, Mo()->aPlayer[i].sSetting );
				apPlayers[i] = pPlayer;
				if ( !pPlayer->IsDLLWorking() ) {
					bFailed = true;
					sFailedDLL = Mo()->aPlayer[i].sAIDLL;
				}
			}
			else {			// ������ �ڸ��� �ִ� ����
				CPlayerDummy* pPlayer = new CPlayerDummy(i, *m_pBoard );
				apPlayers[i] = pPlayer;
			}
		}
		else {	// NetworkPlayer

			CPlayerNetwork* pPlayer = new CPlayerNetwork(
				i, m_aInfo[i].sName, *m_pBoard );
			apPlayers[i] = pPlayer;
		}
	}

	// ���� ������ ���
	for ( i = 0; i < nPlayers; i++ ) {
		apPlayers[i]->SetMoney( m_aInfo[i].dfa[3] );
		apPlayers[i]->GetAllRecord().wm = LOWORD( m_aInfo[i].dfa[0] );
		apPlayers[i]->GetAllRecord().lm = HIWORD( m_aInfo[i].dfa[0] );
		apPlayers[i]->GetAllRecord().wf = LOWORD( m_aInfo[i].dfa[1] );
		apPlayers[i]->GetAllRecord().lf = HIWORD( m_aInfo[i].dfa[1] );
		apPlayers[i]->GetAllRecord().wa = LOWORD( m_aInfo[i].dfa[2] );
		apPlayers[i]->GetAllRecord().la = HIWORD( m_aInfo[i].dfa[2] );
	}

	// ���� SetMFSM �� �ϴ� ���� this �� ����� ��� DSB �� �������
	// ��� �޽����� ���� ���� m_pBoard �� ��������
	CBoardWrap* pBoard = m_pBoard;

	// ���ο� SocketBag �� MFSM �� �����Ѵ�
	CSocketBag* pSB = new CSocketBag();
	CMFSM* pMFSM = new CMFSM( m_rule.Encode(), apPlayers, pSB );

	pSB->SetMFSM( pMFSM );
	for ( int uid = 0; uid < MAX_CONNECTION; uid++ )
		if ( !m_aInfo[uid].bComputer ) {
			pSB->InitForServer( uid, m_aInfo[uid].pSocket );
			m_aInfo[uid].pSocket = 0;	// detach
		}

	pMFSM->Init( 0 );

	pBoard->SetMFSM(pMFSM);

	// DLL �ε尡 �������� �� ��� �޽����� �ش�
	if ( bFailed && !sFailedDLL.IsEmpty() ) {

		LPCTSTR asLine[8];
		asLine[0] = _T("������ AI DLL �� �дµ� �����߽��ϴ�");
		asLine[1] = _T("");
		asLine[2] = sFailedDLL;
		asLine[3] = _T("");
		asLine[4] = _T("�ɼ� ��ȭ������ AI �ǿ��� �� ������");
		asLine[5] = _T("��θ� �ٽ� Ȯ���ϼ���");
		asLine[6] = _T("");
		asLine[7] = _T("�ӽ÷� �⺻ AI �� �о���Դϴ�");
		DMessageBox* pMB = new DMessageBox(pBoard);
		if ( Mo()->bUseSound ) MessageBeep( MB_ICONEXCLAMATION );
		pMB->Create( true, 8, asLine );
	}

	return true;
}

// ������ ���� (Ŭ���̾�Ʈ�μ�)
bool DConnect::BeginClient()
{
	int j;
	int nPlayers = m_rule.nPlayerNum;

	CPlayer* apPlayers[MAX_CONNECTION];

	// Human / Network Players ����
	for ( j = 0; j < MAX_CONNECTION; j++ ) {
		int i = ( m_uid >= nPlayers ? j : j < nPlayers ? ( m_uid + j ) % nPlayers : j );
		
		if ( i == m_uid )
			apPlayers[j] = new CPlayerHuman(
				j, m_aInfo[i].sName, *m_pBoard );
		else
			apPlayers[j] = new CPlayerNetwork(
				j, m_aInfo[i].sName, *m_pBoard );
	}

	// ���� ������ ���
	for ( j = 0; j < nPlayers; j++ ) {
		int i = m_uid >= nPlayers ? j : ( m_uid + j ) % nPlayers;
		apPlayers[j]->SetMoney( m_aInfo[i].dfa[3] );
		apPlayers[j]->GetAllRecord().wm = LOWORD( m_aInfo[i].dfa[0] );
		apPlayers[j]->GetAllRecord().lm = HIWORD( m_aInfo[i].dfa[0] );
		apPlayers[j]->GetAllRecord().wf = LOWORD( m_aInfo[i].dfa[1] );
		apPlayers[j]->GetAllRecord().lf = HIWORD( m_aInfo[i].dfa[1] );
		apPlayers[j]->GetAllRecord().wa = LOWORD( m_aInfo[i].dfa[2] );
		apPlayers[j]->GetAllRecord().la = HIWORD( m_aInfo[i].dfa[2] );
	}

	// ���ο� SocketBag �� MFSM �� �����Ѵ�
	CSocketBag* pSB = new CSocketBag();
	CMFSM* pMFSM = new CMFSM( m_rule.Encode(), apPlayers, pSB );

	pSB->SetMFSM( pMFSM );

	for ( int uid = 0; uid < nPlayers; uid++ )
		pSB->InitForClient( uid );
	pSB->InitForClient( m_pServerSocket );
	ASSERT( m_pServerSocket );
	m_pServerSocket = 0;	// detach

	pMFSM->Init( m_uid );

	m_pBoard->SetMFSM(pMFSM);

	return true;
}
