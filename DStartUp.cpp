// DStartUp.cpp: implementation of the DStartUp class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mighty.h"
#include "DSB.h"
#include "DStartUp.h"
#include "DEtc.h"

#include "MFSM.h"
#include "Play.h"
#include "Player.h"
#include "PlayerHuman.h"
#include "PlayerMai.h"

#include "ZSocket.h"
#include "BoardWrap.h"
#include "DConnect.h"
#include "InfoBar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// nMode  0 : �ʱ� �޴�  1 : ��Ƽ�÷��̾� �޴�
void DStartUp::Create( int nMode )
{
	m_nMode = nMode;

	if ( m_nMode == 0 )	// �ʱ� �޴�
		DSB::Create( 0, 0, 20, 20, -1 );
	else
		DSB::Create( 0, 0, 18, 17, -1 );

	// �� DSB �� �����Ǵ� ���´�, ������ ���� �ʱ� �����̹Ƿ�
	// ���¹ٸ� �����Ѵ�
	Ib()->ShowChat( false );
	Ib()->Reset();
}

// �ʱ�ȭ ( ������ �� ȣ��� )
void DStartUp::OnInit()
{
	DSB::OnInit();

	RegisterHotspot(
		3, 4, -1, -1, true, 0,
		m_nMode == 0 ? _T("��Ȧ�� ����Ƽ") : _T("���⼭ �ϱ�"),
		&s_colWhite, &s_tdMidium, &s_colCyan, &s_tdMidiumOutline,
		(LPVOID)1 );
	RegisterHotspot(
		3, 7, -1, -1, true, 0,
		m_nMode == 0 ? _T("���Բ� ����Ƽ") : _T("�ٸ������� ����"),
		&s_colWhite, &s_tdMidium, &s_colCyan, &s_tdMidiumOutline,
		(LPVOID)2 );
	RegisterHotspot(
		3, 13, -1, -1, true, 0,
		m_nMode == 0 ? _T("����Ƽ ������") : _T("���� �޴���"),
		&s_colWhite, &s_tdMidium, &s_colCyan, &s_tdMidiumOutline,
		(LPVOID)0 );

	if ( m_nMode == 0 )
		RegisterHotspot(
			3, 10, -1, -1, true, 0,
			_T("����Ƽ��..."),
			&s_colWhite, &s_tdMidium, &s_colCyan, &s_tdMidiumOutline,
			(LPVOID)3 );
	else
		RegisterHotspot(
			3, 10, -1, -1, true, 0,
			_T("Mighty.Net"),
			&s_colGray, &s_tdMidium, &s_colGray, &s_tdMidium,
			(LPVOID)3 );
}

// �� ������ Highlight �ǰų� Normal �� �� �� ȣ��Ǵ� �Լ�
// bHighlight �� ���̸� highlight �Ǿ���
void DStartUp::OnHighlight( HOTSPOT& hs, bool bHighlight )
{
	if ( !bHighlight ) Ib()->Reset();
	else if ( m_nMode == 0 ) {

		switch ((int)hs.pVoid) {
		case 0: Ib()->SetText( _T("����Ƽ ������ �����մϴ�") ); break;
		case 1: Ib()->SetText( _T("ȥ�ڼ� ��ǻ�Ϳ� ����Ƽ ������ �մϴ�") ); break;
		case 2: Ib()->SetText( _T("��Ʈ��ũ�� ���ؼ� �ٸ� ����ڿ� ����Ƽ ������ �մϴ�") ); break;
		case 3: Ib()->SetText( _T("����Ƽ ������ �����ݴϴ�") ); break;
		}
	}
	else {

		switch ((int)hs.pVoid) {
		case 0: Ib()->SetText( _T("���� �޴��� ���ư��ϴ�") ); break;
		case 1: Ib()->SetText( _T("������ �Ǿ� �ٸ� ������� ������ ��ٸ��ϴ�") ); break;
		case 2: Ib()->SetText( _T("�ٸ� �������� �����մϴ�") ); break;
		case 3: Ib()->SetText( _T("Mighty.Net �� �����մϴ�") ); break;
		}
	}
}

// CBoard �� ȣ���ϴ� �׸��� �Լ� - ��µ� shade-box �� �� �Լ���
// �������̵� �Ͽ� �����ؾ� �Ѵ�
// ���̽� �Լ��� ������ ��ü�� ���� ���� ������ �׸���
void DStartUp::OnDraw(
	CDC* pDC,
	bool bBrushOrigin )
{
	DSB::OnDraw( pDC, bBrushOrigin );

	if ( m_nMode == 0 ) {

		PutText( pDC, _T("����Ƽ ������ �����մϴ� !!"), 2, 1, true,
			s_colYellow, s_tdShade );
		PutText( pDC, _T("Mighty Network Ver 3.2"), 3, 16, true,
			s_colCyan, s_tdShade );
		PutText( pDC, _T("Copyright 1999 �幮��"), 3, 17, true,
			s_colCyan, s_tdShade );
	}
	else {

		PutText( pDC, _T("�ٸ� ����ڿ� �Բ� �մϴ�"), 2, 1, true,
			s_colYellow, s_tdShade );
	}
}

