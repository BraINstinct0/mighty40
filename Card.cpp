// Card.cpp: implementation of the CCard class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "State.h"
#include "Rule.h"
#include "Card.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

const CState* CCard::s_pState = 0;

// state �� ����� �������� ����

int CCard::GetKiruda()
{
	ASSERT(s_pState);
	return s_pState->goal.nKiruda;
}

CCard CCard::GetMighty()
{
	ASSERT(s_pState && s_pState->pRule);
	if ( GetKiruda() == CCard(s_pState->pRule->nMighty).GetShape() )
		// ����Ƽ ���� ���ٰ� ����
		return CCard(s_pState->pRule->nAlterMighty);
	else return CCard(s_pState->pRule->nMighty);
}

CCard CCard::GetJokercall()
{
	ASSERT(s_pState && s_pState->pRule);
	if ( GetKiruda() == CCard(s_pState->pRule->nJokercall).GetShape() )
		// ��Ŀ�� ���� ���ٰ� ����
		return CCard(s_pState->pRule->nAlterJokercall);
	else return CCard(s_pState->pRule->nJokercall);
}

CString CCard::GetString( bool bUseTerm ) const
{
	static LPCTSTR s_aShapeTerm[][2] = {
		{ _T(""), _T("") },
		{ _T("��"), _T("�����̵� ") },
		{ _T("�ٸ�"), _T("���̾Ƹ�� ") },
		{ _T("Ʈ"), _T("��Ʈ ") },
		{ _T("��"), _T("Ŭ�ι� ") } };

	CString sRet;
	if ( bUseTerm ) {

		if ( IsMighty() ) return CString( _T("����Ƽ") );
		else if ( IsJoker() ) return CString( _T("��ī") );
		else if ( IsJokercall() ) return CString( _T("����") );
		else {
			if ( IsKiruda() ) sRet = _T("��");
			else sRet = s_aShapeTerm[GetShape()][0];
			if ( IsAce() ) sRet += _T("��");
			else if ( GetNum() == KING ) sRet += _T("ī");
			else if ( GetNum() == QUEEN ) sRet += _T("ť");
			else if ( GetNum() == JACK ) sRet += _T("��");
			else if ( GetNum() == 10 ) sRet += _T("10");
			else sRet += (TCHAR)( GetNum() + (int)_T('0') );
		}
	}
	else {

		if ( IsMighty() ) return CString( _T("����Ƽ") );
		else if ( IsJoker() ) return CString( _T("��Ŀ") );
		else if ( IsJokercall() ) return CString( _T("��Ŀ��") );
		else {
			if ( IsKiruda() ) sRet = _T("���� ");
			else sRet = s_aShapeTerm[GetShape()][1];
			if ( IsAce() ) sRet += _T("A");
			else if ( GetNum() == KING ) sRet += _T("K");
			else if ( GetNum() == QUEEN ) sRet += _T("Q");
			else if ( GetNum() == JACK ) sRet += _T("J");
			else if ( GetNum() == 10 ) sRet += _T("10");
			else sRet += (TCHAR)( GetNum() + (int)_T('0') );
		}
	}
	return sRet;
}

// ���� ���࿡ ���� ���ڿ��� �����Ѵ�
// (��: "�����̵�13" "��Ʈ" "���" )
CString CCard::GetGoalString( bool bUseTerm,
	int nKiruda, int nMinScore, int nDefaultMinScore )
{
	static LPCTSTR s_aScoreTerm[] = {
		_T("��"), _T("��"), _T("��"), _T("��"), _T("��"),
		_T("��"), _T("��"), _T("��"), _T("ȩ"), _T("Ǯ") };
	static LPCTSTR s_aShapeTerm[][2] = {
		{ _T("��"), _T("�����") },
		{ _T("��"), _T("�����̵� ") },
		{ _T("�ٸ�"), _T("���̾Ƹ�� ") },
		{ _T("Ʈ"), _T("��Ʈ ") },
		{ _T("��"), _T("Ŭ�ι� ") } };

	CString sRet;

	if ( !bUseTerm ) {
		// �״�� ����Ѵ�
		sRet.Format( _T("%s %d"), s_aShapeTerm[nKiruda][1], nMinScore );
		return sRet;
	}
	else {
		// ������ �ּ� ������ 11 ���� �۰ų� 20���� ū ����̴�
		// ���� "��" �� ���̰� "Ǯ" �� ���δ�
		// 30 �̻��� ���� �������� �ʴ´�
		if ( nMinScore <= 10 ) sRet = _T("��");
		if ( nMinScore > 20 ) sRet = _T("Ǯ");
		sRet += s_aScoreTerm[(nMinScore+9)%10];
		sRet += s_aShapeTerm[nKiruda][0];
	}
	return sRet;
}

