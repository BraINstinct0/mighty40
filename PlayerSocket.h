// PlayerSocket.h: interface for the CPlayerSocket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLAYERSOCKET_H__2BCC6B65_8ECD_11D3_9A8E_000000000000__INCLUDED_)
#define AFX_PLAYERSOCKET_H__2BCC6B65_8ECD_11D3_9A8E_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMsg;

// CZSocket �� ������� �ϴ� ����Ƽ ����
// �߰��Ǵ� �����,
// �޽��� �ۼ���(�޽��� ����Ʈ ���� �̿�)
// Ʈ����(�ݹ��Լ�) ��Ʈ���
// �ݹ��Լ��� �����带 �� �����忡�� ȣ���ϵ��� ��
// (MFC �� ���� ����)
// �� ������ ���� ������ �������� WM_CALLSOCKPROC �޽�����
// �����Ѵ�

class CPlayerSocket : public CZSocket
{
public:
	CPlayerSocket();
	virtual ~CPlayerSocket();

public:
	// UID ��
	long GetUID() const								{ return m_uid; }
	void SetUID( long uid )							{ m_uid = uid; }

public:
	// �޽����� ���涧 ���� ��ٷȴٰ� (�̹� ������ �־��ٸ� ���)
	// pfnProc �� ȣ���Ѵ� (pMsg �� pfnProc �� ������ ��)
	void SetTrigger( DWORD dwUser1, DWORD dwUser2,
		void (*pfnProc)( CPlayerSocket* pSocket, CMsg* pMsg,
						DWORD dwUser1, DWORD dwUser2 ) );
	// ������ ��Ʈ�� �ڵ鷯�� �����Ѵ�
	void ClearTrigger() { SetTrigger( 0, 0, 0 ); }

	// �޽����� ť�� �ִ´� (������ �޽����� �������� CPlayerSocket�̵�)
	// bTop �� ���̸�, ������ �� ���� �ִ´� (������ GetMsg ���� �� �޽�����
	// ������� )
	void PushMsg( CMsg*, bool bTop = false );

	// �޽����� ������
	bool SendMsg( CMsg* pMsg );

	// ���� ����
	virtual BOOL Close();

protected:
	long m_uid;
	CCriticalSection m_cs;
	CList<CMsg*,CMsg*> m_lpMsg;

	void (*m_pfnProc)( CPlayerSocket* pSocket, CMsg* pMsg,
						DWORD dwUser1, DWORD dwUser2 );
	DWORD m_dwUser1;
	DWORD m_dwUser2;

	virtual void OnReceive( int nErr );
	virtual void OnConnect( int nErr );
};

#endif // !defined(AFX_PLAYERSOCKET_H__2BCC6B65_8ECD_11D3_9A8E_000000000000__INCLUDED_)
