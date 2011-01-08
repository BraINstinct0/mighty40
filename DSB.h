// DSB.h: interface for the DSB class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DSB_H__14D84F86_D8F9_11D2_982F_000000000000__INCLUDED_)
#define AFX_DSB_H__14D84F86_D8F9_11D2_982F_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CBoardWrap;
class CBmpMan;

// Shade Box class
// �� Ŭ������ CBoard ���� �� �ִ� ���� ���ڷ�,
// CBoard �� ����ؼ� ����Ѵ�

class DSB  
{
public:
	DSB( CBoardWrap* pBoard );
protected:
	virtual ~DSB();

public:
	// �� DSB �� ������ ID �� �����Ѵ�
	int GetID() const								{ return m_nID; }

	// �� DSB �� ������鼭 ��Ʈ�� �̺�Ʈ��,
	// ����� ���� �ּҸ� �����Ѵ�
	void SetAction( CEvent* pEvent = 0, void* pResult = 0 )	{ m_pEvent = pEvent; m_pResult = pResult; }

	// DSB �� �⺻ ������ ��Ʈ�Ѵ�
	static void SetDefaultColor(
		COLORREF colText = RGB(255,255,255),
		COLORREF colStrong1 = RGB(0,255,255),
		COLORREF colStrong2 = RGB(255,255,0),
		COLORREF colGray = RGB(192,192,192) );

public:
	// ���ο� ���ڸ� �����Ѵ�
	// CBoard �� �ڽ��� ����ϰ� ���� �׸���
	// ��ġ xDSB, yDSB �� �»�� �𼭸��� �ƴ϶�
	// ��ũ�� �߽ɿ��� ���� �߽��� ������� ����°�
	// �ϴ� ���̴� !!!
	void Create(
		int xDSB, int yDSB,				// ��ġ ( DSB ���� )
		int cxDSB, int cyDSB,			// DSB ������ ����� ũ��
		int nTimeOut );					// ���� (�и���:-1�� ����)

	// ���ο� ���ڸ� �����Ѵ�
	// CBoard �� �ڽ��� ����ϰ� ���� �׸���
	// x, y �� DP ��ǥ
	void Create2(
		int x, int y,					// ��ġ
		int cxDSB, int cyDSB,			// DSB ������ ����� ũ��
		int nTimeOut );					// ���� (�и���:-1�� ����)

	// ���ڸ� �����ϰ� �ڽ��� delete �Ѵ�
	// CBoard ���� ����� ����� ���� �׸���
	virtual void Destroy();

	// �� ���ڴ� ���� ũ�Ⱑ ���ص�
	// ũ�Ⱑ ������ �ʵ��� �����
	// ( Create ���� �� �Լ��� ȣ���ؾ� �Ѵ� )
	void SetFixed();

	// �� ���ڴ� modal ��忡�� �����Ѵ�
	void SetModal()									{ m_bModal = true; }
	bool IsModal()									{ return m_bModal; }
	// �� ���ڴ� Below ��忡�� �����Ѵ�
	void SetBelow()									{ m_bBelow = true; }
	bool IsBelow()									{ return m_bBelow; }

	// �� ������ ���� DP �簢�� ��ǥ�� ����Ѵ�
	void GetRect( LPRECT prc );

public:
	// CBoard �� ȣ���ϴ� �Լ���

	// ������ �׸� Bitmap manager �� ������ �ش�
	// ( Create ������ ȣ��� )
	void SetBmpMan( CBmpMan* pBM )					{ m_pBmpMan = pBM; }

	// ���콺�� �� �簢���� �� ��(CBoard���� ������ǥ)��
	// Ŭ���ϰų� ������ �� ������ ȿ���� �ش�
	// Ŭ��(bCheckOnly==true)�ߴٸ� �� ���� �̺�Ʈ��
	// ����ġ �ϸ�, ���콺�� �������ٸ�(bCheckOnly==false)
	// �� �� ������ ���̶���Ʈ �Ѵ�
	// ���� � �� ���̿� �ɷ� �ִٸ� ����,
	// �׷��� �ʴٸ� ������ �����Ѵ�
	virtual bool DispatchHotspot( int x, int y, bool bCheckOnly );

	// Ÿ�Ӿƿ��� ���� �ʰ� ������ �� �ִ� �Ӽ����� �����
	void SetPermanent()								{ m_nTimeOut = -1; }
	bool IsPermanent() const						{ return m_nTimeOut == -1; }

	// Ÿ�̸Ӹ� ��Ʈ ( SetPermanent �� �ݴ� �Լ� )
	void SetTimeout( int nTimeOut );

	// Ÿ�̸Ӹ� ���� (millisecond)
	int GetTimeOut() const							{ return m_nTimeOut; }

	// ��ǥ�� �̵���Ų��
	void Offset( POINT pnt )						{ Offset(pnt.x,pnt.y); }
	void Offset( SIZE pnt )							{ Offset(pnt.cx,pnt.cy); }
	void Offset( int x, int y );

