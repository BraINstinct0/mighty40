// MaiBSW.cpp: implementation of the CMaiBSW class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "mighty.h"

#include "Play.h"
#include "Player.h"

#include "MaiBSW.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// Black Sheep Wall Algorithm





// ���ٸ� ���Ƿ� ��Ʈ�� �� �ִ� ����� �����Ǵ� ī�� ����Ʈ

class CMaiBSWCardList : public CCardList  
{
public:
	int m_nKiruda;
	CMaiBSWCardList( int nKiruda = CCard::GetKiruda() ) : m_nKiruda(nKiruda)	{}
	CMaiBSWCardList( const CCardList& cl ) : m_nKiruda(CCard::GetKiruda()) { *((CCardList*)this) = cl; }

public:
	virtual bool IsKiruda( CCard c ) const
	{	return SPADE <= m_nKiruda && m_nKiruda <= CLOVER
			&& c.GetShape() == m_nKiruda; }
};


// ���� ���̺� ����

class CMaiBSWPenaltyTable
{
public:
	CMaiBSWPenaltyTable();

	// ������ ����Ѵ�
	int CalcPenalty(
		int nKiruda,
		const CState* pState,		// ��ƿ��Ƽ ����� ���� ���� ����
		int nPlayer,				// ���� ����ϰ� �ִ°�
		const CCardList& lc,		// ����ؾ� �� ī�� ����Ʈ
		bool bJokercallEffect,		// ��Ŀ�� ȿ�����ΰ�
		int nJokerShape,			// ��Ŀ ���
		const double adDefProb[] = 0 // �� �÷��̾��� ���� Ȯ�� 0.0~1.0
	) const;

protected:
	// �� ī���� �Ҹ� ������ ���Ѵ�
	int CalcUsingPenalty( int nKiruda, CCard c,
		const long anUsedCards[4], bool bTheyHaveJoker ) const;
	// �� ī�带 ���ؽ����� ���� ������ ���Ѵ�
	int CalcKillingPenalty( int nKiruda, CCard c,
		const long anUsedCards[4], bool bTheyHaveJoker ) const;
	// ������ �̸�ŭ�� �Ҿ��� �� ����� ������ ���Ѵ�
	int CalcDefLostPenalty( double dRemain, double dLostScore ) const;
	// �ߴ��� �̸�ŭ�� ���� �� ����� ������ ���Ѵ�
	int CalcAttGainPenalty( double dGainScore ) const;

	// ���� ���̺�
	int m_nUSK;		// Using 1st Score Kiruda
	int m_nUSK_d;	//       diff
	int m_nUNK;		// Using 1st Non-Score Kiruda
	int m_nUNK_d;	//       diff
	int m_nUSN;		// Using 1st Score Normal Card
	int m_nUSN_d;	//       diff
	int m_nUNN;		// Using 1st Non-Score Normal Card
	int m_nUNN_d;	//       diff
	int m_nUM;		// Using Mighty
	int m_nUJ;		// Using Joker
	int m_nUSJ;		// Using Safe Joker ( Free for Joker Call )
	int m_nULJ;		// Using Last Joker ( Free for Joker Call and Mighty )
	int m_nUEJC;	// Using Effective Jokercall
	int m_nDL30;	// D, Lost 0% ~ 30%
	int m_nDL45;	// D, Lost 30% ~ 45%
	int m_nDL60;	// D, Lost 45% ~ 60%
	int m_nDL75;	// D, Lost 60% ~ 75%
	int m_nDL100;	// D, Lost 75% ~ 100%
	int m_nDLOver;	// D, Lost all
	int m_nAS1;		// A, Score 1 pt
	int m_nAS2;		// A, Score 2 pt
	int m_nAS3;		// A, Score 3 pt
	int m_nAS4;		// A, Score 4 pt
	int m_nAS5;		// A, Score 5 pt
};


// �ְ� ���� ���̺�
// �������� ���̺��� �����ϴ� ����

class CMaiBSWPrideTable
{
public:
	CMaiBSWPrideTable();

	// ������ ����Ѵ�
	int CalcPride( int nKiruda, const CCardList& lc ) const;

	// �������� ��ǥ ������ ����Ѵ�
	// nPrideFac : 0 ~ 10 ����, �ְ��� �Ƿ��� ���⵵�� �����Ѵ�
	int PrideToMinScore( int nPride, int nPrideFac = 5 ) const;

protected:
	// ��ǻ� ���� ī�带 ������ ���� ī��� �ٲ۴�
	void Refine( CCardList* pList ) const;

	// ���� ���̺�

	// bias
	int m_nBias;
	// ���� ������ �߿䵵
	int m_nKirudaCountWeight;
	// ���� A,K,Q,10 �� �߿䵵
	int m_nKirudaAWeight;
	int m_nKirudaKWeight;
	int m_nKirudaQWeight;
	int m_nKirudaJWeight;
	// ����� K, Q, J, 10 ī���� �߿䵵
	int m_nEtcKWeight;
	int m_nEtcQWeight;
	int m_nEtcJWeight;
	int m_nEtc10Weight;
	// �� ��翡 ���� ����ġ
	int m_nEmptyWeight;
	// Mighty ������ �߿䵵
	int m_nMightyWeight;
	// Joker ������ �߿䵵
	int m_nJokerWeight;
	// Jokercall ������ �߿䵵(��Ŀ ������)
	int m_nJokercallWeight;
};


// BSW algorithm

class CMaiBSWAlgo  
{
public:
	CMaiBSWAlgo( const CMaiBSWPenaltyTable* pPnT, const CMaiBSWPrideTable* pPrT,
		MAIDLL_UPDATE* pUpdate )
		: m_pPenaltyTable(pPnT), m_pPrideTable(pPrT), m_pUpdate(pUpdate) {}

	// �־��� 13 ���� ī�忡 ���ؼ�
	// ������ ���ϰ� ���� 3 ���� ī�带 �����Ѵ�
	void Goal( int* pnKiruda, int* pnMinScore, CCard acDrop[3],
		const CCardList* pHand,
		int nPrideFac = 5,	// 0 ~ 10 ����, �ְ� �Ƿ��� ����
		const CCardList* pDeck = 0 );

	// ������ ���� (�ʿ��� ��� ���ڴ� CCard::CState ���� ��´� )
	int Friend( int nKiruda, const CCardList* pHand, const CCardList* pDeck = 0 ) const;

	// ���� ī�� ����
	CCard Kill( const CCardList* pHand, const CCardList* pTillNow ) const;

	// ī�� ���� (�ʿ��� ��� ���ڴ� CCard::CState ���� ��´� )
	CCard Turn( int& eff, bool bUseSimulation ) const;

protected:
	// �� ���̺�
	const CMaiBSWPenaltyTable* m_pPenaltyTable;
	const CMaiBSWPrideTable* m_pPrideTable;
	// ���� �� Update ��ü
	MAIDLL_UPDATE* m_pUpdate;

	// ���� �ɷ��� ī�带 ����
	CCard GetEqualCard( CCard c ) const;
	// ��� �ݺ� �˰������� ī�� ����
	CCard TurnIteration( int& eff ) const;
	// �ù����̼� �˰������� ī�� ����
	CCard TurnSimulation( int& eff ) const;
};


// Mai wrapping object

class CMaiBSWWrap : public CPlay  
{
public:
	CMaiBSWWrap( LPCTSTR sOption, MAIDLL_UPDATE* pUpdate );
	virtual ~CMaiBSWWrap();

	// �ɼ� ���
	CString GetOption() const						{ return m_sOption; }
	// �ɼ� ��Ʈ
	void SetOption( HWND hWnd );

	// CPlay �������̽� ����

	// �ϳ��� ������ ���۵�
	// �� ���ӿ��� ���Ǵ� ���� ������ �˷��ش�
	// �� ���� ������ CCard::GetState() �ε� �� �� �ִ�
	// pState->nCurrentPlayer ���� �ٷ� �ڱ� �ڽ���
	// ��ȣ�̸�, �� ���� �ڽ��� �ν��ϴµ� ���ȴ�
	virtual void OnBegin( const CState* pState );

