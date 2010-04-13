// PlayerMai.h: interface for the CPlayerMai class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLAYERMAI_H__96075965_D2BC_11D2_981A_0000212035B8__INCLUDED_)
#define AFX_PLAYERMAI_H__96075965_D2BC_11D2_981A_0000212035B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CInfoBar;
class CMsg;

// MAI-DLL �� ������� �ϴ� �÷��̾� ��ü
// ������ ���� ���� �Ѱ��� �� �ְ� �Ǹ�
// call-back �����쿡 WM_PLAYER �޽����� �����ش�
// �� �� LPARAM ���� ������ CMsg* ��
// �� ���� ����ڰ� ���� �ؾ� �Ѵ�


class CPlayerMai : public CPlayer
{
public:
	CPlayerMai(
		int nID,		// ���� ��ȣ ( 6���� �÷��̾��� ���� )
		LPCTSTR sName,	// �̸�
		CWnd* pCallback,// call-back window
		LPCTSTR sDLLPath,// MAI-DLL ���
		LPCTSTR sOption);// �ɼ� ��Ʈ�� (null �� ��� �ʱⰪ)
	virtual ~CPlayerMai();

	// DLL �� ����� �ε�Ǿ��°��� �� �Լ��� �� �� �ִ�
	bool IsDLLWorking() const						{ return m_pos ? true : false; }

	// �� �÷��̾�� ��ǻ�� AI �ΰ�
	virtual bool IsComputer() const					{ return true; }

	// CPlayer �������̽� ����

	virtual void OnSelect2MA( int* selecting, CCard* pcShow, CEvent* e );
	virtual void OnKillOneFromSix( CCard* pcCardToKill,
		CCardList* plcFailedCardsTillNow, CEvent* );
	virtual void OnElection( CGoal* pNewGoal, CEvent* );
	virtual void OnTurn( CCard* pc, int* pe, CEvent* e );

protected:
	// ���� ��� & �Լ�

	// update �� ���� CInfoBar ĸ��
	class MAIDLL_UPDATE_IMPL : public MAIDLL_UPDATE
	{
	public:
		MAIDLL_UPDATE_IMPL() : m_pInfo(Ib()) {}
		void SetProgress( int nPercentage );
		void SetText( LPCTSTR sText );
	protected:
		CInfoBar* m_pInfo;
	};

	// DLL exported �Լ� ������
	struct MAI_FUNCTIONS {
		pfMaiGetInfo		MaiGetInfo;
		pfMaiGetPlay		MaiGetPlay;
		pfMaiOption			MaiOption;
		pfMaiReleasePlay	MaiReleasePlay;
	} m_funcs;

	// update ��ü
	MAIDLL_UPDATE_IMPL m_update;

	// �̺�Ʈ
	CEvent m_eTemp;

	// �������� DLL ���� �����Ѵ�
	struct MAIDLL {
		CString sPath;			// DLL �н�
		HMODULE hDLL;			// �ڵ�
		MAI_FUNCTIONS funcs;	// �Լ���
		int ref;				// ���� ī����
	};
	static CList< MAIDLL, MAIDLL& > s_lMaiDll;
	static CCriticalSection s_csMaiDll;

	// s_lMaiDll �������� �� DLL ���� ������
	// �� ���� 0 �̸� �� PlayerMai ��ü�� invalid �� ���̴�
	POSITION m_pos;

	// DLL ���� �Լ� �����͸� ��� (m_funcs),
	// CPlay ��ü�� ��� ��
	// ���� s_lMaiDll �� �̹� ���� DLL �� �ִ� ��� ref ���� ������
	bool LoadMaiDll( LPCTSTR sPath, LPCTSTR sOption, MAIDLL_UPDATE* );
	// CPlay ��ü�� ������ - ���� �ش� DLL �� ref ��
	// 0 �� �Ǵ� ��� �� DLL �� Unload ��
	bool UnloadMaiDll();
	// ����Ʈ AI �� �ε��� ( DLL �б� ���н� )
	void LoadDefault( LPCTSTR sOption, MAIDLL_UPDATE* );

	// �ּ� nMillisecToEnsure �� ��ٸ��� �ϴ� �𷡽ð�
	struct CSandClock {
		CSandClock( long nMillisecToEnsure )
			: m_nMillisecToEnsure( nMillisecToEnsure ) {
			m_dwLastTick = ::GetTickCount(); }
		~CSandClock() {
			long diff = (long)( ::GetTickCount() - m_dwLastTick );
			if ( diff < m_nMillisecToEnsure )
				Sleep( min( m_nMillisecToEnsure - diff, 5000 ) );
			else Sleep(0); }
		long m_nMillisecToEnsure;
		DWORD m_dwLastTick;
	};
	friend CSandClock;
};

#endif // !defined(AFX_PLAYERMAI_H__96075965_D2BC_11D2_981A_0000212035B8__INCLUDED_)
