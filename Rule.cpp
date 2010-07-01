// Rule.cpp: implementation of the CRule class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Rule.h"
#include "card.h"
#include <vector>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

static const LPCTSTR presetFileName = "rule.txt";
static LPCTSTR asPreset[][2] = {
	{ STANDARD_RULE_STRING, _T("ǥ�� 5��") },
	{ _T("'1$1M&%)2K%,C5%"), _T("ǥ�� 4��") },
	{ _T("&/$1M&#)2K%,C5%"), _T("ǥ�� 3��") },
	{ _T(")0$1M&))2K%,C5%"), _T("ǥ�� 6��") },
	{ _T("*0$1M&))2K%,C5%"), _T("ǥ�� 7��") },
	{ _T("%*$1M&#)2K%,C5%"), _T("ǥ�� 2��") },
	{ _T(")0$1M@I7BKU&D=%"), _T("���� 6��") },
	{ _T("(1$1M&Z8B+].C]%"), _T("���� 5��") },
	{ _T("(/$1M@1%bb&+C]%"), _T("��� 5��") },
	{ _T("'0$1M&E7BKE.C=%"), _T("�λ� 4��") },
	{ _T("(/$1M&I8@K%,C=%"), _T("�뱸 5��") },
	{ _T(")0$1M&I7BKU.C=%"), _T("���� 6��") },
	{ _T("(/$1M&*9P-%,C=%"), _T("���� 5��") },
	{ _T("&/$1M&#%BK].C=%"), _T("���� 3��") },
	{ _T("(0$1M@IYb-],C]'"), _T("SNUCSE 5��") },
	{ _T(")1$1M@IYb-]+C]'"), _T("SNUCSE 6��") },
	{ _T("*1$1M@IYb-]+C]'"), _T("SNUCSE 7��") },
};
static std::vector<CString> asUserPreset;

// ��Ģ�� �ε��Ѵ�.
// ����ڰ� �߰��� �⺻ ��Ģ�� ����Ǿ� �ִ�
// ������ �ִٸ�, �� ���Ϸκ��� ��Ģ�� �ε��Ѵ�.
void CRule::LoadPreset()
{
	CFile presetFile;
	CFileException fileException;

	char szBuffer;
	bool change = true;
	CFileStatus status;
	if ( CFile::GetStatus( presetFileName, status ) )
	{
		try
		{
			if ( !presetFile.Open( presetFileName, 
				CFile::modeRead, &fileException ) )
			{
				TRACE( "Can't open file %s, error = %u\n",
				   presetFileName, fileException.m_cause );
			}
			else
			{
				while ( presetFile.Read ( &szBuffer, 1 ) )
				{
					switch ( szBuffer )
					{
					case '\t':
					case '\n':
						change = true;
						break;
					default:
						if ( change )
						{
							asUserPreset.push_back("");
							change = false;
						}
						asUserPreset.back() += szBuffer;
					}
				}
			}
		} catch(...) { }
		presetFile.Close();
	}
}
// ����ڰ� �߰��� ��Ģ�� �⺻ ��Ģ�� �ִ� ���Ϸ� �����Ѵ�.
void CRule::SavePreset()
{
	CFile presetFile;
	CFileException fileException;

	CString line;
	CFileStatus status;
	try
	{
		// Open the file with the Create flag
		if ( !presetFile.Open( presetFileName, 
						CFile::modeCreate | CFile::modeWrite, &fileException ) )
		{
			TRACE( "Can't open file %s, error = %u\n",
				presetFileName, fileException.m_cause );
			return;
		}
		else
		{
			for ( int i = 0; i < (int)asUserPreset.size() / 2; i++ )
			{
				line = asUserPreset[i * 2] + '\t' + asUserPreset[i * 2 + 1] + "\r\n";
				presetFile.Write(line, line.GetLength());
			}
		}
	} catch(...) { }
	presetFile.Close();
	return;
}
// �⺻ ��Ģ ��Ͽ� ����� ���� ��Ģ�� �߰��Ѵ�.
void CRule::AddPreset( CString ruleString, CString ruleName )
{
	asUserPreset.push_back( ruleString );
	asUserPreset.push_back( ruleName );
	SavePreset();
}
// �⺻ ��Ģ ��Ͽ��� ����� ���� ��Ģ�� �����Ѵ�.
void CRule::RemovePreset( int rulenum )
{
	if ( rulenum * 2 + 1 >= (int)asUserPreset.size() ) return;
	asUserPreset.erase( asUserPreset.begin() + rulenum * 2 + 1 );
	asUserPreset.erase( asUserPreset.begin() + rulenum * 2 );
	SavePreset();
}

