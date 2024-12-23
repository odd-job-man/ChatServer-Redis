#pragma once
#include<windows.h>
#include "CLinkedList.h"
#define MYLIST_SECTOR

struct Player
{
	static constexpr int ID_LEN = 20;
	static constexpr int NICK_NAME_LEN = 20;
	static constexpr int SESSION_KEY_LEN = 64;
	static inline int MAX_PLAYER_NUM;
	static inline Player* pPlayerArr;
	static constexpr int INITIAL_SECTOR_VALUE = 51;

	bool bLogin_;
	bool bRegisterAtSector_;
	WORD sectorX_;
	WORD sectorY_;
	LINKED_NODE contentLink;
#ifdef MYLIST_SECTOR
	LINKED_NODE sectorLink;
#endif
	ULONGLONG sessionId_;
	INT64 accountNo_;
	WCHAR ID_[ID_LEN];
	WCHAR nickName_[NICK_NAME_LEN];
	// �� �ָ���
	Player()
		:
#ifdef MYLIST_SECTOR
		sectorLink{ offsetof(Player,sectorLink) },
#endif
		contentLink{ offsetof(Player,contentLink) }, bLogin_{ false }, bRegisterAtSector_{ false }
	{}

	__forceinline void LoginInit()
	{
		bLogin_ = true;
		bRegisterAtSector_ = false;
	}

	static WORD MAKE_PLAYER_INDEX(ULONGLONG sessionId)
	{
		return sessionId & 0xFFFF;
	}
};
