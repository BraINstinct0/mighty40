// DEtc.h: interface for the DEtc class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DETC_H__39C76F83_DE75_11D2_984A_000000000000__INCLUDED_)
#define AFX_DETC_H__39C76F83_DE75_11D2_984A_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// ����� DSB ���� ���⿡ ���� ���Ҵ�


/////////////////////////////////////////////////////////////////////////////
// about box
class DAbout : public DSB
{
public:
	DAbout( CBoardWrap* pBoard ) : DSB(pBoard) {}
	virtual ~DAbout() {}

	void Create();

protected:
	// �ʱ�ȭ ( ������ �� ȣ��� )
	virtual void OnInit();

	// CBoard �� ȣ���ϴ� �׸��� �Լ� - ��µ� shade-box �� �� �Լ���
	// �������̵� �Ͽ� �����ؾ� �Ѵ�
	// ���̽� �Լ��� ������ ��ü�� ���� ���� ������ �׸���
	virtual void OnDraw(
		CDC* pDC,
		bool bBrushOrigin );
};


/////////////////////////////////////////////////////////////////////////////
// message box
class DMessageBox : public DSB
{
public:
	DMessageBox( CBoardWrap* pBoard );
	virtual ~DMessageBox();

	// nLines ���� �޽����� ok ��ư�� ����Ѵ�
	// nTimeOut �� -1 �� �ƴϸ� ok ��ư�� ����
	void Create( bool bCenter, int nLines, LPCTSTR asLine[],
		int nTimeOut = -1 );

protected:
	// �ʱ�ȭ ( ������ �� ȣ��� )
	virtual void OnInit();

	// CBoard �� ȣ���ϴ� �׸��� �Լ� - ��µ� shade-box �� �� �Լ���
	// �������̵� �Ͽ� �����ؾ� �Ѵ�
	// ���̽� �Լ��� ������ ��ü�� ���� ���� ������ �׸���
	virtual void OnDraw(
		CDC* pDC,
		bool bBrushOrigin );

protected:
	bool m_bCenter;
	CString* m_asLine;
	int m_nLines;
};

/////////////////////////////////////////////////////////////////////////////
// short message box (ª�� �޽����� ��� ǥ���ߴٰ� �����)
// ��񵿾� ��Ʈ�� + ���ڿ� ��Ʈ���� ȭ���� �����
// ������ �ð����� ǥ���Ѵ�
class DShortMessage : public DSB
{
public:
	DShortMessage( CBoardWrap* pBoard ) : DSB(pBoard) {}
	virtual ~DShortMessage() {}

	// idBitmap : ��Ʈ�� ( ������ 0 )
	// s        : �޽��� ( ������ 0 )
	// bCenter  : �޽����� ������ ��� ��µǴ°�?
	// bFixed   : Fixed DSB �ΰ�
	// nTimeOut : Ÿ�Ӿƿ� (millisecond)
	// nPlayer  : ǥ�õ� �÷��̾� ��ȣ ( -1 �̸� ��� )
	// sName    : �� �÷��̾��� �̸� ( 0 �̸� �̸� ǥ�þȵ� )
	void Create( UINT idBitmap, LPCTSTR s,
		bool bCenter, bool bFixedSize,
		int nTimeOut, int nPlayer = -1, LPCTSTR sName = 0 );

protected:
	// CBoard �� ȣ���ϴ� �׸��� �Լ� - ��µ� shade-box �� �� �Լ���
	// �������̵� �Ͽ� �����ؾ� �Ѵ�
	// ���̽� �Լ��� ������ ��ü�� ���� ���� ������ �׸���
	virtual void OnDraw(
		CDC* pDC,
		bool bBrushOrigin );

protected:
	UINT m_idBitmap;
	CString m_s;
	bool m_bCenter;
	CString m_sName;
};


/////////////////////////////////////////////////////////////////////////////
// �� ���� �ؽ�Ʈ �߿��� �ϳ��� �����ϴ� ��� DSB
class DSelect : public DSB
{
public:
	DSelect( CBoardWrap* pBoard ) : DSB(pBoard) {}
	virtual ~DSelect() {}

