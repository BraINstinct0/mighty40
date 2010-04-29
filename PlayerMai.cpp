// PlayerMai.cpp: implementation of the CPlayerMai class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mighty.h"

#include "Play.h"
#include "Player.h"
#include "PlayerMai.h"
#include "MaiBSW.h"
#include "InfoBar.h"
#include "MFSM.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// update �� ���� CInfoBar ĸ��
void CPlayerMai::MAIDLL_UPDATE_IMPL::SetProgress( int nPercentage )
{
	m_pInfo->SetProgress( nPercentage );
}

void CPlayerMai::MAIDLL_UPDATE_IMPL::SetText( LPCTSTR sText )
{
	m_pInfo->SetText( sText );
}


/////////////////////////////////////////////////////////////////////////////
// CPlayerMai

// ���� ������

// dll
CList< CPlayerMai::MAIDLL, CPlayerMai::MAIDLL& > CPlayerMai::s_lMaiDll;
CCriticalSection CPlayerMai::s_csMaiDll;

/////////////////////////////////////////////////////////////////////////////
// CPlayerMai ��ü

CPlayerMai::CPlayerMai(
	int nID,		// ���� ��ȣ ( 6���� �÷��̾��� ���� )
	LPCTSTR sName,	// �̸�
	CWnd* pCallback,// call-back window
	LPCTSTR sDLLPath,// MAI-DLL ���
	LPCTSTR sOption) // �ɼ� ��Ʈ�� (null �� ��� �ʱⰪ)
	: CPlayer( nID, sName, pCallback )
{
	m_pos = 0;

	// DLL �� �ε��ϰ� ���ε� �Ѵ�
	// �н��� �����̰ų� �ش� DLL �ε尡 ����������
	// �⺻ AI �� �ε��Ѵ�
	if ( !sDLLPath || !*sDLLPath
			|| !LoadMaiDll( sDLLPath, sOption, &m_update ) )
		LoadDefault( sOption, &m_update );
}

CPlayerMai::~CPlayerMai()
{
	VERIFY( UnloadMaiDll() );
}

// DLL ���� �Լ� �����͸� ��� (m_funcs),
// CPlay ��ü�� ��� ��
// ���� s_lMaiDll �� �̹� ���� DLL �� �ִ� ��� ref ���� ������
bool CPlayerMai::LoadMaiDll( LPCTSTR sPath, LPCTSTR sOption, MAIDLL_UPDATE* pUpdate )
{
	s_csMaiDll.Lock();

	// s_lMaiDll �� ���� �̸��� DLL �� �ִ°� �����Ѵ�
	POSITION pos = s_lMaiDll.GetHeadPosition();
	for ( ; pos; s_lMaiDll.GetNext(pos) ) {
		MAIDLL& dll = s_lMaiDll.GetAt(pos);

		if ( !dll.sPath.CompareNoCase( sPath ) ) {
			// �߰� !
			m_pos = pos;
			m_funcs = dll.funcs;
			m_pPlay = m_funcs.MaiGetPlay( sOption, pUpdate );
			dll.ref++;
			s_csMaiDll.Unlock();
			return true;
		}
	}
	ASSERT( !pos );
	s_csMaiDll.Unlock();

	// �̹߰� - ������ �ε��ؼ� ��´�

	HMODULE hDLL = ::LoadLibrary( sPath );
	if ( !hDLL ) return false;

	m_funcs.MaiGetInfo = (pfMaiGetInfo)::GetProcAddress( hDLL, "MaiGetInfo" );
	m_funcs.MaiGetPlay = (pfMaiGetPlay)::GetProcAddress( hDLL, "MaiGetPlay" );
	m_funcs.MaiOption = (pfMaiOption)::GetProcAddress( hDLL, "MaiGetOption" );
	m_funcs.MaiReleasePlay = (pfMaiReleasePlay)::GetProcAddress( hDLL, "MaiReleasePlay" );

	if ( !m_funcs.MaiGetInfo
		|| !m_funcs.MaiGetPlay
		|| !m_funcs.MaiReleasePlay ) return false;

	// ���� ������ ����ϰ� CPlay �� ��´�

	MAIDLL m;
	m.hDLL = hDLL;
	m.funcs = m_funcs;
	m.ref = 1;
	m.sPath = sPath;
	m_pPlay = m.funcs.MaiGetPlay( sOption, pUpdate );

	s_csMaiDll.Lock();
	m_pos = s_lMaiDll.AddTail( m );
	s_csMaiDll.Unlock();

	return true;
}

