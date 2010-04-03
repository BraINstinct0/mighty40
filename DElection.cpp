// DElection.cpp: implementation of the DElection class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mighty.h"

#include "BoardWrap.h"
#include "BmpMan.h"
#include "DSB.h"
#include "DEtc.h"
#include "DElection.h"

#include "Play.h"
#include "Player.h"
#include "MFSM.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


/////////////////////////////////////////////////////////////////////////////
// ���� ���� DSB

DGoal::DGoal( CBoardWrap* pBoard ) : DSB(pBoard)
{
	m_bTerminateToggler = false;
	m_bTogglerTerminated = true;

	m_colConfirm = s_colGray;
	m_tdConfirm = 0;
	m_tdConfirmSel = 0;
}

DGoal::~DGoal()
{
	if ( !m_bTogglerTerminated ) {

		m_bTerminateToggler = true;
		while ( !m_bTogglerTerminated ) Sleep(100);
		// ���� - ��������
		// Toggler �� Board �� ���� ���� �ξ����Ƿ� ���⼭
		// �̸� �����Ѵ� !! �� �̰��� Toggler �� ���ϴ°� ?
		// Toggler �� �����ϴ� ��� Board ���� SendMessage ��
		// ���ϵǱ⸦ ��ٸ���, ���� Sleep ��
		// �� Toggler �� ��ٸ��� Board�� �����尡 �����Ѵ�
		// ��, Dead-Lock �߻�, ���� ���⼭ �̸� ȣ���Ѵ�
		m_pBoard->CancelSelection();
	}
}

// �־��� ������� ���ٸ� �ٲ۴�
void DGoal::ChangeKiruda( int nShape )
{
	m_nKiruda = nShape;

	for ( int i = 0; i < 5; i++ ) {
		m_aatdShape[i][0] = i == m_nKiruda ? s_tdOutline : s_tdNormal;
		m_aatdShape[i][1] = i == m_nKiruda ? s_tdOutline : TD_SUNKEN;
	}

	// ����Ƽ ��� ���� ǥ�õ� ��Ʈ���� ���ۼ� �ؾ��Ѵ�
	if ( Mo()->bUseTerm )
		m_sMinScore = CCard::GetGoalString(
			Mo()->bUseTerm, m_nKiruda, m_nMinScore,
			m_pState->pRule->nMinScore );

	CRect rc; GetRect( &rc );
	m_pBoard->UpdateDSB( &rc );
}

// �־��� ������ �ּ� ������ �ٲ۴�
// (valid �� ������ ��������)
void DGoal::ChangeMinScore( int nMinScore )
{
	int nDiff = -1;

	while ( !m_pState->IsValidNewGoal( m_nKiruda, nMinScore ) ) {

		// ��� ���ҽ�Ű�ٰ� 1 �� �����ص� valid ���� ������
		// �ݴ�� �������� ����
		if ( nMinScore == 1 ) nDiff = 1;
		nMinScore += nDiff;
	}

	m_nMinScore = nMinScore;

	// ȭ��ǥ ������ �����Ѵ�
	for ( int i = 0; i < 2; i++ ) {
		int nExpectedDiff = i == 0 ? 1 : -1;
		if ( m_pState->IsValidNewGoal(
				m_nKiruda, nMinScore+nExpectedDiff ) ) {
			// �� �������� ���� ������
			m_aacolArrow[i][0] = s_colCyan;
			m_aacolArrow[i][1] = s_colCyan;
			m_aatdArrow[i][0] = s_tdMidiumShade;
			m_aatdArrow[i][1] = s_tdMidiumOutline;
		}
		else { // �Ұ����� - disable �Ѵ�
			m_aacolArrow[i][0] = s_colGray;
			m_aacolArrow[i][1] = s_colGray;
			m_aatdArrow[i][0] = s_tdMidium;
			m_aatdArrow[i][1] = s_tdMidium;
		}
	}

	// ǥ�õ� ��Ʈ���� �ۼ��Ѵ�
	if ( Mo()->bUseTerm )
		m_sMinScore = CCard::GetGoalString(
			Mo()->bUseTerm, m_nKiruda, nMinScore,
			m_pState->pRule->nMinScore );
	else
		m_sMinScore.Format( _T("%d"), nMinScore );

	// ����
	CRect rc; GetRect( &rc );
	m_pBoard->UpdateDSB( &rc );
}


// bPrivilege : �缱�ڰ� ���� �����ϴ� ���ΰ�?
// pMFSM      : ��� �÷��̾��� �տ� �� ī���
//              ��ǥ ���� validity �� �׽�Ʈ �ϱ� ���� �����Ѵ�
void DGoal::Create( bool bPrivilege,
	const CState* pState, CEvent* pEvent, CGoal* pResult )
{
	m_bPrivilege = bPrivilege;

	m_pState = pState;
	m_plHand = m_pState->apPlayers[0]->GetHand();

	SetFixed();

	ASSERT(pState);
	ASSERT(m_plHand);

	m_bDealMiss = !m_bPrivilege && m_pState->IsDealMiss(
		m_pState->apPlayers[0]->GetHand() );

	m_nKiruda = pResult->nKiruda;
	m_nMinScore = pResult->nMinScore;

	if ( !m_bPrivilege ) {

		CRect rc = m_pBoard->CalcPlayerExtent( 0 );
		int cxDSB = 15, cyDSB = 8;

		DSB::Create2(
			rc.left+rc.Width()/2 - cxDSB*GetDSBUnit()/2,
			rc.bottom - cyDSB*GetDSBUnit(),
			cxDSB, cyDSB, -1 );
	}
	else {

		int cxDSB = 18, cyDSB = 8+9;

		DSB::Create( 0, -cyDSB/6, cxDSB, cyDSB, -1 );
	}

	SetAction( pEvent, pResult );

	if ( m_bPrivilege ) {
		// Toggler �����带 ���۽�Ų��
		m_bTogglerTerminated = false;
		VERIFY( AfxBeginThread( TogglerProc, (LPVOID)this ) );
	}
}