	// 6������ �缱�� ��� �� ����� �׿��� �Ѵ�
	// ���� ī�带 �����ϸ� �ȴ� - �� �� �Լ���
	// �ݺ������� ȣ��� �� �ִ� - �� ���
	// CCardList �� ���ݱ��� ������ ī���� ����Ʈ��
	// �����Ǿ� ȣ��ȴ�
	// 5�� �����ϸ� (�� ��� �˰����� �߸��Ǿ��ų�
	// ����� �� �� �����ϴ� ���) ���Ƿ� ������ 5�� ��
	// �ϳ��� �״´� !
	virtual void OnKillOneFromSix(
		CCard* pcCardToKill, CCardList* plcFailedCardsTillNow );
	// 7������ �缱�� ��� �� ����� �׿��� �Ѵ�
	// �� �Լ��� �� �� �ϳ��� ���̴� �Լ���,
	// OnKillOneFromSix�� ����.
	// 5�� �����ϸ� (�� ��� �˰����� �߸��Ǿ��ų�
	// ����� �� �� �����ϴ� ���) ���Ƿ� ������ 5�� ��
	// �ϳ��� �״´� !
	virtual void OnKillOneFromSeven(
		CCard* pcCardToKill, CCardList* plcFailedCardsTillNow );

	// ������ ��´�
	// pNewGoal �� state.goal �� ��������� ������ ��ϵǾ� �ִ�
	// �н��Ϸ��� nMinScore �� ���� ���ຸ�� ���� ������ ��Ʈ
	// pNewGoal.nFriend �� ������� �ʴ´�
	virtual void OnElection( CGoal* pNewGoal );

	// �缱�� ���
	// pNewGoal �� state.goal �� ��������� ������ ��ϵǾ� �ִ�
	// pNewGoal �� ���ο� ������ �����ϰ� (nFriend����)
	// acDrop �� ���� �� ���� ī�带 �����Ѵ�
	virtual void OnElected( CGoal* pNewGoal, CCard acDrop[3] );

	// ���Ű� ������ �� ������ ���۵Ǿ����� �˸���
	virtual void OnElectionEnd();

	// ī�带 ����
	// pc �� �� ī��, pe �� pc�� ��Ŀ�ΰ�� ī�� ���,
	// pc�� ��Ŀ���ΰ�� ���� ��Ŀ�� ����(0:��Ŀ�� �ƴ�)
	// �� �ܿ��� ����
	virtual void OnTurn( CCard* pc, int* pe );

	// �� ���� �������� �˸���
	// ���� state.plCurrent ���� �״�� �����ִ�
	virtual void OnTurnEnd();

protected:
	// �տ� �� ī�带 ���Ѵ�
	const CCardList* GetHand() const;

	CString m_sOption;
	MAIDLL_UPDATE* m_pUpdate;
	const CState* m_pState;
	int m_nPrideFac;	// �ְ� ����
	bool m_bGoalMode;	// ��ǥ������ �ѹ��� ���ϴ°�
	bool m_bUseSimulation;	// Ȯ�� �˰����� ����ϴ´�

	CMaiBSWPenaltyTable* m_pPenaltyTable;
	CMaiBSWPrideTable* m_pPrideTable;
	CMaiBSWAlgo* m_pBSW;
};


// CMaiBSWSetting

class CMaiBSWSetting : public CDialog
{
// Construction
public:
	CMaiBSWSetting(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMaiBSWSetting)
	enum { IDD = IDD_SETTING };
	int		m_nFactor;
	BOOL	m_bGoalMode;
	BOOL	m_bUseSimulation;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMaiBSWSetting)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMaiBSWSetting)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
// 4 ���� DLL �������̼� �Լ�

// DLL ������ ��´�
void CMaiBSW::MaiGetInfo( MAIDLL_INFO* pInfo )
{
	pInfo->sName = _T("'���� �綼 ��' �˰��� (V2.17)");
	pInfo->sExplain = _T("�̸� ���ǵ� '���� ���̺�'�� ��������, "
						"������ ��� ��쿡 ���� ������ ����Ͽ� "
						"�� �� ���� ���� ������ ������ �ظ� ���Ѵ� : "
						"������ 'Greedy' �˰�������, "
						"�̷������δ� ���� �ظ� �������� "
						"����ģ ���� ������ �� ���� �͸��� �ƴ϶�� "
						"����� ���� ��ݰ� �ȴ� : ����: "
						"�� �˰����� �ٸ� ����� �и� ��� "
						"���� �� �ִ� !");
}

// �ɼ� ��Ʈ���� ������ CPlay ��ü�� ��´�
// ��, sOption �� NULL �� ���� ����Ʈ ��
CPlay* CMaiBSW::MaiGetPlay( LPCTSTR sOption, MAIDLL_UPDATE* pUpdate )
{
	return new CMaiBSWWrap( sOption, pUpdate );
}

// DLL ������ ��´�
// �� ��ü�� ���� �ɼ� ���̾�α� �ڽ��� ����ϰ�
// �ɼ��� �����Ѵ�
// ������ �ɼ��� ���� AI �� ��� �� �Լ��� �����Ǿ ����
// hWnd : ���̾�α� �ڽ��� �θ� ������
void CMaiBSW::MaiOption( CPlay* pPlay, HWND hWnd )
{
	((CMaiBSWWrap*)pPlay)->SetOption( hWnd );
}

// CPlay ��ü�� �ǵ����ش�
// �ɼ� ��Ʈ���� ������
void CMaiBSW::MaiReleasePlay( CPlay* pPlay, CString* psOption )
{
	*psOption = ((CMaiBSWWrap*)pPlay)->GetOption();
	delete pPlay;
}


/////////////////////////////////////////////////////////////////////////////

static CCard GetJokercall( int nKiruda )
{
	CCard cOri = CCard::GetState()->pRule->nJokercall;
	CCard cAlt = CCard::GetState()->pRule->nAlterJokercall;
	return nKiruda == cOri.GetShape()	// ���ٿ� ��ħ
		? cAlt : cOri;
}

static CCard GetMighty( int nKiruda )
{
	CCard cOri = CCard::GetState()->pRule->nMighty;
	CCard cAlt = CCard::GetState()->pRule->nAlterMighty;
	return nKiruda == cOri.GetShape()	// ���ٿ� ��ħ
		? cAlt : cOri;
}

static bool IsUsed( CCard c, const long anUsedCards[] )
{
	return anUsedCards[c.GetShape()-SPADE]
		& ( 1 << (c.GetPowerNum()-1) ) ? true : false;
}

static bool IsJokercallUsed( int nKiruda, const long anUsedCards[] )
{
	return IsUsed( GetJokercall(nKiruda), anUsedCards );
}

static bool IsMightyUsed( int nKiruda, const long anUsedCards[] )
{
	return IsUsed( GetMighty(nKiruda), anUsedCards );
}

static bool IsKiruda( CCard c, int nKiruda )
{
	return nKiruda && c.GetShape() == nKiruda;
}

static bool IsMighty( CCard c, int nKiruda )
{
	return GetMighty(nKiruda) == c;
}


CMaiBSWPenaltyTable::CMaiBSWPenaltyTable()
{
	// ���� ���̺�
	m_nUSK = 500;
	m_nUSK_d = -50;
	m_nUNK = 200;
	m_nUNK_d = -20;
	m_nUSN = 100;
	m_nUSN_d = -20;
	m_nUNN = 10;
	m_nUNN_d = -1;
	m_nUM = 1999;
	m_nUJ = 1997;
	m_nUSJ = 1998;
	m_nULJ = 2000;
	m_nUEJC = 300;
	m_nDL30 = 200;
	m_nDL45 = 350;
	m_nDL60 = 1000;
	m_nDL75 = 2500;
	m_nDL100 = 5000;
	m_nDLOver = 100000000;
	m_nAS1 = -400;
	m_nAS2 = -500;
	m_nAS3 = -3000;
	m_nAS4 = -5000;
	m_nAS5 = -100000000;
}

// �� ī���� �Ҹ� ������ ���Ѵ�
int CMaiBSWPenaltyTable::CalcUsingPenalty( int nKiruda, CCard c,
	const long anUsedCards[4], bool bTheyHaveJoker ) const
{
	if ( IsMighty(c,nKiruda) ) return m_nUM;

	if ( c.IsJoker() ) {

		if ( !IsJokercallUsed(nKiruda,anUsedCards) ) return m_nUJ;
		else if ( IsMightyUsed(nKiruda,anUsedCards) ) return m_nULJ;
		else return m_nUSJ;
	}
	if ( c.IsJokercall() ) {

		if ( !bTheyHaveJoker ) return m_nUEJC;
	}

	int nShape = c.GetShape()-SPADE;
	int nNumber = c.GetPowerNum()-1;

	int nOrder = 0;

	for ( int i = c.IsPoint() ? 12 : 7; i > nNumber; i-- )
		nOrder += anUsedCards[nShape] & (1<<i) ? 1 : 0;

	return IsKiruda(c,nKiruda)
			? ( c.IsPoint() ? m_nUSK + m_nUSK_d*nOrder
						: m_nUNK + m_nUNK_d*nOrder )
			: ( c.IsPoint() ? m_nUSN + m_nUSN_d*nOrder
						: m_nUNN + m_nUNN_d*nOrder );
}

