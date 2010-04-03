// ZSocket.h: interface for the CZSocket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ZSOCKET_H__47501A73_93F2_11D3_9AA3_0000212035B8__INCLUDED_)
#define AFX_ZSOCKET_H__47501A73_93F2_11D3_9AA3_0000212035B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// CAsyncSocket �� ���� ȯ��� ������ ���
// �� ���̺귯���� �ۼ���  1999. 11. 6. �幮��

// ���� : wsock32.lib(�Ǵ� ws2_32.lib) �� �������� ��ũ�ϰ�,
//        winsock.h(�Ǵ� winsock2.h) �� include �ؾ� �Ѵ�


// �ּ� Ŭ����
class CZInetAddr
{
public:
	// �ּ�
	SOCKADDR_IN m_addr;

	operator const SOCKADDR_IN*() const;

public:
	// �پ��� ������� �ּҸ� �����Ѵ�
	CZInetAddr( LPCTSTR sAddr = 0, UINT uPort = 0 );
	CZInetAddr( const SOCKADDR_IN* pAddr );
	CZInetAddr& operator=( const SOCKADDR_IN* pAddr );

	// �� �ּ��� ��Ʈ�� ǥ���� ��´�
	BOOL GetAddr( CString* psAddr = 0, UINT* puPort = 0 );

public:
	// ��Ÿ ��ƿ��Ƽ

	// �� ����� �ּҸ� ��´�
	static BOOL GetHostAddr( CZInetAddr* pAddr );

protected:
	void Init();
};


// ���� Ŭ���� ����
class CZSocket  
{
public:
	// ���� �ڵ�
	SOCKET m_hSocket;

public:
	// �� ���� Ŭ������ ����
	CZSocket();

	// ���� Ŭ������ �ı���
	// Close �� ���������� ȣ���Ѵ�
	virtual ~CZSocket();

	// ������ ����
	virtual BOOL Close();

	// �������� Attach, Detach
	BOOL Attach( SOCKET hSocket );
	SOCKET Detach();

	// ������ nType ( SOCK_STREAM �Ǵ� SOCK_DGRAM ) ������ ����
	// uPort, sAddr �� Bind ��
	// SOCK_STREAM �� ���, Listen �ϰ�, OnAccept �Լ��� ȣ���
	// SOCK_DGRAM �� ���, ��� Send(To), Receive(From) �Լ� ȣ�Ⱑ��
	BOOL CreateServerSocket( const SOCKADDR_IN* pAddr, int nType = SOCK_STREAM );

	// Ŭ���̾�Ʈ�� ��Ʈ�� ������ ����
	BOOL CreateClientSocket( const SOCKADDR_IN* pAddr );

public:
	// ���� ��û�� �޾Ƶ���
	// �� �Լ��� OnAccept �Լ������� ȣ���ؾ� Blocking ���� �ʴ´�
	// ( sock �� �� CZSocket ��ü )
	BOOL Accept( CZSocket& sock );

	// �� ������ ���ε� �� �ּҸ� ����
	CZInetAddr GetSockName();
	CZInetAddr GetPeerName();

public:
	// ������ �ۼ��� �Լ�
	// ���н� FALSE �� �����Ѵ� - Ư�� ������ ������ ���� FALSE ��
	// �����Ѵ�

	// �����͸� �����Ѵ�
	BOOL Send( LPCVOID pData, DWORD dwSize, DWORD* pdwSent = 0, BOOL bOOB = FALSE );
	BOOL SyncSend( LPCVOID pData, DWORD dwSize, BOOL bOOB = FALSE );
	BOOL SendTo( LPCVOID pData, DWORD dwSize, DWORD* pdwSent = 0,
					const SOCKADDR_IN* pAddr = 0, BOOL bOOB = FALSE );

	// �����͸� �о� ���δ�
	BOOL Receive( LPVOID pData, DWORD dwSize, DWORD* pdwReceived = 0, BOOL bOOB = FALSE );
	BOOL SyncReceive( LPVOID pData, DWORD dwSize, BOOL bOOB = FALSE );
	BOOL ReceiveFrom( LPVOID pData, DWORD dwSize, DWORD* pdwReceived = 0,
						SOCKADDR_IN* pAddr = 0, BOOL bOOB = FALSE );

protected:
	// ���� �Լ�
	// ���� : �� �Լ����� �� ������� �ٸ� �����忡�� ȣ��ȴ�

	// ���ŵ� ���ӿ�û�� ���� ��
	// (���������� ������ ��, �ٸ� ������ �� �ּҷ� �������� ��
	//  ȣ��Ǹ�, ���⼭ Accept() �Լ��� ȣ�� �� �� �ִ�)
	virtual void OnAccept( int nErr );
	// ������ �������� ��
	// (Ŭ���̾�Ʈ ������ Connect() �� ȣ�� �� ��,
	//  �����κ��� ������ ���� ��)
	virtual void OnConnect( int nErr );
	// ���ŵ� �����Ͱ� ���� �� (�� Receive �Լ��� ��� �����Ҽ� ���� ��)
	// �Ǵ� ������ �������� �� (Read�Լ��� FALSE ����)
	virtual void OnReceive( int nErr );

protected:
	// ������ �����

	// ���� ���� ������
	CWinThread* m_pSockThread;
	static UINT Proc( LPVOID pParam );
	UINT Proc();
	volatile BOOL m_bTerminate;
	volatile BOOL m_bTerminated;
	void InvokeThread();

	// ���� ����
	enum SOCKSTATE {
		empty,	// �ƹ��͵� �ƴ� ����
		accepting,	// ���Ӵ����
		connecting,	// ������
		waiting,	// ���� ���ۻ���
	} m_state;

	// �����ؾ� �� �ּ� ( m_state == connecting �϶��� ��� )
	CZInetAddr m_addrConn;

	// ������ select �Ѵ� ( select() �� ����ȭ�� ���� )
	// ���ϰ� : 0 �̸� Ÿ�Ӿƿ�, SOCKET_ERROR �� ����
	//          ��� �̸� �� ���� �ϳ��� ����
	int Select( BOOL* pbCanRead, BOOL* pbCanWrite, BOOL* pbException, DWORD dwTime = INFINITE );

	// �־��� �ּҷ� Connect �Ѵ� - m_bTerminate ��
	// ���� �Ǹ� �ٷ� FALSE �� �����Ѵ�
	BOOL AsyncConnect( const SOCKADDR_IN* pAddr );
};



// �ζ��� �Լ���
__inline CZInetAddr::CZInetAddr( const SOCKADDR_IN* pAddr )
{
	if ( !pAddr ) Init();
	else memcpy( &m_addr, pAddr, sizeof(SOCKADDR_IN) );
}

__inline CZInetAddr& CZInetAddr::operator=( const SOCKADDR_IN* pAddr )
{
	if ( !pAddr ) Init();
	else memcpy( &m_addr, pAddr, sizeof(SOCKADDR_IN) );
	return *this;
}

__inline void CZInetAddr::Init()
{
	memset( &m_addr, 0, sizeof(SOCKADDR_IN) );
	m_addr.sin_family = AF_INET;
	m_addr.sin_port = htons( 0 );
	m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
}

__inline CZInetAddr::operator const SOCKADDR_IN*() const
{
	return &m_addr;
}

#endif // !defined(AFX_ZSOCKET_H__47501A73_93F2_11D3_9AA3_0000212035B8__INCLUDED_)
