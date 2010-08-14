// Board_Layout.cpp : CBoard �� ���̾ƿ��� ���� �Լ��� ����
//

#include "stdafx.h"
#include "Mighty.h"
#include "BmpMan.h"
#include "Board.h"
#include "Play.h"
#include "Player.h"
#include "MFSM.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// �÷��̾��ȣ�� �����Ѵ� - �Ʒ��� �÷��̾����
// �ð� �������� 0 ~ 6(v4.0) �� ǥ�� ���(���밪)��
// ���� �÷��̾� ��ȣ(����)�� �����ϰų� ����ȯ �Ѵ�
int CBoard::MapAbsToLogic( int nAbsPlayer ) const
{
	const CState* pState = m_pMFSM ? m_pMFSM->GetState() : 0;

	ASSERT( pState && pState->nPlayers > nAbsPlayer );

	if ( !pState ) return nAbsPlayer;

	return nAbsPlayer;
}

int CBoard::MapLogicToAbs( int nLogicalPlayer ) const
{
	return MapAbsToLogic( nLogicalPlayer );
}

// �� �÷��̾��� �̸� ��ġ�� ���Ѵ�
CRect CBoard::CalcNameRect( int nPlayers, int nPlayer,
	CDC* pDC, LPCTSTR sName, CFont* pFont ) const
{
	// ī�� ũ��
	int nCardWidth = m_szCard.cx;
	int nCardHeight = m_szCard.cy;

	// ���� ������ ī�� ���� ������ ������ �׸� ��ġ�� ���Ѵ�
	bool bVert;
	CRect rcScore = CalcRect( nPlayers, CR_SCORE, nPlayer,
		nCardWidth, nCardHeight, -1, 0, &bVert );
	CRect rcHand = CalcRect( nPlayers, CR_HAND, nPlayer,
		nCardWidth, nCardHeight );
	CRect rcScreen; GetClientRect( &rcScreen );

	// ������ ī�� ���� 1/16
	int nMargin = nCardWidth/16;

	// �׸� ��ġ
	int x, y;

	// �ؽ�Ʈ ���� ���
	UINT ta;

	// ��Ʈ ����
	CFont* pfntOld;
	if ( pFont )
		pfntOld = pDC->SelectObject( pFont );
	else pfntOld = (CFont*)pDC->SelectStockObject( SYSTEM_FONT );

	// �׷��� ũ��
	CSize szText = pDC->GetTextExtent( CString(sName) );

	if ( m_pMFSM && m_pMFSM->GetState()->state == msReady ) {

		CRect rc = ( rcHand | rcScore ) & rcScreen;

		ta = TA_CENTER|TA_BOTTOM;

		x = ( rc.left + rc.right ) /2;
		y = ( rc.top + rc.bottom ) /2;
		// ������ ����� ��츦 ����Ͽ� ��ġ�� �����Ѵ�
		x = max( rcScreen.left, x - szText.cx/2 ) + szText.cx/2;
		x = min( rcScreen.right, x + szText.cx/2 ) - szText.cx/2;
		y = max( rcScreen.top, y - szText.cy/2 ) + szText.cy/2;
		y = min( rcScreen.bottom, y + szText.cy/2 ) - szText.cy/2;
		if ( !bVert && rcHand.top > (rcScreen.top+rcScreen.bottom)/2 ) {
			// �Ʒ��ʿ� �ִ� ���
			y = rcHand.top - 3;
		}
	}
	else if ( bVert ) {		// ���η� �� ���

		if ( rcHand.left < (rcScreen.left+rcScreen.right)/2 ) {
			// �߾ӿ��� ���ʿ� �ִ� ���
			ta = TA_LEFT|TA_TOP;
			x = rcHand.right + nMargin;
			y = nPlayers == 7 ? rcScore.top + nMargin : rcScore.bottom + nMargin ;
		}
		else {
			// �߾ӿ��� �����ʿ� �ִ� ���
			ta = TA_RIGHT|TA_TOP;
			x = rcHand.left - nMargin;
			y = nPlayers == 7 ? rcScore.top + nMargin : rcScore.bottom + nMargin ;
		}
	}
	else {	// ���η� �� ���

		int xCenter = (rcScreen.left+rcScreen.right)/2;
		int bCenter = rcHand.left < xCenter && xCenter < rcHand.right;

		if ( bCenter ) {
			// �߾ӿ� �ִ� ���
			ta = TA_LEFT;
			x = rcHand.right + nMargin;
		}
		else if ( rcHand.left < xCenter ) {
			// �߾ӿ��� ���ʿ� �ִ� ���
			ta = TA_RIGHT;
			x = rcScore.left - nMargin;
			if ( x - szText.cx < nMargin ) {
				// ȭ���� �����
				ta = TA_LEFT;
				pDC->SetTextAlign( TA_LEFT );
				x = nMargin;
			}
		}
		else {
			// �߾ӿ��� �����ʿ� �ִ� ���
			ta = TA_LEFT;
			x = rcScore.right + nMargin;
			if ( x + szText.cx > rcScreen.right - nMargin ) {
				// ȭ���� �����
				ta = TA_RIGHT;
				pDC->SetTextAlign( TA_RIGHT );
				x = rcScreen.right - nMargin;
			}
		}

		if ( rcHand.top < (rcScreen.top+rcScreen.bottom)/2 ) {
			// ���ʿ� �ִ� ���
			ta |= TA_TOP;
			if ( bCenter )
				// ��� �ִ� ���
				y = rcScreen.top + nMargin;
			else // ���ʿ� �ִ� ���
				y = rcHand.bottom + nMargin;
		}
		else {
			// �Ʒ��ʿ� �ִ� ���
			ta |= TA_BOTTOM;
			if ( bCenter )
				// ��� �ִ� ���
				y = rcScreen.bottom - nMargin;
			else // ���ʿ� �ִ� ���
				y = rcHand.top - nMargin;
		}
	}
	pDC->SelectObject( pfntOld );

	CRect ret( x, y, x + szText.cx, y + szText.cy );

	if ( ( ta & TA_CENTER ) == TA_CENTER )
		ret.OffsetRect( +szText.cx/2, 0 );
	if ( ( ta & TA_RIGHT ) == TA_RIGHT )
		ret.OffsetRect( -szText.cx, 0 );
	if ( ( ta & TA_BOTTOM ) == TA_BOTTOM )
		ret.OffsetRect( 0, -szText.cy );

	return ret;
}