// �� ī�带 ���ؽ����� ���� ������ ���Ѵ�
int CMaiBSWPenaltyTable::CalcKillingPenalty( int nKiruda, CCard c,
	const long anUsedCards[], bool bTheyHaveJoker ) const
{
	return -CalcUsingPenalty( nKiruda, c, anUsedCards, bTheyHaveJoker )/2;
}

// ������ �̸�ŭ�� �Ҿ��� �� ����� ������ ���Ѵ�
int CMaiBSWPenaltyTable::CalcDefLostPenalty( double dRemain, double dLostScore ) const
{
	// ���� �ʾ����� 0
	if ( dLostScore < 0.001 ) return 0;

	if ( dRemain < 0.0 ) // �̹� ����
		return m_nDL60; // �׳� 60% ���� ����
	else if ( dRemain < 0.001 || dRemain < dLostScore )
		// remain ���� 0 �̴�
		return m_nDLOver;
	else if ( dLostScore / dRemain < 0.3 + 0.001 )
		return m_nDL30;
	else if ( dLostScore / dRemain < 0.45 + 0.001 )
		return m_nDL45;
	else if ( dLostScore / dRemain < 0.60 + 0.001 )
		return m_nDL60;
	else if ( dLostScore / dRemain < 0.75 + 0.001 )
		return m_nDL75;
	else return m_nDL100;
}

// �ߴ��� �̸�ŭ�� ���� �� ����� ������ ���Ѵ�
int CMaiBSWPenaltyTable::CalcAttGainPenalty( double dGainScore ) const
{
	if ( dGainScore < 0.001 ) return 0;
	else if ( dGainScore < 1.001 ) return m_nAS1;
	else if ( dGainScore < 2.001 ) return m_nAS2;
	else if ( dGainScore < 3.001 ) return m_nAS3;
	else if ( dGainScore < 4.001 ) return m_nAS4;
	else return m_nAS5;
}

// ������ ����Ѵ�
int CMaiBSWPenaltyTable::CalcPenalty(
	int nKiruda,
	const CState* pState,		// ��ƿ��Ƽ ����� ���� ���� ����
	int nPlayer,				// ���� ����ϰ� �ִ°�
	const CCardList& lc,		// ����ؾ� �� ī�� ����Ʈ
	bool bJokercallEffect,		// ��Ŀ�� ȿ�����ΰ�
	int nJokerShape,			// ��Ŀ ���
	const double adDefProb[]	// �� �÷��̾��� ���� Ȯ�� 0.0~1.0
) const
{
	// ������ ��Ŀ�� ��������? (����Ƽ �����尡 �ƴϸ� ��Ŀ�� ���������� ��)
	bool bDefHasJoker = pState->goal.nFriend != (int)GetMighty(nKiruda);

	// Ȯ�� ������ ���� ��� �ٸ� �÷��̾� ������ ���� ��
	double adProb2[MAX_PLAYERS];
	if ( !adDefProb ) {
		adDefProb = adProb2;
		for ( int i = 0; i < pState->nPlayers; i++ )
			adProb2[i] = pState->IsDefender(i) ? 1.0 : 0.0;
	}
	// pState->anUsedCards �� pState->anUsingCards �� OR ��
	// ������ �ʿ��ϴ�
	long anUsedCards[4];
	{	for ( int i = 0; i < 4; i++ )
			anUsedCards[i] = pState->anUsedCards[i] | pState->anUsingCards[i];
	}

	// �ڽ��� ����, �ߴ��� Ȯ��
	double dMyProbDef = adDefProb[nPlayer];
	double dMyProbAtt = 1.0 - dMyProbDef;

	// ����, ���ڰ� ����, �ߴ��� Ȯ��, ���ڰ� ���� ī���
	int nWinner = pState->GetWinner( &lc, 0,
		(int)bJokercallEffect, nJokerShape );
	double dWinnerProbDef = adDefProb[nWinner];
	double dWinnerProbAtt = 1.0 - dWinnerProbDef;
	int nPoint = lc.GetPoint();

	// �ڽ��� ������ ���� ����
	double dDefPenalty = 0.0;
	{
		// �� ī�忡 ���� �Ҹ� & ������ ������ ���Ѵ�
		POSITION pos = lc.GetHeadPosition();
		for ( int i = 0; pos; i++ ) {
			int n = (pState->nBeginer+i) % pState->nPlayers;
			CCard c = lc.GetNext(pos);
			// ī�� ������ �����̸� �Ҹ�, �ߴ��̸� ���� �����̹Ƿ�
			// ��밪�� ���Ѵ�
			dDefPenalty
				+= (double)CalcUsingPenalty( nKiruda, c, anUsedCards, !bDefHasJoker )
					* adDefProb[n]
				+ (double)CalcKillingPenalty( nKiruda, c, anUsedCards, !bDefHasJoker )
					* (1.0-adDefProb[n]);
		}

		// ������ ���� ������ ������ ���Ѵ�
		double dRemained;
		{
			dRemained = (double) max( 0, 20 - pState->goal.nMinScore
				- pState->pRule->bAttScoreThrownPoints ? pState->nThrownPoints : 0 );
			for ( int i = 0; i < pState->nPlayers; i++ )
				dRemained -= (1.0-adDefProb[i])
					* (double)pState->apPlayers[i]
						->GetScore()->GetCount();
		}

		// �Ҵ� ������ ���� ����
		dDefPenalty += dWinnerProbAtt
			* (double)CalcDefLostPenalty( dRemained, (double)nPoint );
	}

	// �ڽ��� �ߴ��� ���� ����
	double dAttPenalty = 0.0;
	{
		// �� ī�忡 ���� �Ҹ� & ������ ������ ���Ѵ�
		POSITION pos = lc.GetHeadPosition();
		for ( int i = 0; pos; i++ ) {
			int n = (pState->nBeginer+i) % pState->nPlayers;
			CCard c = lc.GetNext(pos);
			// ī�� ������ �ߴ��̸� �Ҹ�, �����̸� ���� �����̹Ƿ�
			// ��밪�� ���Ѵ�
			dAttPenalty
				+= (double)CalcUsingPenalty( nKiruda, c, anUsedCards, bDefHasJoker )
					* (1.0-adDefProb[n])
				+ (double)CalcKillingPenalty( nKiruda, c, anUsedCards, bDefHasJoker )
					* adDefProb[n];
		}

		// ���� ������ ���� ����
		dAttPenalty += dWinnerProbAtt
			* (double)CalcAttGainPenalty( (double)nPoint );
	}

	return (int)( dMyProbDef * dDefPenalty
				+ dMyProbAtt * dAttPenalty );
}


/////////////////////////////////////////////////////////////////////////////
// CMaiBSWPrideTable

CMaiBSWPrideTable::CMaiBSWPrideTable()
{
	// 4 �� + �� A, {K,Q} ���� ������ 13 �� �θ����� �Ѵ�
	// Bias ���� ���ٿ� ���� + �� ����ٿ� ���� - ��
	// ������ �̷絵�� �Ѵ�
	// ��, ��Ŀ�� ������ 18 �̻��� �θ��� ���ϰ� �Ѵ�

	// bias (���� 40%,kirudable)
	m_nBias = 1500;

	// (���� ���� - 40% ~ 60%) �� �߿䵵
	m_nKirudaCountWeight = 300;
	// ���� A,K,Q,J,10 �� �߿䵵 (K���ʹ� ����)
	m_nKirudaAWeight = 350;
	m_nKirudaKWeight = 100;
	m_nKirudaQWeight = 40;
	m_nKirudaJWeight = 30;
	// ����� K, Q, J, 10 ī��� ���� ���ҵǴ� �߿䵵
	m_nEtcKWeight = 50;
	m_nEtcQWeight = 60;
	m_nEtcJWeight = 70;
	m_nEtc10Weight = 80;
	// �� ��翡 ���� ����ġ
	m_nEmptyWeight = 30;
	// Mighty ������ �߿䵵 (�ְ��� Mighty �� ���� �ʿ�� ����)
	m_nMightyWeight = 100;
	// Joker ������ �߿䵵
	m_nJokerWeight = 200;
	// Jokercall ������ �߿䵵 (��Ŀ ������)
	m_nJokercallWeight = 100;
}