/////////////////////////////////////////////////////////////////////////////

CCardList::CCardList( const CCardList& cl ) : CList<CCard,CCard>(5)
{
	for ( POSITION pos = cl.GetHeadPosition(); pos; )
		AddTail( cl.GetNext(pos) );
}

CCardList::CCardList( const CCardList& cl1, const CCardList& cl2 ) : CList<CCard,CCard>(5)
{
	for ( POSITION pos1 = cl1.GetHeadPosition(); pos1; )
		AddTail( cl1.GetNext(pos1) );
	for ( POSITION pos2 = cl2.GetHeadPosition(); pos2; )
		AddTail( cl2.GetNext(pos2) );
}

CCardList& CCardList::operator=( const CCardList& cl )
{
	RemoveAll();
	for ( POSITION pos = cl.GetHeadPosition(); pos; )
		AddTail( cl.GetNext(pos) );
	return *this;
}

// ī�� �� ���� �����
// nPlayers : �ο��� (5�� 53 ��, 4�� 43 ��, 3�� 33 ��)
void CCardList::NewDeck( int nPlayers, CCard cJokercall, CCard cAlterJokercall )
{
	// 2���� 6�� �̻��� 5���� ���� ���� (v4.0)
	if ( nPlayers >= 6 || nPlayers == 2 ) nPlayers = 5;

	ASSERT( nPlayers == 3 || nPlayers == 4 || nPlayers == 5 );

	RemoveAll();

	// �� ��Ŀ��(��,����)�� ���ڸ� ��� �д�
	// ī�带 ������ �� �̵��� �������� �ȵȴ�
	CCard cJC1( cJokercall );
	CCard cJC2( cAlterJokercall );

	// ��Ŀ
	AddTail( CCard(JOKER) );

	if ( nPlayers == 5 ) {	// 5��

		// ��� ī�带 �߰�
		for ( int i = ACE; i <= KING; i++ )
			for ( int j = SPADE; j <= CLOVER; j++ )
				AddTail( CCard( j, i ) );
	}
	else if ( nPlayers == 3 ) {	// 3��

		// 2�������� ��Ŀ���� �� 5������ ���ڸ� ����
		int skip = 0;
		for ( int i = ACE; i <= KING; i++ ) {
			if ( i >= 2 && i <= 9 && skip < 5
					&& i != cJC1.GetNum()
					&& i != cJC2.GetNum() ) {
				skip++;
				continue;
			}
			else for ( int j = SPADE; j <= CLOVER; j++ )
				AddTail( CCard( j, i ) );
		}
	}
	else if ( nPlayers == 4 ) {	// 4��

		if ( cJC1.GetNum() == cJC2.GetNum()
				&& cJC1.GetNum() >= 2 && cJC1.GetNum() <= 4 ) {
			// �� ��Ŀ�� ����� ���� 2�� 4 �����̴�

				// 2, 3, 4 �� ���� ���� ���� ��Ŀ�� 2���� �߰�
				for ( int i = ACE; i <= KING; i++ )
					if ( i >= 2 && i <= 4 ) continue;
					else for ( int j = SPADE; j <= CLOVER; j++ )
						AddTail( CCard( j, i ) );
				AddTail( cJC1 ); AddTail( cJC2 );
		}
		else {
			// 2 ���� ��Ŀ���� �ƴ� ī�带 ����
			// �� 10���� �����Ѵ�

			// ��� ī�带 �߰�
			for ( int i = ACE; i <= KING; i++ )
				for ( int j = SPADE; j <= CLOVER; j++ )
					AddTail( CCard( j, i ) );
			// ����
			int skip = 0;
			for ( int s = 2; s <= 9 && skip < 10; s++ )
				for ( int t = CLOVER; t >= SPADE && skip < 10; t-- )
					if ( CCard(t,s) != cJC1 && CCard(t,s) != cJC2 ) {
						RemoveAt( Find( CCard(t,s) ) );
						skip++;
					}
		}
	}

	ASSERT( nPlayers == 5 && GetCount() == 53
		|| nPlayers == 4 && GetCount() == 43
		|| nPlayers == 3 && GetCount() == 33 );
}

