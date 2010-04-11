// DReport.cpp: implementation of the DReport class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mighty.h"
#include "DSB.h"
#include "DReport.h"

#include "Play.h"
#include "Player.h"
#include "MFSM.h"

#include "BoardWrap.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

void DReport::Create( const CMFSM* pMFSM, bool* pbCont, CEvent* pEvent )
{
	int i;

	SetPage(0);

	m_pMFSM = pMFSM;
	const CState* pState = m_pMFSM->GetState();

	for ( i = 0; i < pState->nPlayers; i++ ) {
		for ( int j = 0; j < 4; j++ )
			m_aanHistory[j][i] = pState->aanHistory[j][i];
		// �̹� ���� ����Ǿ��� - ���� ���� ������ ����ؾ� �ȴ�
		m_anMoneyBefore[i] = m_anMoneyAfter[i] =
			pState->apPlayers[i]->GetPrevMoney();
	}

	m_pMFSM->GetReport( m_bDefWin, &m_nDefPointed, m_aanHistory[4],
		m_asCalcMethod, m_asExpr, m_anMoneyAfter );

	m_bGameOver = false;
	for ( i = 0; i < pState->nPlayers; i++ )
		if ( m_anMoneyAfter[i] <= 0 ) {
			m_bGameOver = true;
			break;
		}

	m_nRank = 1;
	for ( i = 1; i < pState->pRule->nPlayerNum; i++ )
		if ( pState->apAllPlayers[i]->GetMoney() > pState->apAllPlayers[0]->GetMoney() )
			m_nRank++;

	m_bNetworkGame = pState->IsNetworkGame();

	m_nMe = pState->apPlayers[0]->IsHuman() ? 0 : -1;
	// ���� ���������ٸ�, �ְ��� ���а� ���� ���θ� �����Ѵ�
	int nMe = m_nMe; if ( nMe == -1 ) nMe = pState->nMaster;
	m_bWin = m_bDefWin && ( pState->nMaster == nMe
							|| pState->nFriend == nMe )
			|| !m_bDefWin && !( pState->nMaster == nMe
							|| pState->nFriend == nMe );

	if ( m_bWin ) PlaySound( IDW_WIN, true );
	else PlaySound( IDW_LOSE, true );

	DSB::Create( 0, 0, 25, 25, -1 );
	SetAction( pEvent, pbCont );
}

// �ʱ�ȭ ( ������ �� ȣ��� )
void DReport::OnInit()
{
	DSB::OnInit();

	if ( !m_bGameOver ) {

		RegisterHotspot( 7, 23, -1, -1, true, 0, _T("����"),
			&s_colWhite, &s_tdShade, &s_colCyan, &s_tdShadeOutline,
			(LPVOID)1 );

		RegisterHotspot( 14, 23, -1, -1, true, 0, _T("   Ȯ��   "),
			&s_colWhite, &s_tdShade, &s_colCyan, &s_tdShadeOutline,
			(LPVOID)0 );
	}
	else {

		RegisterHotspot( 10, 23, -1, -1, true, 0, _T("   ���� ��   "),
			&s_colWhite, &s_tdShade, &s_colCyan, &s_tdShadeOutline,
			(LPVOID)0 );
	}

	RegisterHotspot( 18, 1, -1, -1, true, 0,
		Mo()->bMoneyAsSupport ? _T("��������ȭ") : _T("������ȭ"),
		&m_acolPage[0], &m_atdPage[0], &m_acolPage[0], &s_tdShadeOutline,
		(LPVOID)100 );
	RegisterHotspot( 18, 2, -1, -1, true, 0, _T("�м�����"),
		&m_acolPage[1], &m_atdPage[1], &m_acolPage[1], &s_tdShadeOutline,
		(LPVOID)101 );
	RegisterHotspot( 18, 3, -1, -1, true, 0, _T("��������"),
		&m_acolPage[2], &m_atdPage[2], &m_acolPage[2], &s_tdShadeOutline,
		(LPVOID)102 );

	if ( m_bNetworkGame ) {

		RegisterHotspot( 18, 4, -1, -1, true, 0, _T("�������"),
			&m_acolPage[3], &m_atdPage[3], &m_acolPage[3], &s_tdShadeOutline,
			(LPVOID)103 );
	}
}