// �� ������ Ŭ���� �� �Ҹ��� �Լ�
// LPVOID ���� 0 �̸� ���� (X�ڸ� Ŭ��) ��û
void DStartUp::OnClick( LPVOID pVoid )
{
	if ( m_nMode == 0 ) {	// �ʱ� �޴�

		// �� ���̾�αװ� pVoid==0 ���� ������
		// �ʱ�޴��� ��� ���� ������
		if ( !pVoid )
			Mw()->PostMessage( WM_CLOSE );

		switch ( (int)pVoid ) {

		case 0: DSB::OnClick(0); break;

		case 0xffffffff:
		case 1: {
			VERIFY( CreateOnePlayerGame() );
			break;
		}
		case 2:
			(new DStartUp(m_pBoard))->Create( 1 );
			DSB::OnClick(0); break;
			break;
		case 3:
			(new DAbout(m_pBoard))->Create();
			break;
		}
	}
	else {	// ��Ƽ�÷��̾� �޴�

		// �� ���̾�αװ� pVoid==0 ���� ������
		// �ʱ� �޴��� ȣ��
		if ( !pVoid )
			(new DStartUp(m_pBoard))->Create( 0 );

		switch ( (int)pVoid ) {

		case 0: DSB::OnClick(0); break;

		case 0xffffffff:
		case 1:		// ������ ��
			Ib()->ShowChat();	// ä��â ǥ��
			(new DConnect(m_pBoard))->Create( 0 );
			DSB::OnClick(0);
			break;
		case 2:		// �ٸ� ������ ����
			Ib()->ShowChat();	// ä��â ǥ��
			(new DConnecting(m_pBoard))->Create();
			DSB::OnClick(0);
			break;
		case 3:		// Mighty.Net
			break;
		}
	}
}

void DStartUp::OnClickSound()
{
	PlaySound( IDW_CHULK, true );
}

// ���� �ɼǿ� �����ؼ� 1�ο� ����Ƽ MFSM �� ����(new)�Ѵ�
// AI ���� �ε� ���� �� ���� DSB �� ����
// 0 �� �����Ѵ�
bool DStartUp::CreateOnePlayerGame()
{
	int nPlayers = Mo()->rule.nPlayerNum;

	CPlayer* apPlayers[MAX_PLAYERS];

	// Human Player ����
	apPlayers[0] = new CPlayerHuman(
		0, Mo()->aPlayer[0].sName, *m_pBoard );

	// AI Players ����
	bool bFailed = false;
	CString sFailedDLL;
	for ( int i = 1; i < nPlayers; i++ ) {

		CPlayerMai* pPlayer = new CPlayerMai(
			i, Mo()->aPlayer[i].sName, *m_pBoard,
			Mo()->aPlayer[i].sAIDLL, Mo()->aPlayer[i].sSetting );
		apPlayers[i] = pPlayer;
		if ( !pPlayer->IsDLLWorking() ) {
			bFailed = true;
			sFailedDLL = Mo()->aPlayer[i].sAIDLL;
		}
	}

	// ���� ������ �ش�
	for ( int j = 0; j < nPlayers; j++ )
		apPlayers[j]->SetMoney( BASE_MONEY );

	// ���� SetMFSM �� �ϴ� ���� this �� ����� ��� DSB �� �������
	// ��� �޽����� ���� ���� m_pBoard �� ��������
	CBoardWrap* pBoard = m_pBoard;

	// ���ο� MFSM �� �����Ѵ�
	CMFSM* pMFSM = new CMFSM( Mo()->rule.Encode(), apPlayers );
	pMFSM->Init( 0 );

	m_pBoard->SetMFSM(pMFSM);

	// DLL �ε尡 �������� �� ��� �޽����� �ش�
	if ( bFailed && !sFailedDLL.IsEmpty() ) {

		LPCTSTR asLine[8];
		asLine[0] = _T("������ AI DLL �� �дµ� �����߽��ϴ�");
		asLine[1] = _T("");
		asLine[2] = sFailedDLL;
		asLine[3] = _T("");
		asLine[4] = _T("�ɼ� ��ȭ������ AI �ǿ��� �� ������");
		asLine[5] = _T("��θ� �ٽ� Ȯ���ϼ���");
		asLine[6] = _T("");
		asLine[7] = _T("�ӽ÷� �⺻ AI �� �о���Դϴ�");
		DMessageBox* pMB = new DMessageBox(pBoard);	// ���� pBoard �� ���
		if ( Mo()->bUseSound ) MessageBeep( MB_ICONEXCLAMATION );
		pMB->Create( true, 8, asLine );
	}

	return true;
}