// �ʱ�ȭ ( ������ �� ȣ��� )
void DGoal::OnInit()
{
	DSB::OnInit();

	// Hotspot �� ������ �Ǵ� y ��ǥ
	int y = 0;

	// ���� ���̶�� ��õ ī��� �ּ� ���� ������ �����Ѵ�
	if ( !this->m_bPrivilege ) {

		ChangeKiruda( m_pState->apPlayers[0]
						->GetHand()->GetKirudable() );
		ChangeMinScore( 1 );	// �ڵ����� ����
	}
	else {	// �׷��� ������ ���� �־��� �������� �����Ѵ�

		ChangeKiruda( m_nKiruda );
		ChangeMinScore( m_nMinScore );

		y = 8;
	}

	// 5 ���� shape ����

	CRect rcSpade = RegisterHotspot(
		1, y+2, -1, -1, true,
		IDB_SPADE, 0,
		&s_colWhite, &m_aatdShape[1][0], &s_colWhite, &m_aatdShape[1][1],
		(LPVOID)2 );

	RegisterHotspot(
		rcSpade.right+1, rcSpade.top, -1, -1, false,
		IDB_DIAMOND, 0,
		&s_colWhite, &m_aatdShape[2][0], &s_colWhite, &m_aatdShape[2][1],
		(LPVOID)3 );

	RegisterHotspot(
		rcSpade.left, rcSpade.bottom+1, -1, -1, false,
		IDB_HEART, 0,
		&s_colWhite, &m_aatdShape[3][0], &s_colWhite, &m_aatdShape[3][1],
		(LPVOID)4 );

	CRect rcClover = RegisterHotspot(
		rcSpade.right+1, rcSpade.bottom+1, -1, -1, false,
		IDB_CLOVER, 0,
		&s_colWhite, &m_aatdShape[4][0], &s_colWhite, &m_aatdShape[4][1],
		(LPVOID)5 );

	CRect rcNokiruda = RegisterHotspot(
		rcClover.right+1, rcSpade.bottom+1, -1, -1, false,
		IDB_NOTHING, 0,
		&s_colWhite, &m_aatdShape[0][0], &s_colWhite, &m_aatdShape[0][1],
		(LPVOID)1 );

	// < > ��ư

	CRect rc1 = RegisterHotspot(
		12, y+2, -1, -1, true, 0, _T("��"),
		&m_aacolArrow[0][0], &m_aatdArrow[0][0],
		&m_aacolArrow[0][1], &m_aatdArrow[0][1],
		(LPVOID)6 );

	CRect rc2 = RegisterHotspot(
		12, y+4, -1, -1, true, 0, _T("��"),
		&m_aacolArrow[1][0], &m_aatdArrow[1][0],
		&m_aacolArrow[1][1], &m_aatdArrow[1][1],
		(LPVOID)7 );

	// �⸶ / �⸶���� ��ư

	if ( !m_bDealMiss ) {

		if ( !m_bPrivilege ) {

			RegisterHotspot(
				2, y+6, -1, -1, true, 0, _T("����"),
				&s_colWhite, &s_tdShade, &s_colCyan, &s_tdShadeOutline,
				(LPVOID)0 );

			RegisterHotspot(
				9, y+6, -1, -1, true, 0, _T("�⸶ !!"),
				&s_colWhite, &s_tdShade, &s_colCyan, &s_tdShadeOutline,
				(LPVOID)8 );
		}
		else RegisterHotspot(
				8, y+7, -1, -1, true, 0, _T("Ȯ��"),
				&m_colConfirm, &m_tdConfirm,
				&m_colConfirm, &m_tdConfirmSel,
				(LPVOID)9 );
	}
	else {

		RegisterHotspot(
			8, y+6, -1, -1, true, 0, _T("���̽� !!"),
			&s_colYellow, &s_tdShade, &s_colYellow, &s_tdShadeOutline,
			(LPVOID)0 );

		RegisterHotspot(
			2, y+6, -1, -1, true, 0, _T("�⸶ !!"),
			&s_colWhite, &s_tdShade, &s_colCyan, &s_tdShadeOutline,
			(LPVOID)8 );
	}

	m_rcMinScore.top = rc1.top;
	m_rcMinScore.bottom = rc2.bottom;
	m_rcMinScore.left = rcNokiruda.right;
	m_rcMinScore.right = rc2.left;

	CRect rcBox; GetRect( &rcBox );
	m_rcMinScore.OffsetRect( - rcBox.TopLeft() );
}

// CBoard �� ȣ���ϴ� �׸��� �Լ� - ��µ� shade-box �� �� �Լ���
// �������̵� �Ͽ� �����ؾ� �Ѵ�
// ���̽� �Լ��� ������ ��ü�� ���� ���� ������ �׸���
void DGoal::OnDraw(
	CDC* pDC,
	bool bBrushOrigin )
{
	static LPCTSTR asShape[] = {
		_T("�����"), _T("�����̵�"), _T("���̾Ƹ��"),
		_T("��Ʈ"), _T("Ŭ�ι�") };

	DSB::OnDraw( pDC, bBrushOrigin );

	if ( !m_bPrivilege ) {
		PutText( pDC, _T("������ ���� �ּ���"), 0, 0, true,
			s_colWhite, s_tdShade );
	}
	else {
		static LPCTSTR asText[] = {
			_T("�缱�� ���ϵ帳�ϴ� !!"),
			_T(""),
			_T("������ ���� ī�� �� ����"),
			_T("������ ����, �Ʒ��ʿ� �ִ�"),
			_T("'Ȯ��'�� Ŭ���ϼ���"),
			_T(""),
			_T("���� ���ٿ� ��ǥ ������"),
			_T("�ٲ� �� �ֽ��ϴ�")
		};
		for ( int i = 0; i < sizeof(asText)/sizeof(LPCTSTR); i++ )
			PutText( pDC, asText[i], -1, i+1, true,
				s_colWhite, s_tdShade );
	}

	CRect rcBox; GetRect( &rcBox );

	// ���ڸ� ������ ��� �� �� ū �۾���
	int td = !Mo()->bUseTerm ? TD_SIZEBIG|TD_SHADE : s_tdMidiumShade;
	CSize szText = GetTextExtent(
		Mo()->bUseTerm ? 1 : 2 , m_sMinScore );

	int x = m_rcMinScore.left + m_rcMinScore.Width()/2 - szText.cx/2 + rcBox.left;
	int y = m_rcMinScore.top + m_rcMinScore.Height()/2 - szText.cy/2 + rcBox.top;

	if ( m_bPrivilege ) y += 8;

	PutText( pDC, m_sMinScore, x, y, false, s_colYellow, td );

	if ( !Mo()->bUseTerm )
		PutText( pDC, asShape[m_nKiruda],
			m_rcMinScore.left + rcBox.left,
			y - GetTextExtent( 0, asShape[m_nKiruda] ).cy,
			false, s_colWhite, s_tdShade );
}

// �� ������ Ŭ���� �� �Ҹ��� �Լ�
// LPVOID ���� 0 �̸� ���� (X�ڸ� Ŭ��) ��û
void DGoal::OnClick( LPVOID pVoid )
{
	if ( pVoid == (LPVOID)0xffffffff ) {	// Enter key
		if ( m_bPrivilege ) pVoid = (LPVOID)9;
		else pVoid = (LPVOID)8;
	}

	int nSel = (int)pVoid;

	switch ( nSel ) {
	case 1: case 2:
	case 3: case 4: case 5: {
		DSB::OnClickSound();
		if ( !m_bPrivilege ) ChangeKiruda( nSel-1 );
		else {
			int nKiruda = nSel-1;
			int nMinScore = m_pState->goal.nMinScore;
			while ( !m_pState->IsValidNewGoal( nKiruda, nMinScore )
				&& nMinScore <= HIGHSCORE_MAXLIMIT ) nMinScore++;
			if ( nMinScore > HIGHSCORE_MAXLIMIT ) return;
			else {
				ChangeKiruda( nKiruda );
				if ( m_nMinScore != nMinScore )
					ChangeMinScore( nMinScore );
			}
		}
	} break;
	case 6: {
		DSB::OnClickSound();
		ChangeMinScore( m_nMinScore+1 );
	} break;
	case 7: {
		DSB::OnClickSound();
		ChangeMinScore( m_nMinScore-1 );
	} break;
	case 0: {

		if ( m_bPrivilege ) {

			if ( Mo()->bUseSound ) MessageBeep( MB_ICONEXCLAMATION );
			(new DShortMessage(m_pBoard))->Create(
				0, _T("'Ȯ��'�� ��������"),
				true, false, 2000 );
		}
		else if ( !m_bDealMiss
			&& !m_pState->IsValidNewGoal( m_nKiruda, 0 ) ) {

			if ( Mo()->bUseSound ) MessageBeep( MB_ICONEXCLAMATION );
			(new DShortMessage(m_pBoard))->Create(
				0, _T("���� ù �Ͽ� �ݵ�� �⸶�Ͽ��� �մϴ�"),
				true, false, 2000 );
		}
		else {

			CGoal* pGoal = (CGoal*)m_pResult;

			pGoal->nFriend = 0;
			pGoal->nKiruda = m_nKiruda;
			pGoal->nMinScore = m_bDealMiss ? -1 : 0;

			Destroy();
		}
	} break;
	case 8: {

		if ( !m_pState->IsValidNewGoal( m_nKiruda, m_nMinScore ) ) {

			if ( Mo()->bUseSound ) MessageBeep( MB_ICONEXCLAMATION );
			(new DShortMessage(m_pBoard))->Create(
				0, _T("�� �������δ� �⸶�� �� �����ϴ�"),
				true, false, 2000 );
		}
		else {

			CGoal* pGoal = (CGoal*)m_pResult;

			pGoal->nFriend = 0;
			pGoal->nKiruda = m_nKiruda;
			pGoal->nMinScore = m_nMinScore;

			Destroy();
		}
	} break;
	case 9: {	// Ȯ��
		ASSERT(m_bPrivilege);

		if ( !m_pState->IsValidNewGoal( m_nKiruda, m_nMinScore ) ) {

			if ( Mo()->bUseSound ) MessageBeep( MB_ICONEXCLAMATION );
			(new DShortMessage(m_pBoard))->Create(
				0, _T("��ǥ������ �� �÷��� �մϴ�"),
				true, false, 2000 );
		}
		else if ( m_pBoard->GetSelectionCount() != 3 ) {

			if ( Mo()->bUseSound ) MessageBeep( MB_ICONEXCLAMATION );
			(new DShortMessage(m_pBoard))->Create(
				0, _T("����, ���� ī�� �� ���� �����ϼ���"),
				true, false, 2000 );
		}
		else {
			CGoal* pGoal = (CGoal*)m_pResult;

			pGoal->nFriend = 0;
			pGoal->nKiruda = m_nKiruda;
			pGoal->nMinScore = m_nMinScore;

			Destroy();
		}

	} break;
	}
}