// ���� ȭ�� ũ�⿡ ���� ������ ī�� �� ũ�⸦ ��´�
CSize CBoard::GetCardZoom( bool bUseZoom ) const
{
	CSize szCard = GetBitmapSize( IDB_SA );

	int nCardWidth = szCard.cx;
	int nCardHeight = szCard.cy;

	if ( !bUseZoom ) // ī�� ���� ������� ������
		return szCard;		// �׻� ���� ī�� ũ�⸦ ����

	int nHZoomBy1000 = 1000;
	int nVZoomBy1000 = 1000;

	CRect rc; GetClientRect( &rc );
	int nMinProperWidth = nCardWidth*5/2 + 2*nCardHeight;
	int nMinProperHeight = nCardHeight*9/2;
	int nMaxProperWidth = nCardWidth*7/2 + 2*nCardHeight;
	int nMaxProperHeight = nCardHeight*11/2;

	if ( rc.Width() < 10 || rc.Height() < 10 )
		// �ʹ� ������ ���� ī�� ũ�⸦ ����
		return szCard;

	if ( rc.Width() < nMinProperWidth )
		nHZoomBy1000 = 1000*rc.Width()/nMinProperWidth;
	else if ( rc.Width() > nMaxProperWidth )
		nHZoomBy1000 = 1000*rc.Width()/nMaxProperWidth;
	if ( rc.Height() < nMinProperHeight )
		nVZoomBy1000 = 1000*rc.Height()/nMinProperHeight;
	else if ( rc.Height() > nMaxProperHeight )
		nVZoomBy1000 = 1000*rc.Height()/nMaxProperHeight;

	// ��� �ʿ伺�� �켱������ �˻�ȴ�
	if ( nHZoomBy1000 < 1000 || nVZoomBy1000 < 1000 ) {
		int nZoom = min( nHZoomBy1000, nVZoomBy1000 );
		return CSize( szCard.cx*nZoom/1000, szCard.cy*nZoom/1000 );
	}
	// �� �������� �� Ȯ�� �ص� ���� ��� �� �۰� Ȯ���ϴ� ��
	else if ( nHZoomBy1000 > 1000 && nVZoomBy1000 > 1000 ) {
		int nZoom = min( nHZoomBy1000, nVZoomBy1000 );
		// Ȯ�� �� ���� 0.5 ��(==500)�� ��������� ������
		nZoom = max( ( nZoom + 250 ) / 500 * 500, 1000 );
		return CSize( szCard.cx*nZoom/1000, szCard.cy*nZoom/1000 );
	}
	else return szCard;
}

