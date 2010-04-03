// DStartUp.h: interface for the DStartUp class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DSTARTUP_H__4643EA36_DCDA_11D2_9840_0000212035B8__INCLUDED_)
#define AFX_DSTARTUP_H__4643EA36_DCDA_11D2_9840_0000212035B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMFSM;
struct COption;


class DStartUp : public DSB  
{
public:
	DStartUp( CBoardWrap* pBoard ) : DSB(pBoard) {}
	virtual ~DStartUp() {}

	// nMode  0 : �ʱ� �޴�  1 : ��Ƽ�÷��̾� �޴�
	void Create( int nMode );

public:
	// ���� �ɼǿ� �����ؼ� 1�ο� ����Ƽ MFSM �� ����(new)�Ѵ�
	// �����ϸ� Board �� �� MFSM �� ��Ʈ
	// AI ���� �ε� ���� �� ���� DSB �� ���� (������ �����)
	bool CreateOnePlayerGame();

protected:
	// �ʱ�ȭ ( ������ �� ȣ��� )
	virtual void OnInit();

	// CBoard �� ȣ���ϴ� �׸��� �Լ� - ��µ� shade-box �� �� �Լ���
	// �������̵� �Ͽ� �����ؾ� �Ѵ�
	// ���̽� �Լ��� ������ ��ü�� ���� ���� ������ �׸���
	virtual void OnDraw(
		CDC* pDC,
		bool bBrushOrigin );

	// �� ������ Ŭ���� �� �Ҹ��� �Լ�
	// LPVOID ���� 0 �̸� ���� (X�ڸ� Ŭ��) ��û
	virtual void OnClick( LPVOID pVoid );

	// �� ������ Highlight �ǰų� Normal �� �� �� ȣ��Ǵ� �Լ�
	// bHighlight �� ���̸� highlight �Ǿ���
	virtual void OnHighlight( HOTSPOT& , bool /*bHighlight*/ );

	// �Ҹ�
	virtual void OnClickSound();

protected:
	// ���� ���
	int m_nMode;
};

#endif // !defined(AFX_DSTARTUP_H__4643EA36_DCDA_11D2_9840_0000212035B8__INCLUDED_)