// Board �� Selection �� ������ ������
// ī�� ������ ����ϴ� ���Ҹ��� �Ѵ�
UINT DGoal::TogglerProc( LPVOID _pThis )
{
	TRACE("DGoal::TogglerProc began\n");

	DGoal* pThis = (DGoal*)_pThis;

	long nCard = 0;

	pThis->m_pBoard->WaitSelection( &pThis->m_ev, &nCard );

	while ( !pThis->m_bTerminateToggler ) {

		// 0.1 �ʿ� �� ���� ���� ������ Ȯ���Ѵ�
		while ( !pThis->m_bTerminateToggler && !pThis->m_ev.Lock( 100 ) );
		if ( pThis->m_bTerminateToggler ) break;

		// �Ҹ�����
		PlaySound( IDW_GETCARD, true );

		// ���õ� ī�带 ����Ѵ�
		int nIndex = pThis->m_plHand->IndexFromPOSITION(
			pThis->m_plHand->Find( CCard(nCard) ) );
		pThis->m_pBoard->SetSelection( nIndex,
			!pThis->m_pBoard->GetSelection( nIndex ) );

		// 3 ���� ���õǾ��ٸ� "Ȯ��" ������ �ٲ۴�
		ASSERT( pThis->m_bPrivilege );
		COLORREF colOld = pThis->m_colConfirm;

		if ( pThis->m_pBoard->GetSelectionCount() == 3 ) {
			pThis->m_colConfirm = s_colCyan;	// �þ� ������
			pThis->m_tdConfirm = s_tdShade;
			pThis->m_tdConfirmSel = s_tdShadeOutline;
		}
		else {
			pThis->m_colConfirm = s_colGray;
			pThis->m_tdConfirm = 0;
			pThis->m_tdConfirmSel = 0;
		}

		// ������ ����Ǿ��ٸ� Update
		if ( colOld != pThis->m_colConfirm ) {
			pThis->UpdateHotspot(pThis->m_lHotspot.GetTailPosition());
		}

		// �ٽ� ȭ�� ����
		pThis->m_pBoard->UpdatePlayer( 0, 0, true, true );
	}
	pThis->m_bTogglerTerminated = true;
	TRACE("DGoal::TogglerProc ended\n");
	return 0;
}



/////////////////////////////////////////////////////////////////////////////
// ī�� ���̱� DSB

// m_nShape �� Ư�� ������ ��Ʈ�ϸ鼭 m_aatdShape ��
// �� �����Ѵ�
void DKill::SetCurShape( int nShape )
{
	m_nShape = nShape;
	for ( int i = 0; i < 4; i++ ) {
		m_aatdShape[i][0] = SPADE+i == nShape ? TD_OUTLINE : TD_NORMAL;
		m_aatdShape[i][1] = SPADE+i == nShape ? TD_OUTLINE : TD_SUNKEN;
	}
	CRect rc; GetRect( &rc );
	m_pBoard->UpdateDSB( &rc );
}

// m_pcToKill ���� �����ϰ� �� ����� ȭ�鿡 ������Ʈ �Ѵ�
void DKill::SetCurCard( CCard c )
{
	*m_pcToKill = c;
	CRect rc; CalcCurCardRect( &rc );
	m_pBoard->UpdateDSB( &rc );
}

// pcToKill : �� DSB �� ��� ī�带 ������ ��� (���-�� ��������)
// plDead   : ���ݱ��� ��ٸ� ¤�� ī���
void DKill::Create( CEvent* pEvent, CCard* pcToKill, const CCardList* plDead, const CCardList* plHand )
{
	m_pcToKill = pcToKill;
	m_plDead = plDead;
	m_plHand = plHand;
	m_nShape = m_pcToKill && m_pcToKill->GetShape() > 0
				? m_pcToKill->GetShape() : SPADE;

	SetFixed();

	int nUnit = GetDSBUnit();

	// �ּ��� ũ��� ( 16 * 14 ) * ( card.cy + 32 + nUnit ) �� ī�� ���ÿ��� ���� ��
	int cxDSB = ( 16*14 + nUnit-1 ) / nUnit;
	int cyDSB = ( GetBitmapSize(IDB_SA).cy + 32 + nUnit + nUnit-1 ) / nUnit;

	// �ٸ� ���� �� ũ��
	cxDSB += 5;
	cyDSB += 1;

	CRect rc = m_pBoard->CalcPlayerExtent( 0 );

	DSB::Create2(
		rc.left+rc.Width()/2 - cxDSB*GetDSBUnit()/2,
		rc.bottom - cyDSB*GetDSBUnit(),
		cxDSB, cyDSB, -1 );

	SetAction( pEvent, pcToKill );
}