// ī�带 ���´�
void CCardList::Suffle()
{
	// �� ����Ʈ�� ���� �� �װͿ�
	// �� ����Ʈ�� ��� ���Ҹ� �� �ڷ� �����ϰ� �ű��
	// �̰��� ������ �ٲ㼭 �ٽ��ѹ� �Ѵ�

	int nCards = GetCount();

	CCardList lTemp;

	for ( int i = 0; i < 3; i++ ) {
		// �� �� �ݺ�

		// ��� ���Ҹ� lTemp �� �ű��
		for ( int j = 0; j < nCards; j++ )
			if ( rand()%2 ) lTemp.AddTail( RemoveHead() );
			else lTemp.AddHead( RemoveHead() );
		// �ٽ� �Ű� �´�
		for ( int k = 0; k < nCards; k++ )
			if ( rand()%2 ) AddTail( lTemp.RemoveHead() );
			else AddHead( lTemp.RemoveHead() );

		ASSERT( lTemp.GetCount() == 0 && GetCount() == nCards );
	}
}

// ī�带 ã�´� ( ������ NULL )
POSITION CCardList::Find( CCard c ) const
{
	for ( POSITION pos = GetHeadPosition(); pos; GetNext(pos) )
		if ( GetAt(pos) == c ) return pos;
	return NULL;
}

// ����� ������ ���Ѵ�
int CCardList::GetCountOf( int nShape ) const
{
	int nCount = 0;
	for ( POSITION pos = GetHeadPosition(); pos; )
		if ( GetNext(pos).GetShape() == nShape )
			nCount++;
	return nCount;
}

// �ε����� ī�带 ��´�
CCard& CCardList::operator[]( int index )
{
	int i = 0;
	for ( POSITION pos = GetHeadPosition(); pos; i++, GetNext(pos) )
		if ( i == index ) return GetAt(pos);
	ASSERT(0);
	return *(CCard*)0;
}

CCard CCardList::operator[]( int index) const
{
	int i = 0;
	for ( POSITION pos = GetHeadPosition(); pos; i++, GetNext(pos) )
		if ( i == index ) return GetAt(pos);
	ASSERT(0);
	return CCard(0);
}

// ��ġ���� ī�� �ε����� ���Ѵ� ( ������ ASSERT �� -1 )
int CCardList::IndexFromPOSITION( POSITION pos ) const
{
	POSITION posSearch = GetHeadPosition();

	for ( int index = 0; posSearch; index++ )
		if ( posSearch == pos ) return index;
		else GetNext(posSearch);

	ASSERT(0);
	return -1;
}

// �ε����� ��ġ�� ���Ѵ�
POSITION CCardList::POSITIONFromIndex( int index ) const
{
	POSITION posSearch = GetHeadPosition();

	for ( int i = 0; posSearch; i++, GetNext(posSearch) )
		if ( i == index ) return posSearch;

	return 0;
}

// ���ٷ� �غ� ���� ����� �����Ѵ� ( ������ ���� ���� ū�� )
int CCardList::GetKirudable() const
{
	int nMaxShape = 0;
	int nMaxSum = 0;

	for ( int i = SPADE; i <= CLOVER; i++ ) {
		// �� ����� ���� ���� ���� ( 2 ... K A ���� �� 1 ~ 13 )
		int nSum = 0;
		for ( POSITION pos = GetHeadPosition(); pos; GetNext(pos) )
			if ( GetAt(pos).GetShape() == i )
				nSum += GetAt(pos).GetPowerNum();
		if ( nSum > nMaxSum ) {
			nMaxSum = nSum;
			nMaxShape = i;
		}
	}
	ASSERT( nMaxShape && nMaxSum );
	return nMaxShape;
}

