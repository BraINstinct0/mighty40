// Rule.h: interface for the CRule class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RULE_H__6433F792_C1B5_11D2_97F2_000000000000__INCLUDED_)
#define AFX_RULE_H__6433F792_C1B5_11D2_97F2_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define STANDARD_RULE_STRING _T("(0$1M&))2O%,C5%")

#define BASIC_PRESETS 17

// ����Ƽ ���� ��Ģ

struct CRule  
{
	CRule() { Preset( 1 ); }

	// ��Ģ�� �ε��Ѵ�.
	// ����ڰ� �߰��� �⺻ ��Ģ�� ����Ǿ� �ִ�
	// ������ �ִٸ�, �� ���Ϸκ��� ��Ģ�� �ε��Ѵ�.
	static void LoadPreset();
	// ����ڰ� �߰��� ��Ģ�� �⺻ ��Ģ�� �ִ� ���Ϸ� �����Ѵ�.
	static void SavePreset();
	// �⺻ ��Ģ ��Ͽ� ����� ���� ��Ģ�� �߰��Ѵ�.
	static void AddPreset( CString ruleString, CString ruleName );
	// �⺻ ��Ģ ��Ͽ��� ����� ���� ��Ģ�� �����Ѵ�.
	static void RemovePreset( int rulenum );

	// �̸� ���ǵ� ǥ�� ��� ��Ʈ
	// (���ϰ��� �� ���� �̸�, NULL �̸� �ش� ���� ����)
	// 1 : ǥ�� 5��  2 : ǥ�� 4��  3 : ǥ�� 3��
	// 4 : ǥ�� 6��
	LPCTSTR Preset( int nRule );

	// ���� �̸��� ����
	static CString GetName( int nRule );
	// ���� ���� ��Ʈ������ ���ڵ��ؼ� ����
	CString Encode() const;
	// ��Ʈ������ ���ο� ���� ���ڵ���
	void Decode( LPCTSTR sRule );

	/////////////////////////////////////////////////////////////////////////

	// ����Ƽ �ο�

	// �ο� ( 2-7 : v4.0 )
	int nPlayerNum;

	// ����

	// ���� ī��� ������ ����
	bool bHideScore;
	// �����尡 �ִ°�
	bool bFriend;
	// ��Ŀ�� ������� �θ� �� �ִ°�
	bool bJokerFriend;
	// ù ������ ������ ����
	bool bShowFriend;
	// ���� ī��� �ߴ��� �������� ġ�°�
	bool bAttScoreThrownPoints;
	// �������� ù �⸶�ڴ� �������� �������ΰ� (false:���Ǽ�)
	bool bFriendGetsBeginer;

	// ����

	// �ּ� �⺻ ���� ( 4 - 20 )
	int nMinScore;
	// �⺻������ 20 ���� ���� �θ� �� ���� (25����==HIGHSCORE_MAX_LIMIT)
	bool bHighScore;
	// ���� �н��� �� �ִ°�
	bool bBeginerPass;
	// ���� �ٲ� �� �߰��� �ø��� ������ 2 �ΰ� (false:1)
	bool bRaise2ForKirudaChange;
	// �н��� ����� �ٽ� �θ� �� �ִ°�
	bool bPassAgain;
	// ����� ó���� �ٲ� �� -1
	bool bNoKirudaAdvantage;
	// ����� ���� �ٸ� ��� -1
	bool bNoKirudaAlways;

	// ���� ī�� & ȿ��

	// ù�Ͽ� ����Ƽ �� �� ���� & ȿ��
	bool bInitMighty;
	bool bInitMightyEffect;
	// ���Ͽ� ����Ƽ �� �� ���� & ȿ��
	bool bLastMighty;
	bool bLastMightyEffect;
	// ��Ŀ�ݿ� ��Ŀ��� ����Ƽ �� �� ����
	bool bOverrideMighty;
	// ù�Ͽ� ��Ŀ �� �� ���� & ȿ��
	bool bInitJoker;
	bool bInitJokerEffect;
	// ���Ͽ� ��Ŀ �� �� ���� & ȿ��
	bool bLastJoker;
	bool bLastJokerEffect;
	// ù�Ͽ� ���� �� �� ����
	bool bInitKiruda;
	// ù�Ͽ� ���� ���� �� �� ����
	bool bInitBeginKiruda;
	// ù�Ͽ� ��Ŀ�� ȿ�� ����
	bool bInitJokercallEffect;
	// ��Ŀ�ݽ� ��Ŀ ȿ�� ����
	bool bJokercallJokerEffect;

	// �� �̽� ����

	// ����ī�尡 ������
	bool bDM_NoPoint;
	// ����ī�� 1��
	bool bDM_OnlyOne;
	// ��� ����ī���϶�
	bool bDM_AllPoint;
	// ��Ŀ�� �� ����ī��� ���
	bool bDM_JokerIsReversePoint;
	// ����Ƽ 1��
	bool bDM_OnlyMighty;
	// ����ī�� 10
	bool bDM_Only10;
	// �ֲٴ� J 1��
	bool bDM_OneEyedJack;
	// 10, �ֲٴ� ��� ��Ŀ&����Ƽ �ߺ� ����
	bool bDM_Duplicate;

	// ���� (��:��ǥ���� ��:�⺻���� ��:�������)

	// ����
	// ���躸�� 2.0 ( (��-��)*2+(��-��) )
	bool bS_Use20;
	// ���躸�� 4.0 ( (��-��)*1.5+(��-��) )
	bool bS_Use40;
	// �θ��°� �� ( (��-��+1)*2 )
	bool bS_Call;
	// Eye for an Eye ( (��-��) )
	bool bS_Efe;
	// Modified Eye for an Eye ( (��-��)+1 )
	bool bS_MEfe;
	// Base Ten ( (��-10) )
	bool bS_Base10;
	// Base Thirteen ( (��-13) )
	bool bS_Base13;
	// Base Min ( (��-��) )
	bool bS_BaseM;

	// �ߴ�
	// Eye for an Eye( (��-��) )
	bool bSS_Efe;
	// Tooth for an Tooth( (��>=��)?(��-��):(��-��)+2*(��-��) )
	bool bSS_Tft;

	// Ư�� ����

	// ���� 2��
	bool bS_DoubleForRun;
	// ����� ���� 2�� ( bS_DoubleForRun �� �������̵��� )
	bool bS_DoubleForDeclaredRun;
	// �鷱�� 2��
	bool bS_DoubleForReverseRun;
	// ����� 2��
	bool bS_DoubleForNoKiruda;
	// �������� 1.5�辿 �� �ֱ� ( ���������� 5������ 3��, 4������ 4.5�谡 �� )
	bool bS_DoubleForNoFriend;
	// ���϶� ������ ���� ( ..ForRun �� �������̵��� )
	// �� ���� 20 ���� �Ѵ�
	bool bS_StaticRun;
	// �鷱����
	// �ߴ��� ������ ���� �̻��� ���� ��
	bool bS_AGoalReverse;
	// �ߴ��� 11�� �̻��� ���� ��
	bool bS_A11Reverse;
	// �ߴ��� �⺻ �̻��� ���� ��
	bool bS_AMReverse;

	// ī�� ����

	// ����Ƽ
	int nMighty;
	// ��ü ����Ƽ
	int nAlterMighty;
	// ��Ŀ��
	int nJokercall;
	// ��ü ��Ŀ��
	int nAlterJokercall;
};

#endif // !defined(AFX_RULE_H__6433F792_C1B5_11D2_97F2_000000000000__INCLUDED_)