// �ʱ�ȭ ( ������ �� ȣ��� )
void DKill::OnInit()
{
	DSB::OnInit();

	SetCurShape( m_nShape );

	int nDSBUnit = GetDSBUnit();
	CRect rc; GetRect( &rc );

	int x = rc.right - 16*14 - nDSBUnit;
	int xDiff = 16;
	int cx = 16;
	int y = rc.bottom - 32 - nDSBUnit - 3;
	int cy = 32 + 3;
	for ( int i = 0; i < 14; i++, x += xDiff )

		RegisterHotspot(
			x, y, cx, cy, false,
			0, 0, &s_colWhite, &s_tdNormal, &s_colWhite, &s_tdNormal,
			(LPVOID)(i+100) );

	CRect rcCurCard; CalcCurCardRect( &rcCurCard );
	RegisterHotspot(
		rcCurCard.left, rcCurCard.top, rcCurCard.Width(), rcCurCard.Height(),
		false, 0, 0, &s_colWhite, &s_tdNormal, &s_colWhite, &s_tdNormal,
			(LPVOID)0 );

	// 4 ���� Shape ���� ����

	CRect rcSpade = RegisterHotspot(
		rc.right - 16*14 - nDSBUnit - GetBitmapSize(IDB_SPADE).cx*2 - 8,
		rc.bottom - GetBitmapSize(IDB_SPADE).cy*2 - nDSBUnit, -1, -1, false,
		IDB_SPADE, 0,
		&s_colWhite, &m_aatdShape[0][0], &s_colWhite, &m_aatdShape[0][1],
		(LPVOID)1000 );

	RegisterHotspot(
		rcSpade.right, rcSpade.top, -1, -1, false,
		IDB_DIAMOND, 0,
		&s_colWhite, &m_aatdShape[1][0], &s_colWhite, &m_aatdShape[1][1],
		(LPVOID)1001 );

	RegisterHotspot(
		rcSpade.left, rcSpade.bottom, -1, -1, false,
		IDB_HEART, 0,
		&s_colWhite, &m_aatdShape[2][0], &s_colWhite, &m_aatdShape[2][1],
		(LPVOID)1002 );

	CRect rcClover = RegisterHotspot(
		rcSpade.right, rcSpade.bottom, -1, -1, false,
		IDB_CLOVER, 0,
		&s_colWhite, &m_aatdShape[3][0], &s_colWhite, &m_aatdShape[3][1],
		(LPVOID)1003 );
}

// ���� ī�带 �׸� ������ �����Ѵ�
void DKill::CalcCurCardRect( LPRECT pRc )
{
	CRect rc; GetRect( &rc );
	int nDSBUnit = GetDSBUnit();
	CSize szCard = GetBitmapSize(IDB_SA);
	pRc->left = rc.right - nDSBUnit - szCard.cx;
	pRc->top = rc.bottom - 32 - nDSBUnit - 5 - szCard.cy;
	pRc->right = pRc->left + szCard.cx;
	pRc->bottom = pRc->top + szCard.cy;
}

// CBoard �� ȣ���ϴ� �׸��� �Լ� - ��µ� shade-box �� �� �Լ���
// �������̵� �Ͽ� �����ؾ� �Ѵ�
// ���̽� �Լ��� ������ ��ü�� ���� ���� ������ �׸���
void DKill::OnDraw(
	CDC* pDC,
	bool bBrushOrigin )
{
	DSB::OnDraw( pDC, bBrushOrigin );

	// ���� ���õ� ī�带 �׸���
	CRect rcCurCard; CalcCurCardRect( &rcCurCard );
	m_pBoard->GetBmpMan()->DrawCard( pDC, *m_pcToKill, rcCurCard );

	// �� ī�尡 �տ� �ִ� ī��ų� dead ī���̸� x ǥ�� �Ѵ�
	if ( m_plDead->Find( *m_pcToKill ) || m_plHand->Find( *m_pcToKill ) ) {
		CRect rc(
			rcCurCard.left + rcCurCard.Width()/2 - rcCurCard.Width()/3,
			rcCurCard.top + rcCurCard.Height()/2 - rcCurCard.Width()/3,
			rcCurCard.left + rcCurCard.Width()/2 + rcCurCard.Width()/3,
			rcCurCard.top + rcCurCard.Height()/2 + rcCurCard.Width()/3 );

		CPen pen( PS_SOLID, 10, RGB(255,0,0) );
		CPen* pnOld = pDC->SelectObject( &pen );
		pDC->MoveTo( rc.TopLeft() ); pDC->LineTo( rc.BottomRight() );
		pDC->MoveTo( rc.right, rc.top ); pDC->LineTo( rc.left, rc.bottom );
		pDC->SelectObject( pnOld );
	}

	// 13 ���� ī�带 �׸���
	int nDSBUnit = GetDSBUnit();
	CRect rc; GetRect( &rc );
	int x = rc.right - 16*14 - nDSBUnit;
	int xDiff = 16;
	int cx = 16;
	int y = rc.bottom - 32 - nDSBUnit;
	int cy = 32;
	CCard c;
	for ( int i = 0; i < 14; i++, x += xDiff ) {

		if ( i == 0 ) c = CCard::GetJoker();	// ������ ���� ��Ŀ
		else if ( i == 1 ) c = CCard( m_nShape, ACE );	// ù���� ���̽�
		else c = CCard( m_nShape, KING - (i-2) );

		bool bSel =		// ���� ���õ� Hotspot �ΰ�
			m_posSel && m_lHotspot.GetAt(m_posSel).pVoid == (LPVOID)(100+i)
			&& !m_plDead->Find(c)
			&& !m_plHand->Find(c);
		int yDiff = bSel ? 3 : 0;

		m_pBoard->GetBmpMan()->DrawCardEx(
			pDC, (int)c, x, y - yDiff, cx, cy + yDiff,
			0, 0, cx, cy+yDiff );
	}

	static LPCTSTR asText[] = {
		_T("�� �ĺ��� Ż�� ��ŵ�ϴ�"),
		_T(""),
		_T("���ϴ� ǥ�� ī�带"),
		_T("�Ʒ����� �����ϰ�"),
		_T("�������� ū ī�带"),
		_T("Ŭ���ϼ���")
	};
	for ( int m = 0; m < sizeof(asText)/sizeof(LPCTSTR); m++ )
		PutText( pDC, asText[m], 1, 1+m, true, s_colWhite, s_tdShade );
}

// �� ������ Ŭ���� �� �Ҹ��� �Լ�
// LPVOID ���� 0 �̸� ���� (X�ڸ� Ŭ��) ��û
void DKill::OnClick( LPVOID pVoid )
{
	int n = (int)pVoid;

	if ( n >= 1000 && n <= 1003 ) {
		// ��� ����
		DSB::OnClickSound();
		SetCurShape( n-1000+SPADE );
		return;
	}
	else if ( n >= 100 && n <= 113 ) {
		// ī�� ����
		DSB::OnClickSound();
		CCard c;
		if ( n == 100 )			// Joker
			c = CCard(JOKER);
		else if ( n == 101 )	// ACE
			c = CCard(m_nShape,ACE);
		else
			c = CCard( m_nShape, 115-n );
		SetCurCard( c );
	}
	else if ( n == 0 || n == 0xffffffff ) {	// ����

		// �ҹ� ī�带 ���� ���ϰ� �Ϸ��� �ּ��� �����Ұ�
		//		if ( m_plDead->Find(*m_pcToKill)
		//			|| m_plHand->Find(*m_pcToKill) ) return;
		DSB::OnClick(0);
		return;
	}

	RECT rc;
	GetRect( &rc );
	m_pBoard->UpdateDSB( &rc, true );
}



/////////////////////////////////////////////////////////////////////////////
// �缱�� ��ǥ DSB

// bTemp       : true �̸� �� DSB �� ������ �缱�� ��ǥ DSB
//               false �̸� �� DSB �� ���� Ȯ�� DSB
// bToKill     : �ٸ� �÷��̾ �׿��� �ϴ°�
// bUseFriend  : ������ ������ �ִ°�
// sMasterName : �缱�� �̸�
// goal        : ����
void DMasterDecl::Create( bool bTemp, bool bHuman, bool bToKill,
	bool bUseFriend, int nMinScore, const CGoal& goal,
	LPCTSTR sMasterName, LPCTSTR sFriendName,
	int nTimeOut )
{
	SetFixed();

	m_bTemp = bTemp;
	m_bHuman = bHuman;
	m_bToKill = bToKill;
	m_bUseFriend = bUseFriend;
	m_nDefaultMinScore = nMinScore;
	m_sMasterName = sMasterName;
	m_sFriendName = sFriendName ? sFriendName : _T("");
	m_goal = goal;

	// �缱 DSB �� ���� ���ʿ� ��Ÿ������ �Ѵ�
	// (ȭ�� ������ ��������)

	CRect rcClient; m_pBoard->GetClientRect( &rcClient );

	int nUnit = GetDSBUnit();
	int cxDSB = 18;
	int cyDSB = 8;
	int cyDSB2 = cyDSB + max( ( GetBitmapSize(IDB_SA).cy + nUnit-1 )
				/ nUnit + 2, 8 );

	int y = ( rcClient.top + rcClient.bottom ) / 2
				- ( cyDSB + 1 ) * GetDSBUnit();

	if ( !( m_bHuman && m_bTemp || !m_bTemp && !m_bUseFriend ) )
		cyDSB  = cyDSB2;

	DSB::Create2(
		rcClient.left + rcClient.Width()/2 - cxDSB*GetDSBUnit()/2,
		y, cxDSB, cyDSB, nTimeOut );
}

