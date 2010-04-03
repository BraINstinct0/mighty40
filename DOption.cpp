// DOption.cpp : implementation file
//

#include "stdafx.h"
#include "Mighty.h"
#include "MightyToolTip.h"
#include "DOption.h"

#include "POptionGeneral.h"
#include "POptionBosskey.h"
#include "POptionCard.h"
#include "POptionBackground.h"
#include "POptionDSB.h"
#include "POptionSpeed.h"
#include "POptionRule.h"
#include "POptionAI.h"
#include "POptionComm.h"
#include "POptionChat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DOption

IMPLEMENT_DYNAMIC(DOption, CPropertySheet)

DOption::DOption(CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(_T("����Ƽ �ɼ�"), pParentWnd, iSelectPage)
{
	m_nPages = 10;
	m_apPages = new CPropertyPage*[m_nPages];

	m_apPages[0] = new POptionGeneral;
	m_apPages[1] = new POptionBosskey;
	m_apPages[2] = new POptionCard;
	m_apPages[3] = new POptionBackground;
	m_apPages[4] = new POptionDSB;
	m_apPages[5] = new POptionSpeed;
	m_apPages[6] = new POptionRule;
	m_apPages[7] = new POptionAI;
	m_apPages[8] = new POptionComm;
	m_apPages[9] = new POptionChat;

	for ( int i = 0; i < m_nPages; i++ )
		AddPage( m_apPages[i] );

	m_psh.dwFlags |= PSH_NOAPPLYNOW;
	m_psh.dwFlags &= ~PSH_HASHELP;
}

DOption::~DOption()
{
	for ( int i = 0; i < m_nPages; i++ )
		delete m_apPages[i];
	delete[] m_apPages;
}


BEGIN_MESSAGE_MAP(DOption, CPropertySheet)
	//{{AFX_MSG_MAP(DOption)
	ON_WM_CREATE()
	ON_WM_HELPINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

int DOption::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
//  ����: ���� ��ũ�� �ϰ� �ߴ��� ������ �����ߴ� !
//	EnableStackedTabs( FALSE );

	if (CPropertySheet::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	ModifyStyleEx( 0, WS_EX_CONTEXTHELP );

	return 0;
}

BOOL DOption::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	// TODO: Add your message handler code here and/or call default

	// �� ���� ǥ��
	CString sTip;
	if ( !sTip.LoadString( pHelpInfo->iCtrlId ) )
		sTip = _T("�� �����ۿ� ���� ������ �����ϴ�\n")
				_T("��ó�� �ִ� �ٸ� �������� Ŭ���� ���ʽÿ�");

	Tt()->Tip(
		pHelpInfo->MousePos.x+TOOLTIP_BORDER_MARGIN*2,
		pHelpInfo->MousePos.y+TOOLTIP_BORDER_MARGIN*2,
		sTip );

//	return CPropertySheet::OnHelpInfo(pHelpInfo);
	return TRUE;
}
