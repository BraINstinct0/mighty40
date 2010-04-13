// Card.h: interface for the CCard class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CARD_H__BE37E2C9_C23D_11D2_97F3_000000000000__INCLUDED_)
#define AFX_CARD_H__BE37E2C9_C23D_11D2_97F3_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef LAST_TURN_2MA
#	define LAST_TURN_2MA	12
#endif

#ifndef LAST_TURN
#	define LAST_TURN	9
#endif

#ifndef MIGHTY_EXPORT
#	ifdef _WINDLL
#		define MIGHTY_EXPORT __declspec(dllimport)
#	else
#		define MIGHTY_EXPORT __declspec(dllexport)
#	endif
#endif


struct CState;

// ����Ƽ���� ���Ǵ� ī��
// ī��� ���� ���� ������ 0 ~ 52 �� ����
// Nothing SA S2 ... S10 SJ SQ SK DA ... DK HA ... HK CA ... CK
// �̰�, �� �̿��� ���� Joker �̴� ( 53 ���� ���������� ����ȭ�Ѵ� )
// �� ������ ������ 1 ~ 4 �� ���� S D H C �̰�
// ���ڿ��� ������ 1 ~ 13 �� ���� A 2 3 ... K �̴�
// ��, ������ ����� Joker �̴�

#define SPADE 1
#define DIAMOND 2
#define HEART 3
#define CLOVER 4
#define ACE 1
#define JACK 11
#define QUEEN 12
#define KING 13
#define JOKER 53

class MIGHTY_EXPORT CCard  
{
public:
	// ���� ���� ������ ���� �����͸� ���� ( ���� ���� ���� )
	// �� ���� ������, �ɼǰ� ���ٸ� ��µ��� ���ȴ�
	static const CState* SetState( const CState* pNewState ) { const CState* pOld = s_pState; s_pState = pNewState; return pOld; }
	static const CState* GetState()					{ return s_pState; }

public:
	// state �� ����� �������� ����
	static int GetKiruda();
	static CCard GetMighty();
	static CCard GetJoker()							{ return CCard(JOKER); }
	static CCard GetJokercall();

public:
	// ������ & �⺻ �Լ�
	CCard( int nCard = 0 )							{ n = nCard >= 0 && nCard < JOKER ? nCard : JOKER; }
	CCard( int nShape, int nNumber )				{ if ( nShape >= 1 && nShape <= 4 && nNumber >= 1 && nNumber <= 13 ) n = (nShape-1)*13 + (nNumber-1) + 1; else n = JOKER; }
	CCard( const CCard& c )							: n(c.n) {}
	CCard& operator=( const CCard& c )				{ n = c.n; return *this; }
	operator int() const							{ return n; }
	int GetShape() const							{ if ( n == 53 || n == 0 ) return 0; else return (n-1)/13+1; }
	int GetNum() const								{ if ( n == 53 || n == 0 ) return 0; else return (n-1)%13+1; }

	// 2 3 .. Q K A Joker �� ���� 1 ~ 14 �� ������ ���� ����
	int GetPowerNum() const							{ return GetNum() ? ( GetNum() - 2 + 13 ) % 13 + 1 : 14; }

public:
	// ������ & �Ӽ�
	bool operator==( const CCard& c ) const			{ return n == c.n; }
	bool operator!=( const CCard& c ) const			{ return n != c.n; }
	bool IsPoint() const							{ return GetNum() >= 10 && GetNum() <= KING || GetNum() == ACE; }
	bool IsKiruda() const							{ int k = GetKiruda(); return SPADE <= k && k <= CLOVER && GetShape() == k; }
	bool IsMighty() const							{ return *this == GetMighty(); }
	bool IsJoker() const							{ return *this == GetJoker(); }
	bool IsJokercall() const						{ return *this == GetJokercall(); }
	bool IsOneEyedJack() const						{ return GetNum() == JACK && (GetShape() == HEART || GetShape() == SPADE); }
	bool IsAce() const								{ return GetNum() == ACE; }
	bool IsNothing() const							{ return n == 0; }

public:
	// ���ڿ� ����

