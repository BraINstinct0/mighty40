// PlayerNetwork.h: interface for the CPlayerNetwork class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLAYERNETWORK_H__C199C143_91F5_11D3_9A9C_000000000000__INCLUDED_)
#define AFX_PLAYERNETWORK_H__C199C143_91F5_11D3_9A9C_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CSocketBag;
class CPlayerSocket;


// ��Ʈ��ũ�� ����� �÷��̾� ��ü

class CPlayerNetwork  : public CPlayer
{
public:
	CPlayerNetwork(
		int nID,		// ���� ��ȣ ( 6���� �÷����� ���� )
		LPCTSTR sName,	// �̸�
		CWnd* pCallback );	// call-back window
	virtual ~CPlayerNetwork();

	// �� �÷��̾�� ��Ʈ��ũ�ΰ�
	virtual bool IsNetwork() const					{ return true; }

public:
	// �������̽� ( CPlay Ȯ�� )

	virtual void OnSelect2MA( int* selecting, CCardList* plCard, CEvent* );
	virtual void OnKillOneFromSix( CCard* pcCardToKill,
		CCardList* plcFailedCardsTillNow, CEvent* );
	virtual void OnElection( CGoal* pNewGoal, CEvent* );
	virtual void OnElected( CGoal* pNewGoal, CCard acDrop[3], CEvent* );
	virtual void OnTurn( CCard* pc, int* pe, CEvent* );

public:
	// �������̽� ( CMFSM �� ȣ���ϴ� Notify )

	// �� ���� ���۵� - OnBegin ��, ���� ���� �й���� ���� ����
	virtual void OnInit( CEvent* e );
	// �ٸ� ����� ���� ����� ��´�
	virtual void OnElecting( int nPlayerID, int nKiruda,
		int nMinScore, CEvent* e );
	// ī�带 �����
	virtual void OnSelect2MA( int* selecting, CEvent* e );
	// �ְ��� �ٸ� �÷��̾ ���δ�
	// bKilled : ���̸� ������ �׿���, �����̸� ��ٸ� ¤����
	virtual void OnKillOneFromSix( CCard cKill,
		bool bKilled, CEvent* e );
	// �÷��̾ ���� �� ī�带 ������
	virtual void OnSuffledForDead( CEvent* e );
	// Ư�� ��尡 ������ ���� ����� ����Ǿ���
	virtual void OnPrivilegeEnd( CGoal* pNewGoal, CCard acDrop[3], CEvent* e );
	// ī�带 ������ �˷��ش�
	// ��� ������ State �� �ִ�
	// nHandIndex �� �� ī�尡 �տ��� ī���� ���° �ε����� ī�忴�°�
	virtual void OnTurn( CCard c, int eff, int nHandIndex, CEvent* e );
	// ä�� �޽��� (bSource : ä��â�� �ҽ�)
	virtual void OnChat( int nPlayerID, LPCTSTR sNick, LPCTSTR sMsg, bool bSource );

protected:
	CSocketBag* GetSB()								{ return m_pMFSM->GetSockBag(); }
	CPlayerSocket* GetSocket()						{ return GetSB()->GetSocket( GetUID() ); }
	bool IsServer() const							{ return m_pMFSM->IsServer(); }
	long GetUID() const								{ return m_pMFSM->GetPlayerUIDFromID(GetID()); }
	long GetPlayerUIDFromID( long nPlayerID ) const	{ return m_pMFSM->GetPlayerUIDFromID(nPlayerID); }
	long GetPlayerUIDFromNum( long nPlayerNum ) const	{ return m_pMFSM->GetPlayerUIDFromNum(nPlayerNum); }
	long GetPlayerNumFromUID( long nUID ) const		{ return m_pMFSM->GetPlayerNumFromUID(nUID); }
	long GetPlayerIDFromUID( long nUID ) const		{ return m_pMFSM->GetPlayerIDFromUID( nUID ); }
	long GetPlayerIDFromNum( long nPlayerNum ) const	{ return m_pMFSM->GetPlayerIDFromNum(nPlayerNum); }

	// ��� �޽����� �����ؾ� �ϴ°��� �˻��Ѵ�
	// MFSM �� ������ �� : �� �̸��� �۽���(sNick)�� �ƴ϶��...
	// MFSM �� Ŭ���̾�Ʈ�� �� :
	//    ���� �۽��ڶ��...
	// ���� �����Ѵ�
	// v4.0���� nPlayerID ��� sNick���� ���ϴ� ������ �ٲ����. (2011.3.3)
	bool NeedSendingIfIDIs( int nPlayerID, LPCTSTR sNick ) const
	{
		return IsServer() && GetName() != sNick
			|| !IsServer() && GetName() == sNick;
	}
	bool NeedSendingIfNumIs( int nPlayerNum ) const
	{
		int nPlayerID = GetPlayerIDFromNum( nPlayerNum );

		return IsServer() && GetID() != nPlayerID
			|| GetUID() == 0 && nPlayerID == 0
			|| m_pMFSM->GetUID() >= m_pMFSM->GetState()->pRule->nPlayerNum && GetUID() == 0 && nPlayerID == m_pMFSM->GetUID();
	}

	// ���� �߻� - MFSM ���� �˸���
	// nErr  :  1 : ���� ����
	//          2 : Send ����
	//          3 : �̻��� �޽���
	void Error( int nErr );

	void SendMsg( CMsg* pMsg );

	// �̺�Ʈ ��ü ( ������ �̺�Ʈ�� ����� )
	CEvent m_e;
};

#endif // !defined(AFX_PLAYERNETWORK_H__C199C143_91F5_11D3_9A9C_000000000000__INCLUDED_)
