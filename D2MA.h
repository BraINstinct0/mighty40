// D2MA.h: interface for the all 2MA-related DSB class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_D2MA)
#define AFX_D2MA


/////////////////////////////////////////////////////////////////////////////
// ī�� ���� DSB

class DSelect2MA  : public DSB
{
public:
	DSelect2MA( CBoardWrap* pBoard ) : DSB(pBoard) {}
	virtual ~DSelect2MA() {}

	// pcToKill : �� DSB �� ��� ī�带 ������ ��� (���-�� ��������)
	// plShow   : ���̴� (���� �ִ�) ī��
	// plHide   : ������ (�Ʒ��� �ִ�) ī��
	void Create( CEvent* pEvent, int* selecting, CCard* pcShow );

protected:
	// �ʱ�ȭ ( ������ �� ȣ��� )
	virtual void OnInit();

	// CBoard �� ȣ���ϴ� �׸��� �Լ� - ��µ� shade-box �� �� �Լ���
	// �������̵� �Ͽ� �����ؾ� �Ѵ�
	// ���̽� �Լ��� ������ ��ü�� ���� ���� ������ �׸���
	virtual void OnDraw(
		CDC* pDC, bool bBrushOrigin );

	// �� ������ Ŭ���� �� �Ҹ��� �Լ�
	// LPVOID ���� 0 �̸� ���� (X�ڸ� Ŭ��) ��û
	virtual void OnClick( LPVOID pVoid );

	// Ȯ�� �Ҹ��� �ȳ���
	virtual void OnClickSound() {}

protected:
	// ���� ������ & �Լ�

	// ���õ� ī�� ������
	int* m_pselecting;
	// ���̴� ī��
	CCard* m_pcShow;
	// ���̴� ī�带 �׸� ������ �����Ѵ�
	void CalcShowCardRect( LPRECT pRc );
	// ������ ī�带 �׸� ������ �����Ѵ�
	void CalcHideCardRect( LPRECT pRc );
};

#endif // !defined(AFX_D2MA)