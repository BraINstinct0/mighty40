// DEtc.cpp: implementation of the DEtc class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mighty.h"
#include "BoardWrap.h"
#include "DSB.h"
#include "DEtc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


/////////////////////////////////////////////////////////////////////////////
// DAbout

void DAbout::Create()
{
	SetFixed();
	DSB::Create( 0, 0, 18, 10, -1 );
}

// �ʱ�ȭ ( ������ �� ȣ��� )
void DAbout::OnInit()
{
	DSB::OnInit();

	RegisterHotspot(
		8, 8, -1, -1, true, 0,
		_T("Ȯ��"),
		&s_colWhite, &s_tdShade, &s_colCyan, &s_tdShadeOutline,
		(LPVOID)0 );
}

// CBoard �� ȣ���ϴ� �׸��� �Լ� - ��µ� shade-box �� �� �Լ���
// �������̵� �Ͽ� �����ؾ� �Ѵ�
// ���̽� �Լ��� ������ ��ü�� ���� ���� ������ �׸���
void DAbout::OnDraw(
	CDC* pDC,
	bool bBrushOrigin )
{
	DSB::OnDraw( pDC, bBrushOrigin );

	CRect rc = PutBitmap( pDC, IDB_MIGHTY,
		_T(" ����Ƽ ��Ʈ��ũ 3.2"), 1, 1, true,
		s_colWhite, s_tdShade );

	rc = PutText( pDC, _T("Copyright (C) 1999 �幮��"),
		rc.left, rc.bottom + rc.Height()/2, false,
		s_colWhite, s_tdShade );
	PutText( pDC, _T("sw6ueyz@hitel.net"),
		rc.left, rc.bottom + rc.Height()/4, false,
		s_colWhite, s_tdShade );
}


/////////////////////////////////////////////////////////////////////////////
// DMessageBox

DMessageBox::DMessageBox( CBoardWrap* pBoard ) : DSB(pBoard)
{
	m_nLines = 0;
	m_asLine = 0;
}

DMessageBox::~DMessageBox()
{
	delete[] m_asLine;
}

// nLines ���� �޽����� ok ��ư�� ����Ѵ�
// nTimeOut �� -1 �� �ƴϸ� ok ��ư�� ����
void DMessageBox::Create( bool bCenter, int nLines,
	LPCTSTR asLine[], int nTimeOut )
{
	int nUnit = GetDSBUnit();

	m_bCenter = bCenter;
	m_nLines = nLines;
	m_asLine = new CString[nLines];

	// ��Ʈ���� m_asLine �� �����ϸ鼭
	// �ִ� ���� ũ�⸦ ���Ѵ�
	int nMaxWidth = 0;
	for ( int i = 0; i < nLines; i++ ) {
		m_asLine[i] = asLine[i];
		int nWidth = GetTextExtent( 0, asLine[i] ).cx;
		if ( nMaxWidth < nWidth )
			nMaxWidth = nWidth;
	}

	// ���� DSB �� ũ�⸦ ����ϰ� DSB �� ����
	int xDSB = ( nMaxWidth + nUnit - 1 ) / nUnit + 2;
	int yDSB = nLines + 2 + ( nTimeOut == -1 ? 2 : 0 );

	DSB::Create( 0, 0, xDSB, yDSB, nTimeOut );
}

// �ʱ�ȭ ( ������ �� ȣ��� )
void DMessageBox::OnInit()
{
	DSB::OnInit();

	if ( m_nTimeOut == -1 )

		RegisterHotspot(
			m_rcDSB.right/2-1, m_rcDSB.bottom-2, -1, -1, true, 0,
			_T("Ȯ��"),
			&s_colWhite, &s_tdShade, &s_colCyan, &s_tdShadeOutline,
			(LPVOID)0 );
}

// CBoard �� ȣ���ϴ� �׸��� �Լ� - ��µ� shade-box �� �� �Լ���
// �������̵� �Ͽ� �����ؾ� �Ѵ�
// ���̽� �Լ��� ������ ��ü�� ���� ���� ������ �׸���
void DMessageBox::OnDraw(
	CDC* pDC,
	bool bBrushOrigin )
{
	DSB::OnDraw( pDC, bBrushOrigin );

	for ( int i = 0; i < m_nLines; i++ )
		PutText( pDC, m_asLine[i],
			( m_bCenter ? -1 : 1 ),
			i + 1, true, s_colWhite, s_tdShade );
}


