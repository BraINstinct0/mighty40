// SocketBag.cpp: implementation of the CSocketBag class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mighty.h"

#include "Play.h"
#include "Player.h"
#include "MFSM.h"

#include "ZSocket.h"
#include "PlayerSocket.h"

#include "SocketBag.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CSocketBag::CSocketBag()
{
	m_pMFSM = 0;
	m_pServerSocket = 0;
	m_nClients = 0;
}

CSocketBag::~CSocketBag()
{
	if ( m_pServerSocket ) {
		m_pServerSocket->ClearTrigger();
		delete m_pServerSocket;
	}
	for ( int i = 0; i < m_nClients; i++ ) {
		if ( m_aClients[i].pSocket ) {
			m_aClients[i].pSocket->ClearTrigger();
			delete m_aClients[i].pSocket;
		}
		POSITION pos = m_aClients[i].lmsg.GetHeadPosition();
		while (pos) delete m_aClients[i].lmsg.GetNext(pos);
		m_aClients[i].lmsg.RemoveAll();
	}
}

// Ŭ���̾�Ʈ�� �ʱ�ȭ (�� �÷��̾ ���� �ݺ��ؼ� ȣ������
// �ι�° ������ �θ���)
void CSocketBag::InitForClient( long uid )
{
	CLIENTITEM* pItem = &m_aClients[m_nClients++];
	ASSERT( m_nClients <= MAX_PLAYERS );

	pItem->uid = uid;
	pItem->pSocket = 0;
	pItem->ppMsg = 0;
	pItem->pEvent = 0;
}

void CSocketBag::InitForClient( CPlayerSocket* pServerSocket )
{
	ASSERT( pServerSocket );
	m_pServerSocket = pServerSocket;
	m_pServerSocket->SetTrigger( (DWORD)(LPVOID)this, (DWORD)-1, SockProc );
}

// ������ �ʱ�ȭ (�� �÷��̾ ���� �ݺ��ؼ� ȣ���Ѵ�)
void CSocketBag::InitForServer( long uid, CPlayerSocket* pSocket )
{
	CLIENTITEM* pItem = &m_aClients[m_nClients++];
	ASSERT( m_nClients <= MAX_PLAYERS );

	pItem->uid = uid;
	pItem->pSocket = pSocket;
	pItem->ppMsg = 0;
	pItem->pEvent = 0;

	if ( pSocket )
		pSocket->SetTrigger( (DWORD)(LPVOID)this, uid, SockProc );
}

// �ش� uid �� ���� �޽����� Async �ϰ�
// pMsg �� ��Ʈ�ϰ� pEvent �� ��Ʈ�����μ� �˸���
void CSocketBag::GetMsgFor( long uid, CMsg*& pMsg, CEvent* pEvent )
{
	m_cs.Lock();

	CLIENTITEM* pItem = FindUID( uid );

	ASSERT( pItem );
	ASSERT( !pItem->ppMsg && !pItem->pEvent );

	if ( !pItem->lmsg.IsEmpty() ) {	// �̹� �޽����� �ִ� !
		pMsg = pItem->lmsg.RemoveHead();
		pEvent->SetEvent();
	}
	else {	// ���� - ���߿� �˷��ش�
		pItem->ppMsg = &pMsg;
		pItem->pEvent = pEvent;
	}

	m_cs.Unlock();
}

// socket callback
void CSocketBag::SockProc( CPlayerSocket* pSocket, CMsg* pMsg,
				DWORD dwUser1, DWORD dwUser2 )
{	((CSocketBag*)(LPVOID)dwUser1)->SockProc( (long)dwUser2, pMsg, pSocket ); }


void CSocketBag::SockProc( long uid, CMsg* pMsg, CPlayerSocket* pSocket )
{
	AUTODELETE_MSG_EX(pMsg,adme);

	if ( pMsg->GetType() == CMsg::mmChat ) {
		// ä�� �޽���

		// MFSM ���� �̺�Ʈ�μ� �����Ѵ�
		m_pMFSM->EventChat( *adme.Detach(), false );
		// �ڵ鷯�� �缳��
		pSocket->SetTrigger( (DWORD)(LPVOID)this, (DWORD)uid, SockProc );
		return;
	}
	else if ( pMsg->GetType() == CMsg::mmError
		|| pMsg->GetType() == CMsg::mmDisconnected ) {
		// ���� ���� �޽���

		long lDummy;
		CString sMsg;

		if ( pMsg->GetType() == CMsg::mmError
				&& pMsg->PumpLong( lDummy )
				&& pMsg->PumpString( sMsg ) )
			// �����޽����� ���ŵ� ���
			m_pMFSM->EventExit( sMsg );

		else if ( uid == -1 )
			m_pMFSM->EventExit( _T("�����κ��� ������ ����Ǿ����ϴ�") );

		else {
			CString sMsg;
			sMsg.Format( _T("%s ���� ������ �����Ͽ����ϴ�"),
				m_pMFSM->GetState()
				->apAllPlayers[m_pMFSM->GetPlayerIDFromUID(uid)]
				->GetName() );
			m_pMFSM->EventExit( sMsg );
		}
		return;
	}


	long ori_uid = uid;

	if ( uid == -1 ) {	// ���� ���� (Ŭ���̾�Ʈ���)
		// ���� ���� �޽����� �� ��° ���Ҵ� �׻� uid �̴�
		pMsg->PumpLong( uid );
		pMsg->PumpLong( uid );
		ASSERT( uid >= 0 && uid < m_pMFSM->GetState()->pRule->nPlayerNum );
		pMsg->Rewind();
	}

	m_cs.Lock();

	CLIENTITEM* pItem = FindUID( uid );

	pItem->lmsg.AddTail( *adme.Detach() );

	if ( pItem->ppMsg ) {
		// �̺�Ʈ�� �ɷ������Ƿ� ó��
		*pItem->ppMsg = pItem->lmsg.RemoveHead();
		pItem->pEvent->SetEvent();

		pItem->ppMsg = 0;
		pItem->pEvent = 0;
	}

	m_cs.Unlock();

	// �ڵ鷯�� �缳��
	pSocket->SetTrigger( (DWORD)(LPVOID)this, (DWORD)ori_uid, SockProc );
}