// CBoard �� ȣ���ϴ� �׸��� �Լ� - ��µ� shade-box �� �� �Լ���
// �������̵� �Ͽ� �����ؾ� �Ѵ�
// ���̽� �Լ��� ������ ��ü�� ���� ���� ������ �׸���
void DMasterDecl::OnDraw(
	CDC* pDC,
	bool bBrushOrigin )
{
	UINT arcShape[] = { IDB_NOTHING, IDB_SPADE, IDB_DIAMOND, IDB_HEART, IDB_CLOVER };
	DSB::OnDraw( pDC, bBrushOrigin );

	// �Ӹ���
	LPCTSTR sTitle;
	if ( m_bTemp ) {	// �ӽ� �缱
		if ( m_bHuman ) sTitle = _T("�缱�� ���ϵ帳�ϴ� !!");
		else sTitle = _T("�缱�ڰ� Ȯ���Ǿ����ϴ� !!");
	} else sTitle = _T("-- ���� ��� ��ǥ --");

	// ���� ��Ʈ��
	CString sGoal(_T(" "));
	sGoal += CCard::GetGoalString( Mo()->bUseTerm,
		m_goal.nKiruda, m_goal.nMinScore, m_nDefaultMinScore );

	PutText( pDC, sTitle, -1, 1, true, s_colWhite, s_tdShade );
	PutText( pDC, m_sMasterName, -1, 3, true, s_colYellow, s_tdShadeOutline );
	PutBitmap( pDC, arcShape[m_goal.nKiruda], sGoal, -1, 5, true, s_colWhite, s_tdShade );

	// ������ ������ ä���

	// �ڽ��� �缱���̰� �ӽ� �缱�ڸ� ��ǥ�ϴ� ���
	// ������ ������ �������� �ʴ´�
	if ( m_bHuman && m_bTemp
		|| !m_bTemp && !m_bUseFriend ) return;

	// �����带 ���ؾ� �ϴ� ���
	// ������ ������ �� ������ ǥ���Ѵ�
	if ( !m_bTemp && m_bUseFriend ) {

		// �����带 ī��� ���ϴ� ���
		if ( m_goal.nFriend > 0 && m_goal.nFriend < 54 ) {

			CRect rc; GetRect( &rc );
			CSize szCard = GetBitmapSize(IDB_SA);
			CString sText;
			sText.Format( _T("%s ������"),
				CCard(m_goal.nFriend).GetString( Mo()->bUseTerm ) );

			CRect rcText = PutText( pDC, sText,
				-1, 8, true, s_colCyan, s_tdShade );
			m_pBoard->GetBmpMan()->DrawCard( pDC, m_goal.nFriend,
				rc.left + (rc.Width()-15)/2 - szCard.cx/2,
				rcText.bottom + 2,
				szCard.cx, szCard.cy );
		}
		else {	// ��� �׸��ڰ� ������ ���

			CRect rc; GetRect( &rc );
			CSize szCard = GetBitmapSize(IDB_SA);
			CString sText;
			if ( m_goal.nFriend < 0 )	// ���
				sText = m_sFriendName;
			else if ( m_goal.nFriend == 0 )	// ��
				sText = _T("��");
			else if ( m_goal.nFriend == 100 )	// �ʱ�
				sText = _T("�ʱ�");

			CRect rcText = PutText( pDC, sText + _T(" ������"),
				-1, 8, true, s_colCyan, s_tdShade );

			CRect rcCurCard( CPoint(
				rc.left + (rc.Width()-15)/2 - szCard.cx/2,
				rcText.bottom + 2 ), szCard );

			DrawBitmap( pDC, IDB_PERSON,
				rcCurCard.left, rcCurCard.top,
				szCard.cx, szCard.cy,
				0, 0, szCard.cx, szCard.cy );

			if ( m_goal.nFriend == 0 ) {	// ��������(X)
				CRect rc(
					rcCurCard.left + rcCurCard.Width()/2 - rcCurCard.Width()/3,
					rcCurCard.top + rcCurCard.Height()/2 - rcCurCard.Width()/3,
					rcCurCard.left + rcCurCard.Width()/2 + rcCurCard.Width()/3,
					rcCurCard.top + rcCurCard.Height()/2 + rcCurCard.Width()/3 );

				CPen pen( PS_SOLID, 10, RGB(255,0,0) );
				CPen* pnOld = pDC->SelectObject( &pen );
				pDC->MoveTo( rc.TopLeft() ); pDC->LineTo( rc.BottomRight() );
				pDC->MoveTo( rc.right, rc.top ); pDC->LineTo( rc.left, rc.bottom );
				pDC->SelectObject( pnOld );
			}
			else {

				// �ؽ�Ʈ ��ġ (s,x,y)
				int x = rcCurCard.left + rcCurCard.Width()/2 - GetTextExtent( 0, sText ).cx/2;
				int y = rcCurCard.bottom - GetBitmapSize(IDB_SA).cy / 3;

				PutText( pDC, sText, x, y, false, s_colWhite, s_tdShadeOutline );
			}
		}
		return;
	}

	// ���� ���ζ� ������ ������ ä����� �Ѵ�
	LPCTSTR aMsg[6];
	for ( int i = 0; i < sizeof(aMsg)/sizeof(LPCTSTR); i++ )
		aMsg[i] = _T("");

	// ���� ����
	if ( m_bToKill ) {
		aMsg[0] = _T("�缱�ڴ� ���� �ʱ���");
		aMsg[1] = _T("�������� ������ �ֵ��մϴ� !");
		aMsg[3] = _T("�񸮰� ������ �ĺ��� ���ڱ���");
		aMsg[4] = _T("���� ����� ȯ���˴ϴ�");
	}
	// ������ ���ϱ�
	else {
		aMsg[0] = _T("�缱�ڴ� ������ ī�带 �ް�");
		if ( m_bUseFriend )
			aMsg[1] = _T("������� ���� ������ �����մϴ�");
		else aMsg[1] = _T("���� ������ �մϴ�");
		aMsg[3] = _T("���� �ߴ� �������");
		aMsg[4] = _T("\"���� Ÿ��\"��� ���� ��ǥ �Ͽ�");
		aMsg[5] = _T("�ϳ��� �����߽��ϴ�");
	}

	for ( int j = 0; j < sizeof(aMsg)/sizeof(LPCTSTR); j++ )
		PutText( pDC, aMsg[j], -1, 8 + j,
			true, s_colWhite, s_tdShade );
}

// �ƹ����� Ŭ���ص� �ٷ� ��������� �Ѵ�
bool DMasterDecl::DispatchHotspot( int x, int y, bool bCheckOnly )
{
	if ( m_bTemp ) return DSB::DispatchHotspot( x, y, bCheckOnly );
	else if ( bCheckOnly ) return false;
	else { Destroy(); return true; }
}