	// x, y     : ���̾�αװ� ���ܾ� �� ��ġ (CBoard ������ǥ)
	// asText   : ������ �ؽ�Ʈ �迭
	// nText    : asText �� ������ ����
	// pResult  :���� ����� �ε��� (-1�� ���)
	void Create( int x, int y,
		LPCTSTR asText[], int nText, CEvent* pEvent, long* pResult );

protected:
	// �ʱ�ȭ ( ������ �� ȣ��� )
	virtual void OnInit();

	// �� ������ Ŭ���� �� �Ҹ��� �Լ� ( ������ ȣ���Ѵ� )
	// LPVOID ���� 0 �̸� ���� (X�ڸ� Ŭ��) ��û
	virtual void OnClick( LPVOID pVoid );

protected:
	LPCTSTR* m_asText;
	int m_nText;
};


/////////////////////////////////////////////////////////////////////////////
// select jokershape box ( ù ��Ŀ�� ����� �����ϴ� ����)
// �� DSB �� �÷��̾ ù ��Ŀ�� ���� ��
// �� ����� �����ϱ� ���� ǥ�õȴ�
// pShape �� ���ϵǴ� �� : 0 (���), �Ǵ� SPADE/DIAMOND/HEART/CLOVER
class DSelectJokerShape : public DSB
{
public:
	DSelectJokerShape( CBoardWrap* pBoard ) : DSB(pBoard) {}
	virtual ~DSelectJokerShape() {}

	// x, y           : ���̾�αװ� ���ܾ� �� ��ġ (CBoard ������ǥ)
	void Create( int x, int y,
		CEvent* pEvent, long* pShape);

protected:
	// �ʱ�ȭ ( ������ �� ȣ��� )
	virtual void OnInit();

	// �� ������ Ŭ���� �� �Ҹ��� �Լ� ( ������ ȣ���Ѵ� )
	// LPVOID ���� 0 �̸� ���� (X�ڸ� Ŭ��) ��û
	virtual void OnClick( LPVOID pVoid );
};


/////////////////////////////////////////////////////////////////////////////
// �÷��̾��� Chat ������ ����� �ִ� ���ڸ� ����

class DSay  : public DSB
{
public:
	DSay( CBoardWrap* pBoard );
	virtual ~DSay();

	// x, y  : ������ �߽� �� (DP ��ǥ)
	//         ���ڰ� pBoard�� Ŭ���̾�Ʈ ������ ���� �� ���
	//         �������� �����鿩 ���� ���߾� ����
	// cLine : �� �ٿ� ���� ������ �뷫���� �� (DSB����)
	//         (�� ���� �Ѿ�� �ٹٲ� ��)
	// sWho  : ���� ����� �̸�����, �� ���ٿ� ����ü�� ǥ�õȴ�
	// sMsg  : ���� ����
	// nTimeOut : ���� �ð� (millisecond)
	// pDSB  : Board �� �� �ִ� �� �÷��̾��� ���� ��ȭ ����
	// bSetBelow : �� ������ �� ���ΰ�
	void Create( DSay* pDSB, int x, int y,
		int cLine, LPCTSTR sWho, LPCTSTR sMsg, int nTimeOut,
		bool bSetBelow = false );

protected:
	// CBoard �� ȣ���ϴ� �׸��� �Լ� - ��µ� shade-box �� �� �Լ���
	// �������̵� �Ͽ� �����ؾ� �Ѵ�
	// ���̽� �Լ��� ������ ��ü�� ���� ���� ������ �׸���
	virtual void OnDraw(
		CDC* pDC,
		bool bBrushOrigin );

protected:
	// ���� ������ & �Լ�
	CString m_sWho;		// ���� ���
	TCHAR** m_asLine;	// ���� ���� (���� ������ �Ľ� �� ����)
	int m_nLines;		// ���� �迭�� ũ��

	CString	m_asLast[2];// ���� DSB �� ������ 2 ��
	int m_nLastLines;	// ���� DSB �� �����ΰ� (0~2)
};


#endif // !defined(AFX_DETC_H__39C76F83_DE75_11D2_984A_000000000000__INCLUDED_)
