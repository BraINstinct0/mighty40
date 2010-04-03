// Play.h: interface for the CPlay class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLAY_H__E21B2747_CFA8_11D2_9810_000000000000__INCLUDED_)
#define AFX_PLAY_H__E21B2747_CFA8_11D2_9810_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

struct CState;
struct CGoal;
class CCard;

// ����Ƽ���� AI, �Ǵ� ����� ���� �� �ִ�
// ���۵��̳� �̺�Ʈ���� �����ϴ� �������̽�

struct CPlay  
{
	virtual ~CPlay() {}

	// �ϳ��� ������ ���۵�
	// �� ���ӿ��� ���Ǵ� ���� ������ �˷��ش�
	// �� ���� ������ CCard::GetState() �ε� �� �� �ִ�
	// pState->nCurrentPlayer ���� �ٷ� �ڱ� �ڽ���
	// ��ȣ�̸�, �� ���� �ڽ��� �ν��ϴµ� ���ȴ�
	virtual void OnBegin( const CState* pState ) = 0;

	// ������ ��´�
	// pNewGoal �� state.goal �� ��������� ������ ��ϵǾ� �ִ�
	// �н��Ϸ��� nMinScore �� 0���� ��Ʈ
	// Ư�� �� �̽��� ���ϴ� ��� ������ -1�� ��Ʈ
	// pNewGoal.nFriend �� ������� �ʴ´�
	virtual void OnElection( CGoal* pNewGoal ) = 0;

	// 6������ �缱�� ��� �� ����� �׿��� �Ѵ�
	// ���� ī�带 �����ϸ� �ȴ� - �� �� �Լ���
	// �ݺ������� ȣ��� �� �ִ� - �� ���
	// CCardList �� ���ݱ��� ������ ī���� ����Ʈ��
	// �����Ǿ� ȣ��ȴ�
	// 5�� �����ϸ� (�� ��� �˰����� �߸��Ǿ��ų�
	// ����� �� �� �����ϴ� ���) ���Ƿ� ������ 5�� ��
	// �ϳ��� �״´� !
	virtual void OnKillOneFromSix(
		CCard* pcCardToKill, CCardList* plcFailedCardsTillNow ) = 0;
	// 7������ �缱�� ��� �� ����� �׿��� �Ѵ�
	// �� �Լ��� �� �� �ϳ��� ���̴� �Լ���,
	// OnKillOneFromSix�� ����.
	// 5�� �����ϸ� (�� ��� �˰����� �߸��Ǿ��ų�
	// ����� �� �� �����ϴ� ���) ���Ƿ� ������ 5�� ��
	// �ϳ��� �״´� !
	virtual void OnKillOneFromSeven(
		CCard* pcCardToKill, CCardList* plcFailedCardsTillNow ) = 0;

	// �缱�� ���
	// pNewGoal �� state.goal �� ��������� ������ ��ϵǾ� �ִ�
	// pNewGoal �� ���ο� ������ �����ϰ� (nFriend����)
	// acDrop �� ���� �� ���� ī�带 �����Ѵ�
	// * ���� * �� �Լ� ���Ŀ� �Ѹ��� �׾ �÷��̾���
	// ��ȣ(pState->nCurrentPlayer)�� ����Ǿ��� �� �ִ�
	virtual void OnElected( CGoal* pNewGoal, CCard acDrop[3] ) = 0;

	// ���Ű� ������ �� ������ ���۵Ǿ����� �˸���
	// * ���� * �� �Լ� ���Ŀ� �Ѹ��� �׾ �÷��̾���
	// ��ȣ(pState->nCurrentPlayer)�� ����Ǿ��� �� �ִ�
	virtual void OnElectionEnd() = 0;

	// ī�带 ����
	// pc �� �� ī��, pe �� pc�� ��Ŀ�ΰ�� ī�� ���,
	// pc�� ��Ŀ���ΰ�� ���� ��Ŀ�� ����(0:��Ŀ�� �ƴ�)
	// �� �ܿ��� ����
	virtual void OnTurn( CCard* pc, int* pe ) = 0;

	// �� ���� �������� �˸���
	// ���� state.plCurrent ���� �״�� �����ִ�
	virtual void OnTurnEnd() = 0;
};


/////////////////////////////////////////////////////////////////////////////
// Mighty AI DLL ���� export �ؾ� �ϴ� �Լ���� ���� ����ü

// MAI DLL ����
struct MAIDLL_INFO {
	CString sName;		// MAI �̸�
	CString sExplain;	// ����
};

// ������Ʈ �Լ�
// ���� AI DLL �� ������ ���� ��Ȳ�� ����� �ش�
struct MAIDLL_UPDATE {
	// ���� ���� �ܰ� ( 100 ��� �� )
	virtual void SetProgress( int nPercentage ) = 0;
	// ��µ� ��Ʈ�� ( ����Ʈ�� '������..' ���� ���� �޽��� )
	virtual void SetText( LPCTSTR ) = 0;
};

// DLL ������ ��´�
typedef void (*pfMaiGetInfo)( MAIDLL_INFO* );

// �ɼ� ��Ʈ���� ������ CPlay ��ü�� ��´�
// ��, sOption �� NULL �� ���� ����Ʈ ��
typedef CPlay* (*pfMaiGetPlay)( LPCTSTR sOption, MAIDLL_UPDATE* );

// �� ��ü�� ���� �ɼ� ���̾�α� �ڽ��� ����ϰ�
// �ɼ��� �����Ѵ�
// ������ �ɼ��� ���� AI �� ��� �� �Լ��� �����Ǿ ����
// hWnd : ���̾�α� �ڽ��� �θ� ������
typedef void (*pfMaiOption)( CPlay*, HWND );

// CPlay ��ü�� �ǵ����ش�
// �ɼ� ��Ʈ���� ������
typedef void (*pfMaiReleasePlay)( CPlay*, CString* psOption );


#endif // !defined(AFX_PLAY_H__E21B2747_CFA8_11D2_9810_000000000000__INCLUDED_)