// ������ ����Ѵ� ( CCardList �� ���ٴ� ��Ʈ�Ǿ� �ִٰ� ���� )
int CMaiBSWPrideTable::CalcPride( int nKiruda, const CCardList& lc ) const
{
	POSITION pos;

	CMaiBSWCardList lCard = lc;
	lCard.m_nKiruda = nKiruda;

	ASSERT( lCard.GetCount() <= 10 );

	int nPride = 0;

	// Ư�� ī�� �������� ��� (Refine �� ���� ������ �ҽǵǱ� ����)

	if ( !!lCard.Find( GetMighty(nKiruda) ) )
		nPride += m_nMightyWeight;

	bool bHasJoker;
	if ( false != ( bHasJoker = !!lCard.Find( CCard::GetJoker() ) ) )
		nPride += m_nJokerWeight;

	// ��Ŀ�� ����Ƽ�� �ִٸ� ��Ŀ���� �ҿ� ����
	if ( !bHasJoker && !!lCard.Find( GetJokercall(nKiruda) )
		&& !lCard.Find( GetMighty(nKiruda) ) )
		nPride += m_nJokercallWeight;

	// ���� ����

	if ( nKiruda ) {

		int nKirudaCount = lCard.GetCountOf( nKiruda );
		int nAllCount = lCard.GetCount();
		int ratio = 100 * nKirudaCount / nAllCount;
		if ( ratio >= 60 ) nPride += m_nKirudaCountWeight;
		else if ( ratio >= 40 )
			nPride += ( ratio - 40 ) * m_nKirudaCountWeight / 20;

		if ( ratio >= 40 && lCard.GetKirudable() == nKiruda )
			nPride += m_nBias;

		// ���� ���ٿ� ���� ����
		int d = 0;
		if ( !lCard.Find( CCard( nKiruda, ACE ) ) )
			nPride -= m_nKirudaAWeight;
		if ( !lCard.Find( CCard( nKiruda, KING ) ) )
			d += m_nKirudaKWeight,
			nPride -= d;
		if ( !lCard.Find( CCard( nKiruda, QUEEN ) ) )
			d += m_nKirudaQWeight,
			nPride -= d;
		if ( !lCard.Find( CCard( nKiruda, JACK ) ) )
			d += m_nKirudaJWeight,
			nPride -= d;
	}

	// ����� ī�� ���

	// ACE �� ��ġ�� ���� ���� ī��� (Refine �Ͽ� Ace ��
	// ���� ���� ī���) �� ���� ����, ACE ���� �Ÿ��� �����Ѵ�

	// ����ϱ� ���� ���� ȿ���� ���� ī�带 ��¥�� ���� �����
	Refine( &lCard );

	int nDem = 0;
	bool abEmpty[] = { true, true, true, true };

	pos = lCard.GetHeadPosition();
	while (pos) {
		CCard c = lCard.GetNext(pos);
		if ( c.IsJoker()
			|| c == GetJokercall(nKiruda)
				&& !lCard.Find(CCard::GetJoker())
				&& !lCard.Find(GetMighty(nKiruda))
			|| c == GetMighty(nKiruda)
			|| c.GetShape() == nKiruda
			|| c.IsAce()
			|| c.GetNum() == KING
				&& c.GetShape() == GetMighty(nKiruda).GetShape() )
			continue;

		abEmpty[c.GetShape()-1] = false;

		int d = 13 - c.GetPowerNum();
		if ( d == 1 ) nDem -= m_nEtcKWeight;
		else if ( d == 2 ) nDem -= m_nEtcQWeight;
		else if ( d == 3 ) nDem -= m_nEtcJWeight;
		else nDem -= m_nEtc10Weight - 4 + d;
	}

	// �� ����� �ִ� ��� ����ġ�� �ο��ȴ�
	for ( int s = 0; s < 4; s++ )
		if ( abEmpty[s] ) nDem += m_nEmptyWeight;

	// 2 �� ������ �����尡 ���´ٰ� �����Ѵ�
	if ( nDem < -2*(m_nEtc10Weight+12) )
		nDem += 2*(m_nEtc10Weight+12);

	if ( nKiruda )
		nPride += nDem;
	else {
		// �����! (���� ���� ����ġ�� etc ����ġ�� ��ü�Ѵ�
		nPride += m_nBias + m_nKirudaCountWeight;
		nPride += nDem * 3;
	}

	if ( lCard.GetCount() == 8 )
		nPride = nPride * 10 / 8;

	return nPride;
}

// ��ǻ� ���� ī�带 ������ ���� ī��� �ٲ۴�
void CMaiBSWPrideTable::Refine( CCardList* pList ) const
{
	pList->Sort( true, false );	// left kiruda, right ace

	CCard cBefore;
	int nCurShape = 0;
	bool bCurScore = false;
	int nCurPNum = -1;

	POSITION pos = pList->GetTailPosition();
	for ( ; pos; pList->GetPrev(pos) ) {
		CCard c = pList->GetAt(pos);

		if ( nCurShape != c.GetShape() ) {
			nCurShape = c.GetShape();
			bCurScore = c.IsPoint();
			nCurPNum = c.GetPowerNum();
			cBefore = c;
		}
		else if ( bCurScore != c.IsPoint() ) {
			bCurScore = c.IsPoint();
			nCurPNum = c.GetPowerNum();
			cBefore = c;
		}
		else if ( nCurPNum - 1 != c.GetPowerNum() ) {
			nCurPNum = c.GetPowerNum();
			cBefore = c;
		}
		else {
			ASSERT( (int)cBefore );
			pList->SetAt( pos, cBefore );
			nCurPNum = c.GetPowerNum();
		}
	}
}

// �������� ��ǥ ������ ����Ѵ�
// nPrideFac : 0 ~ 10 ����, �ְ��� �Ƿ��� ���⵵�� �����Ѵ�
int CMaiBSWPrideTable::PrideToMinScore( int nPride, int nPrideFac ) const
{
	// -4 ~ +4 ���� factor �� ������ �ֵ��� ����
	// 18 ������ ������ ���� �ʵ��� �Ѵ�
	int nRealPride;

	if ( nPrideFac > 5 && nPride < 1850 )
		nRealPride = nPride + ( 1850 - nPride ) / ( 12 - nPrideFac );
	else if ( nPrideFac < 5 && nPride > 1000 )
		nRealPride = nPride + ( nPride - 1200 ) / ( nPrideFac + 2 );
	else nRealPride = nPride;

	int nMinScore = ( nRealPride + 50 ) / 100;

	// ����: 12 �� �θ� �� �ִٸ� 13 �� �θ����� �Ѵ�
	if ( nMinScore + 1 == CCard::GetState()->pRule->nMinScore )
		nMinScore++;

	nMinScore = min( 20, max( 0, nMinScore ) );

	return nMinScore;
}


/////////////////////////////////////////////////////////////////////////////
// CMaiBSWAlgo

#ifdef _DEBUG
static CString GetCardString( CCard c )
{
	static LPCTSTR as1[] = { _T("X"), _T("S"), _T("D"), _T("H"), _T("C") };
	static LPCTSTR as2[] = { _T("X"), _T("A"), _T("2"), _T("3"), _T("4"),
		_T("5"), _T("6"), _T("7"), _T("8"), _T("9"), _T("10"),
		_T("J"), _T("Q"), _T("K") };
	CString sRet;
	if ( c.IsJoker() ) sRet = _T("Joker");
	else sRet.Format(_T("%s%s"),as1[c.GetShape()],as2[c.GetNum()]);
	return sRet;
}
#else
#	define GetCardString(EXPR) _T("")
#endif

// �������� ī���ΰ��� �����Ѵ�
static bool IsUseless( CCard c, int nKiruda )
{
	if ( c == GetMighty(nKiruda) || c.IsJoker()
		|| c.IsAce() || c.GetShape() == nKiruda
		|| c.GetNum() == KING ) return false;
	else return true;
}

