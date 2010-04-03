// BmpMan.h: interface for the CBmpMan class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BMPMAN_H__930A70A3_C49E_11D2_97F8_000000000000__INCLUDED_)
#define AFX_BMPMAN_H__930A70A3_C49E_11D2_97F8_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// ��Ʈ�� ������
// ��Ʈ�ʰ� ���õ� ��� ���� �Ѵ� !

class CBmpMan  
{
public:
	CBmpMan();
	virtual ~CBmpMan();

public:
	// �ε� �Լ�
	// �� �Լ��� ������ �ε�� �̹����� �����Ѵ�
	// �Է����� �ִ� DC �� ������ ��Ʈ���� ������ �Ǵ� DC �̴�
	// ���н� false ����

	// �ޱ׸��� ������ ��� ī�� �̹����� �о� ���δ�
	// �� �� ��Ʈ�� �Բ� ��Ʈ�Ѵ�
	// �� ��Ʈ�� ī�带 ��� & Ȯ���Ͽ� ����� �� ���ȴ�
	// ���� ũ���� ī�忡�� �� ��Ʈ�� �����
	// ǥ�� ũ���� ��Ʈ�� ����Ѵ�
	bool LoadAllCards( CDC* pDC, CFont* pFont );
	// �ޱ׸��� �д´�
	// nIndex �� �� �׸� �ε��� (0~10),
	// nIndex == -1 �϶� ���Ͽ��� �д´�
	bool LoadBackPicture( CDC* pDC, int nIndex, LPCTSTR sFileName = 0 );
	// ��� �׸��� �д´�
	bool LoadBackground( CDC* pDC, LPCTSTR sFileName );

public:
	// �׸��� �Լ�

	// ī�� �� ���� �׸���
	// nCard : int �� ġȯ�� ī��
	void DrawCard( CDC* pDC, int nCard, int x, int y, int cx, int cy );
	void DrawCard( CDC* pDC, int nCard, const CRect& rc );
	void DrawCardEx( CDC* pDC, int nCard,
		int xTgt, int yTgt, int cxTgt, int cyTgt,
		int xSrc, int ySrc, int cxSrc, int cySrc );
	void DrawCardEx( CDC* pDC, int nCard,
		const CRect& rcTgt, const CRect& rcSrc );

	// ��� �׸��� �׸���
	// �� �Լ��� Invalid ������ ���� ��� �׸� ������
	// �׸��� ���ؼ� ���ȴ�
	void DrawBackground( bool bUseBackground,
		bool bTile, bool bExpand, COLORREF colBack,
		CDC* pDC, int x, int y, int cx, int cy,
		int xView, int yView, int xOffset = 0, int yOffset = 0 );
	void DrawBackground( bool bUseBackground,
		bool bTile, bool bExpand, COLORREF colBack,
		CDC* pDC, const CRect& rc,
		int xView, int yView, int xOffset = 0, int yOffset = 0 );

	// ������ �׸���
	// bOrigin : �� ���� true �̸� 0,0 ����, false �̸� 1,0 ���� ������ �׸��� ^^
	void DrawShade( CDC* pDC, int x, int y, int cx, int cy, bool bOrigin );
	void DrawShade( CDC* pDC, const CRect& rc, bool bOrigin );

protected:
	// ���������� ���Ǵ� �Լ��� ������

	// ��� ī�� �׸��� �Ϸ� Ⱦ��� ��� �ִ�
	// Ŀ�ٶ� ��Ʈ�� : �޸� ��Ʈ�ʱ��� �����Ѵ�
	CBitmap m_bmCards;
	// ī�� ũ��
	CSize m_szCards;
	// ��� ��Ʈ��
	CBitmap m_bmBackground;
	// ��� ��Ʈ�� ũ��
	CSize m_szBackground;
	// Ȯ���� ��� ��Ʈ�� ( ��� Ȯ�� ��忡�� �׸��� �����°��� ���� )
	CBitmap m_bmExpandedBackground;
	// Ȯ���� ��� ��Ʈ�� ũ��
	CSize m_szExpandedBackground;
	// ��Ʈ
	CFont* m_pZoomFont;
	CFont m_font;
	// �׸��ڸ� ���� �귯���� ��Ʈ��
	CBitmap m_bmShadow;
	CBrush m_brShadow;
};

#endif // !defined(AFX_BMPMAN_H__930A70A3_C49E_11D2_97F8_000000000000__INCLUDED_)
