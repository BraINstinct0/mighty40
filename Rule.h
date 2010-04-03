// Rule.h: interface for the CRule class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RULE_H__6433F792_C1B5_11D2_97F2_000000000000__INCLUDED_)
#define AFX_RULE_H__6433F792_C1B5_11D2_97F2_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define STANDARD_RULE_STRING _T("(0$1M&WO*7H'5#")

// ����Ƽ ���� ��Ģ

struct CRule  
{
	CRule() { Preset( 1 ); }

	// �̸� ���ǵ� ǥ�� ��� ��Ʈ
	// (���ϰ��� �� ���� �̸�, NULL �̸� �ش� ���� ����)
	// 1 : ǥ�� 5��  2 : ǥ�� 4��  3 : ǥ�� 3��
	// 4 : ǥ�� 6��
	LPCTSTR Preset( int nRule );

	// ���� ���� ��Ʈ������ ���ڵ��ؼ� ����
	CString Encode() const;
	// ��Ʈ������ ���ο� ���� ���ڵ���
	void Decode( LPCTSTR sRule );

	/////////////////////////////////////////////////////////////////////////

	// ����Ƽ �ο�

	// �ο� ( 3-6 )
	int nPlayerNum;
	// �ð�����ΰ�
	bool bClockWise;

	// ����

	// �ּ� �⺻ ���� ( 5 - 18 )
	int nMinScore;
	// ������϶� �⺻�������� 1 ���� �θ� �� ����
	bool bNoKirudaAdvantage;
	// �⺻������ 20 ���� ���� �θ� �� ���� (25����==HIGHSCORE_MAX_LIMIT)
	bool bHighScore;
	// �������� ù �⸶�ڴ� �������� �������ΰ� (false:���Ǽ�)
	bool bFriendGetsBeginer;
	// ���� �н��� �� �ִ°�
	bool bBeginerPass;
	// ���� �ٲ� �� �߰��� �ø��� ������ 2 �ΰ� (false:1)
	bool bRaise2ForKirudaChange;
	// ����ٿ��� �ٲٰų� ����ٷ� �ٲ� ���� 1 �� �÷��� �Ǵ°�
	bool bRaise1ForNoKirudaChange;
	// �����
	bool bReserved;
	// �����尡 �ִ°�
	bool bFriend;
	// ��Ŀ�� ������� �θ� �� �ִ°�
	bool bJokerFriend;
	// ���� ī��� �ߴ��� �������� ġ�°�
	bool bAttScoreThrownPoints;

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
	// ��� ����ī���϶�
	bool bDM_AllPoint;
	// ���� �� ��� ����Ƽ�� ����ī��� ���
	bool bDM_MightyIsPoint;
	// ���� �� ��� ��Ŀ�� ����ī��� ���
	bool bDM_JokerIsPoint;
	// ��Ŀ�� �� ����ī��� ���
	bool bDM_JokerIsReversePoint;
	// ����ī�� 10
	bool bDM_Only10;
	// �ֲٴ� J 1��
	bool bDM_OneEyedJack;
	// ����Ƽ 1��
	bool bDM_OnlyMighty;

	// ���� (��:��ǥ���� ��:�⺻���� ��:�������)

	// Mighty 2.0 ȣȯ ( (��-��)*2+(��-��) vs (��-��) )
	bool bS_Use20;
	// Eye for an Eye ( (��-��) vs (��-��) )
	bool bS_Efe;
	// Modified Eye for an Eye ( (��-��)+1 vs (��-��) )
	bool bS_MEfe;
	// Base Ten ( (��-10) vs (��-��) )
	bool bS_Base10;
	// Base Thirteen ( (��-��) vs (��-��) )
	bool bS_Base13;
	// ���� 2��
	bool bS_DoubleForRun;
	// ����� ���� 2�� ( bS_DoubleForRun �� �������̵��� )
	bool bS_DoubleForDeclaredRun;
	// �鷱�� 2��
	bool bS_DoubleForReverseRun;
	// ����� 2��
	bool bS_DoubleForNoKiruda;
	// �������� 2�辿 �� �ֱ� ( ���������� 5������ 4��, 4������ 6�谡 �� )
	bool bS_DoubleForNoFriend;
	// ���϶� ������ ���� ( ..ForRun �� �������̵��� )
	// �� ���� 20 ���� �Ѵ�
	bool bS_StaticRun;

	// ����

	// ù ������ ������ ����
	bool bShowFriend;
	// ���� ī��� ������ ����
	bool bHideScore;

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
