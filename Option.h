// Option.h: interface for the COption class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OPTION_H__BE37E2C8_C23D_11D2_97F3_000000000000__INCLUDED_)
#define AFX_OPTION_H__BE37E2C8_C23D_11D2_97F3_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// ���� �ɼ�

struct COption  
{
	// �ɼ��� ������Ʈ���� ���̺�
	void Save() const;
	// �ɼ��� ������Ʈ������ �ε�
	void Load();

	/////////////////////////////////////////////////////////////////////////

	// �Ϲ�

	// �ð� ����
	bool bClockwise;
	// �Ҹ�
	bool bUseSound;
	// ī�� ���� ����
	bool bNoSort;
	// ���� ����
	bool bLeftKiruda;
	// ACE ����
	bool bLeftAce;
	// ����Ƽ ��� ���
	bool bUseTerm;
	// ���� �������� ǥ��
	bool bMoneyAsSupport;

	// ����Ű ( 4 ���� + ���� )
	int nBossType;

	// �׸�

	// ī�� �� �׸� ( -1 : ����� ���� )
	int nBackPicture;
	// ī�� �� �׸� ���
	CString sBackPicturePath;
	// ������
	int nBackColor;
	// �ߴ� ���� ���ڻ�
	int nAttColor;
	// ���� ���� ���ڻ�
	int nDefColor;
	// DSB ǥ�� ���ڻ�
	int nDSBText;
	// DSB ������ 1
	int nDSBStrong1;
	// DSB ������ 2
	int nDSBStrong2;
	// DSB �帰��
	int nDSBGray;
	// DSB ������
	bool bDSBOpaque;
	// DSB ���� ���
	int nDSBShadeMode;
	// ��� �׸� ���
	bool bUseBackground;
	// ��� �׸� ���
	CString sBackgroundPath;
	// ä��� ��� ( true : Ÿ�� false : �ѹ��� )
	bool bTile;
	// ä��� ��� ( true : Ȯ�� false : ��� )
	bool bExpand;
	// ī�� �׸� Ȯ��/���
	bool bZoomCard;
	// ī�� ���� ���
	bool bCardHelp;
	// ī�� �� ���
	bool bCardTip;

	// ���� �ӵ�

	// ī�� �̵��ӵ� 0 - 10
	int nCardSpeed;
	// �ε巯�� ī�� �̵� 0 - 10
	int nCardMotion;
	// �ϳ� ���� ���� ��� �ð� 0 - 10 (x100 millisecond)
	int nDelayOneCard;
	// �� �Ͽ� ���� ��� �ð� 0 - 10 (x200 millisecond)
	int nDelayOneTurn;
	// ī�� ���� �ִ� ���ϸ��̼� ����
	bool bShowDealing;
	// �����
	bool bSlowMachine;

	// ��Ģ

	// �̸� ���ǵ� ��Ģ - 0 �϶� ����� ����
	int nPreset;
	// ����� ���� ��Ģ
	CString sCustom;
	// ���� ��Ģ ����
	CRule rule;

	// AI

	// �� �÷��̾ ���� ����
	struct PLAYER {
		// �̸�
		CString sName;
		// ����ϴ� AI DLL
		CString sAIDLL;
		// ���� ��Ʈ��
		CString sSetting;

	} aPlayer[MAX_PLAYERS];
	// 2�� ~ 7�� ����
	int anPlayerState[MAX_PLAYERS-1][3];

	// ���

	// ������ �� �� ��Ʈ ��ȣ
	int nPort;
	// ������ �� �� �ּ� (�Ϻ�firewall)
	CString sAddress;
	// ������ �����ߴ� �ּ� ���
#	define ADDRESSBOOK_SIZE		4
	CString asAddressBook[ADDRESSBOOK_SIZE];
	// Mighty.Net �ּ�
	CString sMightyNetAddress;
	// �ٸ� �������� ���� �߿� ��Ʈ�� �� �� ����
	bool bUseHintInNetwork;
	// ä��â�� �� �ִ� �ð� (x2��)
	int nChatDSBTimeOut;
	// ä��â ���� ũ��
	int nChatBufferSize;

	// ������ ����

	// �� ȭ��
	CRect rcLast;
	// ������
	bool bLastScoreboard;		// on/off
	CPoint pntLastScoreboard;	// ��ġ

	// ó���ΰ�
	bool bInit;
};

#endif // !defined(AFX_OPTION_H__BE37E2C8_C23D_11D2_97F3_000000000000__INCLUDED_)