// �־��� 13 ���� ī�忡 ���ؼ�
// ������ ���ϰ� ���� 3 ���� ī�带 �����Ѵ�
void CMaiBSWAlgo::Goal( int* pnKiruda, int* pnMinScore, CCard acDrop[3],
	const CCardList* pHand,
	int nPrideFac,	// 0 ~ 10 ����, �ְ� �Ƿ��� ����
	const CCardList* pDeck )
{
	CCardList lc = *pHand;
	if ( pDeck )
		lc.AddTail( (CList<CCard,CCard>*)pDeck );

	ASSERT( lc.GetCount() == 13 || lc.GetCount() == 8
		|| lc.GetCount() == 7 || lc.GetCount() == 10 );	//v4.0���� ������

	m_pUpdate->SetProgress( 10 );	// 10 % �Ϸ�

	int nMaxPride = 0;
	int nMaxKiruda = 0;
	int anDrop[3] = { 0, 0, 0 };

	// 4������ ��翡 ����, ������ ������ ���
	// �ִ��� ���� ������ ��츦 ����Ѵ� (��� ����)

	for ( int nKiruda = 0; nKiruda <= CLOVER; nKiruda++ ) {

		m_pUpdate->SetProgress( 25 + 15 * nKiruda );

		if ( lc.GetCount() != 13 )	{	// 13 ���� ��� ���� �ʰ� �ִ�

			int nPride = m_pPrideTable->CalcPride( nKiruda, lc );

			if ( nPride > nMaxPride ) {
				nMaxPride = nPride;
				nMaxKiruda = nKiruda;
			}
		}
		else {				// 13 �� �߿��� 10 �� (13C10) �� �̾Ƽ�
							// �����Ѵ�
			for ( int i = 0; i < 13-2; i++ ) {
				if ( !IsUseless( lc.GetAt( lc.POSITIONFromIndex(i) ), nKiruda ) )
					continue;
			for ( int j = i+1; j < 13-1; j++ ) {
				if ( !IsUseless( lc.GetAt( lc.POSITIONFromIndex(j) ), nKiruda ) )
					continue;
			for ( int k = j+1; k < 13-0; k++ ) {
				if ( !IsUseless( lc.GetAt( lc.POSITIONFromIndex(k) ), nKiruda ) )
					continue;

				CCardList l;
				int nPride;

				POSITION pos = lc.GetHeadPosition();
				for ( int r = 0; pos; r++ ) {
					CCard c = lc.GetNext(pos);
					if ( r == i || r == j || r ==k ) continue;
					l.AddTail(c);
				}

				ASSERT( l.GetCount() == 10 );

				nPride = m_pPrideTable->CalcPride( nKiruda, l );

				if ( nPride > nMaxPride ) {
					nMaxPride = nPride;
					nMaxKiruda = nKiruda;
					anDrop[0] = i;
					anDrop[1] = j;
					anDrop[2] = k;
				}
			} } }
		}
	}

	m_pUpdate->SetProgress( 100 );

	// nMaxKiruda �� ���ٷ� ����
	*pnKiruda = nMaxKiruda;

	// nMaxPride ���� ��ǥ ������ ����
	*pnMinScore = m_pPrideTable->PrideToMinScore(
						nMaxPride, nPrideFac );

	// Drop ī�带 ����
	acDrop[0] = lc.GetAt( lc.POSITIONFromIndex( anDrop[0] ) );
	acDrop[1] = lc.GetAt( lc.POSITIONFromIndex( anDrop[1] ) );
	acDrop[2] = lc.GetAt( lc.POSITIONFromIndex( anDrop[2] ) );

#ifdef _DEBUG
	TRACE("------player %d--\n",CCard::GetState()->nCurrentPlayer);
	TRACE("Kiruda : %d, Pride : %d\n", *pnKiruda, nMaxPride );
	TRACE("Drop : %s %s %s\n", acDrop[0].GetString(true),
		acDrop[1].GetString(true), acDrop[2].GetString(true) );
#endif
}

int CMaiBSWAlgo::Friend( int nKiruda, const CCardList* pHand, const CCardList* pDeck ) const
{
	// pHand �� pDeck �� pHand �� ��ģ��
	CCardList cl;
	if ( pDeck ) {
		cl = *pHand;
		cl.AddTail( (CList<CCard,CCard>*)pDeck );
		pHand = &cl;
	}

	int nFriend = 0;

	// ������ ����
	int i;
	CCard c;
	do {
		// ����, ����Ƽ !
		c = GetMighty(nKiruda);
		if ( !pHand->Find(c) ) break;

		// ����, ��Ŀ
		c = CCard::GetJoker();
		if ( CCard::GetState()->pRule->bJokerFriend
			&& !pHand->Find(c) ) break;

		// ��A
		c = CCard( nKiruda, ACE );
		if ( !pHand->Find(c) ) break;

		// �� K
		c = CCard( nKiruda, KING );
		if ( !pHand->Find(c) ) break;

		// �ٸ� ACE
		for ( i = SPADE; i <= CLOVER; i++ ) {
			c = CCard( i, ACE );
			if ( !pHand->Find(c) ) break;
		}
		if ( i <= CLOVER ) break;

		// �� Q
		c = CCard( nKiruda, QUEEN );
		if ( !pHand->Find(c) ) break;

	} while(0);

	if ( (int)c ) nFriend = (int)c;
	else {
		// �ʱ�������� �Ѵ�.
		nFriend = 1;
	}

	return nFriend;
}

// ���� ī�� ����
CCard CMaiBSWAlgo::Kill( const CCardList* pHand, const CCardList* pTillNow ) const
{
	// �տ� ���� �ְ� ���ٸ� ã�´�

	int nKiruda = CCard::GetState()->goal.nKiruda;
	int nFrom, nTo;

	if ( !nKiruda )	// nokiruda
		nFrom = SPADE, nTo = CLOVER;
	else nFrom = nKiruda, nTo = nKiruda;

	for ( int s = nFrom; s <= nTo; s++ )
		if ( !pHand->Find( CCard( s, ACE ) )
			&& !pTillNow->Find( CCard( s, ACE ) ) )
			return (int)CCard( s, ACE );
		else for ( int i = KING; i > 2; i-- )
			if ( !pHand->Find( CCard( s, i ) )
				&& !pTillNow->Find( CCard( s, i ) ) )
				return (int)CCard( s, i );
	ASSERT(0);
	return (int)CCard(nKiruda,2);	// never reached
}


/////////////////////////////////////////////////////////////////////////////
// Turn ���� �Լ�

// �� ī�尡 ������ ������°� �����Ѵ�
static bool IsUsed( CCard c )
{
	for ( int i = 0; i < CCard::GetState()->nPlayers; i++ )
		if ( CCard::GetState()->apPlayers[i]->GetHand()
				->Find(c) ) return false;
	if ( CCard::GetState()->lCurrent.Find(c) ) return false;
	return true;
}

// ���� �ɷ��� ī�带 ����
CCard CMaiBSWAlgo::GetEqualCard( CCard c ) const
{
	const CCardList* pHand =
		CCard::GetState()->apPlayers[
			CCard::GetState()->nCurrentPlayer]->GetHand();

	ASSERT( pHand->Find(c) );

	if ( c.IsJoker() || c.IsMighty() || c.IsJokercall() )
		return c;

	// c �� ���� ������ ī���
	CCardList l;

	int from, to;
	if ( c.IsPoint() ) from = 9, to = 13;	// 10 ~ Ace
	else from = 1, to = 8;	// 2 ~ 9

	int i;
	for ( i = c.GetPowerNum(); i <= to; i++ ) {
		CCard d( c.GetShape(), i % 13 + 1 );
		if ( pHand->Find(d) ) l.AddTail(d);
		else if ( !IsUsed(d) ) break;
	}
	for ( i = c.GetPowerNum()-1; i >= from; i-- ) {
		CCard d( c.GetShape(), i % 13 + 1 );
		if ( pHand->Find(d) ) l.AddHead(d);
		else if ( !IsUsed(d) ) break;
	}

#ifdef _DEBUG
	TRACE("%s is same as :\n",c.GetString(false));
	POSITION pd = l.GetHeadPosition();
	while (pd) {
		c = l.GetNext(pd);
		TRACE("%s ",c.GetString(false));
	}
	TRACE("\n");
#endif

	// ���� ���� ī�带 ������, �����ϰ� ����
	if ( !CCard::GetState()->bFriendRevealed || rand()%3 )
		return l.GetTail();
	else return l.GetAt( l.POSITIONFromIndex( rand()%l.GetCount() ) );
}

