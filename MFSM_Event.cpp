// MFSM_Event.cpp: implementation of the CMFSM Events & Waits
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mighty.h"

#include "Play.h"
#include "Player.h"
#include "MFSM.h"
#include "MFSM_Notify.h"
#include "SocketBag.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// ���� �������� �ڵ��� Duplicate �Ѵ� (Wait �ϱ� ����)
HANDLE CMFSM::DuplicateThreadHandle()
{
	if ( m_pServerThread ) {

		HANDLE hRet = 0;
		if ( ::DuplicateHandle(
			GetCurrentProcess(), m_pServerThread->m_hThread,
			GetCurrentProcess(), &hRet,
			0, FALSE, DUPLICATE_SAME_ACCESS ) ) return hRet;
	}
	ASSERT(0);
	return 0;
}


// ���񽺸� �����Ѵ�
// ���ο� �����尡 ��ٷ� ���۵ȴ�
// uid : �� MFSM �� uid ( 0 �̸� �� MFSM �� �ݵ�� �����̴� )
void CMFSM::Init( long uid )
{
	m_uid = uid;

	InitStageData( 0 );

	InvokeAIThread();

	m_bTerminated = false;
	m_pServerThread = AfxBeginThread( ServiceProc, (LPVOID)this );

	ASSERT( m_pServerThread );
}

// ������ ������
UINT CMFSM::ServiceProc( LPVOID _pThis )
{
	CMFSM* pThis = (CMFSM*)_pThis;

	// ������ ������ random seed �� �ʱ�ȭ
	srand((long)time(0));

	UINT ret = pThis->Server();

	// ���� �����尡 ����Ǹ� ������ �ڽ��� �����
	delete pThis;
	return ret;
}

// m_eNotify �̺�Ʈ�� ó��
void CMFSM::ProcessNotifyEvents()
{
	if ( m_bTerminate ) {
		// ����
		// m_bTermRequest �� ���ϼ���, �ƴҼ��� �ִ�
		// ��, �Ʒ� else if ���� ���� �ø��� ���ѷ���

		// AI �����带 ���δ�

		m_bAIThreadTerminate = true;
		m_eAIThread.SetEvent();

		while ( !m_bAIThreadTerminated ) Sleep(100);

		throw this;	// ���� ���ܸ� ������ !!
		ASSERT(0);
	}
	else if ( m_bTermRequest ) {
		// ���� ��û
		for ( int i = pRule->nPlayerNum - 1; i >= 0; i-- )
			apAllPlayers[i]->OnTerminate( m_sTermRequestReason );

		m_bTerminate = true;
		m_eNotify.SetEvent();
		Wait();
	}
	else {
		// ä�� �޽���
		m_csChatMsg.Lock();
			bool bChat = !m_lChatMsg.IsEmpty();
		m_csChatMsg.Unlock();

		if ( bChat ) {
			ProcessChatMessage();
		}
		else {
			ASSERT(0);
		}
	}
}

// ä�� �޽����� ó��
void CMFSM::ProcessChatMessage()
{
	// �ϳ��� �޽����� �� �ͼ� ������
	for(;;) {

		m_csChatMsg.Lock();
			BOOL bEmpty = m_lChatMsg.IsEmpty();
			CHATITEM ci = { 0, 0 };
			if ( !bEmpty ) ci = m_lChatMsg.RemoveHead();
		m_csChatMsg.Unlock();

		if ( bEmpty ) break;

		// �̾ƿ� �޽��� ����
		bool bSource = ci.bSource;
		CMsg* pMsg = ci.pMsg;
		long uid; CString sChat;

		AUTODELETE_MSG(pMsg);

		if ( !pMsg->PumpLong( uid )
			|| !pMsg->PumpLong( uid )
			|| !pMsg->PumpString( sChat ) )
			continue;	// �߸��� �޽���

		long nPlayerID = GetPlayerIDFromUID( uid );

		// ��ο��� �Ѹ���
		for ( int i = pRule->nPlayerNum - 1; i >= 0; i-- )
			apAllPlayers[i]->OnChat( nPlayerID, sChat, bSource );
	}
}

// ���� �޽���
void CMFSM::EventExit( LPCTSTR sMsg )
{
	m_sTermRequestReason = sMsg;
	m_bTermRequest = true;
	m_eNotify.SetEvent();
}

// ä�� �޽��� ( bSource �� ���̸�, ä��â���� ������� �޽��� )
void CMFSM::EventChat( CMsg* pMsg, bool bSource )
{
	CHATITEM ci;
	ci.pMsg = pMsg;
	ci.bSource = bSource;

	m_csChatMsg.Lock();
		m_lChatMsg.AddTail( ci );
	m_csChatMsg.Unlock();

	m_eNotify.SetEvent();
}