	// ī���� �̸��� �����Ѵ�
	// (��: "�����̵忡�̽�" "����K" "���" ��)
	CString GetString( bool bUseTerm ) const;
	// ���� ���࿡ ���� ���ڿ��� �����Ѵ�
	// (��: "�����̵�13" "��Ʈ" "���" )
	static CString GetGoalString( bool bUseTerm,
		int nKiruda, int nMinScore, int nDefaultMinScore );

protected:
	static const CState* s_pState;
	int n;
};


// ī�� ����Ʈ

class MIGHTY_EXPORT CCardList : public CList<CCard,CCard>
{
public:
	CCardList() : CList<CCard,CCard>(5) {}
	CCardList( const CCardList& cl );
	CCardList( const CCardList& cl1, const CCardList& cl2 );
	virtual ~CCardList() {}
	CCardList& operator=( const CCardList& cl );

public:
	// �� �Լ�

	// ī�� �� ���� �����
	// nPlayers : �ο��� (5�� 53 ��, 4�� 43 ��, 3�� 33 ��)
	void NewDeck( int nPlayers, CCard cJokercall, CCard cAlterJokercall );
	// ī�带 ���´�
	void Suffle();

public:
	// ī�� ���� �Լ�

	// ī�带 ã�´� ( ������ NULL )
	POSITION Find( CCard c ) const;
	// ��ġ���� ī�� �ε����� ���Ѵ� ( ������ ASSERT �� -1 )
	int IndexFromPOSITION( POSITION pos ) const;
	// �ε����� ��ġ�� ���Ѵ�
	POSITION POSITIONFromIndex( int index ) const;
	// ����� ������ ���Ѵ�
	int GetCountOf( int nShape ) const;
	// �ε����� ī�带 ��´�
	CCard& operator[]( int index );
	CCard operator[]( int index) const;

public:
	// ��ƿ��Ƽ

	// ���ٷ� �غ� ���� ����� �����Ѵ� ( ������ ���� ���� ū�� )
	int GetKirudable() const;
	// ī�� ����
	void Sort( bool bLeftKiruda = false, bool bLeftAce = false );
	// ������ ��ġ�� ����
	// �� �Լ��� CCard�� CState ������ ���ٿ� �ɼ� ������ ����Ѵ�
	// nTurn : ���� �ϼ�
	// nJokerShape : ��Ŀ�� ���� ��� ��Ŀ ���
	// bIsJokercall : ��Ŀ���� ���� ��� ��Ŀ���ΰ�
	POSITION GetWinner( int nTurn, int nJokerShape, bool bIsJokercall ) const;
	// ���� ī���� ������ ���Ѵ�
	int GetPoint() const;
	// �� ��ü�� �տ� �� ī�带 �ǹ��� �� �� �� �ִ� ī����
	// ����Ʈ�� ���Ѵ�
	// CCard�� CState ������ Option �� ����ϸ�, ���� �Ͽ�
	// ���� ������ �־����� �Ѵ�
	void GetAvList(
		CCardList* plResult,		// ��� ���ϰ�
		const CCardList* plCurrent,	// ���� �ٴ���
		int nTurn,					// ���� �ϰ�
		int nJokerShape,			// ��Ŀ�� ���� ��� ��Ŀ ���
		bool bIsJokercall ) const;	// ��Ŀ���� ���� ��� ���� ��Ŀ���ΰ�?

protected:
	// �������� �Լ�

	// ���Ľ� �ʿ��� �Լ�
	int SortComp( CCard c1, CCard c2,
		bool bLeftKiruda, bool bLeftAce ) const;
	// �쿭�� ���Ҷ� �ʿ��� �Լ�
	int PowerComp( CCard c1, CCard c2, int nTurn,
		int nCurrentShape, bool bBeginerIsJokercall ) const;

public:
	// �־��� ī�尡 �����ΰ��� �˻�
	// ī���� ���ٿ� ���� ��� ������ �� �Լ��� ��´�
	virtual bool IsKiruda( CCard c ) const
	{	return c.IsKiruda(); }
};

#endif // !defined(AFX_CARD_H__BE37E2C9_C23D_11D2_97F3_000000000000__INCLUDED_)