/////////////////////////////////////////////////////////////////////////////
// Ż���� ��ǥ DSB

// sMaster  : �缱�� �̸�
// nCard    : ȭ�鿡 ǥ�õ� ī��
// bFail    : �� ī��� Ż�� ���� ī���ΰ� (�޽����� ���� �޶���)
void DDeadDecl::Create( LPCTSTR sMaster, int nCard,
	bool bFail, int nTimeOut )
{
	SetFixed();

	m_sMaster = sMaster;
	m_nCard = nCard;
	m_bFail = bFail;

	int nUnit = GetDSBUnit();

	int cxDSB = 17;
	int cyDSB = 7;
	cyDSB += ( GetBitmapSize(IDB_SA).cy + nUnit-1 ) / nUnit;

	DSB::Create( 0, -2, cxDSB, cyDSB, nTimeOut );
}

// CBoard �� ȣ���ϴ� �׸��� �Լ� - ��µ� shade-box �� �� �Լ���
// �������̵� �Ͽ� �����ؾ� �Ѵ�
// ���̽� �Լ��� ������ ��ü�� ���� ���� ������ �׸���
void DDeadDecl::OnDraw(
	CDC* pDC,
	bool bBrushOrigin )
{
	DSB::OnDraw( pDC, bBrushOrigin );

	// �ְ� �̸�
	PutText( pDC, m_sMaster,
		0, 0, true, s_colYellow, s_tdShadeOutline );

	// ����
	LPCTSTR asMsg[3];
	if ( m_bFail ) {
		asMsg[0] = _T("�׷��� �ش� ī�带 ����");
		asMsg[1] = _T("�÷��̾ �����ϴ�");
		asMsg[2] = _T("�ٽ� ī�带 �����մϴ�");
	} else {
		asMsg[0] = _T("");
		asMsg[1] = _T("������ ī�带 ����");
		asMsg[2] = _T("�÷��̾ Ż����ŵ�ϴ� !!");
	}
	for ( int i = 0; i < 3; i++ )
		PutText( pDC, asMsg[i],
		-1, 1+i, true, s_colWhite, s_tdShade );

	CRect rc; GetRect( &rc );
	CSize szCard = GetBitmapSize(IDB_SA);
	CString sText = CCard(m_nCard).GetString( Mo()->bUseTerm );

	CRect rcItem = PutText( pDC, sText,
		-1, 5, true, s_colCyan, s_tdOutline );
	m_pBoard->GetBmpMan()->DrawCard( pDC, m_nCard,
		rc.left + (rc.Width()-15)/2 - szCard.cx/2,
		rcItem.bottom + 2,
		szCard.cx, szCard.cy );

	if ( m_bFail ) {

		// X ǥ �׸���
		CRect rcCurCard(
			CPoint( rc.left + (rc.Width()-15)/2 - szCard.cx/2,
					rcItem.bottom + 2 ),
			CSize( szCard.cx, szCard.cy ) );
		CRect rc(
			rcCurCard.left + rcCurCard.Width()/2 - rcCurCard.Width()/3,
			rcCurCard.top + rcCurCard.Height()/2 - rcCurCard.Width()/3,
			rcCurCard.left + rcCurCard.Width()/2 + rcCurCard.Width()/3,
			rcCurCard.top + rcCurCard.Height()/2 + rcCurCard.Width()/3 );

		CPen pen( PS_SOLID, 10, RGB(255,0,0) );
		CPen* pnOld = pDC->SelectObject( &pen );
		pDC->MoveTo( rc.TopLeft() ); pDC->LineTo( rc.BottomRight() );
		pDC->MoveTo( rc.right, rc.top ); pDC->LineTo( rc.left, rc.bottom );
		pDC->SelectObject( pnOld );
	}
}



/////////////////////////////////////////////////////////////////////////////
// ������ DSB

// m_nShape �� Ư�� ������ ��Ʈ�ϸ鼭 m_aatdShape ��
// �� �����Ѵ�
void DFriend::SetCurShape( int nShape )
{
	m_nShape = nShape;
	for ( int i = 0; i < 4; i++ ) {
		m_aatdShape[i][0] = SPADE+i == nShape ? TD_OUTLINE : TD_NORMAL;
		m_aatdShape[i][1] = SPADE+i == nShape ? TD_OUTLINE : TD_SUNKEN;
	}
	CRect rc; GetRect( &rc );
	m_pBoard->UpdateDSB( &rc );
}

// m_pnFriend ���� �����ϰ� �� ����� ȭ�鿡 ������Ʈ �Ѵ�
void DFriend::SetCurFriend( int nFriend )
{
	*m_pnFriend = nFriend;

	// 11 ���� ����� ������ ī�� �� ��� �ش��ϴ°� �˻��Ѵ�
	int nIndex = -1;
	if ( nFriend == 0 ) nIndex = 5;
	else if ( 0 < nFriend && nFriend < 54 ) {	// ī��
		if ( nFriend == CCard::GetMighty() ) nIndex = 0;
		else if ( nFriend == CCard::GetJoker() ) nIndex = 1;
		else if ( CCard::GetKiruda() )
			if ( nFriend == CCard(CCard::GetKiruda(),ACE) ) nIndex = 2;
			else if ( nFriend == CCard(CCard::GetKiruda(),KING) ) nIndex = 3;
	}
	else if ( nFriend == 100 ) nIndex = 4;	// �ʱ�
	else if ( nFriend < 0 ) nIndex = 6 - ( nFriend + 1 );	// ���

	for ( int i = 0; i < sizeof(m_acolFriend)/sizeof(COLORREF); i++ )
		if ( i == nIndex ) {
			m_acolFriend[i] = s_colCyan;
			m_atdFriend[i] = s_tdShade;
			m_atdFriendSel[i] = s_tdShadeOutline;
		}
		else {
			bool bEnable = true;

			if ( i < 4 ) {
				CCard c = i == 0 ? CCard::GetMighty()
					: i == 1 ? ( m_pState->pRule->bJokerFriend ? CCard::GetJoker() : CCard(0) )
					: !CCard::GetKiruda() ? CCard(0)
					: i == 2 ? CCard( CCard::GetKiruda(), ACE )
					: CCard( CCard::GetKiruda(), KING );
				if ( c == CCard(0) || m_lHand.Find(c) )
					bEnable = false;
			}

			if ( !bEnable ) {
				m_atdFriend[i] = s_tdNormal;
				m_atdFriendSel[i] = s_tdNormal;
				m_acolFriend[i] = s_colGray;
			}
			else {
				m_atdFriend[i] = s_tdShade;
				m_atdFriendSel[i] = s_tdShadeOutline;
				m_acolFriend[i] = s_colWhite;
			}
		}

	CRect rc; GetRect( &rc );
	m_pBoard->UpdateDSB( &rc );
}

// pcFriend : �� DSB �� ��� ī�带 ������ ��� (���-�� ��������)
void DFriend::Create( CEvent* pEvent, int* pnFriend, const CState* pState )
{
	ASSERT( m_pnFriend);

	m_pnFriend = pnFriend;
	m_lHand.RemoveAll();
	m_lHand.AddTail(pState->apPlayers[0]->GetHand());
	m_lHand.AddTail(&pState->lDeck);
	m_pState = pState;
	// ����Ʈ ����� ���� ���
	m_nShape = CCard::GetKiruda() ? CCard::GetKiruda()
		: 0 < *m_pnFriend && *m_pnFriend < 54	// ����ٸ� ������ ���
		? CCard(*m_pnFriend).GetShape()
		: CCard::GetMighty().GetShape();

	SetFixed();

	int nUnit = GetDSBUnit();

	// �ּ��� ũ��� ( 16 * 14 ) * ( card.cy + 32 + nUnit ) �� ī�� ���ÿ��� ���� ��
	int cxDSB = ( 16*14 + nUnit-1 ) / nUnit;
	int cyDSB = ( GetBitmapSize(IDB_SA).cy + 32 + nUnit + nUnit-1 ) / nUnit;

	// �ٸ� ���� �� ũ��
	cxDSB += 5;
	cyDSB += 13;

	DSB::Create( 0, 0, cxDSB, cyDSB, -1 );

	SetAction( pEvent, pnFriend );
}