// �������� ���� ���´�
// mmGameInit�� ��� �ɴ� �ڸ��� ���´� (v4.0: 2011.1.14)
void CMFSM::GetDeckFromServer()
{
	int i;

	ASSERT( !IsServer() && m_pSockBag );

	CMsg* pMsg = 0;
	AUTODELETE_MSG(pMsg);

	// mmGameInit �޽����� �� ������ �ִ�
	m_pSockBag->GetMsgFor( 0, pMsg, &m_eSock );
	WaitEvent( &m_eSock );

	long nType;
	long uid;
	CCardList lServerDeck;

	if ( pMsg->PumpLong( nType )
			&& ( nType == CMsg::mmGameInit
				|| nType == CMsg::mmGameSuffledForDead )
			&& pMsg->PumpLong( uid )
			&& uid == 0
			&& pMsg->PumpCardList( lServerDeck )
			&& lDeck.GetCount() == lServerDeck.GetCount() )
	{
		lDeck = lServerDeck;
		if ( nType == CMsg::mmGameInit )
		{
			for ( i = 0; i < nPlayers; i++ )
			{
				if( !pMsg->PumpLong( uid ) ) goto hell;
				apPlayers[i] = apAllPlayers[uid];
			}
			return;
		}
		else return;
	}

hell:
	// �߸��� �޽����� ����
	ASSERT(0);
	EventExit( _T("�����κ��� ����ġ ���� ������ �����߽��ϴ�") );
}

// ���� CPlayer �� �θ��� �Լ�
// ������ ��Ų �۾��� �Ϸ�Ǿ����� MFSM ���� �˸���
void CMFSM::Notify( int nPlayerID )
{
#ifdef _DEBUG
	ASSERT( m_apePlayer[nPlayerID] );
	for ( int i = 0; i < m_nWaitingPlayer; i++ )
		if ( m_apePlayer[nPlayerID] == m_apeWaitingPlayer[i] ) {
			m_apePlayer[nPlayerID]->SetEvent();
			return;
		}
	ASSERT(0);
#else
	m_apePlayer[nPlayerID]->SetEvent();
#endif
}

// �̺�Ʈ�� �����ϰ�, m_apeWaitingPlayer ���� �ʱ�ȭ �Ѵ�
void CMFSM::ResetEvents()
{
	m_nWaitingPlayer = 0;
	for ( int i = 0; i < MAX_CONNECTION; i++ ) {
		m_abWaitingPlayerRedo[i] = true;
		m_apeWaitingPlayer[i] = m_apePlayer[i];
		m_apePlayer[i]->ResetEvent();
	}
	m_apeWaitingPlayer[MAX_CONNECTION] = &m_eNotify;
	m_nWaitingPlayer = MAX_CONNECTION;
}

// �̺�Ʈ�� ����ϸ�, ���ܸ� ó���ϴ� �Լ�
// ������� �÷��̾��� �̺�Ʈ�� ����, Set �Ǹ� m_apeWaitingPlayer
// ���� �� �̺�Ʈ�� �����Ѵ�
// ���ϰ��� ��� event �� ��Ʈ�� �÷��̾��� PlayerID �Ǵ� -1 (����)
int CMFSM::Wait()
{
	// ���� m_eNotify �� �˻��Ѵ�
lblCheckNotify:
	while ( m_eNotify.Lock(0) )
		ProcessNotifyEvents();

	// ������ �̺�Ʈ�� m_eNotify �� �˻��Ѵ�
	DWORD ret;
	{
		CMultiLock ml( (CSyncObject**)m_apeWaitingPlayer, m_nWaitingPlayer+1 );
		ret = ml.Lock( INFINITE, FALSE );
	}

	if ( ret == WAIT_OBJECT_0 + m_nWaitingPlayer ) { // m_eNotify
		ProcessNotifyEvents();
		goto lblCheckNotify;
	}

	else if ( WAIT_OBJECT_0 <= ret
		&& ret < WAIT_OBJECT_0 + m_nWaitingPlayer ) {
		// ��� �ϳ��� ��Ʈ�Ǿ���

		int i;
		int nIndex = int( ret - WAIT_OBJECT_0 );
		CEvent* pEvent = m_apeWaitingPlayer[nIndex];

		// m_apeWaitingPlayer[] ���� �� �̺�Ʈ�� ����
		for ( i = nIndex; i < m_nWaitingPlayer; i++ )
			m_apeWaitingPlayer[i] = m_apeWaitingPlayer[i+1];
		m_nWaitingPlayer--;

		// � �÷��̾��� �̺�Ʈ���� �˻�
		for ( i = 0; i < MAX_CONNECTION; i++ )
			if ( m_apePlayer[i] == pEvent )
				break;

		if ( i >= MAX_CONNECTION ) {
			ASSERT(0);
			return -1;
		}

		return i;
	}
	else {
		ASSERT(0);
		return -1;
	}
}

// �ش� �̺�Ʈ�� Lock(=Wait)�Ѵ� - ���� ���ǽ� CMFSM �ڽ���
// throw �ȴ�
void CMFSM::WaitEvent( CEvent* pEvent )
{
	// ���� m_eNotify �� �˻��Ѵ�
	while ( m_eNotify.Lock(0) )
		ProcessNotifyEvents();

	// ������ �̺�Ʈ�� m_eNotify �� �˻��Ѵ�
	CSyncObject* apObj[2];
	apObj[0] = pEvent; apObj[1] = &m_eNotify;
	CMultiLock ml( apObj, 2 );

	DWORD ret;

	for(;;){
		ret = ml.Lock( INFINITE, FALSE );
		if ( ret == WAIT_OBJECT_0 + 1 ) { // m_eNotify
			ProcessNotifyEvents();
			continue;
		}
		else break;
	}
	ASSERT( ret == WAIT_OBJECT_0 );
}