// ���Ľ� �ʿ��� �Լ�
int CCardList::SortComp( CCard c1, CCard c2,
	bool bLeftKiruda, bool bLeftAce ) const
{
	ASSERT( c1 != c2 );

	if ( c1.IsJoker() ) return 1;
	if ( c2.IsJoker() ) return -1;

	if ( IsKiruda(c1) && !IsKiruda(c2) )
		return bLeftKiruda ? 1 : -1;
	if ( !IsKiruda(c1) && IsKiruda(c2) )
		return bLeftKiruda ? -1 : 1;
	if ( c1.GetShape() > c2.GetShape() )
		return -1;
	if ( c1.GetShape() < c2.GetShape() )
		return 1;

	if ( c1.IsAce() && !c2.IsAce() )
		return bLeftAce ? 1 : -1;
	if ( !c1.IsAce() && c2.IsAce() )
		return bLeftAce ? -1 : 1;
	if ( c1.GetNum() > c2.GetNum() )
		return -1;
	if ( c1.GetNum() < c2.GetNum() )
		return 1;

	ASSERT(0);
	return 0;
}

// ī�� ����
void CCardList::Sort( bool bLeftKiruda, bool bLeftAce )
{
	// ���� ��Ʈ

	CCardList lTemp;

	int nCards = GetCount();
	for ( int i = 0; i < nCards; i++ ) {

		CCard c = RemoveHead();
		POSITION pos = lTemp.GetHeadPosition();

		for ( ; pos; lTemp.GetNext(pos) )
			if ( SortComp( c, lTemp.GetAt(pos),
				bLeftKiruda, bLeftAce ) > 0 ) break;

		if ( pos ) lTemp.InsertBefore( pos, c );
		else lTemp.AddTail( c );
	}
	ASSERT( IsEmpty() && lTemp.GetCount() == nCards );
	*this = lTemp;
}

// ���� ī���� ������ ���Ѵ�
int CCardList::GetPoint() const
{
	int nCount = 0;
	for ( POSITION pos = GetHeadPosition(); pos; )
		if ( GetNext(pos).IsPoint() ) nCount++;
	return nCount;
}

// ������ ��ġ�� ����
// �� �Լ��� CCard�� CState ������ ���ٿ� �ɼ� ������ ����Ѵ�
// nTurn : ���� �ϼ�
// nJokerShape : ��Ŀ�� ���� ��� ��Ŀ ���
// bIsJokercall : ��Ŀ���� ���� ��� ��Ŀ���ΰ�
POSITION CCardList::GetWinner( int nTurn, int nJokerShape, bool bIsJokercall ) const
{
	ASSERT( GetCount() );

	// ���� ���
	int nCurrentShape =
		GetHead().IsJoker() ? nJokerShape : GetHead().GetShape();
	// ���� ��Ŀ���ΰ�
	bool bBeginerIsJokercall =
		GetHead().IsJokercall() && bIsJokercall;

	POSITION posWin = 0, pos = GetHeadPosition();
	for ( ; pos; GetNext(pos) ) {
		if ( !posWin
			|| PowerComp( GetAt(posWin), GetAt(pos), nTurn,
				nCurrentShape, bBeginerIsJokercall ) < 0 )
			posWin = pos;
	}
	ASSERT( posWin );
	return posWin;
}

