// SocketBag.h: interface for the CSocketBag class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOCKETBAG_H__4795E9E3_91BA_11D3_9A9B_0000212035B8__INCLUDED_)
#define AFX_SOCKETBAG_H__4795E9E3_91BA_11D3_9A9B_0000212035B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMFSM;
class CPlayerSocket;

// ���ϵ��� ���� - CPlayerSocket �� CMFSM ��
// ���� ����

class CSocketBag  
{
public:
	CSocketBag();
	virtual ~CSocketBag();

	// �ݵ�� ȣ���� ��
	void SetMFSM( CMFSM* pMFSM ) { m_pMFSM = pMFSM; }

	// �ʱ�ȭ - �� ���� �� �ϳ��� �ݵ�� ȣ���ؾ� �Ѵ�

	// Ŭ���̾�Ʈ�� �ʱ�ȭ (�� �÷��̾ ���� �ݺ��ؼ� ȣ������
	// �ι�° ������ �θ���)
	void InitForClient( long uid );
	void InitForClient( CPlayerSocket* pServerSocket );
	// ������ �ʱ�ȭ (�� �÷��̾ ���� �ݺ��ؼ� ȣ���Ѵ�)
	void InitForServer( long uid, CPlayerSocket* pSocket );

public:
	// �ش� uid �� ���� �޽����� Async �ϰ�
	// pMsg �� ��Ʈ�ϰ� pEvent �� ��Ʈ�����μ� �˸���
	void GetMsgFor( long uid, CMsg*& pMsg, CEvent* pEvent );

	// �ش� uid �� ���� ������ ��´� ( Ŭ���̾�Ʈ �����
	// ������ ServerSocket �� ���ϵȴ� )
	CPlayerSocket* GetSocket( long uid )
	{
		if ( m_pServerSocket ) return m_pServerSocket;
		else return FindUID(uid)->pSocket;
	}

protected:
	// �� ����
	CPlayerSocket* m_pServerSocket;
	int m_nClients;
	struct CLIENTITEM {
		long uid;
		CPlayerSocket* pSocket;
		CList<CMsg*,CMsg*> lmsg;	// �޽��� ( AddTail, RemoveHead ť )
		CMsg** ppMsg;	// �޽��� Ʈ���� ���
		CEvent* pEvent;	// �޽��� Ʈ���� �Ϸ��̺�Ʈ
	} m_aClients[MAX_CONNECTION];

	CMFSM* m_pMFSM;
	CCriticalSection m_cs;

protected:
	// UID �� ã�´�
	CLIENTITEM* FindUID( long uid )
	{
		for ( int i = 0; i < m_nClients; i++ )
			if ( m_aClients[i].uid == uid ) return &m_aClients[i];
		ASSERT(0); return 0;
	}
	// socket callback
	static void SockProc( CPlayerSocket* pSocket, CMsg* pMsg,
					DWORD dwUser1, DWORD dwUser2 );
	void SockProc( long uid, CMsg* pMsg, CPlayerSocket* pSocket );
};

#endif // !defined(AFX_SOCKETBAG_H__4795E9E3_91BA_11D3_9A9B_0000212035B8__INCLUDED_)