/////////////////////////////////////////////////////////////////////////////
// DShortMessage

// idBitmap : ��Ʈ�� ( ������ 0 )
// s        : �޽��� ( ������ 0 )
// bCenter  : �޽����� ������ ��� ��µǴ°�?
// bFixed   : Fixed DSB �ΰ�
// nTimeOut : Ÿ�Ӿƿ� (millisecond)
// nPlayer  : ǥ�õ� �÷��̾� ��ȣ ( -1 �̸� ��� )
// sName    : �� �÷��̾��� �̸� ( 0 �̸� �̸� ǥ�þȵ� )
void DShortMessage::Create( UINT idBitmap, LPCTSTR s,
		bool bCenter, bool bFixedSize,
		int nTimeOut, int nPlayer, LPCTSTR sName )
{
	if ( bFixedSize ) SetFixed();

	m_idBitmap = idBitmap;
	m_s = s ? s : _T("");
	m_bCenter = bCenter;
	m_sName = sName ? sName : _T("");

	// �ʿ��� ũ�⸦ DSB ������ ����Ѵ�

	int nUnit = GetDSBUnit();

	SIZE szBitmap;
	if ( idBitmap == 0 ) szBitmap = CSize(0,0);
	else szBitmap = GetBitmapSize(idBitmap);

	int nLen = s ? GetTextExtent(0,s).cx : 0;
	int nNameLen = sName ? GetTextExtent( 0, sName ).cx : 0;
	if ( nNameLen > nLen ) nLen = nNameLen;

	int cxDSB = ( szBitmap.cx + 4 + nLen + nUnit - 1 + 18 /* X��ư */ ) / nUnit;
	int cyDSB = max( ( szBitmap.cy + 4 + nUnit - 1 ) / nUnit, 2 );
	if ( sName ) cyDSB += 2;

	// ǥ�õ� ��ġ�� ����Ѵ�
	if ( nPlayer == -1 )
		DSB::Create( 0, 0, cxDSB, cyDSB, nTimeOut );
	else {
		bool bVert;
		CRect rcClient; m_pBoard->GetClientRect( &rcClient );
		CRect rcRange = m_pBoard->CalcPlayerExtent( nPlayer, &bVert );

		DSB::Create2(
			rcRange.left+rcRange.Width()/2 - cxDSB*nUnit/2,
			bVert || rcRange.bottom < rcClient.top + rcClient.Height()/2
				? rcRange.top+rcRange.Height()/2 - cyDSB*nUnit/2
				: rcRange.bottom - cyDSB*nUnit,
			cxDSB, cyDSB, nTimeOut );
	}
}

// CBoard �� ȣ���ϴ� �׸��� �Լ� - ��µ� shade-box �� �� �Լ���
// �������̵� �Ͽ� �����ؾ� �Ѵ�
// ���̽� �Լ��� ������ ��ü�� ���� ���� ������ �׸���
void DShortMessage::OnDraw(
	CDC* pDC,
	bool bBrushOrigin )
{
	DSB::OnDraw( pDC, bBrushOrigin );

	if ( !m_sName.IsEmpty() ) {

		CRect rcBox; GetRect( &rcBox );
		CRect rcName = PutText( pDC, m_sName,
			0, 0, true, s_colYellow, s_tdShadeOutline );

		if ( m_idBitmap == 0 ) {
			CRect rc = PutText( pDC, m_s, m_bCenter ? -1 : 0, -1, true,
					s_colWhite, s_tdShade, true );
			PutText( pDC, m_s,
				rc.left,
				( rcBox.bottom + rcName.bottom )/2 - rc.Height()/2,
				false, s_colWhite, s_tdShade );
		}
		else {
			CRect rc = PutBitmap( pDC, m_idBitmap, m_s,
					m_bCenter ? -1 : 0, 1, true,
					s_colWhite, s_tdShade, true );
			PutBitmap( pDC, m_idBitmap, m_s,
				rc.left,
				( rcBox.bottom + rcName.bottom )/2 - rc.Height()/2,
				false, s_colWhite, s_tdShade );
		}
	}
	else {

		if ( m_idBitmap == 0 )
			PutText( pDC, m_s, m_bCenter ? -1 : 0, -1, true,
					s_colWhite, s_tdShade );
		else PutBitmap( pDC, m_idBitmap, m_s,
					m_bCenter ? -1 : 0, -1, true,
					s_colWhite, s_tdShade );
	}
}


/////////////////////////////////////////////////////////////////////////////
// DSelect

