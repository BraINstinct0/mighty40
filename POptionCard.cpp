// POptionCard.cpp : implementation file
//

#include "stdafx.h"
#include "Mighty.h"
#include "POptionCard.h"
#include "DBackSelect.h"

#include "InfoBar.h"
#include "BmpMan.h"
#include "Board.h"
#include "BoardWrap.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// POptionCard property page

IMPLEMENT_DYNCREATE(POptionCard, CPropertyPage)

POptionCard::POptionCard() : CPropertyPage(POptionCard::IDD)
{
	m_pBm = Mf()->GetView()->GetWrapper()->GetBmpMan();
	ASSERT( m_pBm );

	//{{AFX_DATA_INIT(POptionCard)
	m_bCardTip = Mo()->bCardTip;
	m_bCardHelp = Mo()->bCardHelp;
	m_bZoomCard = Mo()->bZoomCard;
	//}}AFX_DATA_INIT

	m_nBackPicture = Mo()->nBackPicture;
	m_sBackPicturePath = Mo()->sBackPicturePath;
}

POptionCard::~POptionCard()
{
}

void POptionCard::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(POptionCard)
	DDX_Control(pDX, IDC_SAMPLEBACK, m_cSampleBack);
	DDX_Check(pDX, IDC_CARDTIP, m_bCardTip);
	DDX_Check(pDX, IDC_CARDHELP, m_bCardHelp);
	DDX_Check(pDX, IDC_ZOOMCARD, m_bZoomCard);
	//}}AFX_DATA_MAP

	if ( pDX->m_bSaveAndValidate ) {
		Mo()->bCardTip = !!m_bCardTip;
		Mo()->bCardHelp = !!m_bCardHelp;
		Mo()->bZoomCard = !!m_bZoomCard;
		Mo()->nBackPicture = m_nBackPicture;
		Mo()->sBackPicturePath = m_sBackPicturePath;
	}
}


BEGIN_MESSAGE_MAP(POptionCard, CPropertyPage)
	//{{AFX_MSG_MAP(POptionCard)
	ON_BN_CLICKED(IDC_BACKSELECT, OnBackselect)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// POptionCard message handlers

void POptionCard::DrawSample( CDC* pDC )
{
	if ( !m_pBm ) return;

	// �����ְ� �ִ� ī�� �޸� �׸� �ٲٱ�
	CRect rc;
	GetDlgItem(IDC_SAMPLEBACK)->GetWindowRect( &rc );
	ScreenToClient( &rc );

	CSize sz = ::GetBitmapSize( IDB_SA );
	CPoint pt( ( rc.left + rc.right ) / 2, ( rc.top + rc.bottom ) / 2 );
	rc.left = pt.x - sz.cx/2;
	rc.right = rc.left + sz.cx;
	rc.top = pt.y - sz.cy/2;
	rc.bottom = rc.top + sz.cy;
	m_pBm->DrawCard( pDC, 0, rc );
}

void POptionCard::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	DrawSample( &dc );

	// Quote �� �׸���
	dc.SetTextAlign( TA_LEFT | TA_TOP );
	dc.SetBkMode( TRANSPARENT );
	dc.SetTextColor( GetSysColor( COLOR_BTNTEXT ) );
	dc.SelectStockObject( DEFAULT_GUI_FONT );

	// ���� ��Ʈ
	LOGFONT lf;
	dc.GetCurrentFont()->GetLogFont( &lf );
	lf.lfWeight = FW_BOLD;
	CFont font;
	font.CreateFontIndirect( &lf );

	// �簢��
	CWnd* pWnd = GetDlgItem( IDC_QUOTE );
	CRect rc;
	pWnd->GetWindowRect( &rc );
	ScreenToClient( &rc );
	int left = rc.left, x = rc.left, y = rc.top;

	// ��Ʈ��
	CString s;

	// ��ũ��, �ڳװ� ���� �� Ƽ��Ʈ����
	// ��밡 �� �� ��������� �����ϳ� ?
	//       ���ι̿��� �ٸ��� 3�� 1�塻

	s = _T("��ũ��, �ڳװ� ��");
	dc.TextOut( x, y, s );
	CSize sz = dc.GetTextExtent( s );
	x += sz.cx;

	CFont* pfntOld = dc.SelectObject( &font );
	s = _T("�� �� Ƽ");
	dc.TextOut( x, y, s );
	x += dc.GetTextExtent( s ).cx;
	dc.SelectObject( pfntOld );

	s = _T("��Ʈ����");
	dc.TextOut( x, y, s );
	x = left; y += sz.cy + abs(lf.lfHeight)/4;

	s = _T("��밡 �� �� ��������� �����ϳ� ?");
	dc.TextOut( x, y, s );
	y += sz.cy + abs(lf.lfHeight)/4;

	s = _T("       ���ι̿��� �ٸ��� 3�� 1�塻");
	dc.TextOut( x, y, s );
}

void POptionCard::OnBackselect() 
{
	// TODO: Add your control notification handler code here
	ASSERT( m_pBm );
	if ( !m_pBm ) return;

	DBackSelect dlg;
	if ( dlg.DoModal() == IDOK ) {

		CClientDC dc(*Mf()->GetView()->GetWrapper());

		if ( !m_pBm->LoadBackPicture( &dc,
			dlg.m_nBackPicture, dlg.m_sBackPicturePath ) ) {

			CString sMsg;
			if ( dlg.m_sBackPicturePath.IsEmpty() )
				sMsg.Format( _T("���õ� �׸� ������ �����ϴ�") );
			else
				sMsg.Format( _T("�׸� ���� %s ��(��) ���� �� �����ϴ�"),
					dlg.m_sBackPicturePath );
			AfxMessageBox( sMsg, MB_OK|MB_ICONEXCLAMATION );
		}
		else {
			m_nBackPicture = dlg.m_nBackPicture;
			m_sBackPicturePath = dlg.m_sBackPicturePath;
			Invalidate();
		}
	}
}

void POptionCard::OnCancel() 
{
	// TODO: Add your specialized code here and/or call the base class

	// ����ϸ� �ٽ� �޸� �׸��� �������
	CClientDC dc(*Mf()->GetView()->GetWrapper());
	VERIFY( m_pBm->LoadBackPicture( &dc,
		Mo()->nBackPicture, Mo()->sBackPicturePath ) );

	CPropertyPage::OnCancel();
}