// ���� ȭ�� ũ�⿡ ���� ������ ��Ʈ���� ���� �����
void CBoard::CreateFont()
{
	if ( m_fntSmall.GetSafeHandle() ) m_fntSmall.DeleteObject();
	if ( m_fntMiddle.GetSafeHandle() ) m_fntMiddle.DeleteObject();
	if ( m_fntBig.GetSafeHandle() ) m_fntBig.DeleteObject();

	// ��Ʈ ũ��� ���� �۲��� 14
	LONG lfSmallHeight =
		14 * GetCardZoom(true).cy / GetBitmapSize(IDB_SA).cy;

	LOGFONT lf;
	lf.lfWidth = 0;
	lf.lfEscapement = 0;
	lf.lfOrientation = 0;
	lf.lfWeight = 600;
	lf.lfItalic = 0;
	lf.lfUnderline = 0;
	lf.lfStrikeOut = 0;
	lf.lfCharSet = HANGUL_CHARSET;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = DEFAULT_QUALITY;
	lf.lfPitchAndFamily = DEFAULT_PITCH|FF_SWISS;
	_tcscpy( lf.lfFaceName, _T("����") );

	// ���� �۲�
	lf.lfHeight = -lfSmallHeight;
	m_fntSmall.CreateFontIndirect( &lf );
	// �߰� �۲�
	lf.lfHeight = -lfSmallHeight * 3/2;
	m_fntMiddle.CreateFontIndirect( &lf );
	// ū �۲�
	lf.lfHeight = -lfSmallHeight * 5/2;
	m_fntBig.CreateFontIndirect( &lf );

	// ����ũ�� �۲�
	if ( !m_fntFixedSmall.GetSafeHandle() ) {
		lf.lfHeight = -14;
		m_fntFixedSmall.CreateFontIndirect( &lf );
	}
	if ( !m_fntFixedMiddle.GetSafeHandle() ) {
		lf.lfHeight = -21;
		m_fntFixedMiddle.CreateFontIndirect( &lf );
	}
	if ( !m_fntFixedBig.GetSafeHandle() ) {
		lf.lfHeight = -35;
		m_fntFixedBig.CreateFontIndirect( &lf );
	}
}