	// Ư�� Ű�� ���� ����
	// ESC �� ������ x ��ư�� ���� ȿ���� ����
	virtual void OnESC()							{ OnClick( (LPVOID)0 ); }
	// Enter �� ������ 0xffffffff �� ���� ȿ���� ����
	virtual void OnEnter()							{ OnClick( (LPVOID)0xffffffff ); }

public:
	// ���� �Լ���

	// �ʱ�ȭ ( ������ �� CBoard �� ȣ���� )
	// ��µ� DSB ���� �� �Լ����� Hotspot ���� ����Ѵ�
	virtual void OnInit() {}

	// �̵��Ǿ��� �� CBoard �� ȣ���ϴ� �Լ�
	virtual void OnMove() {}

	// CBoard �� ȣ���ϴ� �׸��� �Լ� - ��µ� shade-box �� �� �Լ���
	// �������̵� �Ͽ� �����ؾ� �Ѵ�
	// ���̽� �Լ��� ������ ��ü�� ���� ���� ������ �׸���
	virtual void OnDraw(
		CDC* pDC,
		bool bBrushOrigin );

	// ��Ŀ���� ����� �� ȣ���ϴ� �Լ�
	// bFocus �� ���̸� ��Ŀ���� ���� �� ���̸�, �����̸� ��Ŀ���� ������ ���̴�
	virtual void OnFocus( bool /*bFocus*/ ) {}

	// �� ������ Ŭ���� �� �Ҹ��� �Լ� ( ������ ȣ���Ѵ� )
	// LPVOID ���� 0 �̸� ���� (X�ڸ� Ŭ��) ��û
protected:
	virtual void OnClick( LPVOID pVoid )			{ if ( !pVoid || pVoid == (LPVOID)0xffffffff ) Destroy(); }
	virtual void OnClickSound()						{ PlaySound( IDW_BEEP ); }

	// �� ������ Highlight �ǰų� Normal �� �� �� ȣ��Ǵ� �Լ�
	// bHighlight �� ���̸� highlight �Ǿ���
	struct HOTSPOT;
	virtual void OnHighlight( HOTSPOT& , bool /*bHighlight*/ ) {}

	// Destroy �� �� �Ҹ��� �Լ�
	virtual void OnDestroy()						{}

protected:
	// �ʱ�ȭ�� ��� �ʿ��� �Լ���

	// �ؽ�Ʈ �Ӽ� ���� ���
#define		TD_NORMAL		0
#define		TD_SIZESMALL	0	// ũ�� (��)
#define		TD_SIZEMIDIUM	1	// ũ�� (��)
#define		TD_SIZEBIG		2	// ũ�� (��)
#define		TD_SHADE		4	// �׸��� ȿ��
#define		TD_OUTLINE		8	// �ܰ��� (�帰 �����)
#define		TD_SUNKEN		16	// ��Ʈ���� ��ǫ ���� ȿ��
#define		TD_OPAQUE		32	// �������� �۾�

	// �� ������ ����Ѵ�
	// �� ������ Ŭ���ϸ� CBoard �� ������ ���� ������
	// �� Ŭ������ OnClick �� ȣ���Ѵ�
	// ��ġ�� ������� (Box�� �»���� 0,0)
	// DSB ��ǥ
	// ���ϰ��� hotspot �� �簢�� ���� ����� DP ��ǥ
	RECT RegisterHotspot(
		int x, int y,					// ��ġ
		int cx, int cy,					// ũ�� (-1,-1 �̸� �ڵ� ���)
		bool bDSB,						// ��ġ ��ǥ�� DSB ��ǥ�ΰ�
		UINT idBitmap, LPCTSTR s,		// �׸� ��Ʈ�ʰ� �� ���ڿ� (������ ���� 0 )
		const COLORREF* pCol,
		const int* pDeco,				// ���ڿ� ����� ��� & ũ��
		const COLORREF* pColSel,
		const int* pDecoSel,			// ���콺�� ������ ���� ����� ��� & ũ��
		LPVOID pVoid );					// OnClick �� �Ѱ����� ��

	// v4.0: �� ������ �����Ѵ� (2011.1.7)
	void DeleteHotspot( LPVOID pVoid );

	// �� ������ Update �ϰ� Invalidate �Ѵ�
	void UpdateHotspot( POSITION pos1, POSITION pos2 = 0 );

	// �־��� pVoid ���� ������ �� ������ ã�´�
	POSITION FindHotspot( LPVOID pVoid );

	// ������� DSB ��ǥ�� ������ ����̽� ��ǥ�� ��ȯ�Ѵ�
	POINT DSBtoDP( POINT ) const;
	RECT DSBtoDP( const RECT& ) const;

	// ���� DSB unit �� ��´�
	int GetDSBUnit() const;
	// ���� ��Ʈ�� ��´�
	CFont* GetFont( int nSize );
	// �� ��Ʈ���� ȭ�鿡���� ũ�⸦ ���Ѵ� (������ ��Ʈ ���)
	CSize GetTextExtent( int nSize, LPCTSTR s );

protected:
	// �׸��� �Լ���

