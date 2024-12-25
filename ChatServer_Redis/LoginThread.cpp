#include "LoginThread.h"
#include "Player.h"
#include "SCCContents.h"
#include "en_ChatContentsType.h"
#include "LoginChatServer.h"
#include "RedisClientWrapper.h"

using namespace cpp_redis;

#pragma warning(disable : 26495)
LoginThread::LoginThread(GameServer* pGameServer)
	:ParallelContent{pGameServer}
{
}
#pragma warning(default : 26495)

void LoginThread::OnEnter(void* pPlayer)
{
}

void LoginThread::OnLeave(void* pPlayer)
{
}


void LoginThread::OnRecv(Packet* pPacket, void* pPlayer)
{
	Player* pAuthPlayer = (Player*)pPlayer;
	WORD Type;
	try
	{
		(*pPacket) >> Type;
		switch ((en_PACKET_TYPE)Type)
		{
		case en_PACKET_CS_CHAT_REQ_LOGIN:
			CS_CHAT_REQ_LOGIN(pPlayer, pPacket);
			break;
		case en_PACKET_CS_CHAT_REQ_HEARTBEAT:
			break;
		default:
			__debugbreak();
			break;
		}
	}
	catch (int errCode)
	{
		if (errCode == ERR_PACKET_EXTRACT_FAIL)
		{
			pGameServer_->Disconnect(pGameServer_->GetSessionID(pPlayer));
		}
		else if (errCode == ERR_PACKET_RESIZE_FAIL)
		{
			// ������ �̰� �����Ҹ��� ���� ��� ����͸�Ŭ�� ���Ӿ��ϸ� ������������ �ƿ� ���Ͼ��������
			LOG(L"RESIZE", ERR, TEXTFILE, L"Resize Fail ShutDown Server");
			__debugbreak();
		}
	}
	InterlockedIncrement(&static_cast<LoginChatServer*>(pGameServer_)->UPDATE_CNT_TPS);
}

void LoginThread::CS_CHAT_REQ_LOGIN(void* pPlayer, Packet* pPacket)
{
	Player* pLoginPlayer = (Player*)pPlayer;

	ULONGLONG sessionID = pGameServer_->GetSessionID(pPlayer);
	if (pGameServer_->lPlayerNum_ >= Player::MAX_PLAYER_NUM)
	{
		pGameServer_->Disconnect(sessionID);
		return;
	}

	INT64 accountNo;
	(*pPacket) >> accountNo;
	WCHAR* pID = (WCHAR*)pPacket->GetPointer(sizeof(WCHAR) * 20);
	WCHAR* pNickName = (WCHAR*)pPacket->GetPointer(sizeof(WCHAR) * 20);
	char* pSessionKey = pPacket->GetPointer(64);


	client* pClient = GetRedisClient();
	auto&& temp = pClient->get(std::to_string(accountNo));
	pClient->sync_commit();
	auto&& ret = temp._Get_value();

	// ���𽺿� ����Ű�� ���ų�, ���𽺿� ����� ����Ű�� �ٸ��ٸ� �α��� ����
	if (ret.is_null() || memcmp(pSessionKey, ret.as_string().c_str(), 64) != 0)
	{
		pGameServer_->Disconnect(sessionID);
		return;
	}

	// �÷��̾� �ʱ�ȭ �� �α��� ó��
	pLoginPlayer->bLogin_ = true;
	pLoginPlayer->bRegisterAtSector_ = false;
	pLoginPlayer->accountNo_ = accountNo;
	pLoginPlayer->sessionId_ = sessionID;
	wcscpy_s(pLoginPlayer->ID_, Player::ID_LEN, pID);
	wcscpy_s(pLoginPlayer->nickName_, Player::NICK_NAME_LEN, pNickName);
	RegisterLeave(pPlayer, (int)en_ChatContentsType::CHAT);
}