// CBoard �� ȣ���ϴ� �׸��� �Լ� - ��µ� shade-box �� �� �Լ���
// �������̵� �Ͽ� �����ؾ� �Ѵ�
// ���̽� �Լ��� ������ ��ü�� ���� ���� ������ �׸���
void DReport::OnDraw(
	CDC* pDC,
	bool bBrushOrigin )
{
	DSB::OnDraw( pDC, bBrushOrigin );

	const CState* pState = m_pMFSM->GetState();
	int nPlayers = pState->nPlayers;

	// �̸����� �׷����� ���ж����� �ʿ��� ����
	CRect rcBox; GetRect( &rcBox );
	int xRight = rcBox.right - GetDSBUnit();
	CPen penY( PS_SOLID, 1, s_colYellow );
	CPen penC( PS_SOLID, 1, s_colCyan );

	// Ÿ��Ʋ
	if ( m_bGameOver && m_nPage == 3 )	// ���� ��
		PutText( pDC, _T("- ���� �� -"), -1, 1, true,
			s_colYellow, s_tdMidiumOutline );
	else if ( m_bWin ) {				// �¸�
		if ( m_nMe == -1 )				// ��������� ������ �¸����� �� �¸��� ������ ��޵�
			PutText( pDC, _T("���� ��"), -1, 1, true,
				s_colYellow, s_tdMidiumOutline );
		else							// �ƴ϶��
			PutText( pDC, _T("�¸� !"), -1, 1, true,
				s_colYellow, s_tdMidiumOutline );
	}
	else {								// �й�
		if ( m_nMe == -1 )				// ��������� ������ �¸����� �� �¸��� ������ ��޵�
			PutText( pDC, _T("�ߴ� ��"), -1, 1, true,
				s_colYellow, s_tdMidiumOutline );
		else							// �ƴ϶��
			PutText( pDC, _T("�й� !"), -1, 1, true,
				s_colYellow, s_tdMidiumOutline );
	}

	if ( m_nPage == 0 ) {	// ������/���� ��ȭ ������

		CString sTitle;
		sTitle.Format(_T("%d �� ���࿡ %d �� ����"),
			pState->goal.nMinScore, m_nDefPointed );
		PutText( pDC, sTitle,
			2, 4, true, s_colWhite, s_tdShade );
		CRect rc = PutText( pDC, _T("[�⺻]: ") + m_asCalcMethod[0],
			2, 5, true, s_colCyan, s_tdShade );
		PutText( pDC, m_asCalcMethod[1],
			rc.left+GetDSBUnit()*4, rc.bottom+GetDSBUnit()/3, false, s_colCyan, s_tdShade );

		int y = 8;
		for ( int j = 0; j < 2; j++ )	// ����, �ߴ翡 ���� ����
		for ( int i = 0; i < nPlayers; i++ ) {

			// ó������ ���縸 ǥ��
			if ( j == 0 && !( i == pState->nFriend
							|| i == pState->nMaster ) ) continue;
			// �ι�°�� �ߴ縸 ǥ��
			if ( j != 0 &&  ( i == pState->nFriend
							|| i == pState->nMaster ) ) continue;

			CRect rc = PutText( pDC, pState->apPlayers[i]->GetName(),
				1, y++, true,
				j == 0 ? s_colCyan : s_colYellow, i == m_nMe ? s_tdShadeOutlineOpaque : s_tdShadeOpaque );

			pDC->SelectObject( i == 0 ? &penC : &penY );
			pDC->MoveTo( rc.right + GetDSBUnit(), rc.CenterPoint().y );
			pDC->LineTo( xRight, rc.CenterPoint().y );

			PutText( pDC, m_asExpr[i], 3, y, true,
				s_colWhite, s_tdShade );

			PutText( pDC, _T("->"), 17, y, true,
				s_colWhite, s_tdShade );

			CString sChange = ::FormatMoney( m_anMoneyAfter[i],
							m_pMFSM->GetState()->pRule->nPlayerNum );
			PutText( pDC, sChange, 20, y, true,
				s_colCyan, s_tdShadeOutlineOpaque );

			y += 2;
		}
	}
	else if ( m_nPage == 1 ) {		// �м� ������

		static const int s_anPos[] = { 6, 9, 12, 15, 21 };
		PutText( pDC, _T("����"), s_anPos[0], 6, true, s_colWhite, s_tdShade );
		PutText( pDC, _T("����"), s_anPos[1], 6, true, s_colWhite, s_tdShade );
		PutText( pDC, _T("����"), s_anPos[2], 6, true, s_colWhite, s_tdShade );
		PutText( pDC, _T("������"), s_anPos[3], 6, true, s_colWhite, s_tdShade );
		PutText( pDC, _T("�����嵵"), s_anPos[4]-1, 6, true, s_colWhite, s_tdShade );

		int y = 8;
		for ( int j = 0; j < 2; j++ )	// ����, �ߴ翡 ���� ����
		for ( int i = 0; i < nPlayers; i++ ) {

			// ó������ ���縸 ǥ��
			if ( j == 0 && !( i == pState->nFriend
							|| i == pState->nMaster ) ) continue;
			// �ι�°�� �ߴ縸 ǥ��
			if ( j != 0 &&  ( i == pState->nFriend
							|| i == pState->nMaster ) ) continue;

			CRect rc = PutText( pDC, pState->apPlayers[i]->GetName(),
				1, y++, true,
				j == 0 ? s_colCyan : s_colYellow,
				i == m_nMe ? s_tdShadeOutlineOpaque : s_tdShadeOpaque );

			pDC->SelectObject( i == 0 ? &penC : &penY );
			pDC->MoveTo( rc.right + GetDSBUnit(), rc.CenterPoint().y );
			pDC->LineTo( xRight, rc.CenterPoint().y );

			for ( int k = 0; k < 5; k++ ) {
				CString s;
				s.Format( k == 3 || k == 4 ? _T("%3d%%") : _T("%3d"),
					m_aanHistory[k][i] );
				PutText( pDC, s, s_anPos[k], y, true,
					s_colCyan, k == 4 ? s_tdShadeOpaque : s_tdShade );
			}
			y += 2;
		}
	}
	else if ( m_nPage == 2
		|| ( !m_bGameOver && m_nPage == 3 ) ) {	// ���� ������

		static const int s_anPos2[] = { 4, 9, 14, 19 };
		PutText( pDC, _T("�ְ�"), s_anPos2[0]+1, 6, true, s_colWhite, s_tdShade );
		PutText( pDC, _T("������"), s_anPos2[1]+1, 6, true, s_colWhite, s_tdShade );
		PutText( pDC, _T("�ߴ�"), s_anPos2[2]+1, 6, true, s_colWhite, s_tdShade );
		PutText( pDC, _T("��ü"), s_anPos2[3]+1, 6, true, s_colWhite, s_tdShade );

		PutText( pDC, _T("��/��"), s_anPos2[0]+1, 4, true, s_colWhite, s_tdShade );

		int y = 8;
		for ( int j = 0; j < 2; j++ )	// ����, �ߴ翡 ���� ����
		for ( int i = 0; i < nPlayers; i++ ) {

			// ó������ ���縸 ǥ��
			if ( j == 0 && !( i == pState->nFriend
							|| i == pState->nMaster ) ) continue;
			// �ι�°�� �ߴ縸 ǥ��
			if ( j != 0 &&  ( i == pState->nFriend
							|| i == pState->nMaster ) ) continue;

			CRect rc = PutText( pDC, pState->apPlayers[i]->GetName(),
				1, y++, true,
				j == 0 ? s_colCyan : s_colYellow,
				i == m_nMe ? s_tdShadeOutlineOpaque : s_tdShadeOpaque );

			pDC->SelectObject( i == 0 ? &penC : &penY );
			pDC->MoveTo( rc.right + GetDSBUnit(), rc.CenterPoint().y );
			pDC->LineTo( xRight, rc.CenterPoint().y );

			const GAME_RECORD& gr =
				m_nPage == 2 ? pState->apPlayers[i]->GetCurRecord()
							: pState->apPlayers[i]->GetAllRecord();

			for ( int k = 0; k < 4; k++ ) {

				int w, l;
				if ( k == 0 ) w = gr.wm, l = gr.lm;
				else if ( k == 1 ) w = gr.wf, l = gr.lf;
				else if ( k == 2 ) w = gr.wa, l = gr.la;
				else w = gr.wm+gr.wf+gr.wa, l = gr.lm+gr.lf+gr.la;

				CString s; s.Format(_T("%3d/%3d"), w, l );

				PutText( pDC, s, s_anPos2[k], y, true,
					s_colCyan, k < 3 ? s_tdShade : s_tdShadeOpaque);
			}
			y += 2;
		}
	}
	else if ( m_bGameOver && m_nPage == 3 ) {	// ���� �� ������

		PutText( pDC, _T("���� ���"), 3, 5, true, s_colCyan, s_tdMidiumShade );

		const GAME_RECORD& gr = pState->apAllPlayers[0]->GetAllRecord();

		CString s; int perm;

		perm = gr.wm + gr.lm == 0 ? 0 : gr.wm * 1000 / ( gr.wm + gr.lm );
		s.Format( _T("  �ְ� : %d �� %d �� (%d.%d%%)"),
			gr.wm, gr.lm, perm/10, perm%10 );
		PutText( pDC, s, 4, 7, true, s_colWhite, s_tdShade );

		perm = gr.wf + gr.lf == 0 ? 0 : gr.wf * 1000 / ( gr.wf + gr.lf );
		s.Format( _T("������ : %d �� %d �� (%d.%d%%)"),
			gr.wf, gr.lf, perm/10, perm%10 );
		PutText( pDC, s, 4, 8, true, s_colWhite, s_tdShade );

		perm = gr.wa + gr.la == 0 ? 0 : gr.wa * 1000 / ( gr.wa + gr.la );
		s.Format( _T("  �ߴ� : %d �� %d �� (%d.%d%%)"),
			gr.wa, gr.la, perm/10, perm%10 );
		CRect rc = PutText( pDC, s, 4, 9, true, s_colWhite, s_tdShade );

		pDC->SelectObject( penY );
		pDC->MoveTo( rc.left, rc.bottom + GetDSBUnit()/2 );
		pDC->LineTo( rc.right + GetDSBUnit()*3, rc.bottom + GetDSBUnit()/2 );

		int w = gr.wa + gr.wf + gr.wm;
		int l = gr.la + gr.lf + gr.lm;
		perm = w+l == 0 ? 0 : w * 1000 / ( w+l );
		s.Format( _T("%d �� %d �� (%d.%d%%)"),
			w, l, perm/10, perm%10 );
		PutText( pDC, s, -1, 12, true, s_colYellow, s_tdMidiumShade );

		s.Format( _T("%d ��"), m_nRank );
		PutText( pDC, s, -1, 16, true, s_colWhite, TD_SIZEBIG|TD_SHADE );
	}

	pDC->SelectStockObject( NULL_PEN );
}

