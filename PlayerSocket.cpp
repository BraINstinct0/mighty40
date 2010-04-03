// PlayerSocket.cpp: implementation of the CPlayerSocket class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mighty.h"

#include "Msg.h"
#include "ZSocket.h"
#include "PlayerSocket.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


static void call_proc( LPVOID toCall, LPVOID pSock, LPVOID pMsg, DWORD dwUser1, DWORD dwUser2 )
{
	// toCall( this, pMsg, dwUser1, dwUser2 );
	CMsg* pCallMsg = new CMsg( _T("llllll"), CMsg::mmCallSockProc,
		(long)toCall, (long)pSock, (long)pMsg,
		(long)dwUser1, (long)dwUser2 );
	Mw()->PostMessage( WM_CALLSOCKPROC, 0, (LPARAM)(LPVOID)pCallMsg );
}

CPlayerSocket::CPlayerSocket()
{
	m_uid = 0;
	m_pfnProc = 0;
	m_dwUser1 = m_dwUser2 = 0;
}

CPlayerSocket::~CPlayerSocket()
{
	Close();

	m_cs.Lock();
		// ���� �޽������� ����
		POSITION pos;
		pos = m_lpMsg.GetHeadPosition();
		while (pos) delete m_lpMsg.GetNext(pos);
		m_lpMsg.RemoveAll();
	m_cs.Unlock();
}

// �޽����� ���涧 ���� ��ٷȴٰ� (�̹� ������ �־��ٸ� ���)
// pfnProc �� ȣ���Ѵ� (pMsg �� pfnProc �� ������ ��)
void CPlayerSocket::SetTrigger( DWORD dwUser1, DWORD dwUser2,
	void (*pfnProc)( CPlayerSocket* pSocket, CMsg* pMsg,
					DWORD dwUser1, DWORD dwUser2 ) )
{
	void (*toCall)( CPlayerSocket* pSocket, CMsg* pMsg,
					DWORD dwUser1, DWORD dwUser2 ) = 0;
	CMsg* pMsg = 0;

	m_cs.Lock();

		if ( !m_lpMsg.IsEmpty() && pfnProc ) {

			toCall = pfnProc;
			pMsg = m_lpMsg.RemoveHead();
		}
		else {
			m_pfnProc = pfnProc;
			m_dwUser1 = dwUser1;
			m_dwUser2 = dwUser2;
		}

	m_cs.Unlock();

	if ( toCall ) call_proc( toCall, this, pMsg, dwUser1, dwUser2 );
}

// �޽����� ť�� �ִ´� (������ �޽����� �������� CPlayerSocket�̵�)
void CPlayerSocket::PushMsg( CMsg* pMsg, bool bTop )
{
	void (*toCall)( CPlayerSocket* pSocket, CMsg* pMsg,
					DWORD dwUser1, DWORD dwUser2 ) = 0;

	m_cs.Lock();

		if ( bTop ) m_lpMsg.AddHead( pMsg );
		else m_lpMsg.AddTail( pMsg );

		if ( m_pfnProc ) {	 // �����

			toCall = m_pfnProc;
			pMsg = m_lpMsg.RemoveHead();
			m_pfnProc = 0;
		}

	m_cs.Unlock();

	if ( toCall ) call_proc( toCall, this, pMsg, m_dwUser1, m_dwUser2 );
}

BOOL CPlayerSocket::Close()
{
	BOOL bRet = CZSocket::Close();
	PushMsg( new CMsg( _T("l"), CMsg::mmDisconnected ) );
	return bRet;
}

void CPlayerSocket::OnConnect( int nErr )
{
	PushMsg( new CMsg( _T("ll"), CMsg::mmConnected, nErr ) );
}

void CPlayerSocket::OnReceive( int nErr )
{
	if ( nErr ) { ASSERT(0); Close(); return; }

	// �޽����� �޴´�
	long len;
	if ( !SyncReceive( &len, sizeof(len) ) ) {
		// ���� ����
		Close(); return;
	}

	CMsg* pMsg = new CMsg;
	AUTODELETE_MSG_EX(pMsg,adme);

	BYTE* pData = (BYTE*)pMsg->LockBuffer( len );

	if ( !SyncReceive( pData, len ) ) {
		// ���� ����
		Close(); return;
	}

	pMsg->ReleaseBuffer();

	PushMsg( *adme.Detach() );
}

// �޽����� ������
bool CPlayerSocket::SendMsg( CMsg* pMsg )
{
	long len = pMsg->GetLength();
	return !!SyncSend( &len, sizeof(len) )
		&& !!SyncSend( pMsg->GetBuffer(), len );
}