// ��� ��ȯ �˰���
// ������ ���� ���� �Ǵ� ī��� �� ���� ������ ���Ѵ�
static void RecursiveIteration(
	CCardList& lPredict,	// ����Ǵ�, ���Ŀ� ���� ī���
	CCardList& lCurrent,	// ���� �ٴڿ� �� ī��

	bool& bJokercallEffect,	// [in,out] ��Ŀ�� ����
	int& nJokerShape,		// [in,out] ��Ŀ ���

	int nPlayer,			// ���� �÷��̾�
	CCardList* aAv,			// ���� �÷��̾������ ������ ī�� ����Ʈ
	const CMaiBSWPenaltyTable* pPenaltyTable,
	MAIDLL_UPDATE* pUpdate )
{
	const CState* pState = CCard::GetState();

	// ���� �÷��̾� ��(nPlayers)�� ��(nBeginer),
	// �׸��� ���� �˰����� ȣ���� ��¥ �÷��̾�(nCurrentPlayer)��
	// ��´�
	int nPlayers = pState->nPlayers;
	int nBeginer = pState->nBeginer;
	int nCurrentPlayer = pState->nCurrentPlayer;
	// ������ ī�带 �� �� ������� �� (�� ����)
	int nPlayersToPlay = nPlayers - lCurrent.GetCount();

	ASSERT( lPredict.GetCount() == 0 );
	ASSERT( lCurrent.GetCount() == (nPlayer-nBeginer+nPlayers)%nPlayers );
	ASSERT( nCurrentPlayer != nPlayer && aAv
		|| nCurrentPlayer == nPlayer && !aAv );


	if ( !aAv ) {	// aAv �� av-list �� �Ҵ��Ѵ�
		aAv = new CCardList[nPlayersToPlay];
		// �� �ڽ��� av-list
		pState->apPlayers[nPlayer]->GetHand()->GetAvList(
			&aAv[0], &lCurrent, pState->nTurn,
			nJokerShape, bJokercallEffect );
	}

	if ( nBeginer == nPlayer ) {
		// ���� ��쿡 �̵� ���� �ʱ�ȭ �Ǿ��־�� �Ѵ�
		bJokercallEffect = false;
		nJokerShape = 0;
	}

	// ���� ������ ���� ������ ī��, ��Ȳ ��
	int nMaxPenalty = 0;
	CCard cMax;
	bool bMaxJokercallEffect = bJokercallEffect;
	int nMaxJokerShape = nJokerShape;

	CCardList lTemp;	// �ӽ� �뵵
	bool bJokercallTesting = false;

	bool bJumpToHere = false;
	int nProgressAmount = 100 / max(aAv->GetCount(),1);
	int nProgress = 0;

	POSITION pos = aAv->GetHeadPosition();
	while (pos) {

		Sleep(0);
		if ( nPlayer == nCurrentPlayer )
			pUpdate->SetProgress( nProgress += nProgressAmount );

		CCard c = aAv->GetNext(pos);
		POSITION posSave;

lbl_here: bJumpToHere = false;

		if ( nBeginer == nPlayer ) {
			// �� ī�尡 ���� ���

			// ��Ŀ�� ��Ŀ ���� ��� �� �ɼǿ� ���� ��� �õ��ؾ� �Ѵ�
			if ( c.IsJoker() ) {
				if ( ++nJokerShape > CLOVER ) continue;
				else bJumpToHere = true;
			}
			if ( c.IsJokercall() && pState->IsEffectiveJokercall() ) {
				if ( !bJokercallTesting ) {
					bJokercallTesting = true;
					bJokercallEffect = false;
					bJumpToHere = true;
				} else if ( !bJokercallEffect ) {
					bJokercallEffect = true;
					bJumpToHere = true;
				} else {
					bJokercallEffect = false;
					bJokercallTesting = false;
					continue;
				}
			}
		}

		// ���� ī�� ����

		// c �� lCurrent �� ���� �� ó�� �ϸ鼭 ���� �÷��̾����
		// lPredict �� ���Ѵ�
		lCurrent.AddTail(c);
		{
			Sleep(0);

			lTemp.RemoveAll();

			if ( nPlayer == nCurrentPlayer ) {
				// ���ʷ� ȣ���� �÷��̾��� ���
				// �� �÷��̾��� av-list �� ���� ���ƾ� �Ѵ�
				for ( int i = 1; i < nPlayersToPlay; i++ )
					pState->apPlayers[(nPlayer+i)%nPlayers]->GetHand()
					->GetAvList( &aAv[i], &lCurrent, pState->nTurn,
						nJokerShape, bJokercallEffect );
			}

			Sleep(0);

			if ( lCurrent.GetCount() < nPlayers )

				RecursiveIteration( lTemp, lCurrent,
					bJokercallEffect, nJokerShape,
					(nPlayer+1)%nPlayers, aAv+1,
					pPenaltyTable, pUpdate );

			posSave = lTemp.GetHeadPosition();
			lTemp.AddHead( (CList<CCard,CCard>*)&lCurrent );
		}
		lCurrent.RemoveTail();

		// ������ ����ϰ� �� ���� ������ ���ؼ� �����Ѵ�
		int nPenalty = pPenaltyTable->CalcPenalty(
			CCard::GetKiruda(), pState,
			nPlayer, lTemp, bJokercallEffect == 0 ? false : true,
			nJokerShape );

		if ( (int)cMax == 0 || nPenalty < nMaxPenalty ) {

			nMaxPenalty = nPenalty;
			cMax = c;
			nMaxJokerShape = nJokerShape;
			bMaxJokercallEffect = bJokercallEffect;
			lPredict.RemoveAll();
			while (posSave) lPredict.AddTail( lTemp.GetNext(posSave) );
			lPredict.AddHead(c);	// ��� �� ī��� ����ī���� ���� ù ī��
		}

		if ( bJumpToHere ) goto lbl_here;
	}

	bJokercallEffect = bMaxJokercallEffect;
	nJokerShape = nMaxJokerShape;

	if ( nPlayer == nCurrentPlayer ) {	// �Ҵ��� ����Ʈ�� ���ش�
		delete[] aAv;
	}

	ASSERT( lCurrent.GetCount() == (nPlayer-nBeginer+nPlayers)%nPlayers );
	ASSERT( lPredict.GetCount() + lCurrent.GetCount() == nPlayers );
}

// ��� ��ȯ �˰������� ī�� ����
CCard CMaiBSWAlgo::TurnIteration( int& eff ) const
{
	// RecursiveIteration �� ���� �ʿ��� ������

	CCardList lPredict;
	CCardList lCurrent = CCard::GetState()->lCurrent;

	bool bJokercallEffect = CCard::GetState()->bJokercallEffect;
	int nJokerShape = CCard::GetState()->nJokerShape;

	// �˰��� ȣ��
	RecursiveIteration( lPredict, lCurrent, bJokercallEffect, nJokerShape,
		CCard::GetState()->nCurrentPlayer, 0, m_pPenaltyTable, m_pUpdate );

	CCard c = lPredict.GetHead();

	if ( lCurrent.GetCount() == 0 ) {	// ��
		if ( c.IsJokercall() ) eff = bJokercallEffect ? 1 : 0;
		else if ( c.IsJoker() ) eff = nJokerShape;
	}

#ifdef _DEBUG
	TRACE("[%d] ",CCard::GetState()->nCurrentPlayer);
	POSITION pos = lPredict.GetHeadPosition();
	while (pos) TRACE(" -> %s",GetCardString(lPredict.GetNext(pos)));
	TRACE("\n");
#endif

	return c;
}