// x, y     : ���̾�αװ� ���ܾ� �� ��ġ (CBoard ������ǥ)
// asText   : ������ �ؽ�Ʈ �迭
// nText    : asText �� ������ ����
// pResult  :���� ����� �ε��� (-1�� ���)
void DSelect::Create( int x, int y,
	LPCTSTR asText[], int nText, CEvent* pEvent, long* pResult )
{
	m_asText = asText;
	m_nText = nText;

	int nMaxWidth = 0;
	for ( int i = 0; i < m_nText; i++ )
		nMaxWidth = max( nMaxWidth, GetTextExtent(0,asText[i]).cx );

	int nUnit = GetDSBUnit();
	int nMaxWidthDSB = ( nMaxWidth + nUnit - 1 ) / nUnit + 2;

	SetModal();

	DSB::Create2( x, y, nMaxWidthDSB, nText, -1 );

	SetAction( pEvent, pResult );
}

// �ʱ�ȭ ( ������ �� ȣ��� )
void DSelect::OnInit()
{
	DSB::OnInit();

	for ( int i = 0; i < m_nText; i++ )

		RegisterHotspot(
			0, i, m_rcDSB.right-2, 1, true, 0,
			m_asText[i],
			&s_colWhite, &s_tdShade, &s_colCyan, &s_tdShadeOutline,
			(LPVOID)(i+1) );
}

// �� ������ Ŭ���� �� �Ҹ��� �Լ� ( ������ ȣ���Ѵ� )
// LPVOID ���� 0 �̸� ���� (X�ڸ� Ŭ��) ��û
void DSelect::OnClick( LPVOID pVoid )
{
	if ( pVoid == (LPVOID)0xffffffff ) pVoid = 0;	// Enter Key

	if ( m_pResult ) *(long*)m_pResult = (long)pVoid - 1;
	Destroy();
}


/////////////////////////////////////////////////////////////////////////////
// DSelectJokerShape

// x, y           : ���̾�αװ� ���ܾ� �� ��ġ (CBoard ������ǥ)
// bUseTerm       : �� �����ΰ� (��Ŀ��/��Ŀ�ݾƴ� ��ī��!/���ݾƴ�)
// nPlayer, nCard : mmTurn �޽����� ����µ� �ʿ��� ��
//                  ī�带 �� �÷��̾�, �� ī��
void DSelectJokerShape::Create( int x, int y,
	CEvent* pEvent, long* pShape )
{
	SetFixed();
	SetModal();
	DSB::Create2( x, y, 5, 4, -1 );
	SetAction( pEvent, pShape );
}

// �ʱ�ȭ ( ������ �� ȣ��� )
void DSelectJokerShape::OnInit()
{
	DSB::OnInit();

	RegisterHotspot(
		0, 0, -1, -1, true, IDB_SPADE, 0,
		&s_colWhite, &s_tdOutline, &s_colWhite, &s_tdShade,
		(LPVOID)1 );
	RegisterHotspot(
		2, 0, -1, -1, true, IDB_DIAMOND, 0,
		&s_colWhite, &s_tdOutline, &s_colWhite, &s_tdShade,
		(LPVOID)2 );
	RegisterHotspot(
		0, 2, -1, -1, true, IDB_HEART, 0,
		&s_colWhite, &s_tdOutline, &s_colWhite, &s_tdShade,
		(LPVOID)3 );
	RegisterHotspot(
		2, 2, -1, -1, true, IDB_CLOVER, 0,
		&s_colWhite, &s_tdOutline, &s_colWhite, &s_tdShade,
		(LPVOID)4 );
}

// �� ������ Ŭ���� �� �Ҹ��� �Լ� ( ������ ȣ���Ѵ� )
// LPVOID ���� 0 �̸� ���� (X�ڸ� Ŭ��) ��û
void DSelectJokerShape::OnClick( LPVOID pVoid )
{
	if ( pVoid == (LPVOID)0xffffffff ) pVoid = 0;	// Enter Key

	*(long*)m_pResult = (int)pVoid;
	Destroy();
}


/////////////////////////////////////////////////////////////////////////////
// DSay

DSay::DSay( CBoardWrap* pBoard ) : DSB(pBoard)
{
	m_asLine = 0;
}

DSay::~DSay()
{
	if ( m_asLine ) {
		for ( int i = 0; i < m_nLines; i++ )
			delete[] m_asLine[i];
		delete[] m_asLine;
	}
}