// �̸� ���ǵ� ǥ�� ��� ��Ʈ
// (���ϰ��� �� ���� �̸�, NULL �̸� �ش� ���� ����)
// 1 : ǥ�� 5��  2 : ǥ�� 4��  3 : ǥ�� 3��  4 : ǥ�� 6��
LPCTSTR CRule::Preset( int nRule )
{
	nRule--;
	if ( nRule < 0 || nRule >= (int)(BASIC_PRESETS + asUserPreset.size() / 2)) {
		return 0;
	}

	if(nRule < BASIC_PRESETS)
	{
		// ���� �����Ǵ� ��Ģ�� ���
		Decode( asPreset[nRule][0] );
		return asPreset[nRule][1];
	}
	else
	{
		// ������ �߰��� �⺻ ��Ģ�� ���
		Decode( asUserPreset[(nRule - BASIC_PRESETS) * 2] );
		return asUserPreset[(nRule - BASIC_PRESETS) * 2 + 1];
	}
}

// ���ڵ�� ������ ���� ���� ����
#define BASE_CHAR  35

// 0 ���� 63 ������ ������ ���� �����Ϳ� ����
static void PushInt( TCHAR* p, int n )
{
	ASSERT( n >= 0 && n < 64 );
	*p = (TCHAR)(n+BASE_CHAR);
}

// ���� ���� �ݴ�� ����
static void PopInt( const TCHAR* p, int& n )
{
	int np = (int)*p;
	ASSERT( np >= BASE_CHAR && np < 127 );	//v4.0
	n = np - BASE_CHAR;
}

// �ϳ��� ���ڸ� 0 ���� 5 ������ ��Ʈ���� �ִ�
// ��Ʈ�����μ� �� ��Ʈ�� ��Ʈ
static void PushBool( TCHAR* p, int nPos, bool bValue )
{
	int np = (int)*p;
	ASSERT( nPos >= 0 && nPos < 6 );
	ASSERT( np >= BASE_CHAR && np < 127 );	//v4.0
	if ( bValue ) *p = (TCHAR)(((np-BASE_CHAR) | (1 << nPos)) + BASE_CHAR);
	else *p = (TCHAR)(((np-BASE_CHAR) & ~(1 << nPos)) + BASE_CHAR);
}

// ���� ���� �ݴ�� ����
static void PopBool( const TCHAR* p, int nPos, bool& bValue )
{
	int np = (int)*p;
	ASSERT( nPos >= 0 && nPos < 6 );
	ASSERT( np >= BASE_CHAR && np < 127 );	//v4.0
	bValue = ( (np-BASE_CHAR) & (1 << nPos) ) ? true : false;
}

#define SYNC_RULE_INT( VALUE ) \
	{	PushInt( p++, VALUE ); n = 0; }
#define SYNC_RULE_BOOL( VALUE ) \
	{	PushBool( p, n++, VALUE ); if ( n == 6 ) p++, n=0; }

CString CRule::GetName( int nRule )
{
	if ( nRule < 0 || nRule >= (int)(BASIC_PRESETS + asUserPreset.size() / 2)) {
		return "���������";
	}

	if ( nRule < BASIC_PRESETS ) return asPreset[nRule][1];
	else return asUserPreset[(nRule - BASIC_PRESETS) * 2 + 1];
}