	// ���ڸ� ����
	// ���ϰ��� �׷��� ���� (����DP)
	// ��, x �� y �� ������ �����̰� bDSB �� ���̸�
	// ��� ���� �Ǿ� �׷����� !
	RECT PutText(
		CDC* pDC,
		LPCTSTR s,				// �� ����
		int x, int y,			// ��ġ (���� DP)
		bool bDSB,				// x, y �� ������� DSB ��ǥ�ΰ�
		COLORREF col,			// ����
		int deco,				// ���� ��� & ũ��
		bool bCheckOnly = false );	// �׸����� �ʰ� ���� �������� ����ؼ� �����Ѵ�

	// ��Ʈ���� �׸��� �� ������ ���ڸ� ����
	// ���ϰ��� �׷��� ����
	// ��, x �� y �� ������ �����̰� bDSB �� ���̸�
	// ��� ���� �Ǿ� �׷����� !
	RECT PutBitmap(
		CDC* pDC,
		UINT id,				// �׸� ��Ʈ��
		LPCTSTR s,				// ��Ʈ�� ������ ���� ����
		int x, int y,			// ��ġ
		bool bDSB,				// x, y �� DSB ��ǥ�ΰ�
		COLORREF col,			// ����
		int deco,				// ���� ��� & ũ��
		bool bCheckOnly = false );	// �׸����� �ʰ� ���� �������� ����ؼ� �����Ѵ�

	// ����� �׸���
	// nDSBShadeMode : ĥ�ϱ� ��� ( 1 : ������ 2 : ����  �׿� : ��ġ )
	static void FillBack( CDC* pDC, LPCRECT prc,
		CBmpMan* pBmpMan, int nDSBShadeMode, bool bBrushOrigin );

	// �� ��Ʈ���� �� �ٷ� �ڸ���
	// cLine     : �� ���� �뷫���� ���� (DSB ����)
	// asLine    : 0 �� �ƴϸ� ���⿡ �� �κ� ��Ʈ���� �����Ѵ�
	// s         : �ڸ� �� ��Ʈ��
	// nMaxWidth : ������ �ʿ��� �� ������ �ִ� ���� ũ��
	// ���ϰ� -1 �� ����, �׹ۿ���, ������ �ʿ��� ���� ��
	int ParseString( LPTSTR* asLine, int cLine,
					LPCTSTR s, size_t& nMaxWidth );

protected:
	CPoint m_pntOffset;
	CRect m_rcDSB;
	CBoardWrap* m_pBoard;
	CBmpMan* m_pBmpMan;
	int m_nTimeOut;
	bool m_bModal;
	bool m_bBelow;
	POSITION m_posSel;	// ���õǾ� �ִ� �� ����
	struct HOTSPOT {
		CRect rc;		// ���� (��ü�� 10000,10000 ���� ���� ���� ����)
		UINT idBitmap;	// �׸� ��Ʈ��
		CString s;		// �׸� ���ڿ�
		const COLORREF* pCol;	// ���� ����
		const COLORREF* pColSel;// ���콺�� ������ ���� ���� ����
		const int* pDeco;		// ���� ��� & ũ��
		const int* pDecoSel;	// ���콺�� ������ ���� ���� ��� & ũ��
		LPVOID pVoid;	// �Ѱ� �ִ� ��
	};
	CList<HOTSPOT,HOTSPOT&> m_lHotspot;

	bool m_bFixed;

	int m_nID;

	CEvent* m_pEvent;
	void* m_pResult;

protected:
	// ���� ID ����Ʈ
	static CList<int,int> s_lID;
	// ������ ����Ʈ
	static CList<DSB*,DSB*> s_lPtr;
	// ���� �� ���� ������ CS
	static CCriticalSection s_csDSB;
	// s_lPtr �� �������� �ʰ� ���� ��� DSB �� ����� ����
	// ���� Ŭ����
	static struct GarbageCollector {
		virtual ~GarbageCollector() {
			s_csDSB.Lock();
			for ( POSITION pos = s_lPtr.GetHeadPosition(); pos; )
				delete s_lPtr.GetNext(pos);
			s_lPtr.RemoveAll();
			s_lID.RemoveAll();
			s_csDSB.Unlock();
		}
	} s_gc;

	friend struct DSB::GarbageCollector;

	// �̸� ���ǵ� ���� & ȿ����
	static COLORREF s_colWhite;
	static COLORREF s_colCyan;
	static COLORREF s_colYellow;
	static COLORREF s_colGray;
	static COLORREF s_colLightGray;
	static bool s_bColorLoaded;
	static const int s_tdNormal;
	static const int s_tdShade;
	static const int s_tdOutline;
	static const int s_tdShadeOutline;
	static const int s_tdShadeOpaque;
	static const int s_tdOutlineOpaque;
	static const int s_tdShadeOutlineOpaque;
	static const int s_tdMidium;
	static const int s_tdMidiumShade;
	static const int s_tdMidiumOutline;
	static const int s_tdMidiumShadeOutline;
};

#endif // !defined(AFX_DSB_H__14D84F86_D8F9_11D2_982F_000000000000__INCLUDED_)