// x, y  : ������ �߽� �� (DP ��ǥ)
//         ���ڰ� pBoard�� Ŭ���̾�Ʈ ������ ���� �� ���
//         �������� �����鿩 ���� ���߾� ����
// cLine : �� �ٿ� ���� ������ �뷫���� �� (DSB����)
//         (�� ���� �Ѿ�� �ٹٲ� ��)
// sWho  : ���� ����� �̸�����, �� ���ٿ� ����ü�� ǥ�õȴ�
// sMsg  : ���� ����
// nTimeOut : ���� �ð� (millisecond)
// pDSB  : Board �� �� �ִ� �� �÷��̾��� ���� ��ȭ ����
// bSetBelow : �� ������ �� ���ΰ�
void DSay::Create( DSay* pDSB, int x, int y,
				  int cLine, LPCTSTR sWho, LPCTSTR sMsg, int nTimeOut,
				  bool bSetBelow )
{
	int i;
	int nUnit = GetDSBUnit();

	// Below ���� ���� (ުBelow ��� ���ں��� ������ �Ʒ���)

	// ����: �� ����� �־����� ��ȭ�� �ʹ� �Ⱥ��̴� ������ �߻�
	//       �ٽ� �� ���� ���� �ߴ�
	if ( bSetBelow ) SetBelow();


	// ���� �� ��Ʈ���� �� �� ���� �Ǵ� ��Ʈ���ΰ��� �����Ѵ�
	size_t nMaxWidth;
	m_nLines = ParseString( 0, cLine, sMsg, nMaxWidth );
	if ( m_nLines < 0 )	{ // �������� ��Ʈ��
		delete this;
		return;
	}

	// ���� DSB �� ������ �� ���� �̾Ƴ��� !
	m_nLastLines = 0;
	if ( pDSB ) {

		for ( int i = 0; i < 2; i++ ) {

			if ( pDSB->m_nLines+pDSB->m_nLastLines > i ) {
				m_asLast[i] =
					pDSB->m_nLines > i
					? pDSB->m_asLine[pDSB->m_nLines-i-1]
					: pDSB->m_asLast[i-pDSB->m_nLines];
				m_nLastLines++;
			}
			size_t len = _tcslen( m_asLast[i] );
			if ( nMaxWidth < len+1 ) nMaxWidth = len+1;
		}
	}

	// �޸𸮸� �Ҵ��ϰ� ������ ��Ʈ���� �Ľ� �Ѵ�
	m_asLine = new TCHAR*[m_nLines];
	for ( i = 0; i < m_nLines; i++ )
		m_asLine[i] = new TCHAR[nMaxWidth];

	ParseString( m_asLine, cLine, sMsg, nMaxWidth );

	// ������ ���� ����
	m_sWho = sWho;

	// ������ ���� cLine DSB����
	int cxDSB = cLine + 2;
	int cx = cxDSB * nUnit;

	// ������ ���̸� ���Ѵ� - DSB ������ �� �� + 3 �̴�
	int cyDSB = m_nLines + 2 + ( m_nLastLines > 0 ? m_nLastLines : 1 );
	int cy = cyDSB * nUnit;

	// ������ ��ġ��?
	int l = x - cx/2;
	int t = y - cy/2;

	// ���ڸ� �����Ѵ�
	DSB::Create2( l, t, cxDSB, cyDSB, nTimeOut );

	// ������ �ִ� ���ڴ� �����
	if ( pDSB ) pDSB->Destroy();
}

// CBoard �� ȣ���ϴ� �׸��� �Լ� - ��µ� shade-box �� �� �Լ���
// �������̵� �Ͽ� �����ؾ� �Ѵ�
// ���̽� �Լ��� ������ ��ü�� ���� ���� ������ �׸���
void DSay::OnDraw(
	CDC* pDC,
	bool bBrushOrigin )
{
	DSB::OnDraw( pDC, bBrushOrigin );

	// �̸� ���
	PutText( pDC, m_sWho, 0, 0, true, s_colYellow, s_tdShadeOutline );

	// ���� �� ���
	for ( int j = 0; j < m_nLastLines; j++ )
			PutText( pDC, m_asLast[j], 1, -j+m_nLastLines, true,
				s_colLightGray, s_tdNormal );

	// ���� ���
	for ( int i = 0; i < m_nLines; i++ )
		PutText( pDC, m_asLine[i], 1, 1+i+m_nLastLines+(m_nLastLines>0?0:1), true,
			s_colWhite, s_tdShade );
}
