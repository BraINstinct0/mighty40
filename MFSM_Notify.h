// MFSM ���� ����ϴ� ���� ��ũ�θ� �����Ѵ�

// redo �÷��� �����Ͽ� ���̸� �� �÷��� ���� EXPR�� ����
#define CALL_IF_WANT_REDO( PLAYERID, EXPR ) \
	do { \
		if ( m_abWaitingPlayerRedo[PLAYERID] ) { \
			m_abWaitingPlayerRedo[PLAYERID] = false; \
			EXPR; \
		} \
	} while (0)

// ��� �÷��̾�� �ϰ������� Notify �ϴ� ���� ��
// �� ���ο��� �� �÷��̾�鿡�� CALL_IF_WAIT_REDO �� ȣ���Ѵ�
#define NOTIFY_BLOCK_BEGIN() \
	ResetEvents(); \
	do {
#define NOTIFY_BLOCK_END() \
		Wait(); \
	} while ( m_nWaitingPlayer > 0 );

// ��� �÷��̾�� �˸��� (FUNC �� ȣ���� �Լ�)
#define PLAYERID _nPlayerID
#define EVENT m_apePlayer[PLAYERID]
#define NOTIFY_ALL( FUNC ) \
	do { \
		NOTIFY_BLOCK_BEGIN() \
		for ( int _nPlayerID = pRule->nPlayerNum-1; _nPlayerID >= 0; _nPlayerID-- ) \
			CALL_IF_WANT_REDO( _nPlayerID, apAllPlayers[_nPlayerID]->FUNC ); \
		NOTIFY_BLOCK_END() \
	} while (0)

// �ϳ��� �÷��̾�� �˸��� (FUNC �� ȣ���� �Լ�)
#define NOTIFY_TO( PLAYERNUM, FUNC ) \
	do { \
		int _nPlayerID = GetPlayerIDFromNum(PLAYERNUM); \
		int _nCurPlayerID = GetPlayerIDFromNum(nCurrentPlayer); \
		do { \
			ResetEvents(); \
			CALL_IF_WANT_REDO( _nPlayerID, apAllPlayers[_nPlayerID]->FUNC ); \
			int _nNotifiedPlayerID; \
			do { \
				_nNotifiedPlayerID = Wait(); \
				ASSERT( _nNotifiedPlayerID == -1 \
					|| _nNotifiedPlayerID == _nCurPlayerID ); \
			} while ( !m_abWaitingPlayerRedo[_nPlayerID] \
					&& _nNotifiedPlayerID != _nCurPlayerID ); \
		} while ( m_abWaitingPlayerRedo[_nPlayerID] ); \
	} while (0)

// �ϳ��� �÷��̾�� �˸��� (FUNC �� ȣ���� �Լ�)
#define NOTIFY_TO_ID( PLAYERID, CURPLAYERID, FUNC ) \
	do { \
		int _nPlayerID = PLAYERID; \
		int _nCurPlayerID = CURPLAYERID; \
		do { \
			ResetEvents(); \
			CALL_IF_WANT_REDO( _nPlayerID, apAllPlayers[_nPlayerID]->FUNC ); \
			int _nNotifiedPlayerID; \
			do { \
				_nNotifiedPlayerID = Wait(); \
				ASSERT( _nNotifiedPlayerID == -1 \
					|| _nNotifiedPlayerID == _nCurPlayerID ); \
			} while ( !m_abWaitingPlayerRedo[_nPlayerID] \
					&& _nNotifiedPlayerID != _nCurPlayerID ); \
		} while ( m_abWaitingPlayerRedo[_nPlayerID] ); \
	} while (0)
