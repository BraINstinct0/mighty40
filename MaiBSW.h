// MaiBSW.h: interface for the CMaiBSW class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAIBSW_H__FC5D5695_E223_11D3_A879_0000212035B8__INCLUDED_)
#define AFX_MAIBSW_H__FC5D5695_E223_11D3_A879_0000212035B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


/////////////////////////////////////////////////////////////////////////////
// �⺻ �˰��� (BlackSheepWall)
struct CMaiBSW : public CPlay
{
	// DLL �������̼� �Լ���
	static void MaiGetInfo( MAIDLL_INFO* pInfo );
	static CPlay* MaiGetPlay( LPCTSTR sOption, MAIDLL_UPDATE* pUpdate );
	static void MaiOption( CPlay* pPlay, HWND hWnd );
	static void MaiReleasePlay( CPlay* pPlay, CString* psOption );
};

#endif // !defined(AFX_MAIBSW_H__FC5D5695_E223_11D3_A879_0000212035B8__INCLUDED_)
