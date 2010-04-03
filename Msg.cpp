// Msg.cpp: implementation of the CMsg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mighty.h"
#include "Msg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


void CMsg::Init()
{
	m_nBufferLength = 64;
	m_pBuffer = new char[64];
	m_rptr = m_pBuffer + sizeof(long);
	m_wptr = m_pBuffer + sizeof(long);
	m_pLen = (long*)m_pBuffer;
	*m_pLen = sizeof(long);

	ASSERT( this != (CMsg*)0x00B32DE0 );
}

// �޽����� ������ �� printf ó�� ���� ��Ʈ������ ������
// �޽����� �����  ��: CMsg( _T("lls"), 10, 10, _T("haha") )
// ���� ��Ʈ���� 0 �̸� �� �޽����� �����Ѵ�
// l : long          s : LPCTSTR
// S : CString*      C : CCardList*
// k : checksum (���� ����)
CMsg::CMsg( LPCTSTR sFormat, ... )
{
	Init();
	if ( !sFormat ) return;

	va_list va;
	va_start( va, sFormat );
	for ( const TCHAR* p = sFormat; *p; p++ )
		switch (*p) {
		case _T('l'): PushLong(va_arg(va,long)); break;
		case _T('s'): PushString(va_arg(va,LPCTSTR)); break;
		case _T('S'): PushString(*va_arg(va,CString*)); break;
		case _T('C'): PushCardList(*va_arg(va,CCardList*)); break;
		case _T('k'): PushChecksum(); break;
		default: ASSERT(0); break;
	}
}

CMsg& CMsg::operator=( const CMsg& msg )
{
	memcpy( LockBuffer( msg.GetLength() ), msg.GetBuffer(), msg.GetLength() );
	m_rptr = msg.m_rptr;
	ReleaseBuffer();
	return *this;
}

CMsg::~CMsg()
{
	delete[] m_pBuffer;
}

// �޽��� ������ ���̸� �����ϰ� ������ �ּҸ� �����Ѵ�
void* CMsg::LockBuffer( int nLen )
{
	ASSERT( nLen >= sizeof(long) );
	EnsureBufferLength( nLen );
	*m_pLen = nLen;
	m_wptr = m_pBuffer+nLen;
	return (void*)m_pBuffer;
}

void CMsg::ReleaseBuffer()
{
}

// ����� �� �޽����ΰ� ( �� ���� üũ���� �˻��Ѵ� )
bool CMsg::IsValid() const
{
	long cs1 = *(long*)(m_pBuffer+GetLength()-sizeof(long));
	long cs2 = CalcChecksum( m_pBuffer+sizeof(long), GetLength()-sizeof(long)*2);
	return cs1 == cs2;
}

// üũ���� �� ���� push �Ѵ�
void CMsg::PushChecksum()
{
	PushLong( CalcChecksum( m_pBuffer+sizeof(long), GetLength()-sizeof(long) ) );
}

// ����, ���ڿ�, ī�� ����Ʈ�� ���� push �Ѵ�
void CMsg::PushLong( long n )
{
	EnsureBufferLength( GetLength() + sizeof(long) );
	*((long*)m_wptr) = n;
	m_wptr += sizeof(long);
	*m_pLen += sizeof(long);
}

void CMsg::PushString( LPCTSTR s )
{
#ifdef _UNICODE
	// �����ڵ� ���

	size_t nNeeded = _wcstombsz( 0, s, 0 );

	if ( nNeeded == (size_t)-1 ) {	// error
		ASSERT(0);
		PushString( _T("") );
		return;
	}

	char* buf = new char[nNeeded+1];
	_wcstombsz( buf, s, nNeeded+1 );

	int len = strlen( buf ) + 1;
	EnsureBufferLength( GetLength() + len );
	_mbscpy( m_wptr, buf );
	m_wptr += len;
	*m_pLen += len;

	delete[] buf;
#else
	// ��Ƽ ����Ʈ ���
	int len = strlen( (const char*)s ) + 1;
	EnsureBufferLength( GetLength() + len );
	strcpy( (char*)m_wptr, (const char*)s );
	m_wptr += len;
	*m_pLen += len;
#endif
}

void CMsg::PushCardList( const CCardList& lc )
{
	PushLong( lc.GetCount() );
	POSITION pos = lc.GetHeadPosition();
	while (pos)
		PushLong( (long)(int)lc.GetNext(pos) );
}

// ����, ���ڿ�, ī�� ����Ʈ�� ���� Pump �Ѵ�
// ���н� false ����
bool CMsg::PumpLong( long& l )
{
	if ( m_rptr + sizeof(long) > (const char*)m_wptr )
		return false;
	l = *(long*)m_rptr;
	m_rptr += sizeof(long);
	return true;
}

bool CMsg::PumpString( CString& s )
{
	const char* p;
	for ( p = m_rptr;
			*p && p < m_wptr; p++ );
	if ( *p ) return false;
	s = m_rptr;	// �ʿ��ϸ� CString �� �����ڵ�� �ڵ� ��ȯ�Ѵ�
	m_rptr = p + 1;
	return true;
}

bool CMsg::PumpCardList( CCardList& lc )
{
	const char* p = m_rptr;	// save
	long nCount;
	if ( !PumpLong( nCount ) ) return false;

	lc.RemoveAll();

	int i;
	for ( i = 0; i < nCount; i++ ) {
		long l;
		if ( !PumpLong(l) ) break;
		lc.AddTail( CCard(l) );
	}
	if ( i < nCount ) {	// fail
		m_rptr = p;
		return false;
	}
	else return true;
}

// ���۰� nMinLength ���� ������ ���۸� Ȯ���Ѵ�
void CMsg::EnsureBufferLength( int nMinLength )
{
	if ( m_nBufferLength < nMinLength ) {

		int nWPtr = m_wptr - m_pBuffer;	// m_wptr �� ��밪���� �����Ѵ�
		int nRPtr = m_rptr - m_pBuffer;

		// 64 ����Ʈ�� ������Ų��
		int nIncrease = ( nMinLength - m_nBufferLength - 1 ) / 64 * 64 + 64;
		m_nBufferLength += nIncrease;
		ASSERT( m_nBufferLength >= nMinLength );
		m_pBuffer = (char*)realloc( m_pBuffer, m_nBufferLength );

		ASSERT( m_pBuffer );

		// m_ptr �� ����
		m_wptr = m_pBuffer + nWPtr;
		m_rptr = m_pBuffer + nRPtr;

		m_pLen = (long*)m_pBuffer;
	}
}

// ������ �޸� ������ üũ���� ����Ѵ�
long CMsg::CalcChecksum( const char* p, int len )
{
	long cs = 0x55555555;
	for ( int i = 0; i < len; i++ )
		cs += (long)*(p+i);
	return cs;
}