// �ù����̼� �˰���
// ������ ������ ��� ��쿡 ���ؼ�, ���� ���� �����ϴ� ��츦 ã�´�
static void Simulation(
	int nTurn,				// ��
	int nBeginer,			// ��
	int nPlayer,			// ���� �÷��̾�
	int& nScored,			// ������ ��� ���� (+,-)
	CCard& c,				// ���� �÷��̾ �� ī��
	bool& bJokercallEffect,	// [in,out] ��Ŀ�� ȿ�� ����
	int& nJokerShape,		// [in,out] ��Ŀ ���
	CCardList alHand[],		// �÷��̾� �տ� �� ī��
	CCardList* pCurrent,	// ��� ���� ī��
	CCardList* aAv,			// �� �÷��̾ �� �� �ִ� ī���

	int nLevel,
	int nProgress,
	int nProgressL1Range,
	MAIDLL_UPDATE* pUpdate )
{
	if ( nTurn > LAST_TURN ) { // ������ ���� ���� (���� ����
		nScored = 0;
		c = 0;
		bJokercallEffect = false;
		nJokerShape = 0;
		return;
	}

	const CState* pState = CCard::GetState();

	// �÷��̾� ��
	int nPlayers = pState->nPlayers;
	int nPlayersToPlay = nPlayers - pCurrent->GetCount();

	// �̻���� �����ΰ�
	bool bIsDefender = pState->IsDefender(nPlayer);

	// ���� aAv �� �Ҵ�Ǿ� ���� ������ �Ҵ��Ѵ�
	bool bAvAllocated = false;
	if ( !aAv ) {
		Sleep(0);
		bAvAllocated = true;
		aAv = new CCardList[nPlayersToPlay];
		for ( int i = 0; i < nPlayersToPlay; i++ )
			alHand[(nPlayer+i)%nPlayers].GetAvList(
				&aAv[i], pCurrent, nTurn, nJokerShape, bJokercallEffect );
	}

	if ( nBeginer == nPlayer ) {
		// ���� ��쿡 �̵� ���� �ʱ�ȭ �Ǿ��־�� �Ѵ�
		bJokercallEffect = false;
		nJokerShape = 0;
	}

	// ���� ������ ������ ���� ���� ī���, �� ����, ��Ȳ ��
	CCard cMax;
	int nMaxScored = 0;
	bool bMaxJokercallEffect = bJokercallEffect;
	int nMaxJokerShape = nJokerShape;

	bool bJumpToHere;
	bool bJokercallTesting = false;

	int nAvCount = max( 1, aAv->GetCount() );
	int nProgressAmount;
	if ( nLevel == 0 ) {
		nProgressAmount = 100 / nAvCount;
		nProgress = -nProgressAmount;
		nProgressL1Range = nProgressAmount;
	} else if ( nLevel == 1 )
		nProgressAmount = nProgressL1Range / nAvCount;
	else nProgressAmount = 0;

	// �� �� �ִ� ��� ī�忡 ���ؼ� �ִ��� ������ ���Ѵ�
	POSITION pos = aAv->GetHeadPosition();
	while (pos) {

		if ( nLevel == 0 || nLevel == 1 )
			pUpdate->SetProgress( nProgress += nProgressAmount );

		CCard c = aAv->GetNext(pos);
		int nScored;

lbl_here: bJumpToHere = false;

		if ( nBeginer == nPlayer ) {
			// �� ī�尡 ���� ���

			// ��Ŀ�� ��Ŀ ���� ��� �� �ɼǿ� ���� ��� �õ��ؾ� �Ѵ�
			if ( c.IsJoker() ) {
				if ( ++nJokerShape > CLOVER ) continue;
				else bJumpToHere = true;
			}
			if ( c.IsJokercall() && pState->IsEffectiveJokercall() ) {
				if ( !bJokercallTesting ) {
					bJokercallTesting = true;
					bJokercallEffect = false;
					bJumpToHere = true;
				} else if ( !bJokercallEffect ) {
					bJokercallEffect = true;
					bJumpToHere = true;
				} else {
					bJokercallTesting = false;
					bJokercallEffect = false;
					continue;
				}
			}
		}

		// ���� ī�� ����

		// c �� pCurrent �� ���� �� ó�� �ٹδ�
		pCurrent->AddTail(c);
		alHand[nPlayer].RemoveAt( alHand[nPlayer].Find(c) );
		{
			Sleep(0);

			CCard cDummy;

			// ��Ŀ�� ��Ŀ���� ��� progress �� ǥ���� �� ���� ī�忡 ����
			// ������ �����ϹǷ� �ٰ� �Դٰ��� �ϴ� ��찡 �ִ�
			// ���� nLevel �� �����ؼ� �̵� ��쿡 �ѹ��� Update ��
			// �ǵ��� nCurLevel �� nLevel ���� ����Ͽ� ����
			int nCurLevel = nLevel;
			if ( nPlayer == nBeginer )
				if ( c.IsJoker() && nJokerShape != SPADE
					|| c.IsJokercall() && bJokercallEffect )
					nCurLevel = 1000;	// Update ���� �ʴ� ������ ����

			// ���� �÷��̾ ī�带 ���� ���� (��� ȣ��)

			// ���� �÷��̾�� ���ο� ���� �������� ����
			if ( pCurrent->GetCount() < nPlayers )

				Simulation( nTurn, nBeginer, (nPlayer+1)%nPlayers, nScored,
					cDummy, bJokercallEffect, nJokerShape, alHand,
					pCurrent, aAv+1, nCurLevel+1,
					nProgress, nProgressL1Range, pUpdate );

			else {	// ���ο� ���� ����

				bool bWinnerIsDef;
				int nWinner = pState->GetWinner( pCurrent,
					&bWinnerIsDef, nJokerShape, bJokercallEffect ? 1 : 0,
					nPlayers, nTurn, nBeginer );

				CCardList lNextCurrent;	// ���� �Ͽ� �� ��� ī��
				bool bNextJokercallEffect = false;
				int nNextJokerShape = 0;

				Simulation( nTurn+1, nWinner, nWinner, nScored,
					cDummy, bNextJokercallEffect, nNextJokerShape,
					alHand, &lNextCurrent, 0, nCurLevel+1,
					nProgress, nProgressL1Range, pUpdate );

				if ( bWinnerIsDef ) nScored += pCurrent->GetPoint();
				else nScored -= pCurrent->GetPoint();
			}
		}
		pCurrent->RemoveTail();
		alHand[nPlayer].AddTail(c);

		// ������ ����ϰ� �� ���� ������ ���ؼ� �����Ѵ�
		if ( (int)cMax == 0	// �ʱ�ȭ�� �ȵǾ��ų�
			|| bIsDefender && nScored > nMaxScored	// �츮���� ���� �� ���
			|| !bIsDefender && nScored < nMaxScored ) {

			nMaxScored = nScored;
			cMax = c;
			nMaxJokerShape = nJokerShape;
			bMaxJokercallEffect = bJokercallEffect;
		}

		if ( bJumpToHere ) goto lbl_here;
	}

	ASSERT( (int)cMax != 0 );

	nScored = nMaxScored;
	c = cMax;
	bJokercallEffect = bMaxJokercallEffect;
	nJokerShape = nMaxJokerShape;

	if ( bAvAllocated ) delete[] aAv;
}

CCard CMaiBSWAlgo::TurnSimulation( int& eff ) const
{
	const CState* pState = CCard::GetState();

	int nScored;
	CCard c;
	bool bJokercallEffect = pState->bJokercallEffect;
	int nJokerShape = pState->nJokerShape;

	CCardList alHand[MAX_PLAYERS];
	for ( int i = 0; i < pState->nPlayers; i++ )
		alHand[i] = *pState->apPlayers[i]->GetHand();

	CCardList lCurrent = pState->lCurrent;

	Simulation( pState->nTurn, pState->nBeginer, pState->nCurrentPlayer,
		nScored, c, bJokercallEffect, nJokerShape, alHand,
		&lCurrent, 0, 0, 0, 0, m_pUpdate );

	if ( pState->nBeginer == pState->nCurrentPlayer ) {	// ��
		if ( c.IsJokercall() ) eff = bJokercallEffect ? 1 : 0;
		else if ( c.IsJoker() ) eff = nJokerShape;
	}

#ifdef _DEBUG
	for ( int j = 0; j < pState->nPlayers; j++ )
		ASSERT( alHand[j].GetCount() == pState->apPlayers[j]->GetHand()->GetCount() );
	ASSERT( lCurrent.GetCount() == pState->lCurrent.GetCount() );
#endif

	return c;
}


// ī�� ���� (�ʿ��� ��� ���ڴ� CCard::CState ���� ��´� )
CCard CMaiBSWAlgo::Turn( int& eff, bool bUseSimulation ) const
{
	// �ù����̼��� ����ϸ�, 3 �Ϻ���, �ƴϸ� 2 �Ϻ���
	// �ù����̼� �˰����� ����Ѵ�
	int nSimulatedTurn = bUseSimulation ? 3 : 2;

	CCard c;
	if ( CCard::GetState()->nTurn + nSimulatedTurn > LAST_TURN )
		c = TurnSimulation(eff);
	else c = TurnIteration(eff);

	// �ְ��� ��Ŀ���̰�, �����忡�� ��Ŀ�� �ִٴ� ����� ����� �� ��
	// '��Ŀ�� �ƴ�' �� �ʹ� �Ӻ��̹Ƿ� �׳� ��Ŀ�ݷ� �ٲ۴�
	if ( c.IsJokercall() ) {

		const CState* pState = CCard::GetState();

		if ( !eff	// ��Ŀ���� �ƴϰ�
			&& pState->lCurrent.IsEmpty()	// ���̰�
			&& pState->nCurrentPlayer == pState->nMaster	// �ְ��̰�
			&& pState->goal.nFriend == (int)CCard::GetMighty()	// �����忡�� ��Ŀ�� �ִ��� �𸣰�
			&& pState->nFriend >= 0	// �����尡 �ְ�
			&& pState->apPlayers[pState->nFriend]->GetHand()
				->Find( CCard::GetJoker() )	// ���������� ��Ŀ�� �ְ�
			)
				eff = 1;
	}

	// c �� ���� �ǹ��� ī�带 ����
	return GetEqualCard(c);
}