// �쿭�� ���Ҷ� �ʿ��� �Լ�
int CCardList::PowerComp( CCard c1, CCard c2, int nTurn,
	int nCurrentShape, bool bBeginerIsJokercall ) const
{
	ASSERT( c1 != c2 );
	const CState* pState = CCard::GetState(); ASSERT( pState );
	const CRule* pRule = pState->pRule; ASSERT( pRule );

	// ����Ƽ ȿ��
	// ù���̰� ù�ǿ� ����Ƽ ȿ�� ���ų�
	// �����̰� ���ǿ� ����Ƽ ȿ�� ������ ��ȿ
	bool bMightyEffect =
		! ( nTurn == 0 && !pRule->bInitMightyEffect
			|| nTurn == LAST_TURN && !pRule->bLastMightyEffect );
	// ��Ŀ�� ȿ��
	// ù���̰� ù�ǿ� ��Ŀ�� ȿ�� ������ ��ȿ
	bool bJokercallEffect =
		! ( nTurn == 0 && !pRule->bInitJokercallEffect );
	// ��Ŀ���� ���ߴ°�
	// ���� ��Ŀ���� �°� ��Ŀ�� ȿ���� ���� ��
	bool bJokercalled =
		bBeginerIsJokercall && bJokercallEffect;
	// ��Ŀ ȿ��
	// ù���̰� ù�ǿ� ��Ŀ ȿ�� ���ų�
	// �����̰� ���ǿ� ��Ŀ ȿ�� ���ų�
	// ��Ŀ�� �������� ��ȿ
	bool bJokerEffect =
		! ( nTurn == 0 && !pRule->bInitJokerEffect
			|| nTurn == LAST_TURN && !pRule->bLastJokerEffect
			|| bJokercalled && !pRule->bJokercallJokerEffect );

	// ����Ƽ�� ���� ó��
	if ( bMightyEffect ) {
		if ( c1.IsMighty() ) return 1;
		else if ( c2.IsMighty() ) return -1;
	}
	// ��Ŀ�� ���� ó��
	if ( bJokerEffect ) {
		if ( c1.IsJoker() ) return 1;
		else if ( c2.IsJoker() ) return -1;
	}
	else {
		if ( c1.IsJoker() ) return -1;
		else if ( c2.IsJoker() ) return 1;
	}
	// ���ٿ� ���� ó��
	if ( IsKiruda(c1) || IsKiruda(c2) ) {
		if ( !IsKiruda(c1) ) return -1;
		else if ( !IsKiruda(c2) ) return 1;
		else return c1.GetPowerNum() > c2.GetPowerNum() ? 1 : -1;
	}
	// ���� ��翡 ���� ó��
	else if ( c1.GetShape() != nCurrentShape
		&& c2.GetShape() != nCurrentShape )
		// ��� ���� ����� �ƴ� ���� �� �ǹ̰� ����
		return 0;
	else if ( c1.GetShape() != nCurrentShape ) return -1;
	else if ( c2.GetShape() != nCurrentShape ) return 1;
	else return c1.GetPowerNum() > c2.GetPowerNum() ? 1 : -1;
}