// ���� ���� ��Ʈ������ ���ڵ��ؼ� ����
CString CRule::Encode() const
{
	TCHAR* pEncode = new TCHAR[64];
	for ( int w = 0; w < 64; w++ ) pEncode[w] = BASE_CHAR;

	TCHAR* p = pEncode;
	int n = 0;

	SYNC_RULE_INT( nPlayerNum );
	SYNC_RULE_INT( nMinScore );
	SYNC_RULE_INT( nMighty );
	SYNC_RULE_INT( nAlterMighty );
	SYNC_RULE_INT( nJokercall );
	SYNC_RULE_INT( nAlterJokercall );
	SYNC_RULE_BOOL( bHideScore );
	SYNC_RULE_BOOL( bFriend );
	SYNC_RULE_BOOL( bJokerFriend );
	SYNC_RULE_BOOL( bShowFriend );
	SYNC_RULE_BOOL( bAttScoreThrownPoints );
	SYNC_RULE_BOOL( bFriendGetsBeginer );
	SYNC_RULE_BOOL( bHighScore );
	SYNC_RULE_BOOL( bBeginerPass );
	SYNC_RULE_BOOL( bRaise2ForKirudaChange );
	SYNC_RULE_BOOL( bPassAgain );
	SYNC_RULE_BOOL( bNoKirudaAdvantage );
	SYNC_RULE_BOOL( bNoKirudaAlways );
	SYNC_RULE_BOOL( bInitMighty );
	SYNC_RULE_BOOL( bInitMightyEffect );
	SYNC_RULE_BOOL( bLastMighty );
	SYNC_RULE_BOOL( bLastMightyEffect );
	SYNC_RULE_BOOL( bOverrideMighty );
	SYNC_RULE_BOOL( bInitJoker );
	SYNC_RULE_BOOL( bInitJokerEffect );
	SYNC_RULE_BOOL( bLastJoker );
	SYNC_RULE_BOOL( bLastJokerEffect );
	SYNC_RULE_BOOL( bInitKiruda );
	SYNC_RULE_BOOL( bInitBeginKiruda );
	SYNC_RULE_BOOL( bInitJokercallEffect );
	SYNC_RULE_BOOL( bJokercallJokerEffect );
	SYNC_RULE_BOOL( bDM_NoPoint );
	SYNC_RULE_BOOL( bDM_OnlyOne );
	SYNC_RULE_BOOL( bDM_AllPoint );
	SYNC_RULE_BOOL( bDM_JokerIsReversePoint );
	SYNC_RULE_BOOL( bDM_OnlyMighty );
	SYNC_RULE_BOOL( bDM_Only10 );
	SYNC_RULE_BOOL( bDM_OneEyedJack );
	SYNC_RULE_BOOL( bDM_Duplicate );
	SYNC_RULE_BOOL( bS_Use20 );
	SYNC_RULE_BOOL( bS_Use40 );
	SYNC_RULE_BOOL( bS_Call );
	SYNC_RULE_BOOL( bS_Efe );
	SYNC_RULE_BOOL( bS_MEfe );
	SYNC_RULE_BOOL( bS_Base10 );
	SYNC_RULE_BOOL( bS_Base13 );
	SYNC_RULE_BOOL( bS_BaseM );
	SYNC_RULE_BOOL( bSS_Efe );
	SYNC_RULE_BOOL( bSS_Tft );
	SYNC_RULE_BOOL( bS_DoubleForRun );
	SYNC_RULE_BOOL( bS_DoubleForDeclaredRun );
	SYNC_RULE_BOOL( bS_DoubleForReverseRun );
	SYNC_RULE_BOOL( bS_DoubleForNoKiruda );
	SYNC_RULE_BOOL( bS_DoubleForNoFriend );
	SYNC_RULE_BOOL( bS_StaticRun );
	SYNC_RULE_BOOL( bS_AGoalReverse );
	SYNC_RULE_BOOL( bS_A11Reverse );
	SYNC_RULE_BOOL( bS_AMReverse );

	if ( n == 0 ) *p = '\0';
	else *(p+1) = '\0';

	CString ret = pEncode;
	delete[] pEncode;
	return ret;
}

#undef SYNC_RULE_INT
#undef SYNC_RULE_BOOL

#define SYNC_RULE_INT( VALUE ) \
	{	PopInt( p++, VALUE ); n = 0; }