// �ʱ�ȭ ( ������ �� ȣ��� )
void DFriend::OnInit()
{
	DSB::OnInit();
 
	SetCurShape( m_nShape );
	SetCurFriend( *m_pnFriend );

	int nDSBUnit = GetDSBUnit();
	CRect rc; GetRect( &rc );

	// "�ٸ� ī��" ����
	int x = rc.right - 16*14 - nDSBUnit;
	int xDiff = 16;
	int cx = 16;
	int y = rc.bottom - 32 - nDSBUnit - 3;
	int cy = 32 + 3;
	for ( int i = 0; i < 14; i++, x += xDiff )

		RegisterHotspot(
			x, y, cx, cy, false,
			0, 0, &s_colWhite, &s_tdNormal, &s_colWhite, &s_tdNormal,
			(LPVOID)(i+100) );

	CRect rcCurCard; CalcCurCardRect( &rcCurCard );
	RegisterHotspot(
		rcCurCard.left, rcCurCard.top, rcCurCard.Width(), rcCurCard.Height(),
		false, 0, 0, &s_colWhite, &s_tdNormal, &s_colWhite, &s_tdNormal,
			(LPVOID)0 );

	// 4 ���� Shape ���� ����

	CRect rcSpade = RegisterHotspot(
		rc.right - 16*14 - nDSBUnit - GetBitmapSize(IDB_SPADE).cx*2 - 8,
		rc.bottom - GetBitmapSize(IDB_SPADE).cy*2 - nDSBUnit, -1, -1, false,
		IDB_SPADE, 0,
		&s_colWhite, &m_aatdShape[0][0], &s_colWhite, &m_aatdShape[0][1],
		(LPVOID)1000 );

	RegisterHotspot(
		rcSpade.right, rcSpade.top, -1, -1, false,
		IDB_DIAMOND, 0,
		&s_colWhite, &m_aatdShape[1][0], &s_colWhite, &m_aatdShape[1][1],
		(LPVOID)1001 );

	RegisterHotspot(
		rcSpade.left, rcSpade.bottom, -1, -1, false,
		IDB_HEART, 0,
		&s_colWhite, &m_aatdShape[2][0], &s_colWhite, &m_aatdShape[2][1],
		(LPVOID)1002 );

	CRect rcClover = RegisterHotspot(
		rcSpade.right, rcSpade.bottom, -1, -1, false,
		IDB_CLOVER, 0,
		&s_colWhite, &m_aatdShape[3][0], &s_colWhite, &m_aatdShape[3][1],
		(LPVOID)1003 );

	// ������ ī���
	rc = RegisterHotspot(		// ����Ƽ
		1, 6, -1, -1, true, 0,
		CCard::GetMighty().GetString(Mo()->bUseTerm) + _T(" ������"),
		&m_acolFriend[0], &m_atdFriend[0],
		&m_acolFriend[0], &m_atdFriendSel[0], (LPVOID)10 );
	RegisterHotspot(			// ��Ŀ
		rc.left, rc.bottom + nDSBUnit/2, -1, -1, false, 0,
		CCard::GetJoker().GetString(Mo()->bUseTerm) + _T(" ������"),
		&m_acolFriend[1], &m_atdFriend[1],
		&m_acolFriend[1], &m_atdFriendSel[1], (LPVOID)11 );
	rc = RegisterHotspot(			// ���� A ������
		9, 6, -1, -1, true, 0,
		Mo()->bUseTerm ? _T("��� ������") : _T("���� A ������"),
		&m_acolFriend[2], &m_atdFriend[2],
		&m_acolFriend[2], &m_atdFriendSel[2], (LPVOID)12 );
	RegisterHotspot(			// ���� K ������
		rc.left, rc.bottom + nDSBUnit/2, -1, -1, false, 0,
		Mo()->bUseTerm ? _T("��ī ������") : _T("���� K ������"),
		&m_acolFriend[3], &m_atdFriend[3],
		&m_acolFriend[3], &m_atdFriendSel[3], (LPVOID)13 );
	RegisterHotspot(			// �ʱ�
		1, 10, -1, -1, true, 0,
		_T("�ʱ� ������"),
		&m_acolFriend[4], &m_atdFriend[4],
		&m_acolFriend[4], &m_atdFriendSel[4], (LPVOID)14 );
	RegisterHotspot(			// �� ������
		9, 10, -1, -1, true, 0,
		_T("�� ������"),
		&m_acolFriend[5], &m_atdFriend[5],
		&m_acolFriend[5], &m_atdFriendSel[5], (LPVOID)15 );

	// �÷��̾�
	if ( m_pState->apPlayers[0] ) {
		for ( int p = 1; p < m_pState->nPlayers; p++ )
			RegisterHotspot(
				1, 13+p, -1, -1, true, 0,
				m_pState->apPlayers[p]->GetName() + _T(" ������"),
				&m_acolFriend[6+p], &m_atdFriend[6+p],
				&m_acolFriend[6+p], &m_atdFriendSel[6+p], (LPVOID)(16+p) );
	}
}

// ���� ī�带 �׸� ������ �����Ѵ�
void DFriend::CalcCurCardRect( LPRECT pRc )
{
	CRect rc; GetRect( &rc );
	int nDSBUnit = GetDSBUnit();
	CSize szCard = GetBitmapSize(IDB_SA);
	pRc->left = rc.right - nDSBUnit - szCard.cx;
	pRc->top = rc.bottom - 32 - nDSBUnit - 5 - szCard.cy;
	pRc->right = pRc->left + szCard.cx;
	pRc->bottom = pRc->top + szCard.cy;
}