// ȭ����� �� �簢�� ��ǥ�� ��´�
// nPlayers   : �÷��� �ϴ� �÷��̾��� �� ( 1 ~ 7 )
// type       : 0 ��� 1 �������� 2 �տ��� ī�� 3 ���°�
// nPlayer    : � �÷��̾ ���� �����ΰ� (nType!=0�϶��� ���)
// nCardWidth : ī���� ���� ũ��
// nCardHeight: ī���� ���� ũ��
// nIndex, nAll : �簢�� �ȿ� ��� �ִ� �� �������� ��ġ��
//                ��ü(nAll)���� ���°(nInex)�ΰ��� ���� ������
//                �����Ѵ�, nIndex == -1 �̸� ��ü �簢���� ���� 
// pbVertical : 0 �� �ƴϸ� �� �簢���� ���η� �� �簢�������� �����Ѵ�
// pbDirection: 0 �� �ƴϸ� ������ ���� ( true:������ �Ʒ�,���ʿ��� ������ false:�� �ݴ�)
CRect CBoard::CalcRect( int nPlayers, CR_TYPE type, int nPlayer, 
	int nCardWidth, int nCardHeight,
	int nIndex, int nAll, bool* pbVertical, bool* pbDirection ) const
{
#define GR_SETBOTTOM( RC, Y )	( (RC).top = (Y) - (RC).Height(), (RC).bottom = (Y) )
#define GR_SETTOP( RC, Y )		( (RC).bottom = (Y) + (RC).Height(), (RC).top = (Y) )
#define GR_SETVCENTER( RC, Y )	( (RC).SetRect( (RC).left, (Y)-(RC).Height()/2, (RC).right, (Y)-(RC).Height()/2+(RC).Height() ) )
#define GR_SETLEFT( RC, X )		( (RC).right = (X) + (RC).Width(), (RC).left = (X) )
#define GR_SETRIGHT( RC, X )	( (RC).left = (X) - (RC).Width(), (RC).right = (X) )
#define GR_SETHCENTER( RC, X )	( (RC).SetRect( (X)-(RC).Width()/2, (RC).top, (X)-(RC).Width()/2+(RC).Width(), (RC).bottom ) )
#define GR_CENTEREDRECT( RC, X, Y, W, H ) RC( (X)-(W)/2, (Y)-(H)/2, (X)-(W)/2+(W), (Y)-(H)/2+(H) )

	// ���⼭ ����ϴ� �÷��̾� ��ȣ�� ���� ��ǥ��
	// ������ �Ʒ��� ���� �ð�������� 0 ~ 6(v4.0) �� ��ȣ�� ���´�
	nPlayer = MapLogicToAbs(nPlayer);

	// �÷��̾� ���� �÷��̾� ��ġ ����
	// aanLocInfo[�ο�][�÷��̾��ȣ] �� ����, ���� ������ �����Ѵ�
	// 31 �� AND �Ͽ� ���� ������ ���� �� �ִ�
	// 1(left) 2(left-center) 4(center) 8(right-center) 16(right)
	// 32(top) 64( top-vcenter) 128(vcenter)
	//                  256(bottom-vcenter) 512(bottom)
	static const int aanLocInfo[MAX_PLAYERS+1][MAX_PLAYERS] = {
			{ 0, 0, 0, 0, 0, 0, 0 },							// 0 ��
			{ 4+512, 0, 0, 0, 0, 0, 0 },						// 1 ��
			{ 4+512, 4+32, 0, 0, 0, 0, 0 },						// 2 ��
			{ 4+512, 1+64, 16+64, 0, 0, 0, 0 },					// 3 ��
			{ 4+512, 1+128, 4+32, 16+128, 0, 0, 0 },			// 4 ��
			{ 4+512, 1+256, 2+32, 8+32, 16+256, 0, 0  },		// 5 ��
			{ 8+512, 2+512, 1+128, 2+32, 8+32, 16+128, 0 },		// 6 ��
			{ 4+512, 1+512, 1+64, 2+32, 8+32, 16+64, 16+512 },	// 7 ��
	};

	if( !Mo()->bClockwise ) {
		nPlayer = nPlayer == 0 ? 0 : nPlayers - nPlayer;
	}

	CRect rc; GetClientRect(&rc);
	int xCenter = rc.left + rc.Width()/2;
	int yCenter = rc.top + rc.Height()/2;

	if ( type == CR_CENTER ) {	// ���

		int hRc = nCardHeight * 2 + 8;
		int wRc = nCardWidth * 3 - nCardWidth/5;
		// �߽��� ��� �ְ� ũ�Ⱑ wRc, hRc ���� �簢��
		CRect GR_CENTEREDRECT( rcCenter, xCenter, yCenter, wRc, hRc );

		// ���� ������ �ʿ� ������ �� �簢���� �����Ѵ�
		if ( nIndex == -1 ) return rcCenter;

		// ���� ������ ������ �簢��
		CRect rcRet( -nCardWidth, -nCardHeight, 0, 0);

		// ��ġ ���� ( nPlayer �� �ƴ϶� nIndex �� ��� ��ġ�� ���� ������)
		int loc;
		if( Mo()->bClockwise )
			loc = aanLocInfo[nPlayers][nIndex];
		else
			loc = aanLocInfo[nPlayers][nIndex==0?0:nPlayers-nIndex];

			 if ( loc & 1 ) GR_SETLEFT( rcRet, rcCenter.left + 2 );
		else if ( loc & 2 ) GR_SETLEFT( rcRet, rcCenter.left + ( xCenter - rcCenter.left )/3 );
		else if ( loc & 4 ) GR_SETHCENTER( rcRet, xCenter );
		else if ( loc & 8 ) GR_SETRIGHT( rcRet, rcCenter.right - ( rcCenter.right - xCenter )/3 );
		else if ( loc & 16 ) GR_SETRIGHT( rcRet, rcCenter.right - 2 );
			 if ( loc & 32 ) GR_SETTOP( rcRet, rcCenter.top + 2 );
		else if ( loc & 64 ) GR_SETTOP( rcRet, rcCenter.top + 2 );
		else if ( loc & 128 ) GR_SETVCENTER( rcRet, yCenter );
		else if ( loc & 256 ) GR_SETVCENTER( rcRet, yCenter + nCardHeight/7 );
		else if ( loc & 512 ) GR_SETBOTTOM( rcRet, rcCenter.bottom - 2 );

		return rcRet;
	}
	else if ( type == CR_SCORE
		|| type == CR_HAND ) {	// ������ , �տ� �� ī��

		int loc = aanLocInfo[nPlayers][nPlayer];

		CRect rcHand;
		bool bVert = loc & 1+16 ? true : false;	// �� ������ ���η� �� �簢���̴�
		if ( pbVertical ) *pbVertical = bVert;

		bool bDir = ( loc & 1 ) ? true	// ������ �Ʒ����� ��
			: ( loc & 16 ) ? false		// �������� ������ �Ʒ�
			: ( loc & 512 ) ? true		// �Ʒ����� ���ʿ��� ������
			: false;					// �� �ܴ� �����ʿ��� ����
		if ( pbDirection ) *pbDirection = bDir;

		if ( type == CR_SCORE ) {
			if ( bVert ) {
					if ( rc.Height() < 5*nCardHeight+nCardHeight/2 )
						rcHand.SetRect( -nCardHeight, -( nCardHeight + nCardWidth ), 0, 0 );
					else rcHand.SetRect( -nCardHeight, -( rc.Height()/2 - nCardHeight*7/4 + nCardWidth), 0, 0 );

				if ( rc.Width() < 4*nCardWidth+2*nCardHeight ) {
						 if ( loc & 1 ) GR_SETRIGHT( rcHand, xCenter - nCardHeight - nCardWidth/2 );
					else if ( loc & 16 ) GR_SETLEFT( rcHand, xCenter + nCardHeight + nCardWidth/2 );
				}
				else {
						 if ( loc & 1 ) GR_SETLEFT( rcHand, nCardWidth/2 );
					else if ( loc & 16 ) GR_SETRIGHT( rcHand, rc.right - nCardWidth/2 );
				}
					 if ( loc & 64 ) GR_SETVCENTER( rcHand, yCenter - rcHand.Height()/7 );
				else if ( loc & 128 ) GR_SETVCENTER( rcHand, yCenter );
				else if ( loc & 256 ) GR_SETVCENTER( rcHand, yCenter + rcHand.Height()/7 );
				else if ( loc & 512 ) GR_SETBOTTOM( rcHand, rc.bottom );
			}
			else {
				if ( rc.Width() < 4*nCardWidth+2*nCardHeight )
					rcHand.SetRect( -nCardWidth*7/4, -nCardHeight, 0, 0 );
				else rcHand.SetRect(
					-( (rc.Width()-4*nCardWidth-2*nCardHeight)/3 + nCardWidth*7/4 ), -nCardHeight, 0, 0 );

					 if ( loc & 2 ) GR_SETRIGHT( rcHand, xCenter - ( xCenter - rc.left ) / 6 );
				else if ( loc & 4 ) GR_SETHCENTER( rcHand, xCenter );
				else if ( loc & 8 ) GR_SETLEFT( rcHand, xCenter + ( rc.right - xCenter ) /6 );

				if ( rc.Height() < 5*nCardHeight + nCardHeight/2) {
						 if ( loc & 32 ) GR_SETBOTTOM( rcHand, yCenter - nCardHeight - nCardHeight/4 );
					else if ( loc & 512 ) GR_SETTOP( rcHand, yCenter + nCardHeight + nCardHeight/4 );
				}
				else {
						 if ( loc & 32 ) GR_SETTOP( rcHand, nCardHeight/2 );
					else if ( loc & 512 ) GR_SETBOTTOM( rcHand, rc.bottom - nCardHeight/2 );
				}
			}
		}
		else {

			if ( bVert ) {
				if ( nPlayers == 7 ) {	// 7������ ���� ������ �÷��̾�� nCardWidth��ŭ ũ�⸦ ���δ� (v4.0 : 2010.4.11)
					if ( rc.Height() < 5*nCardHeight+nCardHeight/2 )
						rcHand.SetRect( -nCardHeight, -nCardHeight*2 , 0, 0 );
					else rcHand.SetRect( -nCardHeight, -( rc.Height()/2 - nCardHeight*3/4 ), 0, 0 );
				}
				else {					// ���� ������ �÷��̾���� ���η� ��� ǥ�� (v3.21)
					if ( rc.Height() < 5*nCardHeight + nCardHeight/2 )
						rcHand.SetRect( -nCardHeight, -( nCardHeight*2 + nCardWidth ), 0, 0 );
					else rcHand.SetRect( -nCardHeight, -( rc.Height()/2 - nCardHeight*3/4 + nCardWidth ), 0, 0 );
				}

				if ( rc.Width() < 4*nCardWidth+2*nCardHeight ) {
						 if ( loc & 1 ) GR_SETRIGHT( rcHand, xCenter - nCardHeight - nCardWidth );
					else if ( loc & 16 ) GR_SETLEFT( rcHand, xCenter + nCardHeight + nCardWidth );
				}
				else {
						 if ( loc & 1 ) GR_SETLEFT( rcHand, 0 );
					else if ( loc & 16 ) GR_SETRIGHT( rcHand, rc.right );
				}
					 if ( loc & 64 ) GR_SETVCENTER( rcHand, yCenter - rcHand.Height()/7 );
				else if ( loc & 128 ) GR_SETVCENTER( rcHand, yCenter );
				else if ( loc & 256 ) GR_SETVCENTER( rcHand, yCenter + rcHand.Height()/7 );
				else if ( loc & 512 ) GR_SETVCENTER( rcHand, rc.bottom - nCardHeight );
			}
			else {
				if ( ( nPlayers == 5 && nPlayer != 0 ) ||	// 5������ �� �÷��̾�
					nPlayers == 6 ||						// 6������ ��, �Ʒ� �÷��̾�
					( nPlayers == 7 && nPlayer != 0 ) ) {	// 7������ �� �÷��̾�� nCardWidth/4��ŭ ũ�⸦ ���δ� (v4.0 : 2010.4.9-11)
					if ( rc.Width() < 4*nCardWidth+2*nCardHeight )
						rcHand.SetRect( -nCardWidth*13/4, -nCardHeight, 0, 0 );
					else rcHand.SetRect(
						-( (rc.Width()-4*nCardWidth-2*nCardHeight)/2 + nCardWidth*13/4 ), -nCardHeight, 0, 0 );
				}
				else {
					if ( rc.Width() < 4*nCardWidth+2*nCardHeight )
						rcHand.SetRect( -nCardWidth*3-nCardWidth/2, -nCardHeight, 0, 0 );
					else rcHand.SetRect(
						-( (rc.Width()-4*nCardWidth-2*nCardHeight)/2 + nCardWidth*3 + nCardWidth/2 ), -nCardHeight, 0, 0 );
				}

					 if ( loc & 2 ) GR_SETRIGHT( rcHand, xCenter - ( xCenter - rc.left )/40 );
				else if ( loc & 4 ) GR_SETHCENTER( rcHand, xCenter );
				else if ( loc & 8 ) GR_SETLEFT( rcHand, xCenter + ( rc.right - xCenter )/40 );
				if ( rc.Height() < 5*nCardHeight + nCardHeight/2 ) {
						 if ( loc & 32 ) GR_SETBOTTOM( rcHand, yCenter - nCardHeight*7/4 );
					else if ( loc & 512 ) GR_SETTOP( rcHand, yCenter + nCardHeight*7/4 );
				}
				else {
						 if ( loc & 32 ) GR_SETTOP( rcHand, 0 );
					else if ( loc & 512 ) GR_SETBOTTOM( rcHand, rc.bottom );
				}
			}
		}

		if ( nIndex == -1 ) return rcHand;

		// ���� ���� ������ ����
		if ( bVert ) {
			int nMargin = rcHand.Height()-nCardWidth*nAll;
			if ( nMargin >= 0 )
				// ��� ī�尡 �� ���� - ����� ������
				return CRect(
					CPoint( rcHand.left,
						bDir ? rcHand.top + nMargin/2 + nCardWidth*nIndex
						: rcHand.bottom - nMargin/2 - nCardWidth*(nIndex+1) ),
					CSize( nCardHeight, nCardWidth ) );
			else // �� ������ ��������
				return CRect(
					CPoint( rcHand.left, rcHand.top
						+ (rcHand.Height()-nCardWidth)
							* (bDir?nIndex:(nAll-nIndex-1)) / (nAll-1) ),
					CSize( nCardHeight, nCardWidth ) );
		}
		else {
			int nMargin = rcHand.Width()-nCardWidth*nAll;
			if ( nMargin >= 0 )
				// ��� ī�尡 �� ���� - ����� ������
				return CRect(
					CPoint(
						bDir ? rcHand.left + nMargin/2 + nCardWidth*nIndex
						: rcHand.right - nMargin/2 - nCardWidth*(nIndex+1),
						rcHand.top ),
					CSize( nCardWidth, nCardHeight ) );
			else // �� ���ʺ��� ��������
				return CRect(
					CPoint( rcHand.left
						+ (rcHand.Width()-nCardWidth)
							* (bDir?nIndex:(nAll-nIndex-1)) / (nAll-1),
						rcHand.top ),
					CSize( nCardWidth, nCardHeight ) );
		}
	}
	else if ( type == CR_HIDDEN ) {	// ���� ��

		int loc = aanLocInfo[nPlayers][nPlayer];

		CRect rcRet( -nCardWidth, -nCardHeight, 0, 0);

			 if ( loc & 1 ) GR_SETRIGHT( rcRet, -5 );
		else if ( loc & 2 ) GR_SETRIGHT( rcRet, xCenter - (xCenter - rc.left)*2/5 );
		else if ( loc & 4 ) GR_SETHCENTER( rcRet, xCenter );
		else if ( loc & 8 ) GR_SETLEFT( rcRet, xCenter + (rc.right - xCenter)*2/5 );
		else if ( loc & 16 ) GR_SETLEFT( rcRet, rc.right + 5 );
			 if ( loc & 32 ) GR_SETBOTTOM( rcRet, -5 );
		else if ( loc & 64 ) GR_SETVCENTER( rcRet, yCenter - rc.Height()/10 );
		else if ( loc & 128 ) GR_SETVCENTER( rcRet, yCenter );
		else if ( loc & 256 ) GR_SETVCENTER( rcRet, yCenter + rc.Height()/10 );
		else if ( loc & 512 ) GR_SETTOP( rcRet, rc.bottom + 5 );

		return rcRet;
	}
	ASSERT(0);
	return CRect(0,0,0,0);
}

