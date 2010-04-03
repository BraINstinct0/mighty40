// ZSocket.cpp: implementation of the CZSocket class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <atlconv.h>

#include "ZSocket.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


/////////////////////////////////////////////////////////////////////////////
// CZInetAddr

CZInetAddr::CZInetAddr( LPCTSTR sAddr, UINT uPort )
{
	USES_CONVERSION;

	Init();

	m_addr.sin_port = htons( (u_short)uPort );

	if ( !sAddr || !*sAddr ) return;

	LPSTR szAddr = T2A( (LPTSTR)sAddr );

	m_addr.sin_addr.s_addr = inet_addr( szAddr );

	if ( m_addr.sin_addr.s_addr == INADDR_NONE ) {
		// string notation

		LPHOSTENT lphost;
		lphost = gethostbyname( szAddr );

		if ( lphost != NULL )
			m_addr.sin_addr.s_addr =
				((LPIN_ADDR)lphost->h_addr)->s_addr;
		else {
			WSASetLastError(WSAEINVAL);
		}
	}
}

// �� �ּ��� ��Ʈ�� ǥ���� ��´�
BOOL CZInetAddr::GetAddr( CString* psAddr, UINT* puPort )
{
	if ( puPort ) *puPort = (UINT)ntohs( m_addr.sin_port );
	if ( psAddr ) {
		LPCSTR a = (LPCSTR)inet_ntoa( m_addr.sin_addr );
		if ( !a ) return FALSE;
		else *psAddr = a;
	}
	return TRUE;
}