// �� ������ Ŭ���� �� �Ҹ��� �Լ�
// LPVOID ���� 0 �̸� ���� (X�ڸ� Ŭ��) ��û
void DReport::OnClick( LPVOID pVoid )
{
	int nSel = (int)pVoid;

	if ( !m_bGameOver && ( nSel == 0 || nSel == 0xffffffff ) ) {	// Ȯ��

		*(bool*)m_pResult = true;
		Destroy();
	}
	else if ( nSel == 1 ) {	// ����

		*(bool*)m_pResult = false;
		Destroy();
	}
	else if ( 100 <= nSel && nSel <= 103 ) {	// ������ ��ȭ

		SetPage( nSel-100 );
		CRect rc; GetRect(&rc);
		m_pBoard->UpdateDSB( &rc );
	}
	else if ( m_bGameOver && ( nSel == 0 || nSel == 0xffffffff ) ) {	// ���� ��

		m_lHotspot.RemoveAll();

		RegisterHotspot( 11, 23, -1, -1, true, 0, _T("   Ȯ��   "),
			&s_colWhite, &s_tdShade, &s_colCyan, &s_tdShadeOutline,
			(LPVOID)1 );
		SetPage( 3 );
		CRect rc; GetRect(&rc);
		m_pBoard->UpdateDSB( &rc );
	}
}