// ����� �� ī�� �߿��� ������ �ε����� �ִ� ī����
// ���� �簢�� ��ġ�� ���� ( ����Ʈ �� ī�� ��ġ���� ��� )
bool CBoard::CalcRealCardRect( LPRECT prc, int index ) const
{
	const CState* pState = m_pMFSM ? m_pMFSM->GetState() : 0;
	if ( !pState ) return false;
	const CPlayer* pPlayer = pState->apPlayers[0];
	const CCardList* pHand = pPlayer->GetHand();
	if ( !pPlayer->IsHuman() ) return false;

	int nCards = pHand->GetCount();

	CRect rc = CalcRect( pState->nPlayers, CR_HAND, 0,
		m_szCard.cx, m_szCard.cy, index, nCards );

	// ���� ������ or 2�� �� ��Ȳ�̸�(v4.0: 2010.4.13)
	if ( ( pState->state == msPrivilege
			|| ( pState->state == msDeal2MA && pState->nPlayers == 2 ) )
			&& GetSelection(index) )
		rc.OffsetRect( 0, -m_szCard.cy/CARD_SHIFT_RATIO_OF_SELECTION );

	*prc = rc;
	return true;
}

// ����� �� ī�� �߿��� ������ ����Ʈ�� �ִ�
// ī���� POSITION �� ����
POSITION CBoard::CardFromPoint( int x, int y ) const
{
	if ( !m_pMFSM ) return 0;
	const CState* pState = m_pMFSM->GetState();
	if ( !pState->apPlayers[0]->IsHuman() ) return 0;

	CRect rc = CalcRect( pState->nPlayers, CR_HAND, 0,
		m_szCard.cx, m_szCard.cy );
	rc.top -= m_szCard.cy / CARD_SHIFT_RATIO_OF_SELECTION;
	if ( !rc.PtInRect(CPoint(x,y)) ) return 0;

	const CCardList* pHand = pState->apPlayers[0]->GetHand();

	int nCards = pHand->GetCount();
	for ( int i = nCards-1; i >= 0; i-- ) {

		// ���� �����ʿ� �ִ� ī����� �˻��Ѵ�
		if ( CalcRealCardRect( &rc, i )
				&& rc.PtInRect( CPoint(x,y) ) )
			// ã�Ҵ�
			return pHand->POSITIONFromIndex(i);
	}
	return 0;
}