// �� ��ü�� �տ� �� ī�带 �ǹ��� �� �� �� �ִ� ī����
// ����Ʈ�� ���Ѵ�
// CCard�� CState ������ Option �� ����ϸ�, ���� �Ͽ�
// ���� ������ �־����� �Ѵ�
void CCardList::GetAvList(
	CCardList* plResult,		// ��� ���ϰ�
	const CCardList* plCurrent,	// ���� �ٴ���
	int nTurn,					// ���� �ϰ�
	int nJokerShape,			// ��Ŀ�� ���� ��� ��Ŀ ���
	bool bIsJokercall ) const	// ��Ŀ���� ���� ��� ���� ��Ŀ���ΰ�?
{
	ASSERT( plResult && plCurrent );
	ASSERT( CCard::GetState() && CCard::GetState()->pRule );

	plResult->RemoveAll();

	const CRule* pRule = CCard::GetState()->pRule;

	// ������ ���鼭 ����Ƽ�� ��Ŀ ���� ���θ� �����Ѵ�
	bool bHasMighty = false, bHasJoker = false;

	// ���� ����� ?   0 �̸� ��
	int nCurrentShape = 0;
	if ( plCurrent->GetCount() > 0 ) {
		CCard c = plCurrent->GetHead();
		nCurrentShape = c.IsJoker() ? nJokerShape : c.GetShape();
	}
	// ���� ����� �ϳ��� ������?
	bool bHasCurrentShape =
		!nCurrentShape ? 0
		: GetCountOf( nCurrentShape ) ? true : false;

	POSITION pos = GetHeadPosition();
	while (pos) {
		CCard c = GetNext(pos);

		if ( c.IsMighty() ) {
			// ����Ƽ

			bHasMighty = true;

			// ù��, �� �ǿ� ����Ƽ �� �� �ִ����� ����
			if ( !( nTurn == 0 && !pRule->bInitMighty
					|| nTurn == LAST_TURN && !pRule->bLastMighty ) )
				// �� �� ����
				plResult->AddTail( c );
		}
		else if ( c.IsJoker() ) {
			// ��Ŀ

			bHasJoker = true;

			// ��Ŀ���̶�� ������ ��Ŀ�� ����
			if ( plCurrent->GetCount() > 0 // ���� �ƴϰ�
				&& plCurrent->GetHead().IsJokercall() // ���� ��Ŀ���̰�
				&& bIsJokercall // �� ��Ŀ���� ���� ��Ŀ���̰�
				&& ( nTurn != 0 || pRule->bInitJokercallEffect ) ) { // ��Ŀ�� ȿ������
				// ������!
				plResult->RemoveAll();
				plResult->AddTail( c );
				// ��Ŀ�� ��� ����Ƽ ������ ����Ƽ�� ���� �ִ� ��Ģ�� ����
				bHasMighty = !!Find(CCard::GetMighty());
				if ( pRule->bOverrideMighty && bHasMighty ) {
					plResult->AddTail( CCard::GetMighty() );
				}
				break;
			}
			// ù��, �� �ǿ� ��Ŀ �� �� �ִ����� ����
			if ( !( nTurn == 0 && !pRule->bInitJoker
					|| nTurn == LAST_TURN && !pRule->bLastJoker ) )
				// �� �� ����
				plResult->AddTail( c );
		}
		else {
			if ( bHasCurrentShape ) {
				// ���� �ٴ� ����� �ֱ� ������ �� �����
				// ���� �Ѵ�
				if ( c.GetShape() == nCurrentShape )
					plResult->AddTail( c );
			}
			else {
				// ���� �ٴ� ����� ���� ������ �ƹ� ����̳� ���� �ȴ�
				if ( IsKiruda(c) ) {
					// ����

					// ù�ǿ� ���� �� �� �ִ����� ����
					if ( !( nTurn == 0 && !pRule->bInitKiruda )
						&& !( nTurn == 0 && plCurrent->GetCount() == 0
							&& !pRule->bInitBeginKiruda ) )
						// �� �� ����
						plResult->AddTail( c );
				}
				else
					// �Ϲ� ���
					plResult->AddTail( c );
			}
		}
	}

	// ���� �������� ��Ŀ�� ����Ƽ�� �� �� ���µ�
	// ������ �� ������ �̵��� ������ �ִٸ�
	// �� �̵��� ���� �Ѵ�
	if ( nTurn == LAST_TURN-2
			&& ( bHasMighty && !pRule->bLastMighty )
			&& ( bHasJoker && !pRule->bLastJoker ) ) {
		plResult->RemoveAll();
		plResult->AddTail( CCard::GetMighty() );
		plResult->AddTail( CCard::GetJoker() );
		return;
	}
	else if ( nTurn == LAST_TURN-1
			&& ( ( bHasMighty && !pRule->bLastMighty )
				|| ( bHasJoker && !pRule->bLastJoker ) ) ) {
		plResult->RemoveAll();
		plResult->AddTail(
			( bHasMighty && !pRule->bLastMighty )
				? CCard::GetMighty() : CCard::GetJoker() );
		return;
	}

	// �ƹ��͵� �� �� ���� ��Ȳ�� �Դ� ??
	if ( plResult->GetCount() == 0 ) {
//		ASSERT(0);
		// ��� ī�尡 ���ٶ�簡..
		// �� ��� �ƹ� ī�峪 �� �� �ִ� Ư���� �ش� !
		*plResult = *this;
	}
}