// CBoard �� ȣ���ϴ� �׸��� �Լ� - ��µ� shade-box �� �� �Լ���
// �������̵� �Ͽ� �����ؾ� �Ѵ�
// ���̽� �Լ��� ������ ��ü�� ���� ���� ������ �׸���
void DFriend::OnDraw(
	CDC* pDC,
	bool bBrushOrigin )
{
	DSB::OnDraw( pDC, bBrushOrigin );

	// ���� ���õ� ī��, �Ǵ� �÷��̾ �׸���
	bool bDrawX;
	CRect rcCurCard; CalcCurCardRect( &rcCurCard );

	if ( 0 < *m_pnFriend && *m_pnFriend < 54 ) {	// ī��
		bDrawX = m_lHand.Find( CCard(*m_pnFriend) )
			|| CCard(*m_pnFriend).IsJoker()	// ��Ŀ������ �Ұ�
				&& !m_pState->pRule->bJokerFriend
			? true : false;
		m_pBoard->GetBmpMan()->DrawCard( pDC,
			CCard(*m_pnFriend), rcCurCard );
	}
	else {	// �÷��̾�, ��������, �ʱ�
		ASSERT( *m_pnFriend == 0 || *m_pnFriend == 100
			|| *m_pnFriend < -1 && *m_pnFriend > -m_pState->nPlayers-1 );
		bDrawX = *m_pnFriend == 0;
		DrawBitmap( pDC, IDB_PERSON, rcCurCard.left, rcCurCard.top,
			rcCurCard.Width(), rcCurCard.Height(),
			0, 0, rcCurCard.Width(), rcCurCard.Height() );
		// �ؽ�Ʈ�� ��ġ (s,x,y)
		CString s = *m_pnFriend == 100 ? _T("�ʱ�")
			: *m_pnFriend != 0 ?
				m_pState->apPlayers[-*m_pnFriend-1]->GetName()
				: _T("");
		int x = rcCurCard.left + rcCurCard.Width()/2 - GetTextExtent( 0, s ).cx/2;
		int y = rcCurCard.bottom - GetBitmapSize(IDB_SA).cy / 3;

		PutText( pDC, s, x, y, false, s_colWhite, s_tdShadeOutline );
	}

	// �� ī�尡 �տ� �ִ� ī���̰ų� ���������̸� x ǥ�� �Ѵ�
	if ( bDrawX ) {
		CRect rc(
			rcCurCard.left + rcCurCard.Width()/2 - rcCurCard.Width()/3,
			rcCurCard.top + rcCurCard.Height()/2 - rcCurCard.Width()/3,
			rcCurCard.left + rcCurCard.Width()/2 + rcCurCard.Width()/3,
			rcCurCard.top + rcCurCard.Height()/2 + rcCurCard.Width()/3 );

		CPen pen( PS_SOLID, 10, RGB(255,0,0) );
		CPen* pnOld = pDC->SelectObject( &pen );
		pDC->MoveTo( rc.TopLeft() ); pDC->LineTo( rc.BottomRight() );
		pDC->MoveTo( rc.right, rc.top ); pDC->LineTo( rc.left, rc.bottom );
		pDC->SelectObject( pnOld );
	}

	// "�ٸ� ī��" ����
	// 13 ���� ���� ī�带 �׸���
	int nDSBUnit = GetDSBUnit();
	CRect rc; GetRect( &rc );
	int x = rc.right - 16*14 - nDSBUnit;
	int xDiff = 16;
	int cx = 16;
	int y = rc.bottom - 32 - nDSBUnit;
	int cy = 32;
	CCard c;
	for ( int i = 0; i < 14; i++, x += xDiff ) {

		if ( i == 0 ) c = CCard::GetJoker();	// ������ ���� ��Ŀ
		else if ( i == 1 ) c = CCard( m_nShape, ACE );	// ù���� ���̽�
		else c = CCard( m_nShape, KING - (i-2) );

		bool bSel =		// ���� ���õ� Hotspot �ΰ�
			m_posSel && m_lHotspot.GetAt(m_posSel).pVoid == (LPVOID)(100+i)
			&& !m_lHand.Find(c);
		int yDiff = bSel ? 3 : 0;

		m_pBoard->GetBmpMan()->DrawCardEx(
			pDC, (int)c, x, y - yDiff, cx, cy + yDiff,
			0, 0, cx, cy+yDiff );
	}

	// '�ٸ� ī��'
	PutText( pDC, _T("�ٸ� ī��:"), rc.left + nDSBUnit,
		rc.bottom - GetBitmapSize(IDB_SPADE).cy*2 - nDSBUnit*5/2,
		false, s_colWhite, s_tdShade );

	// ��Ÿ �ؽ�Ʈ
	static LPCTSTR asText[] = {
		_T("�����带 �����մϴ�"),
		_T(""),
		_T("���ϴ� '������ ī��'�� �÷��̾"),
		_T("�����ϰ� ���⸦ Ŭ���ϼ���")
	};
	CRect rcText;
	for ( int m = 0; m < sizeof(asText)/sizeof(LPCTSTR); m++ )
		rcText = PutText( pDC, asText[m], 1, 1+m,
							true, s_colWhite, s_tdShade );
	// '����' �� ���� ȭ��ǥ
	CPen pnMedium( PS_SOLID, 2, s_colWhite );
	pDC->SelectObject( &pnMedium );
	x = rcText.right + nDSBUnit;
	y = ( rcText.top + rcText.bottom ) / 2;
	pDC->MoveTo( x, y );
	x = ( rcCurCard.left + rcCurCard.right * 3 ) /4;	// 1:3 ����
	pDC->LineTo( x, y );
	y = rcCurCard.top - nDSBUnit/3;
	pDC->LineTo( x, y );
	pDC->LineTo( x - nDSBUnit/2, y - nDSBUnit/2 );
	pDC->MoveTo( x, y );
	pDC->LineTo( x + nDSBUnit/2, y - nDSBUnit/2 );
	pDC->SelectStockObject( WHITE_PEN );

	// '�÷��̾�:'
	PutText( pDC, _T("�÷��̾�:"),
		rc.left+nDSBUnit, rc.top+nDSBUnit*12+nDSBUnit/2, false,
		s_colWhite, s_tdShade );
}

// �� ������ Ŭ���� �� �Ҹ��� �Լ�
// LPVOID ���� 0 �̸� ���� (X�ڸ� Ŭ��) ��û
void DFriend::OnClick( LPVOID pVoid )
{
	int n = (int)pVoid;

	if ( n >= 1000 && n <= 1003 ) {
		// ��� ����
		DSB::OnClickSound();
		SetCurShape( n-1000+SPADE );
	}
	else if ( n >= 100 && n <= 113 ) {
		// ī�� ����
		DSB::OnClickSound();
		CCard c;
		if ( n == 100 )			// Joker
			c = CCard(JOKER);
		else if ( n == 101 )	// ACE
			c = CCard(m_nShape,ACE);
		else
			c = CCard( m_nShape, 115-n );

		SetCurFriend( (int)c );
	}
	else if ( n >= 10 && n < 16 ) {
		// Ư�� ī��
		DSB::OnClickSound();
		if ( n == 10 ) SetCurFriend( (int)CCard::GetMighty() );
		else if ( n == 11 ) SetCurFriend( (int)CCard::GetJoker() );
		else if ( n == 12 && CCard::GetKiruda() )
			SetCurFriend( (int)CCard(CCard::GetKiruda(),ACE) );
		else if ( n == 13 && CCard::GetKiruda() )
			SetCurFriend( (int)CCard(CCard::GetKiruda(),KING) );
		else if ( n == 14 ) SetCurFriend( 100 );
		else if ( n == 15 ) SetCurFriend( 0 );
	}
	else if ( n >= 16 && n < 16+MAX_PLAYERS ) {
		// �÷��̾�
		DSB::OnClickSound();
		SetCurFriend( -( n - 16 ) - 1 );
	}
	else if ( n == 0 || n == 0xffffffff ) {	// ����

		// �տ� �ִ� ī�带 �����ؼ��� �ȵȴ�
		if ( *m_pnFriend > 0 && *m_pnFriend < 54 ) {

			if ( m_lHand.Find(*m_pnFriend) ) {
				// ��� �޽��� ǥ��
				if ( Mo()->bUseSound ) MessageBeep( MB_ICONEXCLAMATION );
				(new DShortMessage(m_pBoard))->Create(
					0, _T("�̹� ������ �ְų� ���� ī���Դϴ�"),
					true, false, 2000 );
			} else if ( CCard(*m_pnFriend).IsJoker()	// ��Ŀ������ �Ұ�
				&& !m_pState->pRule->bJokerFriend ) {
				if ( Mo()->bUseSound ) MessageBeep( MB_ICONEXCLAMATION );
				(new DShortMessage(m_pBoard))->Create(
					0, _T("������� �θ��� ���� ī���Դϴ�"),
					true, false, 2000 );
			}
			else DSB::OnClick(0);
		}
		else {
			DSB::OnClick(0);
		}
	}
}