// �� ����� �ּҸ� ��´�
BOOL CZInetAddr::GetHostAddr( CZInetAddr* pAddr )
{
	USES_CONVERSION;

	char* name = new char[1024];
	int failed = 0;
	for(;;){

		if ( gethostname( name, 1024 ) == 0 ) break;	// success

		if ( ::WSAGetLastError() != WSAEINPROGRESS ) {
			failed = 1; break;	// fail
		}

		Sleep(1000);	// try again
	}

	if ( !failed && pAddr )
		*pAddr = CZInetAddr( A2T(name) );

	delete[] name;
	return failed ? FALSE : TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CZSocket

CZSocket::CZSocket()
{
	m_hSocket = INVALID_SOCKET;
	m_pSockThread = 0;
	m_bTerminate = FALSE;
	m_bTerminated = TRUE;
	m_state = empty;
}

CZSocket::~CZSocket()
{
	// �����Լ����� �ڽ��� delete �� �� ����
	ASSERT( AfxGetThread() != m_pSockThread );
	VERIFY( Close() );
}


// ������ ����
BOOL CZSocket::Close()
{
	if ( m_hSocket != INVALID_SOCKET ) {
		if ( closesocket( m_hSocket ) != 0 )
			return FALSE;
		m_hSocket = INVALID_SOCKET;
	}

	if ( m_pSockThread ) {	// �����带 �׿��� ��

		m_bTerminate = TRUE;

		if ( AfxGetThread() != m_pSockThread ) {
			// ���� �����尡 �ڽ��� �ƴ� ��
			::WaitForSingleObject( m_pSockThread->m_hThread, INFINITE );
			delete m_pSockThread;
			m_pSockThread = 0;
			m_state = empty;
		}
	}

	return TRUE;
}

// �������� Attach, Detach
BOOL CZSocket::Attach( SOCKET hSocket )
{
	ASSERT( m_hSocket == INVALID_SOCKET );
	ASSERT( m_state == empty );

	if ( !Close() ) return FALSE;

	m_hSocket = INVALID_SOCKET;

	return TRUE;
}

SOCKET CZSocket::Detach()
{
	if ( !m_bTerminated ) {	// �����带 �׿��� ��

		m_bTerminate = TRUE;
		while ( !m_bTerminated ) Sleep(1000);
		m_pSockThread = 0;

		m_bTerminate = FALSE;
		m_bTerminated = TRUE;
	}

	SOCKET hSocket = m_hSocket;
	m_hSocket = INVALID_SOCKET;

	m_state = empty;

	return hSocket;
}

// ������ nType ( SOCK_STREAM �Ǵ� SOCK_DGRAM ) ������ ����
// uPort, sAddr �� Bind ��
// SOCK_STREAM �� ���, Listen �ϰ�, OnAccept �Լ��� ȣ���
// SOCK_DGRAM �� ���, ��� Send(To), Receive(From) �Լ� ȣ�Ⱑ��
BOOL CZSocket::CreateServerSocket( const SOCKADDR_IN* pAddr, int nType )
{
	ASSERT( m_state == empty && m_hSocket == INVALID_SOCKET );
	ASSERT( pAddr );

	m_hSocket = socket( AF_INET, nType, 0 );
	if ( m_hSocket == INVALID_SOCKET ) return FALSE;

	if ( bind( m_hSocket,
		(const SOCKADDR*)pAddr,
		sizeof(SOCKADDR_IN) ) != 0 ) {
		// ����
		VERIFY( closesocket( m_hSocket ) == 0 );
		m_hSocket = 0;
		return FALSE;
	}

	if ( nType == SOCK_STREAM ) {
		// ��Ʈ�� �����̹Ƿ� listen ���� �� �ش�
		VERIFY( listen( m_hSocket, 5 ) == 0 );
		m_state = accepting;
	}
	else {
		ASSERT( nType == SOCK_DGRAM );
		m_state = waiting;
	}

	InvokeThread();

	return TRUE;
}

// Ŭ���̾�Ʈ�� ��Ʈ�� ������ ����
BOOL CZSocket::CreateClientSocket( const SOCKADDR_IN* pAddr )
{
	ASSERT( m_state == empty && m_hSocket == INVALID_SOCKET );

	m_hSocket = socket( AF_INET, SOCK_STREAM, 0 );
	if ( m_hSocket == INVALID_SOCKET ) return FALSE;

	m_state = connecting;
	m_addrConn = CZInetAddr( pAddr );

	InvokeThread();

	return TRUE;
}

// ���� ��û�� �޾Ƶ���
// �� �Լ��� OnAccept �Լ������� ȣ���ؾ� Blocking ���� �ʴ´�
// ( sock �� �� CZSocket ��ü )
BOOL CZSocket::Accept( CZSocket& sock )
{
	ASSERT( m_state == accepting && sock.m_state == empty
		&& sock.m_hSocket == INVALID_SOCKET );

	SOCKET s = accept( m_hSocket, 0, 0 );
	if ( s == INVALID_SOCKET ) return FALSE;

	// ���ӵ� s �� ���� ���� �ý����� �����Ѵ�
	sock.m_hSocket = s;
	sock.m_state = waiting;
	sock.InvokeThread();

	return TRUE;
}

// �� ������ ���ε� �� �ּҸ� ����
CZInetAddr CZSocket::GetSockName()
{
	SOCKADDR_IN addr;
	int len = sizeof(addr);

	if ( getsockname( m_hSocket, (SOCKADDR*)&addr, &len ) != 0 )
		return CZInetAddr();
	else return CZInetAddr( &addr );
}

CZInetAddr CZSocket::GetPeerName()
{
	SOCKADDR_IN addr;
	int len = sizeof(addr);

	if ( getpeername( m_hSocket, (SOCKADDR*)&addr, &len ) != 0 )
		return CZInetAddr();
	else return CZInetAddr( &addr );
}


// ������ �ۼ��� ����

// �����͸� �����Ѵ�
BOOL CZSocket::Send( LPCVOID pData, DWORD dwSize, DWORD* pdwSent, BOOL bOOB )
{
	ASSERT( m_state == waiting && m_hSocket != INVALID_SOCKET );

	int nSent = send( m_hSocket, (const char*)pData, (int)dwSize, bOOB ? MSG_OOB : 0 );

	if ( nSent == SOCKET_ERROR ) return FALSE;

	if ( pdwSent ) *pdwSent = (DWORD)nSent;
	return TRUE;
}

BOOL CZSocket::SyncSend( LPCVOID pData, DWORD dwSize, BOOL bOOB )
{
	DWORD dwSent;

	while ( dwSize != 0 ) {

		if ( !Send( pData, dwSize, &dwSent, bOOB ) ) {

			if ( WSAGetLastError() == WSAEINPROGRESS
				|| WSAGetLastError() == WSAEWOULDBLOCK ) {
				Sleep(1000);	// �� �ִ� �ٽ� �ؾ� �Ѵ�
				continue;
			}
			else return FALSE;
		}

		dwSize -= dwSent;
		(const char*&)pData += dwSent;
	}
	return TRUE;
}

BOOL CZSocket::SendTo( LPCVOID pData, DWORD dwSize, DWORD* pdwSent,
						const SOCKADDR_IN* pAddr, BOOL bOOB )
{
	ASSERT( m_state == waiting && m_hSocket != INVALID_SOCKET );

	if ( !pAddr ) return Send( pData, dwSize, pdwSent, bOOB );

	int nSent = sendto( m_hSocket, (const char*)pData, (int)dwSize, bOOB ? MSG_OOB : 0,
						(const SOCKADDR*)pAddr, sizeof(SOCKADDR_IN) );

	if ( nSent == SOCKET_ERROR ) return FALSE;

	if ( pdwSent ) *pdwSent = (DWORD)nSent;
	return TRUE;
}

// �����͸� �о� ���δ�
BOOL CZSocket::Receive( LPVOID pData, DWORD dwSize, DWORD* pdwReceived, BOOL bOOB )
{
	ASSERT( m_state == waiting && m_hSocket != INVALID_SOCKET );

	int nRecv = recv( m_hSocket, (char*)pData, (int)dwSize, bOOB ? MSG_OOB : 0 );

	if ( nRecv == SOCKET_ERROR || nRecv == 0 ) return FALSE;

	if ( pdwReceived ) *pdwReceived = (DWORD)nRecv;
	return TRUE;
}

BOOL CZSocket::SyncReceive( LPVOID pData, DWORD dwSize, BOOL bOOB )
{
	DWORD dwReceived;

	while ( dwSize != 0 ) {

		if ( !Receive( pData, dwSize, &dwReceived, bOOB ) ) {

			if ( WSAGetLastError() == WSAEINPROGRESS
				|| WSAGetLastError() == WSAEWOULDBLOCK ) {
				Sleep(1000);	// �� �ִ� �ٽ� �ؾ� �Ѵ�
				continue;
			}
			else return FALSE;
		}

		dwSize -= dwReceived;
		(char*&)pData += dwReceived;
	}
	return TRUE;
}

BOOL CZSocket::ReceiveFrom( LPVOID pData, DWORD dwSize, DWORD* pdwReceived,
							SOCKADDR_IN* pAddr, BOOL bOOB )
{
	ASSERT( m_state == waiting && m_hSocket != INVALID_SOCKET );

	if ( !pAddr ) return Receive( pData, dwSize, pdwReceived, bOOB );

	int lenAddr = sizeof(SOCKADDR_IN);

	int nRecv = recvfrom( m_hSocket, (char*)pData, (int)dwSize, bOOB ? MSG_OOB : 0, (SOCKADDR*)pAddr, &lenAddr );

	if ( nRecv == SOCKET_ERROR ) return FALSE;

	if ( pdwReceived ) *pdwReceived = (DWORD)nRecv;
	return TRUE;
}

// ���� �Լ���

void CZSocket::OnAccept( int ) {}
void CZSocket::OnConnect( int ) {}
void CZSocket::OnReceive( int ) {}

// ������ ����

// ������ select �Ѵ� ( select() �� ����ȭ�� ���� )
// ���ϰ� : 0 �̸� Ÿ�Ӿƿ�, SOCKET_ERROR �� ����
//          ��� �̸� �� ���� �ϳ��� ����
int CZSocket::Select( BOOL* pbCanRead, BOOL* pbCanWrite, BOOL* pbException, DWORD dwTime )
{
	int nfds = m_hSocket+1;

	fd_set rfs, wfs, efs;
	FD_ZERO( &rfs ); FD_SET( m_hSocket, &rfs );
	FD_ZERO( &wfs ); FD_SET( m_hSocket, &wfs );
	FD_ZERO( &efs ); FD_SET( m_hSocket, &efs );

	timeval tv;
	tv.tv_sec = dwTime / 1000;
	tv.tv_usec = dwTime % 1000 * 1000;

	int ret = select( nfds,
				pbCanRead ? &rfs : 0,
				pbCanWrite ? &wfs : 0,
				pbException ? &efs : 0,
				dwTime == INFINITE ? 0 : &tv );

	if ( ret == SOCKET_ERROR ) return ret;

	if ( pbCanRead )
		*pbCanRead = FD_ISSET( m_hSocket, &rfs ) ? TRUE : FALSE;
	if ( pbCanWrite )
		*pbCanWrite = FD_ISSET( m_hSocket, &wfs ) ? TRUE : FALSE;
	if ( pbException )
		*pbException = FD_ISSET( m_hSocket, &efs ) ? TRUE : FALSE;

	return ret;
}


// �־��� �ּҷ� Connect �Ѵ� - m_bTerminate ��
// ���� �Ǹ� �ٷ� FALSE �� �����Ѵ�
BOOL CZSocket::AsyncConnect( const SOCKADDR_IN* pAddr )
{
	// ������ NB ���� ��Ʈ�Ѵ� (connect �� ����)
	u_long arg = 1;
	if ( ioctlsocket( m_hSocket, FIONBIO, &arg ) != 0 ) return FALSE;

	BOOL bRet = FALSE;
	int nErrCode = 0;

	if ( connect( m_hSocket, (const SOCKADDR*)pAddr, sizeof(SOCKADDR_IN) ) == 0 ) {
		// connection ���� !
		bRet = TRUE; goto lblExit;
	}

	nErrCode = WSAGetLastError();
	if ( nErrCode != WSAEWOULDBLOCK ) {
		// WSAEWOULDBLOCK �� �ƴϸ� ��¥ �����Ѱ��̴�
		goto lblExit;
	}

	// ���� ���� - ���ӵǱ⸦ ��ٸ���
	// m_bTerminate �� ����ؼ� �˻��ؾ� �Ѵ� (1000 �и��ʸ�������)
	{
		BOOL bCanWrite = FALSE;
		BOOL bException = FALSE;

		while ( !m_bTerminate && !bCanWrite ) {

			if ( Select( 0, &bCanWrite, &bException, 1000 )
				== INVALID_SOCKET || bException ) {
				// ���� ���� !
				nErrCode = WSAEINVAL;
				goto lblExit;
			}
		}

		bRet = TRUE;
		ASSERT( m_bTerminate || bCanWrite );
		goto lblExit;
	}

lblExit:
	if ( m_bTerminate ) {
		if ( !nErrCode ) nErrCode = WSAEINTR;
		bRet = FALSE;
	}

	// ���� ��带 ����ġ��
	arg = 0;
	if ( m_hSocket != INVALID_SOCKET )
		VERIFY( ioctlsocket( m_hSocket, FIONBIO, &arg ) == 0 );
	// ���ϰ�
	WSASetLastError( nErrCode );
	return bRet;
}

void CZSocket::InvokeThread()
{
	ASSERT( m_pSockThread == 0 );
	ASSERT( m_bTerminated == TRUE );

	m_bTerminated = FALSE;
	m_bTerminate = FALSE;

	m_pSockThread = AfxBeginThread( Proc, this, THREAD_PRIORITY_NORMAL, 0,
									CREATE_SUSPENDED );
	m_pSockThread->m_bAutoDelete = FALSE;
	m_pSockThread->ResumeThread();
}

UINT CZSocket::Proc( LPVOID pParam )
{	return ((CZSocket*)pParam)->Proc(); }


// �� ������
UINT CZSocket::Proc()
{
	// ������ ���ؼ� �����尡 ���۵Ǿ���
	if ( m_state == connecting ) {

		BOOL bRet = AsyncConnect( m_addrConn );
		if ( m_bTerminate ) goto lblExit;

		if ( !bRet ) {	// ����
			if ( !WSAGetLastError() ) WSASetLastError(WSAEINVAL);
			OnConnect( WSAGetLastError() );
			goto lblExit;
		}
		else {
			OnConnect( 0 );	// ����
			m_state = waiting;
		}
	}
	// ���������� ���� �����尡 ���۵Ǿ���
	if ( m_state == accepting ) {

		while ( !m_bTerminate ) {	// ���ѷ���, ����

			BOOL bCanRead = FALSE;
			BOOL bException = FALSE;

			if ( Select( &bCanRead, 0, &bException, 1000 )
				== INVALID_SOCKET || bException ) {	// ����

				if ( m_bTerminate ) goto lblExit;
				if ( !WSAGetLastError() ) WSASetLastError(WSAEINVAL);
				OnAccept( WSAGetLastError() );
			}

			if ( m_bTerminate ) goto lblExit;

			if ( bCanRead ) {
				// ���� ��û�� �޾Ƶ���
				OnAccept( 0 );
			}
		}
		goto lblExit;
	}
	// �Ϲ����� send/recv ����
	if ( m_state == waiting ) {

		while ( !m_bTerminate ) {	// ���ѷ���, ����

			BOOL bCanRead = FALSE;
			BOOL bException = FALSE;

			if ( Select( &bCanRead, 0, &bException, 1000 )
				== INVALID_SOCKET || bException ) {	// ����

				if ( m_bTerminate ) goto lblExit;
				if ( !WSAGetLastError() ) WSASetLastError(WSAEINVAL);
				OnReceive( WSAGetLastError() );
			}

			if ( m_bTerminate ) goto lblExit;

			if ( bCanRead ) {
				// �б� ����
				OnReceive( 0 );
			}
		}
		goto lblExit;
	}

lblExit:
	m_bTerminated = TRUE;
	m_bTerminate = FALSE;
	return 0;
}