// FlyCard �����Լ�- �־��� ������ ���� �簢���̳� ���� �簢���� ����Ѵ�
// ���� ���ڴ� FlyCard �� ����
RECT CBoard::CalcCardRectForFlyCard( int nType, int nPlayer, int nIndex )
{
	static const CR_TYPE aTable[] = {
		CR_CENTER, CR_SCORE, CR_HAND, CR_HIDDEN };

	ASSERT( m_pMFSM );
	if ( !m_pMFSM ) return CRect( 0, 0, 0, 0 );

	const CState* pState = m_pMFSM->GetState();

	if ( nType == 4 ) {
		// ���� ����� !!
		// �� ���� DrawCenterAnimation �Լ����� �� �����Ƿ� ����
		int nCount, nCardCount = pState->lDeck.GetCount();
		if ( nCardCount < 3 ) nCount = nCardCount;
		else if ( nCardCount < 10 ) nCount = nCardCount*2/3;
		else nCount = m_szCard.cx * (nCardCount-9) / 520 + 6;
		int nMaxCount = m_szCard.cx * (53-9) / 520 + 6;

		CRect rc; GetClientRect( &rc );
		int y = rc.top + rc.Height()/2 - m_szCard.cy/2
			+ nMaxCount*2 - (nCount-1)*2;
		int x = rc.left + rc.Width()/2 - m_szCard.cx/2;
		return CRect( CPoint(x,y), m_szCard );
	}

	// �� ������ ����
	int nAll = 0;
	if ( nType == CR_CENTER )
		nAll = pState->nPlayers;
	else if ( nType == CR_SCORE )
		nAll = pState->apPlayers[nPlayer]->GetScore()->GetCount(),
		nAll = max( nAll-1, 0 );
	else if ( nType == CR_HAND )
		nAll = pState->apPlayers[nPlayer]->GetHand()->GetCount()+1;

	// nAll �� ������ 1 �� ����Ǵ� ���� Score, Hand �� �ε�����
	// �����ϴ� ���� �̹� �տ��� ī��� �������� �����̸�
	// ������ ī��� ���� ���´� - �̸� �������� �ʱ� ����
	// �̿� ���� �Ͽ���

	CRect rc = CalcRect(
		pState->nPlayers, aTable[nType], nPlayer,
		m_szCard.cx, m_szCard.cy, nIndex, nAll );
	if ( nIndex < 0 ) { // �� �����
		rc.left = (rc.left+rc.right)/2 - m_szCard.cx/2;
		rc.right = rc.left + m_szCard.cx;
		rc.top = (rc.top+rc.bottom)/2 - m_szCard.cy/2;
		rc.bottom = rc.top + m_szCard.cy;
	}
	else if ( nType == CR_HAND && nPlayer == 0 )
		rc.OffsetRect( 0, - m_szCard.cy/CARD_SHIFT_RATIO_OF_MOUSE_OVER );

	return rc;
}