// CPlay ��ü�� ������ - ���� �ش� DLL �� ref ��
// 0 �� �Ǵ� ��� �� DLL �� Unload ��
bool CPlayerMai::UnloadMaiDll()
{
	ASSERT( m_pPlay );

	// CPlay ��ü�� �����ش�
	CString sOption;
	m_funcs.MaiReleasePlay( m_pPlay, &sOption );
	m_pPlay = 0;

	// �������� �ɼ� ��Ʈ���� �����Ѵ�
//	Mo()->aPlayer[GetID()].sSetting = sOption;
	// ���� : �ɼ� ��Ʈ���� ������ �ʿ䰡 ����
	//        �÷��̾� �ɼ� ��ȭ���ڿ��� �����Ѵ�

	if ( m_pos ) {

		s_csMaiDll.Lock();
		MAIDLL& dll = s_lMaiDll.GetAt( m_pos );

		if ( ! --dll.ref ) {
			// ���� ī���Ͱ� 0 �� �Ǿ���
			if ( !::FreeLibrary( dll.hDLL ) ) {
				ASSERT(0);
				s_csMaiDll.Unlock();
				return false;
			}
			s_lMaiDll.RemoveAt( m_pos );
			m_pos = 0;
		}
		s_csMaiDll.Unlock();
	}
	return true;
}

// ����Ʈ AI �� �ε��� ( DLL �б� ���н� )
void CPlayerMai::LoadDefault( LPCTSTR sOption, MAIDLL_UPDATE* pUpdate )
{
	m_funcs.MaiGetInfo = CMaiBSW::MaiGetInfo;
	m_funcs.MaiGetPlay = CMaiBSW::MaiGetPlay;
	m_funcs.MaiOption = CMaiBSW::MaiOption;
	m_funcs.MaiReleasePlay = CMaiBSW::MaiReleasePlay;

	m_pos = 0;
	m_pPlay = m_funcs.MaiGetPlay( sOption, pUpdate );
}

// CPlayer ����

void CPlayerMai::OnSelect2MA( int* selecting, CCardList* plCard, CEvent* e )
{
	CSandClock c(DELAY_KILL);

	// CPlayer::OnSelect2MA( pcShow, pcHide, e );
	// Ib()->SetProgress( 0 );	// ���¹��� ����� ǥ�ø� ����

	m_pMFSM->RequestCallAIThread( m_pPlay, 3, &m_eTemp, selecting, plCard );

	m_pMFSM->WaitEvent( &m_eTemp );

	Ib()->SetProgress( 0 );
	e->SetEvent();
}

void CPlayerMai::OnKillOneFromSix( CCard* pcCardToKill,
	CCardList* plcFailedCardsTillNow, CEvent* e )
{
	CSandClock c(DELAY_KILL);

	// CPlayer::OnKillOneFromSix( pcCardToKill, plcFailedCardsTillNow, e );
	// Ib()->SetProgress( 0 );	// ���¹��� ����� ǥ�ø� ����

	m_pMFSM->RequestCallAIThread( m_pPlay, 1, &m_eTemp, pcCardToKill, plcFailedCardsTillNow );

	m_pMFSM->WaitEvent( &m_eTemp );

	Ib()->SetProgress( 0 );
	e->SetEvent();
}

void CPlayerMai::OnElection( CGoal* pNewGoal, CEvent* e )
{
	CSandClock c(DELAY_GOAL);

	// CPlayer::OnElection( pNewGoal, e );

	m_pMFSM->RequestCallAIThread( m_pPlay, 0, &m_eTemp, pNewGoal );

	m_pMFSM->WaitEvent( &m_eTemp );

	Ib()->SetProgress( 0 );
	e->SetEvent();
}

void CPlayerMai::OnTurn( CCard* pc, int* pe, CEvent* e )
{
	CSandClock c( Mo()->nDelayOneCard * 100 );

	// CPlayer::OnTurn( pc, pe, e );

	m_pMFSM->RequestCallAIThread( m_pPlay, 2, &m_eTemp, pc, pe );

	m_pMFSM->WaitEvent( &m_eTemp );

	Ib()->SetProgress( 0 );
	e->SetEvent();
}