#define SYNC_RULE_BOOL( VALUE ) \
	{	PopBool( p, n++, VALUE ); if ( n == 6 ) p++, n=0; }

// ��Ʈ������ ���ο� ���� ���ڵ���
void CRule::Decode( LPCTSTR sRule )
{
	const TCHAR* p = sRule;
	int n = 0;

	SYNC_RULE_INT( nPlayerNum );
	SYNC_RULE_INT( nMinScore );
	SYNC_RULE_INT( nMighty );
	SYNC_RULE_INT( nAlterMighty );
	SYNC_RULE_INT( nJokercall );
	SYNC_RULE_INT( nAlterJokercall );
	SYNC_RULE_BOOL( bHideScore );
	SYNC_RULE_BOOL( bFriend );
	SYNC_RULE_BOOL( bJokerFriend );
	SYNC_RULE_BOOL( bShowFriend );
	SYNC_RULE_BOOL( bAttScoreThrownPoints );
	SYNC_RULE_BOOL( bFriendGetsBeginer );
	SYNC_RULE_BOOL( bHighScore );
	SYNC_RULE_BOOL( bBeginerPass );
	SYNC_RULE_BOOL( bRaise2ForKirudaChange );
	SYNC_RULE_BOOL( bPassAgain );
	SYNC_RULE_BOOL( bNoKirudaAdvantage );
	SYNC_RULE_BOOL( bNoKirudaAlways );
	SYNC_RULE_BOOL( bInitMighty );
	SYNC_RULE_BOOL( bInitMightyEffect );
	SYNC_RULE_BOOL( bLastMighty );
	SYNC_RULE_BOOL( bLastMightyEffect );
	SYNC_RULE_BOOL( bOverrideMighty );
	SYNC_RULE_BOOL( bInitJoker );
	SYNC_RULE_BOOL( bInitJokerEffect );
	SYNC_RULE_BOOL( bLastJoker );
	SYNC_RULE_BOOL( bLastJokerEffect );
	SYNC_RULE_BOOL( bInitKiruda );
	SYNC_RULE_BOOL( bInitBeginKiruda );
	SYNC_RULE_BOOL( bInitJokercallEffect );
	SYNC_RULE_BOOL( bJokercallJokerEffect );
	SYNC_RULE_BOOL( bDM_NoPoint );
	SYNC_RULE_BOOL( bDM_OnlyOne );
	SYNC_RULE_BOOL( bDM_AllPoint );
	SYNC_RULE_BOOL( bDM_JokerIsReversePoint );
	SYNC_RULE_BOOL( bDM_OnlyMighty );
	SYNC_RULE_BOOL( bDM_Only10 );
	SYNC_RULE_BOOL( bDM_OneEyedJack );
	SYNC_RULE_BOOL( bDM_Duplicate );
	SYNC_RULE_BOOL( bS_Use20 );
	SYNC_RULE_BOOL( bS_Use40 );
	SYNC_RULE_BOOL( bS_Call );
	SYNC_RULE_BOOL( bS_Efe );
	SYNC_RULE_BOOL( bS_MEfe );
	SYNC_RULE_BOOL( bS_Base10 );
	SYNC_RULE_BOOL( bS_Base13 );
	SYNC_RULE_BOOL( bS_BaseM );
	SYNC_RULE_BOOL( bSS_Efe );
	SYNC_RULE_BOOL( bSS_Tft );
	SYNC_RULE_BOOL( bS_DoubleForRun );
	SYNC_RULE_BOOL( bS_DoubleForDeclaredRun );
	SYNC_RULE_BOOL( bS_DoubleForReverseRun );
	SYNC_RULE_BOOL( bS_DoubleForNoKiruda );
	SYNC_RULE_BOOL( bS_DoubleForNoFriend );
	SYNC_RULE_BOOL( bS_StaticRun );
	SYNC_RULE_BOOL( bS_AGoalReverse );
	SYNC_RULE_BOOL( bS_A11Reverse );
	SYNC_RULE_BOOL( bS_AMReverse );
}