/////////////////////////////////////////////////////////////////////////////
// CMaiBSWWrap

CMaiBSWWrap::CMaiBSWWrap( LPCTSTR sOption, MAIDLL_UPDATE* pUpdate )
{
	m_sOption = sOption;
	m_pUpdate = pUpdate;
	m_pState = 0;

	if ( !sOption || sscanf_s( sOption, _T("%d %d %d"),
		&m_bGoalMode, &m_bUseSimulation, &m_nPrideFac ) != 3 ) {

		m_nPrideFac = 5;
		m_bGoalMode = true;
		m_bUseSimulation = true;
	}

	m_pPenaltyTable = new CMaiBSWPenaltyTable;
	m_pPrideTable = new CMaiBSWPrideTable;
	m_pBSW = new CMaiBSWAlgo( m_pPenaltyTable, m_pPrideTable, m_pUpdate );
}

CMaiBSWWrap::~CMaiBSWWrap()
{
	delete m_pBSW;
	delete m_pPrideTable;
	delete m_pPenaltyTable;
}

// �տ� �� ī�带 ���Ѵ�
const CCardList* CMaiBSWWrap::GetHand() const
{
	return m_pState->apPlayers[m_pState->nCurrentPlayer]->GetHand();
}

// �ɼ� ��Ʈ
void CMaiBSWWrap::SetOption( HWND hWnd )
{
	CMaiBSWSetting dlg( CWnd::FromHandle(hWnd) );
	if ( sscanf_s( m_sOption, _T("%d %d %d"),
		&dlg.m_bGoalMode, &dlg.m_bUseSimulation, &dlg.m_nFactor )
		!= 3 ) {
		dlg.m_bGoalMode = m_bGoalMode ? TRUE : FALSE;
		dlg.m_bUseSimulation = m_bUseSimulation ? TRUE : FALSE;
		dlg.m_nFactor = m_nPrideFac;
	}

	if ( dlg.DoModal() == IDOK ) {

		sprintf( m_sOption.GetBuffer(16), _T("%d %d %d"),
			dlg.m_bGoalMode, dlg.m_bUseSimulation, dlg.m_nFactor );
		m_sOption.ReleaseBuffer();

		m_bGoalMode = !!dlg.m_bGoalMode;
		m_bUseSimulation = !!dlg.m_bUseSimulation;
		m_nPrideFac = dlg.m_nFactor;
	}
}


// �ϳ��� ������ ���۵�
// �� ���ӿ��� ���Ǵ� ���� ������ �˷��ش�
// �� ���� ������ CCard::GetState() �ε� �� �� �ִ�
// pState->nCurrentPlayer ���� �ٷ� �ڱ� �ڽ���
// ��ȣ�̸�, �� ���� �ڽ��� �ν��ϴµ� ���ȴ�
void CMaiBSWWrap::OnBegin( const CState* pState )
{
	m_pState = pState;
	ASSERT( CCard::GetState() == pState );
}

// ������ ��´�
// pNewGoal �� state.goal �� ��������� ������ ��ϵǾ� �ִ�
// �н��Ϸ��� nMinScore �� ���� ���ຸ�� ���� ������ ��Ʈ
// pNewGoal.nFriend �� ������� �ʴ´�
void CMaiBSWWrap::OnElection( CGoal* pNewGoal )
{
	int nKiruda, nMinScore;
	CCard acDrop[3];
	m_pBSW->Goal( &nKiruda, &nMinScore, acDrop,
		GetHand(), m_nPrideFac,
		// 6,7 ���� ��� �� ���� ���� �����ٴ� ������ ����
		m_pState->pRule->nPlayerNum >= 6 ? 0 : &m_pState->lDeck );

	pNewGoal->nFriend = 0;
	pNewGoal->nKiruda = nKiruda;

	if ( m_pState->pRule->nMinScore > nMinScore )	// ����
		pNewGoal->nMinScore = -1;
	else if ( !m_bGoalMode ) pNewGoal->nMinScore = nMinScore;
	else
		pNewGoal->nMinScore =	// 1 �� �ø���
			pNewGoal->nMinScore < nMinScore && pNewGoal->nMinScore > 0
			? pNewGoal->nMinScore + 1 : m_pState->pRule->nMinScore;
}

// 6 ������ �缱�� ��� �� ���� �׿��� �Ѵ�
// ���� ī�带 �����ϸ� �ȴ� - �� �� �Լ���
// �ݺ������� ȣ��� �� �ִ� - �� ���
// CCardList �� ���ݱ��� ������ ī���� ����Ʈ��
// �����Ǿ� ȣ��ȴ�
// 5�� �����ϸ� (�� ��� �˰����� �߸��Ǿ��ų�
// ����� �� �� �����ϴ� ���) ���Ƿ� ������ 5�� ��
// �ϳ��� �״´� !
void CMaiBSWWrap::OnKillOneFromSix(
	CCard* pcCardToKill, CCardList* plcFailedCardsTillNow )
{
	*pcCardToKill = m_pBSW->Kill(
		GetHand(), plcFailedCardsTillNow );
}
// 7������ �缱�� ��� �� ����� �׿��� �Ѵ�
// �� �Լ��� �� �� �ϳ��� ���̴� �Լ���,
// OnKillOneFromSix�� ����.
// 5�� �����ϸ� (�� ��� �˰����� �߸��Ǿ��ų�
// ����� �� �� �����ϴ� ���) ���Ƿ� ������ 5�� ��
// �ϳ��� �״´� !
void CMaiBSWWrap::OnKillOneFromSeven(
	CCard* pcCardToKill, CCardList* plcFailedCardsTillNow )
{
	*pcCardToKill = m_pBSW->Kill(
		GetHand(), plcFailedCardsTillNow );
}

// �缱�� ���
// pNewGoal �� state.goal �� ��������� ������ ��ϵǾ� �ִ�
// pNewGoal �� ���ο� ������ �����ϰ� (nFriend����)
// acDrop �� ���� �� ���� ī�带 �����Ѵ�
void CMaiBSWWrap::OnElected( CGoal* pNewGoal, CCard acDrop[3] )
{
	int nKiruda, nMinScore;
	m_pBSW->Goal( &nKiruda, &nMinScore, acDrop,
		GetHand(), m_nPrideFac, &m_pState->lDeck );
	int nFriend = m_pBSW->Friend( nKiruda, GetHand(), &m_pState->lDeck );

	pNewGoal->nFriend = nFriend;

	if ( pNewGoal->nKiruda != nKiruda ) {

		for ( int n = pNewGoal->nMinScore; n <= nMinScore; n++ ) {

			if ( m_pState->IsValidNewGoal( nKiruda, n ) ) {
				pNewGoal->nMinScore = n;
				pNewGoal->nKiruda = nKiruda;
				return;
			}
		}
	}
}

// ���Ű� ������ �� ������ ���۵Ǿ����� �˸���
void CMaiBSWWrap::OnElectionEnd()
{
}

// ī�带 ����
// pc �� �� ī��, pe �� pc�� ��Ŀ�ΰ�� ī�� ���,
// pc�� ��Ŀ���ΰ�� ���� ��Ŀ�� ����(0:��Ŀ�� �ƴ�)
// �� �ܿ��� ����
void CMaiBSWWrap::OnTurn( CCard* pc, int* pe )
{
	*pc = m_pBSW->Turn( *pe, m_bUseSimulation );
}

// �� ���� �������� �˸���
// ���� state.plCurrent ���� �״�� �����ִ�
void CMaiBSWWrap::OnTurnEnd()
{
}


/////////////////////////////////////////////////////////////////////////////
// CMaiBSWSetting dialog

CMaiBSWSetting::CMaiBSWSetting(CWnd* pParent /*=NULL*/)
	: CDialog(CMaiBSWSetting::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMaiBSWSetting)
	m_nFactor = 0;
	m_bGoalMode = FALSE;
	m_bUseSimulation = FALSE;
	//}}AFX_DATA_INIT
}


void CMaiBSWSetting::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMaiBSWSetting)
	DDX_Slider(pDX, IDC_FACTOR, m_nFactor);
	DDX_Check(pDX, IDC_GOALMODE, m_bGoalMode);
	DDX_Check(pDX, IDC_USESIMULATION, m_bUseSimulation);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMaiBSWSetting, CDialog)
	//{{AFX_MSG_MAP(CMaiBSWSetting)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMaiBSWSetting message handlers

BOOL CMaiBSWSetting::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	((CSliderCtrl*)GetDlgItem(IDC_FACTOR))->SetRange( 0, 10, TRUE );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
