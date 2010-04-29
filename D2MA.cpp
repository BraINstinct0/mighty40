// DElection.cpp: implementation of the D2MA class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mighty.h"

#include "BoardWrap.h"
#include "BmpMan.h"
#include "DSB.h"
#include "DEtc.h"
#include "D2MA.h"

#include "Play.h"
#include "Player.h"
#include "MFSM.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


/////////////////////////////////////////////////////////////////////////////
// ī�� ���� DSB

// pcToKill : �� DSB �� ��� ī�带 ������ ��� (���-�� ��������)
// plShow   : ���̴� (���� �ִ�) ī��
// plHide   : ������ (�Ʒ��� �ִ�) ī��
void DSelect2MA::Create( CEvent* pEvent, int* selecting, CCardList* plCard )
{
	m_pselecting = selecting;
	m_pcShow = plCard->GetAt(plCard->POSITIONFromIndex(0));
	m_pcHide = plCard->GetAt(plCard->POSITIONFromIndex(1));

	SetFixed();

	int nUnit = GetDSBUnit();

	// �ּ��� ũ��� ( 15 * 14 ) * ( card.cy + 18 + nUnit ) �� ī�� ���ÿ��� ���� ��
	int cxDSB = ( 15*14 + nUnit-1 ) / nUnit;
	int cyDSB = ( GetBitmapSize(IDB_SA).cy + 18 + nUnit + nUnit-1 ) / nUnit;

	// �ٸ� ���� �� ũ��
	cxDSB += 5;
	cyDSB += 1;

	CRect rc = m_pBoard->CalcPlayerExtent( 0 );

	DSB::Create2(
		rc.left+rc.Width()/2 - cxDSB*GetDSBUnit()/2,
		rc.bottom - cyDSB*GetDSBUnit(),
		cxDSB, cyDSB, -1 );

	SetAction( pEvent );
}

// �ʱ�ȭ ( ������ �� ȣ��� )
void DSelect2MA::OnInit()
{
	DSB::OnInit();

	int nDSBUnit = GetDSBUnit();
	CRect rc; GetRect( &rc );

	CRect rcShowCard; CalcShowCardRect( &rcShowCard );
	RegisterHotspot(
		rcShowCard.left, rcShowCard.top, rcShowCard.Width(), rcShowCard.Height(),
		false, 0, 0, &s_colWhite, &s_tdNormal, &s_colWhite, &s_tdNormal,
			(LPVOID)0 );
	CRect rcHideCard; CalcHideCardRect( &rcHideCard );
	RegisterHotspot(
		rcHideCard.left, rcHideCard.top, rcHideCard.Width(), rcHideCard.Height(),
		false, 0, 0, &s_colWhite, &s_tdNormal, &s_colWhite, &s_tdNormal,
			(LPVOID)1 );
}

// ���̴� ī�带 �׸� ������ �����Ѵ�
void DSelect2MA::CalcShowCardRect( LPRECT pRc )
{
	CRect rc; GetRect( &rc );
	int nDSBUnit = GetDSBUnit();
	CSize szCard = GetBitmapSize(IDB_SA);
	pRc->left = rc.right - nDSBUnit - szCard.cx * 2;
	pRc->top = rc.bottom - 14 - nDSBUnit - 5 - szCard.cy;
	pRc->right = pRc->left + szCard.cx;
	pRc->bottom = pRc->top + szCard.cy;
}
// ������ ī�带 �׸� ������ �����Ѵ�.
void DSelect2MA::CalcHideCardRect( LPRECT pRc )
{
	CRect rc; GetRect( &rc );
	int nDSBUnit = GetDSBUnit();
	CSize szCard = GetBitmapSize(IDB_SA);
	pRc->left = rc.right - nDSBUnit - szCard.cx;
	pRc->top = rc.bottom - 14 - nDSBUnit - 5 - szCard.cy;
	pRc->right = pRc->left + szCard.cx;
	pRc->bottom = pRc->top + szCard.cy;
}

// CBoard �� ȣ���ϴ� �׸��� �Լ� - ��µ� shade-box �� �� �Լ���
// �������̵� �Ͽ� �����ؾ� �Ѵ�
// ���̽� �Լ��� ������ ��ü�� ���� ���� ������ �׸���
void DSelect2MA::OnDraw(
	CDC* pDC,
	bool bBrushOrigin )
{
	DSB::OnDraw( pDC, bBrushOrigin );

	// ���̴� ī�带 �׸���
	CRect rcShowCard; CalcShowCardRect( &rcShowCard );
	m_pBoard->GetBmpMan()->DrawCard( pDC, m_pcShow, rcShowCard );
	// ������ ī�带 �׸���
	CRect rcHideCard; CalcHideCardRect( &rcHideCard );
#ifdef _DEBUG
	m_pBoard->GetBmpMan()->DrawCard( pDC, m_pcHide, rcHideCard );
#else
	m_pBoard->GetBmpMan()->DrawCard( pDC, CCard(0/*back*/), rcHideCard );
#endif

	static LPCTSTR asText[] = {
		_T("���̴� ī���"),
		_T("������ ī���� "),
		_T("�ϳ��� ����"),
		_T("�ϼ���"),
	};
	for ( int m = 0; m < sizeof(asText)/sizeof(LPCTSTR); m++ )
		PutText( pDC, asText[m], 1, 3+m, true, s_colWhite, s_tdShade );
}

// �� ������ Ŭ���� �� �Ҹ��� �Լ�
// LPVOID ���� 0 �̸� ���� (X�ڸ� Ŭ��) ��û
void DSelect2MA::OnClick( LPVOID pVoid )
{
	int n = (int)pVoid;

	if ( n == 1 ) {
		// ������ ( �Ʒ��� ) ī�� ����
		DSB::OnClickSound();
		*m_pselecting = 1;
		DSB::OnClick(0);
		return;
	}
	else if ( n == 0 || n == 0xffffffff ) {	// ����

		DSB::OnClickSound();
		*m_pselecting = 0;
		DSB::OnClick(0);
		return;
	}

	RECT rc;
	GetRect( &rc );
	m_pBoard->UpdateDSB( &rc, true );
}
