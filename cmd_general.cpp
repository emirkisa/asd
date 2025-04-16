#include "stdafx.h"
#ifdef __FreeBSD__
#include <md5.h>
#else
#include "../../libthecore/include/xmd5.h"
#endif

#include "utils.h"
#include "config.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "motion.h"
#include "packet.h"
#include "affect.h"
#include "pvp.h"
#include "start_position.h"
#include "party.h"
#include "guild_manager.h"
#include "p2p.h"
#include "dungeon.h"
#include "messenger_manager.h"
#include "war_map.h"
#include "questmanager.h"
#include "item_manager.h"
#include "monarch.h"
#include "mob_manager.h"
#include "dev_log.h"
#include "item.h"
#include "arena.h"
#include "buffer_manager.h"
#include "unique_item.h"
#include "threeway_war.h"
#include "log.h"
#include "../../common/VnumHelper.h"

#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
#include "MountSystem.h"
#endif

#ifdef BOSS_ARENA
#include "bossarena.h"
#endif

#ifdef ENABLE_OPPORTUNITY_SYSTEM
#include "opportunity.h"
#endif

#ifdef ENABLE_REMOTE_SHOP_SYSTEM
#include "shop.h"
#include "shop_manager.h"
#endif

#ifdef ENABLE_DEAL_OR_NO_DEAL
#include "dealornodeal.h"
#endif

#ifdef _ITEM_SHOP_SYSTEM
#include "item_shop.h"
#endif

#ifdef TITLE_SYSTEM

#include "title.h"
#ifdef ENABLE_BOT_CONTROL
#include "NewBotControl.h"
#endif
#ifdef FATE_ROULETTE
#include "fateroulette.h"
#endif
ACMD(do_title)
{

	if (!ch)
		return;

	if (ch->HaveAnotherPagesOpen())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Diger pencereleri kapatmadan bunu yapamazsin.");
		return;
	}

	if (ch->IsHack())
		return;

	int iPulse = thecore_pulse();
	if (iPulse - ch->GetExchangeTime() < PASSES_PER_SEC(g_nPortalLimitTime))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°Å·¡ ÈÄ %dÃÊ ÀÌ³»¿¡´Â ´Ù¸¥Áö¿ªÀ¸·Î ÀÌµ¿ ÇÒ ¼ö ¾ø½À´Ï´Ù."), g_nPortalLimitTime);
		return;
	}
	if (iPulse - ch->GetMyShopTime() < PASSES_PER_SEC(g_nPortalLimitTime))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°Å·¡ ÈÄ %dÃÊ ÀÌ³»¿¡´Â ´Ù¸¥Áö¿ªÀ¸·Î ÀÌµ¿ ÇÒ ¼ö ¾ø½À´Ï´Ù."), g_nPortalLimitTime);
		return;
	}

	char arg1[256];
	char arg2[256];

	DWORD id, type;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2 || !isdigit(*arg1) || !isdigit(*arg2))
	{
		return;
	}

	str_to_number(type, arg1);
	str_to_number(id, arg2);

	if (id < 0 || id > 5)
	{
		return;
	}

	if (type == 0)
	{
		CTitle::instance().ChangeTitle(ch, id);
	}
	else if (type == 1)
	{
		CTitle::instance().UpgradeTitle(ch, id);
	}
	else
	{
		CTitle::instance().Refresh(ch);
	}
	return;
}
#endif

ACMD(do_user_horse_ride)
{
	if (ch->IsObserverMode())
		return;

	if (ch->IsDead() || ch->IsStun())
		return;

	if (ch->IsHorseRiding() == false)
	{

		if (ch->GetMountVnum())
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì Å»°ÍÀ» ÀÌ¿ëÁßÀÔ´Ï´Ù."));
			return;
		}

		if (ch->GetHorse() == NULL)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»À» ¸ÕÀú ¼ÒÈ¯ÇØÁÖ¼¼¿ä."));
			return;
		}

		ch->StartRiding();
	}
	else
	{
		ch->StopRiding();
	}
}

#ifdef ENABLE_REMOTE_SHOP_SYSTEM
ACMD(do_open_range_npc)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	DWORD vnum = 0;
	str_to_number(vnum, arg1);

	if (ch->IsDead())
		return;

	if (ch->IsDead() || ch->GetExchange() || ch->GetMyShop() || ch->IsOpenSafebox() || ch->IsCubeOpen())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PLEASE_BEFORE_CLOSE_WINDOW_AND_USE_THIS_FUNCTION"));
		return;
	}


	LPSHOP shop = CShopManager::instance().Get(vnum);
	if (!shop) return;

	ch->SetShopOwner(ch);
	shop->AddGuest(ch, 0, false);
}
#endif

ACMD(do_user_horse_back)
{
	if (ch->GetHorse() != NULL)
	{
		ch->HorseSummon(false);
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»À» µ¹·Áº¸³Â½À´Ï´Ù."));
	}
	else if (ch->IsHorseRiding() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»¿¡¼­ ¸ÕÀú ³»·Á¾ß ÇÕ´Ï´Ù."));
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»À» ¸ÕÀú ¼ÒÈ¯ÇØÁÖ¼¼¿ä."));
	}
}

ACMD(do_user_horse_feed)
{

	if (ch->GetMyShop())
		return;

	if (ch->GetHorse() == NULL)
	{
		if (ch->IsHorseRiding() == false)
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»À» ¸ÕÀú ¼ÒÈ¯ÇØÁÖ¼¼¿ä."));
		else
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»À» Åº »óÅÂ¿¡¼­´Â ¸ÔÀÌ¸¦ ÁÙ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	DWORD dwFood = ch->GetHorseGrade() + 50054 - 1;

	if (ch->CountSpecifyItem(dwFood) > 0)
	{
		ch->RemoveSpecifyItem(dwFood, 1);
		ch->FeedHorse();
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»¿¡°Ô %s%s ÁÖ¾ú½À´Ï´Ù."),
				ITEM_MANAGER::instance().GetTable(dwFood)->szLocaleName,
				"");
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s ¾ÆÀÌÅÛÀÌ ÇÊ¿äÇÕ´Ï´Ù"), ITEM_MANAGER::instance().GetTable(dwFood)->szLocaleName);
	}
}


#define MAX_REASON_LEN		128

EVENTINFO(TimedEventInfo)
{
	DynamicCharacterPtr ch;
	int		subcmd;
	int         	left_second;
	char		szReason[MAX_REASON_LEN];

	TimedEventInfo()
	: ch()
	, subcmd( 0 )
	, left_second( 0 )
	{
		::memset( szReason, 0, MAX_REASON_LEN );
	}
};

struct SendDisconnectFunc
{
	void operator () (LPDESC d)
	{
		if (d->GetCharacter())
		{
			if (d->GetCharacter()->GetGMLevel() == GM_PLAYER)
				d->GetCharacter()->ChatPacket(CHAT_TYPE_COMMAND, "quit Shutdown(SendDisconnectFunc)");
		}
	}
};

struct DisconnectFunc
{
	void operator () (LPDESC d)
	{
		if (d->GetType() == DESC_TYPE_CONNECTOR)
			return;

		if (d->IsPhase(PHASE_P2P))
			return;

		if (d->GetCharacter())
			d->GetCharacter()->Disconnect("Shutdown(DisconnectFunc)");

		d->SetPhase(PHASE_CLOSE);
	}
};

EVENTINFO(shutdown_event_data)
{
	int seconds;

	shutdown_event_data()
	: seconds( 0 )
	{
	}
};

EVENTFUNC(shutdown_event)
{
	shutdown_event_data* info = dynamic_cast<shutdown_event_data*>( event->info );

	if ( info == NULL )
	{
		sys_err( "shutdown_event> <Factor> Null pointer" );
		return 0;
	}

	int * pSec = & (info->seconds);

	if (*pSec < 0)
	{
		sys_log(0, "shutdown_event sec %d", *pSec);

		if (--*pSec == -10)
		{
			const DESC_MANAGER::DESC_SET & c_set_desc = DESC_MANAGER::instance().GetClientSet();
			std::for_each(c_set_desc.begin(), c_set_desc.end(), DisconnectFunc());
			return passes_per_sec;
		}
		else if (*pSec < -10)
			return 0;

		return passes_per_sec;
	}
	else if (*pSec == 0)
	{
		const DESC_MANAGER::DESC_SET & c_set_desc = DESC_MANAGER::instance().GetClientSet();
		std::for_each(c_set_desc.begin(), c_set_desc.end(), SendDisconnectFunc());
		g_bNoMoreClient = true;
		--*pSec;
		return passes_per_sec;
	}
	else
	{
		char buf[64];
		snprintf(buf, sizeof(buf), LC_TEXT("¼Ë´Ù¿îÀÌ %dÃÊ ³²¾Ò½À´Ï´Ù."), *pSec);
		SendNotice(buf);

		--*pSec;
		return passes_per_sec;
	}
}

void Shutdown(int iSec)
{
	if (g_bNoMoreClient)
	{
		thecore_shutdown();
		return;
	}

	CWarMapManager::instance().OnShutdown();

	char buf[64];
	snprintf(buf, sizeof(buf), LC_TEXT("%dÃÊ ÈÄ °ÔÀÓÀÌ ¼Ë´Ù¿î µË´Ï´Ù."), iSec);

	SendNotice(buf);

	shutdown_event_data* info = AllocEventInfo<shutdown_event_data>();
	info->seconds = iSec;

	event_create(shutdown_event, info, 1);
}

ACMD(do_shutdown)
{
	if (NULL == ch)
	{
		sys_err("Accept shutdown command from %s.", ch->GetName());
	}
	TPacketGGShutdown p;
	p.bHeader = HEADER_GG_SHUTDOWN;
	P2P_MANAGER::instance().Send(&p, sizeof(TPacketGGShutdown));

	Shutdown(10);
}

EVENTFUNC(timed_event)
{
	TimedEventInfo * info = dynamic_cast<TimedEventInfo *>( event->info );

	if ( info == NULL )
	{
		sys_err( "timed_event> <Factor> Null pointer" );
		return 0;
	}

	LPCHARACTER	ch = info->ch;
	if (ch == NULL) { // <Factor>
		return 0;
	}
	LPDESC d = ch->GetDesc();

	if (info->left_second <= 0)
	{
		ch->m_pkTimedEvent = NULL;

		switch (info->subcmd)
		{
			case SCMD_LOGOUT:
			case SCMD_QUIT:
			case SCMD_PHASE_SELECT:
				{
					TPacketNeedLoginLogInfo acc_info;
					acc_info.dwPlayerID = ch->GetDesc()->GetAccountTable().id;

					db_clientdesc->DBPacket( HEADER_GD_VALID_LOGOUT, 0, &acc_info, sizeof(acc_info) );

					LogManager::instance().DetailLoginLog( false, ch );
				}
				break;
		}

		switch (info->subcmd)
		{
			case SCMD_LOGOUT:
				if (d)
					d->SetPhase(PHASE_CLOSE);
				break;

			case SCMD_QUIT:
				ch->ChatPacket(CHAT_TYPE_COMMAND, "quit");
				break;

			case SCMD_PHASE_SELECT:
				{
					ch->Disconnect("timed_event - SCMD_PHASE_SELECT");

					if (d)
					{
						d->SetPhase(PHASE_SELECT);
					}
				}
				break;
		}

		return 0;
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%dÃÊ ³²¾Ò½À´Ï´Ù."), info->left_second);
		--info->left_second;
	}

	return PASSES_PER_SEC(1);
}

ACMD(do_cmd)
{


	if (ch->m_pkTimedEvent)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Ãë¼Ò µÇ¾ú½À´Ï´Ù."));
		event_cancel(&ch->m_pkTimedEvent);
		return;
	}

	switch (subcmd)
	{
		case SCMD_LOGOUT:
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("·Î±×ÀÎ È­¸éÀ¸·Î µ¹¾Æ °©´Ï´Ù. Àá½Ã¸¸ ±â´Ù¸®¼¼¿ä."));
			break;

		case SCMD_QUIT:
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°ÔÀÓÀ» Á¾·á ÇÕ´Ï´Ù. Àá½Ã¸¸ ±â´Ù¸®¼¼¿ä."));
			break;

		case SCMD_PHASE_SELECT:
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Ä³¸¯ÅÍ¸¦ ÀüÈ¯ ÇÕ´Ï´Ù. Àá½Ã¸¸ ±â´Ù¸®¼¼¿ä."));
			break;
	}

	int nExitLimitTime = 10;

	if (ch->IsHack(false, true, nExitLimitTime) &&
		false == CThreeWayWar::instance().IsSungZiMapIndex(ch->GetMapIndex()) &&
	   	(!ch->GetWarMap() || ch->GetWarMap()->GetType() == GUILD_WAR_TYPE_FLAG))
	{
		return;
	}

	switch (subcmd)
	{
		case SCMD_LOGOUT:
		case SCMD_QUIT:
		case SCMD_PHASE_SELECT:
			{
				TimedEventInfo* info = AllocEventInfo<TimedEventInfo>();

				{
					if (ch->IsPosition(POS_FIGHTING))
						info->left_second = 10;
					else
						info->left_second = 3;
				}

				info->ch		= ch;
				info->subcmd		= subcmd;
				strlcpy(info->szReason, argument, sizeof(info->szReason));

				ch->m_pkTimedEvent	= event_create(timed_event, info, 1);
			}
			break;
	}
}

ACMD(do_mount)
{

}

ACMD(do_fishing)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	ch->SetRotation(atof(arg1));
	ch->fishing();
}

ACMD(do_console)
{
	ch->ChatPacket(CHAT_TYPE_COMMAND, "ConsoleEnable");
}

ACMD(do_restart)
{
	if (false == ch->IsDead())
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");
		ch->StartRecoveryEvent();
		return;
	}

	if (NULL == ch->m_pkDeadEvent)
		return;

	int iTimeToDead = (event_time(ch->m_pkDeadEvent) / passes_per_sec);

	if (subcmd != SCMD_RESTART_TOWN && (!ch->GetWarMap() || ch->GetWarMap()->GetType() == GUILD_WAR_TYPE_FLAG))
	{
		if (!test_server)
		{
			if (ch->IsHack())
			{

				if (false == CThreeWayWar::instance().IsSungZiMapIndex(ch->GetMapIndex()))
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÁ÷ Àç½ÃÀÛ ÇÒ ¼ö ¾ø½À´Ï´Ù. (%dÃÊ ³²À½)"), iTimeToDead - (180 - g_nPortalLimitTime));
					return;
				}
			}
#define eFRS_HERESEC	170
			if (iTimeToDead > eFRS_HERESEC)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÁ÷ Àç½ÃÀÛ ÇÒ ¼ö ¾ø½À´Ï´Ù. (%dÃÊ ³²À½)"), iTimeToDead - eFRS_HERESEC);
				return;
			}
		}
	}

	//PREVENT_HACK


	if (subcmd == SCMD_RESTART_TOWN)
	{
		if (ch->IsHack())
		{

			if ((!ch->GetWarMap() || ch->GetWarMap()->GetType() == GUILD_WAR_TYPE_FLAG) ||
			   	false == CThreeWayWar::instance().IsSungZiMapIndex(ch->GetMapIndex()))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÁ÷ Àç½ÃÀÛ ÇÒ ¼ö ¾ø½À´Ï´Ù. (%dÃÊ ³²À½)"), iTimeToDead - (180 - g_nPortalLimitTime));
				return;
			}
		}

#define eFRS_TOWNSEC	173
		if (iTimeToDead > eFRS_TOWNSEC)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÁ÷ ¸¶À»¿¡¼­ Àç½ÃÀÛ ÇÒ ¼ö ¾ø½À´Ï´Ù. (%d ÃÊ ³²À½)"), iTimeToDead - eFRS_TOWNSEC);
			return;
		}
	}
	//END_PREVENT_HACK

	ch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");

	ch->GetDesc()->SetPhase(PHASE_GAME);
	ch->SetPosition(POS_STANDING);
	ch->StartRecoveryEvent();

	//FORKED_LOAD

	if (1 == quest::CQuestManager::instance().GetEventFlag("threeway_war"))
	{
		if (subcmd == SCMD_RESTART_TOWN || subcmd == SCMD_RESTART_HERE)
		{
			if (true == CThreeWayWar::instance().IsThreeWayWarMapIndex(ch->GetMapIndex()) &&
					false == CThreeWayWar::instance().IsSungZiMapIndex(ch->GetMapIndex()))
			{
				ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));

#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
				ch->CheckMount();
#endif

				ch->ReviveInvisible(5);
				ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
				ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());

				return;
			}


			if (true == CThreeWayWar::instance().IsSungZiMapIndex(ch->GetMapIndex()))
			{
				if (CThreeWayWar::instance().GetReviveTokenForPlayer(ch->GetPlayerID()) <= 0)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ºÁö¿¡¼­ ºÎÈ° ±âÈ¸¸¦ ¸ðµÎ ÀÒ¾ú½À´Ï´Ù! ¸¶À»·Î ÀÌµ¿ÇÕ´Ï´Ù!"));
					ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));
				}
				else
				{
					ch->Show(ch->GetMapIndex(), GetSungziStartX(ch->GetEmpire()), GetSungziStartY(ch->GetEmpire()));
				}

				ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
				ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
				ch->CheckMount();
#endif
				ch->ReviveInvisible(5);

				return;
			}
		}
	}
	//END_FORKED_LOAD

	if (ch->GetDungeon())
		ch->GetDungeon()->UseRevive(ch);

	if (ch->GetWarMap() && !ch->IsObserverMode())
	{
		CWarMap * pMap = ch->GetWarMap();
		DWORD dwGuildOpponent = pMap ? pMap->GetGuildOpponent(ch) : 0;

		if (dwGuildOpponent)
		{
			switch (subcmd)
			{
				case SCMD_RESTART_TOWN:
					sys_log(0, "do_restart: restart town");
					PIXEL_POSITION pos;

					if (CWarMapManager::instance().GetStartPosition(ch->GetMapIndex(), ch->GetGuild()->GetID() < dwGuildOpponent ? 0 : 1, pos))
						ch->Show(ch->GetMapIndex(), pos.x, pos.y);
					else
						ch->ExitToSavedLocation();

					ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
					ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
					ch->ReviveInvisible(5);
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
					ch->CheckMount();
#endif
					break;

				case SCMD_RESTART_HERE:
					sys_log(0, "do_restart: restart here");
					ch->RestartAtSamePos();
					//ch->Show(ch->GetMapIndex(), ch->GetX(), ch->GetY());
					ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
					ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
					ch->ReviveInvisible(5);
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
					ch->CheckMount();
#endif
					break;
			}

			return;
		}
	}
	switch (subcmd)
	{
		case SCMD_RESTART_TOWN:
			sys_log(0, "do_restart: restart town");
			PIXEL_POSITION pos;

			if (SECTREE_MANAGER::instance().GetRecallPositionByEmpire(ch->GetMapIndex(), ch->GetEmpire(), pos))
				ch->WarpSet(pos.x, pos.y);
			else
				ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));
			ch->PointChange(POINT_HP, 50 - ch->GetHP());
			ch->DeathPenalty(1);
			break;

		case SCMD_RESTART_HERE:
			sys_log(0, "do_restart: restart here");
			ch->RestartAtSamePos();
			//ch->Show(ch->GetMapIndex(), ch->GetX(), ch->GetY());
			ch->PointChange(POINT_HP, 50 - ch->GetHP());
			ch->DeathPenalty(0);
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
				ch->CheckMount();
#endif
			ch->ReviveInvisible(5);
			break;
	}
}

#define MAX_STAT g_iStatusPointSetMaxValue

ACMD(do_stat_reset)
{
	ch->PointChange(POINT_STAT_RESET_COUNT, 12 - ch->GetPoint(POINT_STAT_RESET_COUNT));
}

ACMD(do_stat_minus)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	if (ch->IsPolymorphed())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("µÐ°© Áß¿¡´Â ´É·ÂÀ» ¿Ã¸± ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	if (ch->GetPoint(POINT_STAT_RESET_COUNT) <= 0)
		return;

	if (!strcmp(arg1, "st"))
	{
		if (ch->GetRealPoint(POINT_ST) <= JobInitialPoints[ch->GetJob()].st)
			return;

		ch->SetRealPoint(POINT_ST, ch->GetRealPoint(POINT_ST) - 1);
		ch->SetPoint(POINT_ST, ch->GetPoint(POINT_ST) - 1);
		ch->ComputePoints();
		ch->PointChange(POINT_ST, 0);
	}
	else if (!strcmp(arg1, "dx"))
	{
		if (ch->GetRealPoint(POINT_DX) <= JobInitialPoints[ch->GetJob()].dx)
			return;

		ch->SetRealPoint(POINT_DX, ch->GetRealPoint(POINT_DX) - 1);
		ch->SetPoint(POINT_DX, ch->GetPoint(POINT_DX) - 1);
		ch->ComputePoints();
		ch->PointChange(POINT_DX, 0);
	}
	else if (!strcmp(arg1, "ht"))
	{
		if (ch->GetRealPoint(POINT_HT) <= JobInitialPoints[ch->GetJob()].ht)
			return;

		ch->SetRealPoint(POINT_HT, ch->GetRealPoint(POINT_HT) - 1);
		ch->SetPoint(POINT_HT, ch->GetPoint(POINT_HT) - 1);
		ch->ComputePoints();
		ch->PointChange(POINT_HT, 0);
		ch->PointChange(POINT_MAX_HP, 0);
	}
	else if (!strcmp(arg1, "iq"))
	{
		if (ch->GetRealPoint(POINT_IQ) <= JobInitialPoints[ch->GetJob()].iq)
			return;

		ch->SetRealPoint(POINT_IQ, ch->GetRealPoint(POINT_IQ) - 1);
		ch->SetPoint(POINT_IQ, ch->GetPoint(POINT_IQ) - 1);
		ch->ComputePoints();
		ch->PointChange(POINT_IQ, 0);
		ch->PointChange(POINT_MAX_SP, 0);
	}
	else
		return;

	ch->PointChange(POINT_STAT, +1);
	ch->PointChange(POINT_STAT_RESET_COUNT, -1);
	ch->ComputePoints();
}

ACMD(do_stat)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	if (ch->IsPolymorphed())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("µÐ°© Áß¿¡´Â ´É·ÂÀ» ¿Ã¸± ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	if (ch->GetPoint(POINT_STAT) <= 0)
		return;

	BYTE idx = 0;

	if (!strcmp(arg1, "st"))
		idx = POINT_ST;
	else if (!strcmp(arg1, "dx"))
		idx = POINT_DX;
	else if (!strcmp(arg1, "ht"))
		idx = POINT_HT;
	else if (!strcmp(arg1, "iq"))
		idx = POINT_IQ;
	else
		return;

	// ch->ChatPacket(CHAT_TYPE_INFO, "%s GRP(%d) idx(%u), MAX_STAT(%d), expr(%d)", __FUNCTION__, ch->GetRealPoint(idx), idx, MAX_STAT, ch->GetRealPoint(idx) >= MAX_STAT);
	if (ch->GetRealPoint(idx) >= MAX_STAT)
		return;

	ch->SetRealPoint(idx, ch->GetRealPoint(idx) + 1);
	ch->SetPoint(idx, ch->GetPoint(idx) + 1);
	ch->ComputePoints();
	ch->PointChange(idx, 0);

	if (idx == POINT_IQ)
	{
		ch->PointChange(POINT_MAX_HP, 0);
	}
	else if (idx == POINT_HT)
	{
		ch->PointChange(POINT_MAX_SP, 0);
	}

	ch->PointChange(POINT_STAT, -1);
	ch->ComputePoints();
}

ACMD(do_pvp)
{
	if (ch->GetArena() != NULL || CArenaManager::instance().IsArenaMap(ch->GetMapIndex()) == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·ÃÀå¿¡¼­ »ç¿ëÇÏ½Ç ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	DWORD vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER pkVictim = CHARACTER_MANAGER::instance().Find(vid);

	if (!pkVictim)
		return;

	if (pkVictim->IsNPC())
		return;

	if (pkVictim->GetArena() != NULL)
	{
		pkVictim->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»ó´ë¹æÀÌ ´ë·ÃÁßÀÔ´Ï´Ù."));
		return;
	}

	CPVPManager::instance().Insert(ch, pkVictim);
}

ACMD(do_guildskillup)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	if (!ch->GetGuild())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±æµå¿¡ ¼ÓÇØÀÖÁö ¾Ê½À´Ï´Ù."));
		return;
	}

	CGuild* g = ch->GetGuild();
	TGuildMember* gm = g->GetMember(ch->GetPlayerID());
	if (gm->grade == GUILD_LEADER_GRADE)
	{
		DWORD vnum = 0;
		str_to_number(vnum, arg1);
		g->SkillLevelUp(vnum);
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±æµå ½ºÅ³ ·¹º§À» º¯°æÇÒ ±ÇÇÑÀÌ ¾ø½À´Ï´Ù."));
	}
}

ACMD(do_skillup)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	DWORD vnum = 0;
	str_to_number(vnum, arg1);

	if (true == ch->CanUseSkill(vnum))
	{
		ch->SkillLevelUp(vnum);
	}
	else
	{
		switch(vnum)
		{
			case SKILL_HORSE_WILDATTACK:
			case SKILL_HORSE_CHARGE:
			case SKILL_HORSE_ESCAPE:
			case SKILL_HORSE_WILDATTACK_RANGE:

			case SKILL_7_A_ANTI_TANHWAN:
			case SKILL_7_B_ANTI_AMSEOP:
			case SKILL_7_C_ANTI_SWAERYUNG:
			case SKILL_7_D_ANTI_YONGBI:

			case SKILL_8_A_ANTI_GIGONGCHAM:
			case SKILL_8_B_ANTI_YEONSA:
			case SKILL_8_C_ANTI_MAHWAN:
			case SKILL_8_D_ANTI_BYEURAK:

			case SKILL_ADD_HP:
			case SKILL_RESIST_PENETRATE:
				ch->SkillLevelUp(vnum);
				break;
		}
	}
}

//
//
ACMD(do_safebox_close)
{
	ch->CloseSafebox();
}

//
//
ACMD(do_safebox_password)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	ch->ReqSafeboxLoad(arg1);
}

ACMD(do_safebox_change_password)
{
	char arg1[256];
	char arg2[256];

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || strlen(arg1)>6)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> Àß¸øµÈ ¾ÏÈ£¸¦ ÀÔ·ÂÇÏ¼Ì½À´Ï´Ù."));
		return;
	}

	if (!*arg2 || strlen(arg2)>6)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> Àß¸øµÈ ¾ÏÈ£¸¦ ÀÔ·ÂÇÏ¼Ì½À´Ï´Ù."));
		return;
	}

	TSafeboxChangePasswordPacket p;

	p.dwID = ch->GetDesc()->GetAccountTable().id;
	strlcpy(p.szOldPassword, arg1, sizeof(p.szOldPassword));
	strlcpy(p.szNewPassword, arg2, sizeof(p.szNewPassword));

	db_clientdesc->DBPacket(HEADER_GD_SAFEBOX_CHANGE_PASSWORD, ch->GetDesc()->GetHandle(), &p, sizeof(p));
}

ACMD(do_mall_password)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1 || strlen(arg1) > 6)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> Àß¸øµÈ ¾ÏÈ£¸¦ ÀÔ·ÂÇÏ¼Ì½À´Ï´Ù."));
		return;
	}

	int iPulse = thecore_pulse();

	if (ch->GetMall())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> Ã¢°í°¡ ÀÌ¹Ì ¿­·ÁÀÖ½À´Ï´Ù."));
		return;
	}

	if (iPulse - ch->GetMallLoadTime() < passes_per_sec * 10)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> Ã¢°í¸¦ ´ÝÀºÁö 10ÃÊ ¾È¿¡´Â ¿­ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	ch->SetMallLoadTime(iPulse);

	TSafeboxLoadPacket p;
	p.dwID = ch->GetDesc()->GetAccountTable().id;
	strlcpy(p.szLogin, ch->GetDesc()->GetAccountTable().login, sizeof(p.szLogin));
	strlcpy(p.szPassword, arg1, sizeof(p.szPassword));

	db_clientdesc->DBPacket(HEADER_GD_MALL_LOAD, ch->GetDesc()->GetHandle(), &p, sizeof(p));
}

ACMD(do_mall_close)
{
	if (ch->GetMall())
	{
		ch->SetMallLoadTime(thecore_pulse());
		ch->CloseMall();
		ch->Save();
	}
}

ACMD(do_ungroup)
{
	if (!ch->GetParty())
		return;

	if (!CPartyManager::instance().IsEnablePCParty())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ¼­¹ö ¹®Á¦·Î ÆÄÆ¼ °ü·Ã Ã³¸®¸¦ ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	if (ch->GetDungeon())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ´øÀü ¾È¿¡¼­´Â ÆÄÆ¼¿¡¼­ ³ª°¥ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	LPPARTY pParty = ch->GetParty();

	if (pParty->GetMemberCount() == 2)
	{
		// party disband
		CPartyManager::instance().DeleteParty(pParty);
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ÆÄÆ¼¿¡¼­ ³ª°¡¼Ì½À´Ï´Ù."));
		//pParty->SendPartyRemoveOneToAll(ch);
		pParty->Quit(ch->GetPlayerID());
		//pParty->SendPartyRemoveAllToOne(ch);
	}
}

ACMD(do_close_shop)
{
	if (ch->GetMyShop())
	{
		ch->CloseMyShop();
		return;
	}
}

ACMD(do_set_walk_mode)
{
	ch->SetNowWalking(true);
	ch->SetWalking(true);
}

ACMD(do_set_run_mode)
{
	ch->SetNowWalking(false);
	ch->SetWalking(false);
}

ACMD(do_war)
{

	CGuild * g = ch->GetGuild();

	if (!g)
		return;


	if (g->UnderAnyWar())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ÀÌ¹Ì ´Ù¸¥ ÀüÀï¿¡ ÂüÀü Áß ÀÔ´Ï´Ù."));
		return;
	}


	char arg1[256], arg2[256];
	DWORD type = GUILD_WAR_TYPE_FIELD; //fixme102 base int modded uint
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
		return;

	if (*arg2)
	{
		str_to_number(type, arg2);

		if (type >= GUILD_WAR_TYPE_MAX_NUM)
			type = GUILD_WAR_TYPE_FIELD;
	}


	DWORD gm_pid = g->GetMasterPID();


	if (gm_pid != ch->GetPlayerID())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±æµåÀü¿¡ ´ëÇÑ ±ÇÇÑÀÌ ¾ø½À´Ï´Ù."));
		return;
	}


	CGuild * opp_g = CGuildManager::instance().FindGuildByName(arg1);

	if (!opp_g)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±×·± ±æµå°¡ ¾ø½À´Ï´Ù."));
		return;
	}


	switch (g->GetGuildWarState(opp_g->GetID()))
	{
		case GUILD_WAR_NONE:
			{
				if (opp_g->UnderAnyWar())
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> »ó´ë¹æ ±æµå°¡ ÀÌ¹Ì ÀüÀï Áß ÀÔ´Ï´Ù."));
					return;
				}

				int iWarPrice = KOR_aGuildWarInfo[type].iWarPrice;

				if (g->GetGuildMoney() < iWarPrice)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> Àüºñ°¡ ºÎÁ·ÇÏ¿© ±æµåÀüÀ» ÇÒ ¼ö ¾ø½À´Ï´Ù."));
					return;
				}

				if (opp_g->GetGuildMoney() < iWarPrice)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> »ó´ë¹æ ±æµåÀÇ Àüºñ°¡ ºÎÁ·ÇÏ¿© ±æµåÀüÀ» ÇÒ ¼ö ¾ø½À´Ï´Ù."));
					return;
				}
			}
			break;

		case GUILD_WAR_SEND_DECLARE:
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì ¼±ÀüÆ÷°í ÁßÀÎ ±æµåÀÔ´Ï´Ù."));
				return;
			}
			break;

		case GUILD_WAR_RECV_DECLARE:
			{
				if (opp_g->UnderAnyWar())
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> »ó´ë¹æ ±æµå°¡ ÀÌ¹Ì ÀüÀï Áß ÀÔ´Ï´Ù."));
					g->RequestRefuseWar(opp_g->GetID());
					return;
				}
			}
			break;

		case GUILD_WAR_RESERVE:
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ÀÌ¹Ì ÀüÀïÀÌ ¿¹¾àµÈ ±æµå ÀÔ´Ï´Ù."));
				return;
			}
			break;

		case GUILD_WAR_END:
			return;

		default:
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ÀÌ¹Ì ÀüÀï ÁßÀÎ ±æµåÀÔ´Ï´Ù."));
			g->RequestRefuseWar(opp_g->GetID());
			return;
	}

	if (!g->CanStartWar(type))
	{

		if (g->GetLadderPoint() == 0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ·¹´õ Á¡¼ö°¡ ¸ðÀÚ¶ó¼­ ±æµåÀüÀ» ÇÒ ¼ö ¾ø½À´Ï´Ù."));
			sys_log(0, "GuildWar.StartError.NEED_LADDER_POINT");
		}
		else if (g->GetMemberCount() < GUILD_WAR_MIN_MEMBER_COUNT)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±æµåÀüÀ» ÇÏ±â À§ÇØ¼± ÃÖ¼ÒÇÑ %d¸íÀÌ ÀÖ¾î¾ß ÇÕ´Ï´Ù."), GUILD_WAR_MIN_MEMBER_COUNT);
			sys_log(0, "GuildWar.StartError.NEED_MINIMUM_MEMBER[%d]", GUILD_WAR_MIN_MEMBER_COUNT);
		}
		else
		{
			sys_log(0, "GuildWar.StartError.UNKNOWN_ERROR");
		}
		return;
	}


	if (!opp_g->CanStartWar(GUILD_WAR_TYPE_FIELD))
	{
		if (opp_g->GetLadderPoint() == 0)
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> »ó´ë¹æ ±æµåÀÇ ·¹´õ Á¡¼ö°¡ ¸ðÀÚ¶ó¼­ ±æµåÀüÀ» ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		else if (opp_g->GetMemberCount() < GUILD_WAR_MIN_MEMBER_COUNT)
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> »ó´ë¹æ ±æµåÀÇ ±æµå¿ø ¼ö°¡ ºÎÁ·ÇÏ¿© ±æµåÀüÀ» ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	do
	{
		if (g->GetMasterCharacter() != NULL)
			break;

		CCI *pCCI = P2P_MANAGER::instance().FindByPID(g->GetMasterPID());

		if (pCCI != NULL)
			break;

		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> »ó´ë¹æ ±æµåÀÇ ±æµåÀåÀÌ Á¢¼ÓÁßÀÌ ¾Æ´Õ´Ï´Ù."));
		g->RequestRefuseWar(opp_g->GetID());
		return;

	} while (false);

	do
	{
		if (opp_g->GetMasterCharacter() != NULL)
			break;

		CCI *pCCI = P2P_MANAGER::instance().FindByPID(opp_g->GetMasterPID());

		if (pCCI != NULL)
			break;

		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> »ó´ë¹æ ±æµåÀÇ ±æµåÀåÀÌ Á¢¼ÓÁßÀÌ ¾Æ´Õ´Ï´Ù."));
		g->RequestRefuseWar(opp_g->GetID());
		return;

	} while (false);

	g->RequestDeclareWar(opp_g->GetID(), type);
}

#ifdef ENABLE_MULTI_STATUS
ACMD(do_stat_val)
{
	char	arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	int val = 0;
	str_to_number(val, arg2);
	
	if (!*arg1 || val <= 0)
		return;

	if (ch->IsPolymorphed())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot change your state as long as you are transformed."));
		return;
	}

	if (ch->GetPoint(POINT_STAT) <= 0)
		return;

	BYTE idx = 0;
	
	if (!strcmp(arg1, "st"))
		idx = POINT_ST;
	else if (!strcmp(arg1, "dx"))
		idx = POINT_DX;
	else if (!strcmp(arg1, "ht"))
		idx = POINT_HT;
	else if (!strcmp(arg1, "iq"))
		idx = POINT_IQ;
	else
		return;

	if (ch->GetRealPoint(idx) >= MAX_STAT)
		return;
	
	if (val > ch->GetPoint(POINT_STAT))
		val = ch->GetPoint(POINT_STAT);
	
	if (ch->GetRealPoint(idx) + val > MAX_STAT)
		val = MAX_STAT - ch->GetRealPoint(idx);

	ch->SetRealPoint(idx, ch->GetRealPoint(idx) + val);
	ch->SetPoint(idx, ch->GetPoint(idx) + val);
	ch->ComputePoints();
	ch->PointChange(idx, 0);

	if (idx == POINT_IQ)
		ch->PointChange(POINT_MAX_HP, 0);
	else if (idx == POINT_HT)
		ch->PointChange(POINT_MAX_SP, 0);

	ch->PointChange(POINT_STAT, -val);
	ch->ComputePoints();
}
#endif

ACMD(do_nowar)
{
	CGuild* g = ch->GetGuild();
	if (!g)
		return;

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	DWORD gm_pid = g->GetMasterPID();

	if (gm_pid != ch->GetPlayerID())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±æµåÀü¿¡ ´ëÇÑ ±ÇÇÑÀÌ ¾ø½À´Ï´Ù."));
		return;
	}

	CGuild* opp_g = CGuildManager::instance().FindGuildByName(arg1);

	if (!opp_g)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±×·± ±æµå°¡ ¾ø½À´Ï´Ù."));
		return;
	}

	g->RequestRefuseWar(opp_g->GetID());
}

ACMD(do_detaillog)
{
	ch->DetailLog();
}

ACMD(do_monsterlog)
{
	ch->ToggleMonsterLog();
}

ACMD(do_pkmode)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	BYTE mode = 0;
	str_to_number(mode, arg1);

	if (mode == PK_MODE_PROTECT)
		return;

	if (ch->GetLevel() < PK_PROTECT_LEVEL && mode != 0)
		return;

	ch->SetPKMode(mode);
}

ACMD(do_messenger_auth)
{
	if (ch->GetArena())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·ÃÀå¿¡¼­ »ç¿ëÇÏ½Ç ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
		return;

	char answer = LOWER(*arg1);
	// @fixme130 AuthToAdd void -> bool
	bool bIsDenied = answer != 'y';
	bool bIsAdded = MessengerManager::instance().AuthToAdd(ch->GetName(), arg2, bIsDenied); // DENY
	if (bIsAdded && bIsDenied)
	{
		LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(arg2);

		if (tch)
			tch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s ´ÔÀ¸·Î ºÎÅÍ Ä£±¸ µî·ÏÀ» °ÅºÎ ´çÇß½À´Ï´Ù."), ch->GetName());
	}

}

ACMD(do_setblockmode)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1)
	{
		BYTE flag = 0;
		str_to_number(flag, arg1);
		ch->SetBlockMode(flag);
	}
}

ACMD(do_unmount)
{
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
	if(ch->GetWear(WEAR_COSTUME_MOUNT))
	{
		CMountSystem* mountSystem = ch->GetMountSystem();
		LPITEM mount = ch->GetWear(WEAR_COSTUME_MOUNT);
		DWORD mobVnum = 0;
		
		if (!mountSystem && !mount)
			return;

		if(mount->GetValue(1) != 0)
			mobVnum = mount->GetValue(1);

		if (ch->GetMountVnum())
		{
			if(mountSystem->CountSummoned() == 0)
			{
				mountSystem->Unmount(mobVnum);
			}
		}
		return;
	}
#endif
	if (true == ch->UnEquipSpecialRideUniqueItem())
	{
		ch->RemoveAffect(AFFECT_MOUNT);
		ch->RemoveAffect(AFFECT_MOUNT_BONUS);

		if (ch->IsHorseRiding())
		{
			ch->StopRiding();
		}
	}
	else
	{
		ch->ChatPacket( CHAT_TYPE_INFO, LC_TEXT("ÀÎº¥Åä¸®°¡ ²Ë Â÷¼­ ³»¸± ¼ö ¾ø½À´Ï´Ù."));
	}

}

ACMD(do_observer_exit)
{
	if (ch->IsObserverMode())
	{
		if (ch->GetWarMap())
			ch->SetWarMap(NULL);

		if (ch->GetArena() != NULL || ch->GetArenaObserverMode() == true)
		{
			ch->SetArenaObserverMode(false);

			if (ch->GetArena() != NULL)
				ch->GetArena()->RemoveObserver(ch->GetPlayerID());

			ch->SetArena(NULL);
			ch->WarpSet(ARENA_RETURN_POINT_X(ch->GetEmpire()), ARENA_RETURN_POINT_Y(ch->GetEmpire()));
		}
		else
		{
			ch->ExitToSavedLocation();
		}
		ch->SetObserverMode(false);
	}
}

ACMD(do_view_equip)
{
	if (ch->GetGMLevel() <= GM_PLAYER)
		return;

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1)
	{
		DWORD vid = 0;
		str_to_number(vid, arg1);
		LPCHARACTER tch = CHARACTER_MANAGER::instance().Find(vid);

		if (!tch)
			return;

		if (!tch->IsPC())
			return;

		tch->SendEquipment(ch);
	}
}

ACMD(do_party_request)
{
	if (ch->GetArena())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·ÃÀå¿¡¼­ »ç¿ëÇÏ½Ç ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	if (ch->GetParty())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì ÆÄÆ¼¿¡ ¼ÓÇØ ÀÖÀ¸¹Ç·Î °¡ÀÔ½ÅÃ»À» ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	DWORD vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER tch = CHARACTER_MANAGER::instance().Find(vid);

	if (tch)
		if (!ch->RequestToParty(tch))
			ch->ChatPacket(CHAT_TYPE_COMMAND, "PartyRequestDenied");
}

ACMD(do_party_request_accept)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	DWORD vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER tch = CHARACTER_MANAGER::instance().Find(vid);

	if (tch)
		ch->AcceptToParty(tch);
}

ACMD(do_party_request_deny)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	DWORD vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER tch = CHARACTER_MANAGER::instance().Find(vid);

	if (tch)
		ch->DenyToParty(tch);
}

ACMD(do_monarch_warpto)
{
	if (!CMonarch::instance().IsMonarch(ch->GetPlayerID(), ch->GetEmpire()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±ºÁÖ¸¸ÀÌ »ç¿ë °¡´ÉÇÑ ±â´ÉÀÔ´Ï´Ù"));
		return;
	}


	if (!ch->IsMCOK(CHARACTER::MI_WARP))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%d ÃÊ°£ ÄðÅ¸ÀÓÀÌ Àû¿ëÁßÀÔ´Ï´Ù."), ch->GetMCLTime(CHARACTER::MI_WARP));
		return;
	}


	const int WarpPrice = 10000;


	if (!CMonarch::instance().IsMoneyOk(WarpPrice, ch->GetEmpire()))
	{
		int NationMoney = CMonarch::instance().GetMoney(ch->GetEmpire());
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±¹°í¿¡ µ·ÀÌ ºÎÁ·ÇÕ´Ï´Ù. ÇöÀç : %u ÇÊ¿ä±Ý¾× : %u"), NationMoney, WarpPrice);
		return;
	}

	int x = 0, y = 0;
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»ç¿ë¹ý: warpto <character name>"));
		return;
	}

	LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(arg1);

	if (!tch)
	{
		CCI * pkCCI = P2P_MANAGER::instance().Find(arg1);

		if (pkCCI)
		{
			if (pkCCI->bEmpire != ch->GetEmpire())
			{
				ch->ChatPacket (CHAT_TYPE_INFO, LC_TEXT("Å¸Á¦±¹ À¯Àú¿¡°Ô´Â ÀÌµ¿ÇÒ¼ö ¾ø½À´Ï´Ù"));
				return;
			}

			if (pkCCI->bChannel != g_bChannel)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÇØ´ç À¯Àú´Â %d Ã¤³Î¿¡ ÀÖ½À´Ï´Ù. (ÇöÀç Ã¤³Î %d)"), pkCCI->bChannel, g_bChannel);
				return;
			}
			if (!IsMonarchWarpZone(pkCCI->lMapIndex))
			{
				ch->ChatPacket (CHAT_TYPE_INFO, LC_TEXT("ÇØ´ç Áö¿ªÀ¸·Î ÀÌµ¿ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				return;
			}

			PIXEL_POSITION pos;

			if (!SECTREE_MANAGER::instance().GetCenterPositionOfMap(pkCCI->lMapIndex, pos))
				ch->ChatPacket(CHAT_TYPE_INFO, "Cannot find map (index %d)", pkCCI->lMapIndex);
			else
			{
				//ch->ChatPacket(CHAT_TYPE_INFO, "You warp to (%d, %d)", pos.x, pos.y);
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s ¿¡°Ô·Î ÀÌµ¿ÇÕ´Ï´Ù"), arg1);
				ch->WarpSet(pos.x, pos.y);


				CMonarch::instance().SendtoDBDecMoney(WarpPrice, ch->GetEmpire(), ch);


				ch->SetMC(CHARACTER::MI_WARP);
			}
		}
		else if (NULL == CHARACTER_MANAGER::instance().FindPC(arg1))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "There is no one by that name");
		}

		return;
	}
	else
	{
		if (tch->GetEmpire() != ch->GetEmpire())
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Å¸Á¦±¹ À¯Àú¿¡°Ô´Â ÀÌµ¿ÇÒ¼ö ¾ø½À´Ï´Ù"));
			return;
		}
		if (!IsMonarchWarpZone(tch->GetMapIndex()))
		{
			ch->ChatPacket (CHAT_TYPE_INFO, LC_TEXT("ÇØ´ç Áö¿ªÀ¸·Î ÀÌµ¿ÇÒ ¼ö ¾ø½À´Ï´Ù."));
			return;
		}
		x = tch->GetX();
		y = tch->GetY();
	}

	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s ¿¡°Ô·Î ÀÌµ¿ÇÕ´Ï´Ù"), arg1);
	ch->WarpSet(x, y);
	ch->Stop();


	CMonarch::instance().SendtoDBDecMoney(WarpPrice, ch->GetEmpire(), ch);


	ch->SetMC(CHARACTER::MI_WARP);
}

ACMD(do_monarch_transfer)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»ç¿ë¹ý: transfer <name>"));
		return;
	}

	if (!CMonarch::instance().IsMonarch(ch->GetPlayerID(), ch->GetEmpire()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±ºÁÖ¸¸ÀÌ »ç¿ë °¡´ÉÇÑ ±â´ÉÀÔ´Ï´Ù"));
		return;
	}


	if (!ch->IsMCOK(CHARACTER::MI_TRANSFER))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%d ÃÊ°£ ÄðÅ¸ÀÓÀÌ Àû¿ëÁßÀÔ´Ï´Ù."), ch->GetMCLTime(CHARACTER::MI_TRANSFER));
		return;
	}


	const int WarpPrice = 10000;


	if (!CMonarch::instance().IsMoneyOk(WarpPrice, ch->GetEmpire()))
	{
		int NationMoney = CMonarch::instance().GetMoney(ch->GetEmpire());
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±¹°í¿¡ µ·ÀÌ ºÎÁ·ÇÕ´Ï´Ù. ÇöÀç : %u ÇÊ¿ä±Ý¾× : %u"), NationMoney, WarpPrice);
		return;
	}


	LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(arg1);

	if (!tch)
	{
		CCI * pkCCI = P2P_MANAGER::instance().Find(arg1);

		if (pkCCI)
		{
			if (pkCCI->bEmpire != ch->GetEmpire())
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´Ù¸¥ Á¦±¹ À¯Àú´Â ¼ÒÈ¯ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				return;
			}
			if (pkCCI->bChannel != g_bChannel)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s ´ÔÀº %d Ã¤³Î¿¡ Á¢¼Ó Áß ÀÔ´Ï´Ù. (ÇöÀç Ã¤³Î: %d)"), arg1, pkCCI->bChannel, g_bChannel);
				return;
			}
			if (!IsMonarchWarpZone(pkCCI->lMapIndex))
			{
				ch->ChatPacket (CHAT_TYPE_INFO, LC_TEXT("ÇØ´ç Áö¿ªÀ¸·Î ÀÌµ¿ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				return;
			}
			if (!IsMonarchWarpZone(ch->GetMapIndex()))
			{
				ch->ChatPacket (CHAT_TYPE_INFO, LC_TEXT("ÇØ´ç Áö¿ªÀ¸·Î ¼ÒÈ¯ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				return;
			}

			TPacketGGTransfer pgg;

			pgg.bHeader = HEADER_GG_TRANSFER;
			strlcpy(pgg.szName, arg1, sizeof(pgg.szName));
			pgg.lX = ch->GetX();
			pgg.lY = ch->GetY();

			P2P_MANAGER::instance().Send(&pgg, sizeof(TPacketGGTransfer));
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s ´ÔÀ» ¼ÒÈ¯ÇÏ¿´½À´Ï´Ù."), arg1);


			CMonarch::instance().SendtoDBDecMoney(WarpPrice, ch->GetEmpire(), ch);

			ch->SetMC(CHARACTER::MI_TRANSFER);
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÔ·ÂÇÏ½Å ÀÌ¸§À» °¡Áø »ç¿ëÀÚ°¡ ¾ø½À´Ï´Ù."));
		}

		return;
	}


	if (ch == tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÚ½ÅÀ» ¼ÒÈ¯ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	if (tch->GetEmpire() != ch->GetEmpire())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´Ù¸¥ Á¦±¹ À¯Àú´Â ¼ÒÈ¯ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}
	if (!IsMonarchWarpZone(tch->GetMapIndex()))
	{
		ch->ChatPacket (CHAT_TYPE_INFO, LC_TEXT("ÇØ´ç Áö¿ªÀ¸·Î ÀÌµ¿ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}
	if (!IsMonarchWarpZone(ch->GetMapIndex()))
	{
		ch->ChatPacket (CHAT_TYPE_INFO, LC_TEXT("ÇØ´ç Áö¿ªÀ¸·Î ¼ÒÈ¯ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	//tch->Show(ch->GetMapIndex(), ch->GetX(), ch->GetY(), ch->GetZ());
	tch->WarpSet(ch->GetX(), ch->GetY(), ch->GetMapIndex());


	CMonarch::instance().SendtoDBDecMoney(WarpPrice, ch->GetEmpire(), ch);

	ch->SetMC(CHARACTER::MI_TRANSFER);
}

ACMD(do_monarch_info)
{
	if (CMonarch::instance().IsMonarch(ch->GetPlayerID(), ch->GetEmpire()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("³ªÀÇ ±ºÁÖ Á¤º¸"));
		TMonarchInfo * p = CMonarch::instance().GetMonarch();
		for (int n = 1; n < 4; ++n)
		{
			if (n == ch->GetEmpire())
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[%s±ºÁÖ] : %s  º¸À¯±Ý¾× %lld "), EMPIRE_NAME(n), p->name[n], p->money[n]);
			else
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[%s±ºÁÖ] : %s  "), EMPIRE_NAME(n), p->name[n]);

		}
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±ºÁÖ Á¤º¸"));
		TMonarchInfo * p = CMonarch::instance().GetMonarch();
		for (int n = 1; n < 4; ++n)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[%s±ºÁÖ] : %s  "), EMPIRE_NAME(n), p->name[n]);

		}
	}

}

ACMD(do_elect)
{
	db_clientdesc->DBPacketHeader(HEADER_GD_COME_TO_VOTE, ch->GetDesc()->GetHandle(), 0);
}

// LUA_ADD_GOTO_INFO
struct GotoInfo
{
	std::string 	st_name;

	BYTE 	empire;
	int 	mapIndex;
	DWORD 	x, y;

	GotoInfo()
	{
		st_name 	= "";
		empire 		= 0;
		mapIndex 	= 0;

		x = 0;
		y = 0;
	}

	GotoInfo(const GotoInfo& c_src)
	{
		__copy__(c_src);
	}

	void operator = (const GotoInfo& c_src)
	{
		__copy__(c_src);
	}

	void __copy__(const GotoInfo& c_src)
	{
		st_name 	= c_src.st_name;
		empire 		= c_src.empire;
		mapIndex 	= c_src.mapIndex;

		x = c_src.x;
		y = c_src.y;
	}
};

ACMD(do_monarch_tax)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: monarch_tax <1-50>");
		return;
	}


	if (!ch->IsMonarch())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±ºÁÖ¸¸ÀÌ »ç¿ëÇÒ¼ö ÀÖ´Â ±â´ÉÀÔ´Ï´Ù"));
		return;
	}


	int tax = 0;
	str_to_number(tax,  arg1);

	if (tax < 1 || tax > 50)
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("1-50 »çÀÌÀÇ ¼öÄ¡¸¦ ¼±ÅÃÇØÁÖ¼¼¿ä"));

	quest::CQuestManager::instance().SetEventFlag("trade_tax", tax);


	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼¼±ÝÀÌ %d %·Î ¼³Á¤µÇ¾ú½À´Ï´Ù"));


	char szMsg[1024];

	snprintf(szMsg, sizeof(szMsg), "±ºÁÖÀÇ ¸íÀ¸·Î ¼¼±ÝÀÌ %d %% ·Î º¯°æµÇ¾ú½À´Ï´Ù", tax);
	BroadcastNotice(szMsg);

	snprintf(szMsg, sizeof(szMsg), "¾ÕÀ¸·Î´Â °Å·¡ ±Ý¾×ÀÇ %d %% °¡ ±¹°í·Î µé¾î°¡°ÔµË´Ï´Ù.", tax);
	BroadcastNotice(szMsg);


	ch->SetMC(CHARACTER::MI_TAX);
}

static const DWORD cs_dwMonarchMobVnums[] =
{
	191,
	192,
	193,
	194,
	391,
	392,
	393,
	394,
	491,
	492,
	493,
	494,
	591,
	691,
	791,
	1304,
	1901,
	2091,
	2191,
	2206,
	0,
};

ACMD(do_monarch_mob)
{
	char arg1[256];
	LPCHARACTER	tch;

	one_argument(argument, arg1, sizeof(arg1));

	if (!ch->IsMonarch())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±ºÁÖ¸¸ÀÌ »ç¿ëÇÒ¼ö ÀÖ´Â ±â´ÉÀÔ´Ï´Ù"));
		return;
	}

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: mmob <mob name>");
		return;
	}

#ifdef ENABLE_MONARCH_MOB_CMD_MAP_CHECK // @warme006
	BYTE pcEmpire = ch->GetEmpire();
	BYTE mapEmpire = SECTREE_MANAGER::instance().GetEmpireFromMapIndex(ch->GetMapIndex());
	if (mapEmpire != pcEmpire && mapEmpire != 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÚ±¹ ¿µÅä¿¡¼­¸¸ »ç¿ëÇÒ ¼ö ÀÖ´Â ±â´ÉÀÔ´Ï´Ù"));
		return;
	}
#endif


	const int SummonPrice = 5000000;


	if (!ch->IsMCOK(CHARACTER::MI_SUMMON))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%d ÃÊ°£ ÄðÅ¸ÀÓÀÌ Àû¿ëÁßÀÔ´Ï´Ù."), ch->GetMCLTime(CHARACTER::MI_SUMMON));
		return;
	}


	if (!CMonarch::instance().IsMoneyOk(SummonPrice, ch->GetEmpire()))
	{
		int NationMoney = CMonarch::instance().GetMoney(ch->GetEmpire());
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±¹°í¿¡ µ·ÀÌ ºÎÁ·ÇÕ´Ï´Ù. ÇöÀç : %u ÇÊ¿ä±Ý¾× : %u"), NationMoney, SummonPrice);
		return;
	}

	const CMob * pkMob;
	DWORD vnum = 0;

	if (isdigit(*arg1))
	{
		str_to_number(vnum, arg1);

		if ((pkMob = CMobManager::instance().Get(vnum)) == NULL)
			vnum = 0;
	}
	else
	{
		pkMob = CMobManager::Instance().Get(arg1, true);

		if (pkMob)
			vnum = pkMob->m_table.dwVnum;
	}

	DWORD count;


	for (count = 0; cs_dwMonarchMobVnums[count] != 0; ++count)
		if (cs_dwMonarchMobVnums[count] == vnum)
			break;

	if (0 == cs_dwMonarchMobVnums[count])
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ÒÈ¯ÇÒ¼ö ¾ø´Â ¸ó½ºÅÍ ÀÔ´Ï´Ù. ¼ÒÈ¯°¡´ÉÇÑ ¸ó½ºÅÍ´Â È¨ÆäÀÌÁö¸¦ ÂüÁ¶ÇÏ¼¼¿ä"));
		return;
	}

	tch = CHARACTER_MANAGER::instance().SpawnMobRange(vnum,
			ch->GetMapIndex(),
			ch->GetX() - number(200, 750),
			ch->GetY() - number(200, 750),
			ch->GetX() + number(200, 750),
			ch->GetY() + number(200, 750),
			true,
			pkMob->m_table.bType == CHAR_TYPE_STONE,
			true);

	if (tch)
	{

		CMonarch::instance().SendtoDBDecMoney(SummonPrice, ch->GetEmpire(), ch);


		ch->SetMC(CHARACTER::MI_SUMMON);
	}
}

static const char* FN_point_string(int apply_number)
{
	switch (apply_number)
	{
		case POINT_MAX_HP:	return LC_TEXT("ÃÖ´ë »ý¸í·Â +%d");
		case POINT_MAX_SP:	return LC_TEXT("ÃÖ´ë Á¤½Å·Â +%d");
		case POINT_HT:		return LC_TEXT("Ã¼·Â +%d");
		case POINT_IQ:		return LC_TEXT("Áö´É +%d");
		case POINT_ST:		return LC_TEXT("±Ù·Â +%d");
		case POINT_DX:		return LC_TEXT("¹ÎÃ¸ +%d");
		case POINT_ATT_SPEED:	return LC_TEXT("°ø°Ý¼Óµµ +%d");
		case POINT_MOV_SPEED:	return LC_TEXT("ÀÌµ¿¼Óµµ %d");
		case POINT_CASTING_SPEED:	return LC_TEXT("ÄðÅ¸ÀÓ -%d");
		case POINT_HP_REGEN:	return LC_TEXT("»ý¸í·Â È¸º¹ +%d");
		case POINT_SP_REGEN:	return LC_TEXT("Á¤½Å·Â È¸º¹ +%d");
		case POINT_POISON_PCT:	return LC_TEXT("µ¶°ø°Ý %d");
#ifdef ENABLE_WOLFMAN_CHARACTER
		case POINT_BLEEDING_PCT:	return LC_TEXT("µ¶°ø°Ý %d");
#endif
		case POINT_STUN_PCT:	return LC_TEXT("½ºÅÏ +%d");
		case POINT_SLOW_PCT:	return LC_TEXT("½½·Î¿ì +%d");
		case POINT_CRITICAL_PCT:	return LC_TEXT("%d%% È®·ü·Î Ä¡¸íÅ¸ °ø°Ý");
		case POINT_RESIST_CRITICAL:	return LC_TEXT("»ó´ëÀÇ Ä¡¸íÅ¸ È®·ü %d%% °¨¼Ò");
		case POINT_PENETRATE_PCT:	return LC_TEXT("%d%% È®·ü·Î °üÅë °ø°Ý");
		case POINT_RESIST_PENETRATE: return LC_TEXT("»ó´ëÀÇ °üÅë °ø°Ý È®·ü %d%% °¨¼Ò");
		case POINT_ATTBONUS_HUMAN:	return LC_TEXT("ÀÎ°£·ù ¸ó½ºÅÍ Å¸°ÝÄ¡ +%d%%");
		case POINT_ATTBONUS_ANIMAL:	return LC_TEXT("µ¿¹°·ù ¸ó½ºÅÍ Å¸°ÝÄ¡ +%d%%");
		case POINT_ATTBONUS_ORC:	return LC_TEXT("¿õ±ÍÁ· Å¸°ÝÄ¡ +%d%%");
		case POINT_ATTBONUS_MILGYO:	return LC_TEXT("¹Ð±³·ù Å¸°ÝÄ¡ +%d%%");
		case POINT_ATTBONUS_UNDEAD:	return LC_TEXT("½ÃÃ¼·ù Å¸°ÝÄ¡ +%d%%");
		case POINT_ATTBONUS_DEVIL:	return LC_TEXT("¾Ç¸¶·ù Å¸°ÝÄ¡ +%d%%");
		case POINT_STEAL_HP:		return LC_TEXT("Å¸°ÝÄ¡ %d%% ¸¦ »ý¸í·ÂÀ¸·Î Èí¼ö");
		case POINT_STEAL_SP:		return LC_TEXT("Å¸·ÂÄ¡ %d%% ¸¦ Á¤½Å·ÂÀ¸·Î Èí¼ö");
		case POINT_MANA_BURN_PCT:	return LC_TEXT("%d%% È®·ü·Î Å¸°Ý½Ã »ó´ë Àü½Å·Â ¼Ò¸ð");
		case POINT_DAMAGE_SP_RECOVER:	return LC_TEXT("%d%% È®·ü·Î ÇÇÇØ½Ã Á¤½Å·Â È¸º¹");
		case POINT_BLOCK:			return LC_TEXT("¹°¸®Å¸°Ý½Ã ºí·° È®·ü %d%%");
		case POINT_DODGE:			return LC_TEXT("È° °ø°Ý È¸ÇÇ È®·ü %d%%");
		case POINT_RESIST_SWORD:	return LC_TEXT("ÇÑ¼Õ°Ë ¹æ¾î %d%%");
		case POINT_RESIST_TWOHAND:	return LC_TEXT("¾ç¼Õ°Ë ¹æ¾î %d%%");
		case POINT_RESIST_DAGGER:	return LC_TEXT("µÎ¼Õ°Ë ¹æ¾î %d%%");
		case POINT_RESIST_BELL:		return LC_TEXT("¹æ¿ï ¹æ¾î %d%%");
		case POINT_RESIST_FAN:		return LC_TEXT("ºÎÃ¤ ¹æ¾î %d%%");
		case POINT_RESIST_BOW:		return LC_TEXT("È°°ø°Ý ÀúÇ× %d%%");
#ifdef ENABLE_WOLFMAN_CHARACTER
		case POINT_RESIST_CLAW:		return LC_TEXT("µÎ¼Õ°Ë ¹æ¾î %d%%");
#endif
		case POINT_RESIST_FIRE:		return LC_TEXT("È­¿° ÀúÇ× %d%%");
		case POINT_RESIST_ELEC:		return LC_TEXT("Àü±â ÀúÇ× %d%%");
		case POINT_RESIST_MAGIC:	return LC_TEXT("¸¶¹ý ÀúÇ× %d%%");
#ifdef ENABLE_MAGIC_REDUCTION_SYSTEM
		case POINT_RESIST_MAGIC_REDUCTION:	return LC_TEXT("¸¶¹ý ÀúÇ× %d%%");
#endif
		case POINT_RESIST_WIND:		return LC_TEXT("¹Ù¶÷ ÀúÇ× %d%%");
		case POINT_RESIST_ICE:		return LC_TEXT("³Ã±â ÀúÇ× %d%%");
		case POINT_RESIST_EARTH:	return LC_TEXT("´ëÁö ÀúÇ× %d%%");
		case POINT_RESIST_DARK:		return LC_TEXT("¾îµÒ ÀúÇ× %d%%");
		case POINT_REFLECT_MELEE:	return LC_TEXT("Á÷Á¢ Å¸°ÝÄ¡ ¹Ý»ç È®·ü : %d%%");
		case POINT_REFLECT_CURSE:	return LC_TEXT("ÀúÁÖ µÇµ¹¸®±â È®·ü %d%%");
		case POINT_POISON_REDUCE:	return LC_TEXT("µ¶ ÀúÇ× %d%%");
#ifdef ENABLE_WOLFMAN_CHARACTER
		case POINT_BLEEDING_REDUCE:	return LC_TEXT("µ¶ ÀúÇ× %d%%");
#endif
		case POINT_KILL_SP_RECOVER:	return LC_TEXT("%d%% È®·ü·Î ÀûÅðÄ¡½Ã Á¤½Å·Â È¸º¹");
		case POINT_EXP_DOUBLE_BONUS:	return LC_TEXT("%d%% È®·ü·Î ÀûÅðÄ¡½Ã °æÇèÄ¡ Ãß°¡ »ó½Â");
		case POINT_GOLD_DOUBLE_BONUS:	return LC_TEXT("%d%% È®·ü·Î ÀûÅðÄ¡½Ã µ· 2¹è µå·Ó");
		case POINT_ITEM_DROP_BONUS:	return LC_TEXT("%d%% È®·ü·Î ÀûÅðÄ¡½Ã ¾ÆÀÌÅÛ 2¹è µå·Ó");
		case POINT_POTION_BONUS:	return LC_TEXT("¹°¾à »ç¿ë½Ã %d%% ¼º´É Áõ°¡");
		case POINT_KILL_HP_RECOVERY:	return LC_TEXT("%d%% È®·ü·Î ÀûÅðÄ¡½Ã »ý¸í·Â È¸º¹");
//		case POINT_SKILL:	return LC_TEXT("");
//		case POINT_BOW_DISTANCE:	return LC_TEXT("");
		case POINT_ATT_GRADE_BONUS:	return LC_TEXT("°ø°Ý·Â +%d");
		case POINT_DEF_GRADE_BONUS:	return LC_TEXT("¹æ¾î·Â +%d");
		case POINT_MAGIC_ATT_GRADE:	return LC_TEXT("¸¶¹ý °ø°Ý·Â +%d");
		case POINT_MAGIC_DEF_GRADE:	return LC_TEXT("¸¶¹ý ¹æ¾î·Â +%d");
//		case POINT_CURSE_PCT:	return LC_TEXT("");
		case POINT_MAX_STAMINA:	return LC_TEXT("ÃÖ´ë Áö±¸·Â +%d");
		case POINT_ATTBONUS_WARRIOR:	return LC_TEXT("¹«»ç¿¡°Ô °­ÇÔ +%d%%");
		case POINT_ATTBONUS_ASSASSIN:	return LC_TEXT("ÀÚ°´¿¡°Ô °­ÇÔ +%d%%");
		case POINT_ATTBONUS_SURA:		return LC_TEXT("¼ö¶ó¿¡°Ô °­ÇÔ +%d%%");
		case POINT_ATTBONUS_SHAMAN:		return LC_TEXT("¹«´ç¿¡°Ô °­ÇÔ +%d%%");
#ifdef ENABLE_WOLFMAN_CHARACTER
		case POINT_ATTBONUS_WOLFMAN:	return LC_TEXT("¹«´ç¿¡°Ô °­ÇÔ +%d%%");
#endif
		case POINT_ATTBONUS_MONSTER:	return LC_TEXT("¸ó½ºÅÍ¿¡°Ô °­ÇÔ +%d%%");
		case POINT_MALL_ATTBONUS:		return LC_TEXT("°ø°Ý·Â +%d%%");
		case POINT_MALL_DEFBONUS:		return LC_TEXT("¹æ¾î·Â +%d%%");
		case POINT_MALL_EXPBONUS:		return LC_TEXT("°æÇèÄ¡ %d%%");
		case POINT_MALL_ITEMBONUS:		return LC_TEXT("¾ÆÀÌÅÛ µå·ÓÀ² %d¹è"); // @fixme180 float to int
		case POINT_MALL_GOLDBONUS:		return LC_TEXT("µ· µå·ÓÀ² %d¹è"); // @fixme180 float to int
		case POINT_MAX_HP_PCT:			return LC_TEXT("ÃÖ´ë »ý¸í·Â +%d%%");
		case POINT_MAX_SP_PCT:			return LC_TEXT("ÃÖ´ë Á¤½Å·Â +%d%%");
		case POINT_SKILL_DAMAGE_BONUS:	return LC_TEXT("½ºÅ³ µ¥¹ÌÁö %d%%");
		case POINT_NORMAL_HIT_DAMAGE_BONUS:	return LC_TEXT("ÆòÅ¸ µ¥¹ÌÁö %d%%");
		case POINT_SKILL_DEFEND_BONUS:		return LC_TEXT("½ºÅ³ µ¥¹ÌÁö ÀúÇ× %d%%");
		case POINT_NORMAL_HIT_DEFEND_BONUS:	return LC_TEXT("ÆòÅ¸ µ¥¹ÌÁö ÀúÇ× %d%%");
//		case POINT_PC_BANG_EXP_BONUS:	return LC_TEXT("");
//		case POINT_PC_BANG_DROP_BONUS:	return LC_TEXT("");
//		case POINT_EXTRACT_HP_PCT:	return LC_TEXT("");
		case POINT_RESIST_WARRIOR:	return LC_TEXT("¹«»ç°ø°Ý¿¡ %d%% ÀúÇ×");
		case POINT_RESIST_ASSASSIN:	return LC_TEXT("ÀÚ°´°ø°Ý¿¡ %d%% ÀúÇ×");
		case POINT_RESIST_SURA:		return LC_TEXT("¼ö¶ó°ø°Ý¿¡ %d%% ÀúÇ×");
		case POINT_RESIST_SHAMAN:	return LC_TEXT("¹«´ç°ø°Ý¿¡ %d%% ÀúÇ×");
#ifdef ENABLE_WOLFMAN_CHARACTER
		case POINT_RESIST_WOLFMAN:	return LC_TEXT("¹«´ç°ø°Ý¿¡ %d%% ÀúÇ×");
#endif
		default:					return "UNK_ID %d%%"; // @fixme180
	}
}

static bool FN_hair_affect_string(LPCHARACTER ch, char *buf, size_t bufsiz)
{
	if (NULL == ch || NULL == buf)
		return false;

	CAffect* aff = NULL;
	time_t expire = 0;
	struct tm ltm;
	int	year, mon, day;
	int	offset = 0;

	aff = ch->FindAffect(AFFECT_HAIR);

	if (NULL == aff)
		return false;

	expire = ch->GetQuestFlag("hair.limit_time");

	if (expire < get_global_time())
		return false;

	// set apply string
	offset = snprintf(buf, bufsiz, FN_point_string(aff->bApplyOn), aff->lApplyValue);

	if (offset < 0 || offset >= (int) bufsiz)
		offset = bufsiz - 1;

	localtime_r(&expire, &ltm);

	year	= ltm.tm_year + 1900;
	mon		= ltm.tm_mon + 1;
	day		= ltm.tm_mday;

	snprintf(buf + offset, bufsiz - offset, LC_TEXT(" (¸¸·áÀÏ : %d³â %d¿ù %dÀÏ)"), year, mon, day);

	return true;
}

#ifdef BOSS_ARENA
ACMD(do_dungeonlist)
{
	if (!ch)
	{
		return;
	}

	if (ch->GetDungeonCD() > get_global_time())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Bu iþlemi bu kadar h?l?yapamazs?.");
		return;
	}

	ch->SetDungeonCD(get_global_time() + BOSS_ARENA_TIME);
	CBossArena::instance().ListAll(ch);
}

ACMD(do_dungeonspawn)
{
	if (!ch)
		return;

	char arg1[256];
	char arg2[256];

	DWORD vnum, type;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
		return;

	if (ch->GetDungeonCD() > get_global_time())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Bu iþlemi bu kadar h?l?yapamazs?.");
		return;
	}
	else if (!ch->GetDungeon())
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "TPBossInfo");
		return;
	}

	str_to_number(vnum, arg1);
	str_to_number(type, arg2);

	ch->SetDungeonCD(get_global_time() + BOSS_ARENA_TIME);

	LPDUNGEON dungeon = ch->GetDungeon();
	if (!dungeon)
		return;

	if (dungeon->GetFlag("BossStatus") != 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "?ðýrdýð? bossu kesmeden yenisini ?ðýramazs?.");
		return;
	}

	char notice[1024];
	DWORD status = CBossArena::instance().CheckBoss(ch, vnum, type);
	if (status == 5)
	{
		LPSECTREE_MAP pMap = SECTREE_MANAGER::instance().GetMap(ch->GetMapIndex());
		DWORD x, y;
		if (pMap)
		{
			x = (ch->GetX() - pMap->m_setting.iBaseX) / 100;
			y = (ch->GetY() - pMap->m_setting.iBaseY) / 100;
		}
		else
		{
			x = (ch->GetX() / 100);
			y = (ch->GetY() / 100);
		}

		LPCHARACTER mob = dungeon->SpawnMob(vnum, x, y);
		if (mob)
		{
			const CMob* pkMob = CMobManager::instance().Get(vnum);

			if (pkMob)
			{
				if (type == 0)
				{
					ch->SetBossArenaTest(true);
					snprintf(notice, sizeof(notice), "%s ortaya çýkt? Test modunda oldu?n i?n ?? alamayacaks?.", pkMob->m_table.szLocaleName);
				}
				else
				{
					ch->SetBossArenaTest(false);
					snprintf(notice, sizeof(notice), "%s ortaya çýkt?", pkMob->m_table.szLocaleName);
				}
				dungeon->Notice(notice);
				dungeon->SetFlag("BossStatus", vnum);
			}
		}


	}
	else if (status == 1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Seviyen bu bossu ?ðýrmak i?n uygun de?l.");
		return;
	}
	else if (status == 2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Bossu ?ðýrmak i?n gereken bilet miktar?a sahip de?lsin.");
		return;
	}
	else if (status == 3)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Bossun hen? bekleme s?esi dolmamýþ.");
		return;
	}
	else if (status == 4)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Bu boss hen? aktif de?l.");
		return;
	}
	// boss hwid //
	else if (status == 6)
	{
		// ch->ChatPacket(CHAT_TYPE_INFO, "Bilgisayar??da zaten zindana girmek i?n maksimum s???? var.");
		ch->ChatPacket(CHAT_TYPE_INFO, "(HWID) Boss'un hen? bekleme s?esi dolmamýþ.");
		return;
	}
	// boss hwid //
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Hatal?de?r girildi.");
		return;
	}
}

ACMD(do_dungeonclear)
{
	if (!ch)
	{
		return;
	}

	char arg1[256];
	char arg2[256];

	DWORD vnum, type;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
	{
		return;
	}

	if (ch->HaveAnotherPagesOpen())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Diger pencereleri kapatmadan bunu yapamazsin.");
		return;
	}

	if (ch->GetDungeonCD() > get_global_time())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Bu iþlemi bu kadar h?l?yapamazs?.");
		return;
	}
	else if (ch->GetDungeon())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Dungeon i?risinde bu iþlemi yapamazs?.");
		return;
	}

	str_to_number(vnum, arg1);
	str_to_number(type, arg2);

	if (type == 0)
	{
		if (ch->CountSpecifyItem(BOSS_ARENA_CLEAR_ITEM) < 1)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "S??lama eþyas?olmadan bu iþlemi yapamazs?.");
			return;
		}
	}
	else
	{
		if (ch->CountSpecifyItem(BOSS_ARENA_ALL_CLEAR_ITEM) < 1)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "S??lama eþyas?olmadan bu iþlemi yapamazs?.");
			return;
		}
	}

	ch->SetDungeonCD(get_global_time() + BOSS_ARENA_TIME);

	CBossArena::instance().ClearCooldown(ch, vnum, type);

}

ACMD(do_dungeontp)
{
	if (!ch)
	{
		return;
	}

	if (ch->HaveAnotherPagesOpen())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Diger pencereleri kapatmadan bunu yapamazsin.");
		return;
	}

	if (ch->GetDungeonCD() > get_global_time())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Bu iþlemi bu kadar h?l?yapamazs?.");
		return;
	}
	else if (ch->GetDungeon())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Zaten zindan? i?risindesin!");
		return;
	}

	ch->SetDungeonCD(get_global_time() + BOSS_ARENA_TIME);

	ch->WarpSet(BOSS_ARENA_WARP_X, BOSS_ARENA_WARP_Y);
}
#endif

ACMD(do_costume)
{
	char buf[1024]; // @warme015
	const size_t bufferSize = sizeof(buf);

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	CItem* pBody = ch->GetWear(WEAR_COSTUME_BODY);
	CItem* pHair = ch->GetWear(WEAR_COSTUME_HAIR);
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
	CItem* pMount = ch->GetWear(WEAR_COSTUME_MOUNT);
#endif
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	CItem* pAcce = ch->GetWear(WEAR_COSTUME_ACCE);
#endif
#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
	CItem* pWeapon = ch->GetWear(WEAR_COSTUME_WEAPON);
#endif

	ch->ChatPacket(CHAT_TYPE_INFO, "COSTUME status:");

	if (pHair)
	{
		const char* itemName = pHair->GetName();
		ch->ChatPacket(CHAT_TYPE_INFO, "  HAIR : %s", itemName);

		for (int i = 0; i < pHair->GetAttributeCount(); ++i)
		{
			const TPlayerItemAttribute& attr = pHair->GetAttribute(i);
			if (0 < attr.bType)
			{
				snprintf(buf, bufferSize, FN_point_string(attr.bType), attr.sValue);
				ch->ChatPacket(CHAT_TYPE_INFO, "     %s", buf);
			}
		}

		if (pHair->IsEquipped() && arg1[0] == 'h')
			ch->UnequipItem(pHair);
	}

	if (pBody)
	{
		const char* itemName = pBody->GetName();
		ch->ChatPacket(CHAT_TYPE_INFO, "  BODY : %s", itemName);

		if (pBody->IsEquipped() && arg1[0] == 'b')
			ch->UnequipItem(pBody);
	}

#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
	if (pMount)
	{
		const char* itemName = pMount->GetName();
		ch->ChatPacket(CHAT_TYPE_INFO, "  MOUNT : %s", itemName);

		if (pMount->IsEquipped() && arg1[0] == 'm')
			ch->UnequipItem(pMount);
	}
#endif

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	if (pAcce)
	{
		const char* itemName = pAcce->GetName();
		ch->ChatPacket(CHAT_TYPE_INFO, "  ACCE : %s", itemName);

		if (pAcce->IsEquipped() && arg1[0] == 'a')
			ch->UnequipItem(pAcce);
	}
#endif

#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
	if (pWeapon)
	{
		const char* itemName = pWeapon->GetName();
		ch->ChatPacket(CHAT_TYPE_INFO, "  WEAPON : %s", itemName);

		if (pWeapon->IsEquipped() && arg1[0] == 'w')
			ch->UnequipItem(pWeapon);
	}
#endif
}

ACMD(do_hair)
{
	char buf[256];

	if (false == FN_hair_affect_string(ch, buf, sizeof(buf)))
		return;

	ch->ChatPacket(CHAT_TYPE_INFO, buf);
}

ACMD(do_inventory)
{
	int	index = 0;
	int	count		= 1;

	char arg1[256];
	char arg2[256];

	LPITEM	item;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: inventory <start_index> <count>");
		return;
	}

	if (!*arg2)
	{
		index = 0;
		str_to_number(count, arg1);
	}
	else
	{
		str_to_number(index, arg1); index = MIN(index, INVENTORY_MAX_NUM);
		str_to_number(count, arg2); count = MIN(count, INVENTORY_MAX_NUM);
	}

	for (int i = 0; i < count; ++i)
	{
		if (index >= INVENTORY_MAX_NUM)
			break;

		item = ch->GetInventoryItem(index);

		ch->ChatPacket(CHAT_TYPE_INFO, "inventory [%d] = %s",
						index, item ? item->GetName() : "<NONE>");
		++index;
	}
}

//gift notify quest command
ACMD(do_gift)
{
	ch->ChatPacket(CHAT_TYPE_COMMAND, "gift");
}

#ifdef ENABLE_CUBE_RENEWAL_WORLDARD
ACMD(do_cube)
{

	const char* line;
	char arg1[256], arg2[256], arg3[256];
	line = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	one_argument(line, arg3, sizeof(arg3));

	if (0 == arg1[0])
	{
		return;
	}

	switch (LOWER(arg1[0]))
	{
	case 'o':	// open
		Cube_open(ch);
		break;

	default:
		return;
	}
}
#else
ACMD(do_cube)
{
	if (!ch->CanDoCube())
		return;

	dev_log(LOG_DEB0, "CUBE COMMAND <%s>: %s", ch->GetName(), argument);
	int cube_index = 0, inven_index = 0;
	const char* line;

	int inven_type = 0;

	char arg1[256], arg2[256], arg3[256];
	line = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	one_argument(line, arg3, sizeof(arg3));

	if (0 == arg1[0])
	{
		// print usage
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: cube open");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube close");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube add <inveltory_index>");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube delete <cube_index>");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube list");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube cancel");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube make [all]");
		return;
	}

	const std::string& strArg1 = std::string(arg1);

	// r_info (request information)
	// /cube r_info     ==> (Client -> Server) CoAc NPC¡Æ¢® ¢¬¢¬¥ìe ¨ùo AO¢¥A ¡¤©ö¨öACC ¢¯aA¡í
	//					    (Server -> Client) /cube r_list npcVNUM resultCOUNT 123,1/125,1/128,1/130,5
	//
	// /cube r_info 3   ==> (Client -> Server) CoAc NPC¡Æ¢® ¢¬¢¬¥ìe¨ùo AO¢¥A ¡¤©ö¨öACC A©¬ 3©ö©ªA¡Æ ¨ú¨¡AIAUA¡í ¢¬¢¬¥ìa¢¥A ¥ì¡Í CE¢¯aCN A¢´¨¬¢¬¢¬| ¢¯aA¡í
	// /cube r_info 3 5 ==> (Client -> Server) CoAc NPC¡Æ¢® ¢¬¢¬¥ìe¨ùo AO¢¥A ¡¤©ö¨öACC A©¬ 3©ö©ªA¡Æ ¨ú¨¡AIAU¨¬IAI AIEA 5¡Æ©øAC ¨ú¨¡AIAUA¡í ¢¬¢¬¥ìa¢¥A ¥ì¡Í CE¢¯aCN Ac¡¤a A¢´¨¬¢¬¢¬| ¢¯aA¡í
	//					   (Server -> Client) /cube m_info startIndex count 125,1|126,2|127,2|123,5&555,5&555,4/120000@125,1|126,2|127,2|123,5&555,5&555,4/120000
	//
	if (strArg1 == "r_info")
	{
		if (0 == arg2[0])
			Cube_request_result_list(ch);
		else
		{
			if (isdigit(*arg2))
			{
				int listIndex = 0, requestCount = 1;
				str_to_number(listIndex, arg2);

				if (0 != arg3[0] && isdigit(*arg3))
					str_to_number(requestCount, arg3);

				Cube_request_material_info(ch, listIndex, requestCount);
			}
		}

		return;
	}

	switch (LOWER(arg1[0]))
	{
	case 'o':	// open
		Cube_open(ch);
		break;

	case 'c':	// close
		Cube_close(ch);
		break;

	case 'l':	// list
		Cube_show_list(ch);
		break;

	case 'a':	// add cue_index inven_index
	{
		if (0 == arg2[0] || !isdigit(*arg2) || 0 == arg3[0] || !isdigit(*arg3))
			return;

		str_to_number(cube_index, arg2);
		str_to_number(inven_index, arg3);
		Cube_add_item(ch, cube_index, inven_index);
	}
	break;

	case 'd':	// delete
	{
		if (0 == arg2[0] || !isdigit(*arg2))
			return;

		str_to_number(cube_index, arg2);
		Cube_delete_item(ch, cube_index);
	}
	break;

	case 'm':	// make
		if (0 != arg2[0])
		{
			while (true == Cube_make(ch))
				dev_log(LOG_DEB0, "cube make success");
		}
		else
			Cube_make(ch);
		break;

	default:
		return;
	}
}
#endif

ACMD(do_in_game_mall)
{
	if (LC_IsEurope() == true)
	{
		char country_code[3];

		switch (LC_GetLocalType())
		{
			case LC_GERMANY:	country_code[0] = 'd'; country_code[1] = 'e'; country_code[2] = '\0'; break;
			case LC_FRANCE:		country_code[0] = 'f'; country_code[1] = 'r'; country_code[2] = '\0'; break;
			case LC_ITALY:		country_code[0] = 'i'; country_code[1] = 't'; country_code[2] = '\0'; break;
			case LC_SPAIN:		country_code[0] = 'e'; country_code[1] = 's'; country_code[2] = '\0'; break;
			case LC_UK:			country_code[0] = 'e'; country_code[1] = 'n'; country_code[2] = '\0'; break;
			case LC_TURKEY:		country_code[0] = 't'; country_code[1] = 'r'; country_code[2] = '\0'; break;
			case LC_POLAND:		country_code[0] = 'p'; country_code[1] = 'l'; country_code[2] = '\0'; break;
			case LC_PORTUGAL:	country_code[0] = 'p'; country_code[1] = 't'; country_code[2] = '\0'; break;
			case LC_GREEK:		country_code[0] = 'g'; country_code[1] = 'r'; country_code[2] = '\0'; break;
			case LC_RUSSIA:		country_code[0] = 'r'; country_code[1] = 'u'; country_code[2] = '\0'; break;
			case LC_DENMARK:	country_code[0] = 'd'; country_code[1] = 'k'; country_code[2] = '\0'; break;
			case LC_BULGARIA:	country_code[0] = 'b'; country_code[1] = 'g'; country_code[2] = '\0'; break;
			case LC_CROATIA:	country_code[0] = 'h'; country_code[1] = 'r'; country_code[2] = '\0'; break;
			case LC_MEXICO:		country_code[0] = 'm'; country_code[1] = 'x'; country_code[2] = '\0'; break;
			case LC_ARABIA:		country_code[0] = 'a'; country_code[1] = 'e'; country_code[2] = '\0'; break;
			case LC_CZECH:		country_code[0] = 'c'; country_code[1] = 'z'; country_code[2] = '\0'; break;
			case LC_ROMANIA:	country_code[0] = 'r'; country_code[1] = 'o'; country_code[2] = '\0'; break;
			case LC_HUNGARY:	country_code[0] = 'h'; country_code[1] = 'u'; country_code[2] = '\0'; break;
			case LC_NETHERLANDS: country_code[0] = 'n'; country_code[1] = 'l'; country_code[2] = '\0'; break;
			case LC_USA:		country_code[0] = 'u'; country_code[1] = 's'; country_code[2] = '\0'; break;
			case LC_CANADA:	country_code[0] = 'c'; country_code[1] = 'a'; country_code[2] = '\0'; break;
			default:
				if (test_server == true)
				{
					country_code[0] = 'd'; country_code[1] = 'e'; country_code[2] = '\0';
				}
				break;
		}

		char buf[512+1];
		char sas[33];
		MD5_CTX ctx;
		const char sas_key[] = "GF9001";

		snprintf(buf, sizeof(buf), "%u%u%s", ch->GetPlayerID(), ch->GetAID(), sas_key);

		MD5Init(&ctx);
		MD5Update(&ctx, (const unsigned char *) buf, strlen(buf));
#ifdef __FreeBSD__
		MD5End(&ctx, sas);
#else
		static const char hex[] = "0123456789abcdef";
		unsigned char digest[16];
		MD5Final(digest, &ctx);
		int i;
		for (i = 0; i < 16; ++i) {
			sas[i+i] = hex[digest[i] >> 4];
			sas[i+i+1] = hex[digest[i] & 0x0f];
		}
		sas[i+i] = '\0';
#endif

		snprintf(buf, sizeof(buf), "mall http://%s/ishop?pid=%u&c=%s&sid=%d&sas=%s",
				g_strWebMallURL.c_str(), ch->GetPlayerID(), country_code, g_server_id, sas);

		ch->ChatPacket(CHAT_TYPE_COMMAND, buf);
	}
}


ACMD(do_dice)
{
	char arg1[256], arg2[256];
	int start = 1, end = 100;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (*arg1 && *arg2)
	{
		start = atoi(arg1);
		end = atoi(arg2);
	}
	else if (*arg1 && !*arg2)
	{
		start = 1;
		end = atoi(arg1);
	}

	end = MAX(start, end);
	start = MIN(start, end);

	int n = number(start, end);

#ifdef ENABLE_DICE_SYSTEM
	if (ch->GetParty())
		ch->GetParty()->ChatPacketToAllMember(CHAT_TYPE_DICE_INFO, LC_TEXT("%s´ÔÀÌ ÁÖ»çÀ§¸¦ ±¼·Á %d°¡ ³ª¿Ô½À´Ï´Ù. (%d-%d)"), ch->GetName(), n, start, end);
	else
		ch->ChatPacket(CHAT_TYPE_DICE_INFO, LC_TEXT("´ç½ÅÀÌ ÁÖ»çÀ§¸¦ ±¼·Á %d°¡ ³ª¿Ô½À´Ï´Ù. (%d-%d)"), n, start, end);
#else
	if (ch->GetParty())
		ch->GetParty()->ChatPacketToAllMember(CHAT_TYPE_INFO, LC_TEXT("%s´ÔÀÌ ÁÖ»çÀ§¸¦ ±¼·Á %d°¡ ³ª¿Ô½À´Ï´Ù. (%d-%d)"), ch->GetName(), n, start, end);
	else
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ç½ÅÀÌ ÁÖ»çÀ§¸¦ ±¼·Á %d°¡ ³ª¿Ô½À´Ï´Ù. (%d-%d)"), n, start, end);
#endif
}

#ifdef ENABLE_NEWSTUFF
ACMD(do_click_safebox)
{
	if ((ch->GetGMLevel() <= GM_PLAYER) && (ch->GetDungeon() || ch->GetWarMap()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot open the safebox in dungeon or at war."));
		return;
	}

	ch->SetSafeboxOpenPosition();
	ch->ChatPacket(CHAT_TYPE_COMMAND, "ShowMeSafeboxPassword");
}
ACMD(do_force_logout)
{
	LPDESC pDesc=DESC_MANAGER::instance().FindByCharacterName(ch->GetName());
	if (!pDesc)
		return;
	pDesc->DelayedDisconnect(0);
}
#endif
#ifdef ENABLE_STARTER_ITEMS
ACMD(do_item_ver_baba)
{
	if (ch->GetQuestFlag("maviruh.source") != 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("hatali_kod"));
		return;
	}
	BYTE job = ch->GetJob();
	LPITEM item;
	for (int i = 0; i < 6; i++)
	{
		item = ch->GetWear(i);
		if (item != NULL)
			ch->UnequipItem(item);
	}
	item = ch->GetWear(WEAR_SHIELD);
	if (item != NULL)
		ch->UnequipItem(item);
	item = ch->GetWear(WEAR_EAR);
	if (item != NULL)
		ch->UnequipItem(item);
	switch (job)
	{
	case JOB_WARRIOR:
		{
			item = ITEM_MANAGER::instance().CreateItem(19);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(11209);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(12209);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(13009);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(14009 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(15009 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(16009 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(17009 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
		}
		break;
	case JOB_ASSASSIN:
		{
			item = ITEM_MANAGER::instance().CreateItem(19);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(11409);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(12349);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(13009);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(14009 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(15009 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(16009 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(17009 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
		}
		break;
	case JOB_SURA:
		{
			item = ITEM_MANAGER::instance().CreateItem(19);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(11609);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(12489);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(13009);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(14009 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(15009 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(16009 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(17009 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
		}
		break;
	case JOB_SHAMAN:
		{
			item = ITEM_MANAGER::instance().CreateItem(7009);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(11809);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(12629);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(13009);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(14009 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(15009 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(16009 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(17009 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
		}
		break;
	}
	ch->SetQuestFlag("maviruh.source", 1);
}
ACMD(do_efsunu_ver_baba)
{
	if (ch->GetQuestFlag("maviruh2.source") != 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("hatali_kod"));
		return;
	}
	BYTE job = ch->GetJob();
	LPITEM item;
	switch (job)
	{
	case JOB_WARRIOR:
	case JOB_ASSASSIN:
	case JOB_SURA:
	case JOB_SHAMAN:
		{
			//  -- Kask Efsunlari
			item = ch->GetWear(WEAR_HEAD);
			if (item != NULL)
			{
				item->ClearAttribute();
				item->SetForceAttribute( 0, APPLY_ATT_SPEED, 8);
				item->SetForceAttribute( 1, APPLY_HP_REGEN, 30);
				item->SetForceAttribute( 2, APPLY_ATTBONUS_ANIMAL, 20);
				item->SetForceAttribute( 3, APPLY_ATTBONUS_UNDEAD, 20);
				item->SetForceAttribute( 4, APPLY_ATTBONUS_DEVIL, 20);
			}
			// -- Silah Efsunlari
			item = ch->GetWear(WEAR_WEAPON);
			if (item != NULL)
			{
				item->ClearAttribute();
				item->SetForceAttribute( 0, APPLY_STR, 12);
				item->SetForceAttribute( 1, APPLY_CRITICAL_PCT, 10);
				item->SetForceAttribute( 2, APPLY_PENETRATE_PCT, 10);
				item->SetForceAttribute( 3, APPLY_DEX, 12);
				item->SetForceAttribute( 4, APPLY_INT, 12);
			}
			// -- Kalkan Efsunlari
			item = ch->GetWear(WEAR_SHIELD);
			if (item != NULL)
			{
				item->ClearAttribute();
				item->SetForceAttribute( 0, APPLY_ATTBONUS_UNDEAD, 20);
				item->SetForceAttribute( 1, APPLY_ATTBONUS_DEVIL, 20);
				item->SetForceAttribute( 2, APPLY_EXP_DOUBLE_BONUS, 20);
				item->SetForceAttribute( 3, APPLY_IMMUNE_STUN, 1);
				item->SetForceAttribute( 4, APPLY_BLOCK, 10);
			}
			//  -- Z?h Efsunlari
			item = ch->GetWear(WEAR_BODY);
			if (item != NULL)
			{
				item->ClearAttribute();
				item->SetForceAttribute( 0, APPLY_MAX_HP, 2000);
				item->SetForceAttribute( 1, APPLY_CAST_SPEED, 20);
				item->SetForceAttribute( 2, APPLY_STEAL_HP, 10);
				item->SetForceAttribute( 3, APPLY_REFLECT_MELEE, 10);
				item->SetForceAttribute( 4, APPLY_ATT_GRADE_BONUS, 50);
				}
			//  -- Ayakkab?Efsunlari
			item = ch->GetWear(WEAR_FOOTS);
			if (item != NULL)
			{
				item->ClearAttribute();
				item->SetForceAttribute( 0, APPLY_MAX_HP, 2000);
				item->SetForceAttribute( 1, APPLY_EXP_DOUBLE_BONUS, 20);
				item->SetForceAttribute( 2, APPLY_MOV_SPEED, 8);
				item->SetForceAttribute( 3, APPLY_ATT_SPEED, 8);
				item->SetForceAttribute( 4, APPLY_CRITICAL_PCT, 10);
			}
			//  -- Bilezik Efsunlari
			item = ch->GetWear(WEAR_WRIST);
			if (item != NULL)
			{
				item->ClearAttribute();
				item->SetForceAttribute( 0, APPLY_MAX_HP, 2000);
				item->SetForceAttribute( 1, APPLY_ATTBONUS_UNDEAD, 20);
				item->SetForceAttribute( 2, APPLY_PENETRATE_PCT, 10);
				item->SetForceAttribute( 3, APPLY_ATTBONUS_DEVIL, 20);
				item->SetForceAttribute( 4, APPLY_STEAL_HP, 10);
			}
			//  -- Kolye Efsunlari
			item = ch->GetWear(WEAR_NECK);
			if (item != NULL)
			{
				item->ClearAttribute();
				item->SetForceAttribute( 0, APPLY_MAX_HP, 2000);
				item->SetForceAttribute( 1, APPLY_EXP_DOUBLE_BONUS, 20);
				item->SetForceAttribute( 2, APPLY_HP_REGEN, 10);
				item->SetForceAttribute( 3, APPLY_CRITICAL_PCT, 10);
				item->SetForceAttribute( 4, APPLY_PENETRATE_PCT, 10);
			}
			//  -- Kupe Efsunlari
			item = ch->GetWear(WEAR_EAR);
			if (item != NULL)
			{
				item->ClearAttribute();
				item->SetForceAttribute( 0, APPLY_MOV_SPEED, 20);
				item->SetForceAttribute( 1, APPLY_ATTBONUS_ANIMAL, 20);
				item->SetForceAttribute( 2, APPLY_POISON_REDUCE, 8);
				item->SetForceAttribute( 3, APPLY_ATTBONUS_DEVIL, 20);
				item->SetForceAttribute( 4, APPLY_ATTBONUS_UNDEAD, 20);
			}
		}
		break;
	}
	ch->SetQuestFlag("maviruh2.source", 1);
}
#endif

ACMD(do_click_mall)
{
	ch->ChatPacket(CHAT_TYPE_COMMAND, "ShowMeMallPassword");
}
ACMD(do_select_skill_group)
{
	if(!ch || !ch->IsPC())
		return;
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;
	BYTE group;
	str_to_number(group, arg1);
	if (group == 0 || group > 2)
		return;
	ch->SelectSkillGroupWithWindow(group);
}
ACMD(do_ride)
{
    dev_log(LOG_DEB0, "[DO_RIDE] start");
    if (ch->IsDead() || ch->IsStun())
	return;

#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
	if (ch->IsPolymorphed() == true){
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("donusmuskenbinegebinemezsin."));
		return;
	}
	if(ch->GetWear(WEAR_COSTUME_MOUNT))
	{
		CMountSystem* mountSystem = ch->GetMountSystem();
		LPITEM mount = ch->GetWear(WEAR_COSTUME_MOUNT);
		DWORD mobVnum = 0;
		
		if (!mountSystem && !mount)
			return;
		
		if(mount->GetValue(1) != 0)
			mobVnum = mount->GetValue(1);
		
		if (ch->GetMountVnum())
		{
			if(mountSystem->CountSummoned() == 0)
			{
				mountSystem->Unmount(mobVnum);
			}
		}
		else
		{
			if(mountSystem->CountSummoned() == 1)
			{
				mountSystem->Mount(mobVnum, mount);
			}
		}
		
		return;
	}
#endif


    {
	if (ch->IsHorseRiding())
	{
	    dev_log(LOG_DEB0, "[DO_RIDE] stop riding");
	    ch->StopRiding();
	    return;
	}

	if (ch->GetMountVnum())
	{
	    dev_log(LOG_DEB0, "[DO_RIDE] unmount");
	    do_unmount(ch, NULL, 0, 0);
	    return;
	}
    }


    {
	if (ch->GetHorse() != NULL)
	{
	    dev_log(LOG_DEB0, "[DO_RIDE] start riding");
	    ch->StartRiding();
	    return;
	}

	for (BYTE i=0; i<INVENTORY_MAX_NUM; ++i)
	{
	    LPITEM item = ch->GetInventoryItem(i);
	    if (NULL == item)
			continue;

		if (item->IsRideItem())
		{
			if (
				NULL==ch->GetWear(WEAR_UNIQUE1)
				|| NULL==ch->GetWear(WEAR_UNIQUE2)
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
					|| NULL == ch->GetWear(WEAR_COSTUME_MOUNT)
#endif
			)
			{
				dev_log(LOG_DEB0, "[DO_RIDE] USE UNIQUE ITEM");
				//ch->EquipItem(item);
				ch->UseItem(TItemPos (INVENTORY, i));
				return;
			}
		}

#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
			if (item->IsMountItem())
			{
				if (NULL==ch->GetWear(WEAR_COSTUME_MOUNT))
				{
					dev_log(LOG_DEB0, "[DO_RIDE] USE UNIQUE ITEM");
					//ch->EquipItem(item);
					ch->UseItem(TItemPos (INVENTORY, i));
					return;
				}
			}
#endif

	    switch (item->GetVnum())
	    {
		case 71114:
		case 71116:
		case 71118:
		case 71120:
		    dev_log(LOG_DEB0, "[DO_RIDE] USE QUEST ITEM");
		    ch->UseItem(TItemPos (INVENTORY, i));
		    return;
	    }

		if( (item->GetVnum() > 52000) && (item->GetVnum() < 52091) )	{
			dev_log(LOG_DEB0, "[DO_RIDE] USE QUEST ITEM");
			ch->UseItem(TItemPos (INVENTORY, i));
		    return;
		}
	}
    }



    ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»À» ¸ÕÀú ¼ÒÈ¯ÇØÁÖ¼¼¿ä."));
}

#ifdef __HIDE_COSTUME_SYSTEM__
ACMD(do_hide_costume)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
		return;

	bool hidden = true;
	BYTE bPartPos = 0;
	BYTE bHidden = 0;

	str_to_number(bPartPos, arg1);

	if (*arg2)
	{
		str_to_number(bHidden, arg2);

		if (bHidden == 0)
			hidden = false;
	}

	if (bPartPos == 1)
		ch->SetBodyCostumeHidden(hidden);
	else if (bPartPos == 2)
		ch->SetHairCostumeHidden(hidden);
	else if (bPartPos == 3)
		ch->SetAcceCostumeHidden(hidden);
	else if (bPartPos == 4)
		ch->SetWeaponCostumeHidden(hidden);
	else
		return;

	ch->UpdatePacket();
}
#endif

#ifdef ENABLE_OPPORTUNITY_SYSTEM
ACMD(do_open_opportunity)
{
	if (quest::CQuestManager::instance().GetEventFlag("enable_opportunity") == 0)
		return;
	
	if (!ch)
		return;
	
	if (!ch->GetDesc())
		return;
	
	if (!ch->IsPC())
		return;

	if (ch->IsStun())
		return;

	if (ch->IsHack())
		return;

	if (ch->GetExchange() || ch->GetMyShop() || ch->GetShopOwner() || ch->IsOpenSafebox() || ch->IsCubeOpen() || ch->IsDead())
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "ShopSearchOpen 1");
		return;
	}

	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
		return;
	
	COpportunity::instance().SendClientPacket(ch);

}

ACMD(do_buy_opportunity)
{

	if (quest::CQuestManager::instance().GetEventFlag("enable_opportunity") == 0)
		return;
	
	if(!ch || !ch->IsPC())
		return;
	
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;
	
	BYTE option;
	str_to_number(option, arg1);
	
	COpportunity::instance().Buy(ch,option);
}
#endif

#ifdef ENABLE_WHEEL_OF_FORTUNE
ACMD(do_wheel_of_fortune)
{
	std::vector<std::string> vecArgs;
	split_argument(argument, vecArgs);
	if (vecArgs.size() < 2) { return; }

	else if (vecArgs[1] == "spin")
	{
		if (ch->GetProtectTime("WheelWorking") != 0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "Wheel already is working!");
			return;
		}
		else if (quest::CQuestManager::instance().GetEventFlag("whell_event") != 1)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "Event is not active!");
			return;
		}

		if (vecArgs.size() < 3) { return; }

		DWORD dwWhellIdx = 0;
		if (!str_to_number(dwWhellIdx, vecArgs[2].c_str()))
			return;

		if (ch->GetExchange())
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "You can't do it with exchange!");
			return;
		}
		//ch->SetExchangeTime();

		const std::map<DWORD, CWhellItemGroup*>* whellDataMap = ITEM_MANAGER::Instance().GetWhellItemsData();
		const auto it = whellDataMap->find(dwWhellIdx);
		if (it == whellDataMap->end())
		{
			sys_err("unknown wheel request! player: %s whellidx: %u", ch->GetName(), dwWhellIdx);
			return;
		}

		if (dwWhellIdx == 0)//prremium cark itemi
		{

			if (ch->CountSpecifyItem(900009) <= 0) 
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Yeterli Malzeme Yok !"));
				return;
			}

			ch->RemoveSpecifyItem(900009, 1);

		}

		else if (dwWhellIdx == 19)
		{
			if (ch->CountSpecifyItem(900011) <= 0)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Yeterli Malzeme Yok !"));
				return;
			}
			ch->RemoveSpecifyItem(900011, 1);
		}
		else if (dwWhellIdx == 29)
		{
			if (ch->CountSpecifyItem(900012) <= 0)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Yeterli Malzeme Yok !"));
				return;
			}
			ch->RemoveSpecifyItem(900012, 1);
		}
		else if (dwWhellIdx == 39)
		{
			if (ch->CountSpecifyItem(900010) <= 0)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Yeterli Malzeme Yok !"));
				return;
			}
			ch->RemoveSpecifyItem(900010, 1);
		}
		else if (dwWhellIdx == 49)
		{
			if (ch->CountSpecifyItem(900013) <= 0)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Yeterli Malzeme Yok !"));
				return;
			}
			ch->RemoveSpecifyItem(900013, 1);
		}
		else
		{
			if (dwWhellIdx < 10 || (dwWhellIdx != 19 && dwWhellIdx != 29 && dwWhellIdx != 39 && dwWhellIdx != 49))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "Invalid wheel index!");
				return;
			}

			if (ch->CountSpecifyItem(dwWhellIdx) < 1)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "You don't have enough item for spin!");
				return;
			}
			ch->RemoveSpecifyItem(dwWhellIdx, 1);
		}

		std::vector<std::pair<DWORD, int>> m_send_items{};
		const auto itemVector = it->second->GetVector();
		std::string cmd_wheel("");
		while (true)
		{
			const auto randomItem = it->second->GetOne();
			if (std::find(m_send_items.begin(), m_send_items.end(), std::make_pair(randomItem.dwItemVnum, randomItem.iCount)) != m_send_items.end())
				continue;
			
			if (randomItem.iRarePct >= number(1, 100))
			{
				m_send_items.emplace_back(randomItem.dwItemVnum, randomItem.iCount);
				if(cmd_wheel.length())
					cmd_wheel += "#";
				
				char tmpBuf[64];
				snprintf(tmpBuf, sizeof(tmpBuf), "%u|%d", randomItem.dwItemVnum, randomItem.iCount);
				cmd_wheel += tmpBuf;
			}
			if (m_send_items.size() >= 10)
				break;
		}

		const BYTE bLuckyIdx = number(0, 9);
		ch->SetProtectTime("WheelIndex", dwWhellIdx);
		ch->SetProtectTime("WheelLuckyIndex", bLuckyIdx);
		ch->SetProtectTime("WheelLuckyItemVnum", m_send_items[bLuckyIdx].first);
		ch->SetProtectTime("WheelLuckyItemCount", m_send_items[bLuckyIdx].second);
		ch->SetProtectTime("WheelWorking", 1);
		ch->ChatPacket(CHAT_TYPE_COMMAND, "SetWhellData %u %s", bLuckyIdx, cmd_wheel.c_str());
	}
	else if (vecArgs[1] == "loaditem")
	{
		if (vecArgs.size() < 3) { return; }

		DWORD dwWhellIdx = 0;
		if (!str_to_number(dwWhellIdx, vecArgs[2].c_str()))
			return;

		const std::map<DWORD, CWhellItemGroup*>* whellDataMap = ITEM_MANAGER::Instance().GetWhellItemsData();
		const auto it = whellDataMap->find(dwWhellIdx);
		if (it == whellDataMap->end())
		{
			sys_err("unknown wheel request! player: %s whellidx: %u", ch->GetName(), dwWhellIdx);
			return;
		}

		const auto itemVector = it->second->GetVector();
		std::string cmd_wheel("");

		for (auto itItem = itemVector->begin(); itItem != itemVector->end(); ++itItem)
		{
			if (cmd_wheel.length())
				cmd_wheel += "#";
			
			char tmpBuf[64];
			snprintf(tmpBuf, sizeof(tmpBuf), "%u|%d", itItem->dwItemVnum, itItem->iCount);
			cmd_wheel += tmpBuf;

			if (cmd_wheel.length() > 200)
			{
				ch->ChatPacket(CHAT_TYPE_COMMAND, "SetLoadWhellData %u %s", dwWhellIdx, cmd_wheel.c_str());
				cmd_wheel.clear();
			}
		}

		if (cmd_wheel.length())
			ch->ChatPacket(CHAT_TYPE_COMMAND, "SetLoadWhellData %u %s", dwWhellIdx, cmd_wheel.c_str());

	}
	// else if (vecArgs[1] == "dcupdate")
	// {
		// if (ch->GetProtectTime("whelllastdcupdate") > time(0))
		// {
			// ch->ChatPacket(CHAT_TYPE_INFO, "Please be slow. You can refresh in %d sec.", ch->GetProtectTime("whelllastdcupdate") - time(0));
			// return;
		// }
		// ch->SetProtectTime("whelllastdcupdate", time(0) + 5);
		// ch->ChatPacket(CHAT_TYPE_COMMAND, "SetWhellDragonCoin %lld", ch->GetGold());
	// }
	else if (vecArgs[1] == "done")
	{
		if (ch->GetProtectTime("WheelWorking") != 0)
		{
			const DWORD dwSelectedItemIdx = ch->GetProtectTime("WheelLuckyItemVnum"), dwSelectedItemCount = ch->GetProtectTime("WheelLuckyItemCount");
			ch->AutoGiveItem(dwSelectedItemIdx, dwSelectedItemCount);

			ch->ChatPacket(CHAT_TYPE_COMMAND, "ShowWhellReward %u %u", dwSelectedItemIdx, dwSelectedItemCount);
			ch->SetProtectTime("WheelIndex", 0);
			ch->SetProtectTime("WheelLuckyIndex", 0);
			ch->SetProtectTime("WheelLuckyItemVnum", 0);
			ch->SetProtectTime("WheelLuckyItemCount", 0);
			ch->SetProtectTime("WheelWorking", 0);
		}
	}
	else if (vecArgs[1] == "event")
	{
		if (!ch->IsGM())
			return;
		if (vecArgs.size() < 3) { return; }

		if (vecArgs[2] == "active")
		{
			BroadcastNotice("Wheel event is started.");
			quest::CQuestManager::instance().RequestSetEventFlag("whell_event", 1);
		}
		else if (vecArgs[2] == "deactive")
		{
			BroadcastNotice("Wheel event is done.");
			quest::CQuestManager::instance().RequestSetEventFlag("whell_event", 0);
		}
	}
}

#endif

#ifdef ENABLE_DEAL_OR_NO_DEAL
ACMD(do_dealornodeal)
{
	if (quest::CQuestManager::instance().GetEventFlag("deal_or_no_deal") < 1 && quest::CQuestManager::instance().GetEventFlag("deal_or_no_deal_yang") < 1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Sistem kapali.");
		return;
	}
	
	std::vector<std::string> vecArgs;
	split_argument(argument,vecArgs);
	
	if (vecArgs.size() < 4)
		return;
	
	BYTE type = 0;
	BYTE box = 0;
	BYTE price_type = 1;
	DWORD coin = 0;
		
	str_to_number(type, vecArgs[1].c_str());
	str_to_number(box, vecArgs[2].c_str());
	str_to_number(coin, vecArgs[3].c_str());
	str_to_number(price_type, vecArgs[4].c_str());
	
	if (type == 1)
		CDealOrNoDeal::Instance().Start(ch,box,coin,price_type);
	else if (type == 2)
		CDealOrNoDeal::Instance().RemoveBox(ch,box);
	else if (type == 3)
		CDealOrNoDeal::Instance().Answer(ch,box);
	else
		CDealOrNoDeal::Instance().Test(ch);
}
#endif

#ifdef _ITEM_SHOP_SYSTEM

ACMD(do_nesne_market)
{
	if (!ch)
		return;
	
	if (!ch->GetDesc())
		return;
	
	if (!ch->IsPC())
		return;

	if (ch->IsStun())
		return;

	if (ch->IsHack())
		return;
	
#ifdef _AURA_SYSTEM
	if (ch->isAuraOpened(true) || ch->isAuraOpened(false))
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "ShopSearchOpen 1");
		return;
	}
#endif
	
	if (ch->GetExchange() || ch->GetMyShop() || ch->GetShopOwner() || ch->IsOpenSafebox() || ch->IsCubeOpen() || ch->IsDead())
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "ShopSearchOpen 1");
		return;
	}

	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
		return;
	
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	DWORD id = 0;
	DWORD count = 0;

	if (!*arg1 || !*arg2)
		return;

	str_to_number(id, arg1);
	str_to_number(count, arg2);

	bool bRes = CItemShopManager::instance().Buy(ch, id, count); // buy func
	if (bRes)
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("nesnemarketbasarili"));
}
#endif

#ifdef ENABLE_COLLECT_QUEST_SYSTEM
LPEVENT biyologtimer = NULL;

EVENTINFO(TMainEventInfo5)
{
	LPCHARACTER	kim;
	int deger;
	int itemim1;
	int itemim2;
	TMainEventInfo5() 
	: kim( NULL )
	, deger( 0 )
	, itemim1( 0 )
	, itemim2( 0 )
	{
	}
} ;

EVENTFUNC(biyolog_event)
{
	TMainEventInfo5 * info = dynamic_cast<TMainEventInfo5 *>(  event->info );
	if ( info == NULL )
	{
		sys_err( "biyolog_event> <Factor> Null pointer" );
		return 0;
	}

	LPCHARACTER	ch = info->kim;
	int deger = info->deger;
	int itemim1 = info->itemim1;
	int itemim2 = info->itemim2;

	if (NULL == ch || deger == 0 || itemim1 == 0 || itemim2 == 0)
		return 0;

	if (!ch)
		return 0;

	if (!ch->GetDesc())
		return 0;

	int sans =  BiyologSistemi[ch->GetQuestFlag("bio.durum")][3];

	if (ch)
	{
		LPITEM item = ch->GetItem(TItemPos(INVENTORY, itemim1));
		if (item != NULL)
		{
			if (item->GetVnum() == 70022)
			{
				if(ch->GetQuestFlag("bio.durum") > 10)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biyologbitmis"));
					return 0;
				}

				if (ch->CountSpecifyItem(70022) < 1)
				{
					return 0;
				}

				if(int(ch->GetQuestFlag("bio.sure")) == 1)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biosurezatenaktif"));
				}
				else if(ch->GetQuestFlag("bio.ruhtasi") == 2)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioruhdayapamazsin"));
				}
				else
				{
					item->SetCount(item->GetCount() - 1);
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biosureverildi"));
					ch->SetQuestFlag("bio.sure",1);
					ch->SetQuestFlag("bio.kalan",get_global_time()+0);
					ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog_update %d %d %d %d %d", ch->GetQuestFlag("bio.durum"), ch->GetQuestFlag("bio.ruhtasi"), ch->GetQuestFlag("bio.verilen"), BiyologSistemi[ch->GetQuestFlag("bio.durum")][1], ch->GetQuestFlag("bio.kalan") - get_global_time());
				}
			}
		}

		LPITEM item2 = ch->GetItem(TItemPos(INVENTORY, itemim2));
		if (item2 != NULL)
		{
			if(ch->GetQuestFlag("bio.durum") > 10)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biyologbitmis"));
				return 0;
			}

			int SANS_ITEMLER[3] = 
			{
				71035,
				76020,
				39023,
			};

			for (int it = 0; it <= 3; it++)
			{
				if (item2->GetVnum() == SANS_ITEMLER[it])
				{
					if (ch->CountSpecifyItem(SANS_ITEMLER[it]) < 1)
					{
						return 0;
					}

					if(int(ch->GetQuestFlag("bio.sans")) == 1)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biosanszatenaktif"));
					}
					else if(ch->GetQuestFlag("bio.ruhtasi") == 2)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioruhdayapamazsin"));
					}
					else
					{
						item2->SetCount(item2->GetCount() - 1);
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biosansverildi"));
						ch->SetQuestFlag("bio.sans", 1);
					}
				}
			}
		}

		if(ch->GetQuestFlag("bio.kalan") > get_global_time())
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biosuredolmadi"));
			return 0;
		}

		if(ch->GetQuestFlag("bio.durum") == 1 || ch->GetQuestFlag("bio.durum") == 2 || ch->GetQuestFlag("bio.durum") == 3 || ch->GetQuestFlag("bio.durum") == 4 || ch->GetQuestFlag("bio.durum") == 5 || ch->GetQuestFlag("bio.durum") == 6 || ch->GetQuestFlag("bio.durum") == 7 || ch->GetQuestFlag("bio.durum") == 8 || ch->GetQuestFlag("bio.durum") == 9 || ch->GetQuestFlag("bio.durum") == 10)
		{
			if(ch->GetQuestFlag("bio.durum") > 10)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biyologbitmis"));
				return 0;
			}

			if (ch->CountSpecifyItem(BiyologSistemi[ch->GetQuestFlag("bio.durum")][0]) < 1)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioitemyok"));
				return 0;
			}
			else
			{
				int prob = number(1,100);
				if(ch->GetQuestFlag("bio.sans") == 1)
				{
					sans = sans +100;
				}
				if(ch->GetQuestFlag("bio.sure") == 1)
				{
					ch->SetQuestFlag("bio.sure",0);
				}

				if(sans >= prob)
				{
					if (ch->GetQuestFlag("bio.verilen") >= BiyologSistemi[ch->GetQuestFlag("bio.durum")][1])
					{
						return 0;
					}

					ch->SetQuestFlag("bio.verilen",ch->GetQuestFlag("bio.verilen")+1);

					if(ch->GetQuestFlag("bio.sans") == 1)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioozutgitti"));
						ch->SetQuestFlag("bio.sans",0);
					}

					if(ch->GetQuestFlag("bio.verilen") == BiyologSistemi[ch->GetQuestFlag("bio.durum")][1])
					{
						if (ch->GetQuestFlag("bio.durum") == 9)
						{
							ch->SetQuestFlag("bio.ruhtasi",3);
							ch->SetQuestFlag("bio.odulvakti",1);
						}
						else
						{
							TItemTable* pTable = ITEM_MANAGER::instance().GetTable(BiyologSistemi[ch->GetQuestFlag("bio.durum")][4]);
							ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biotoplamabittiruhtasibul %s"), pTable->szLocaleName);
							ch->SetQuestFlag("bio.ruhtasi",2);
						}
					}
					else
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogecti %d"), (BiyologSistemi[ch->GetQuestFlag("bio.durum")][1]-ch->GetQuestFlag("bio.verilen")));
						ch->SetQuestFlag("bio.kalan",get_global_time()+(BiyologSistemi[ch->GetQuestFlag("bio.durum")][2]*60));
					}
				}
				else
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biobasarisiz"));
					ch->SetQuestFlag("bio.kalan",get_global_time()+(BiyologSistemi[ch->GetQuestFlag("bio.durum")][2]*60));
				}
				ch->RemoveSpecifyItem(BiyologSistemi[ch->GetQuestFlag("bio.durum")][0],1);
			}
		}

		ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog_update %d %d %d %d %d", ch->GetQuestFlag("bio.durum"), ch->GetQuestFlag("bio.ruhtasi"), ch->GetQuestFlag("bio.verilen"), BiyologSistemi[ch->GetQuestFlag("bio.durum")][1], ch->GetQuestFlag("bio.kalan") - get_global_time());
		return 1;
	}

	return 0;
}

ACMD(do_biyolog)
{
	if (quest::CQuestManager::instance().GetEventFlag("biyolog_disable") == 1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Sistem suan icin devre disi!");
		return;
	}

	char arg1[256], arg2[256], arg3[256];
	three_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2), arg3, sizeof(arg3));

	if (!*arg1 && !*arg2 && !*arg3)
		return;

	if (!ch->IsPC())
		return;

#ifdef ENABLE_OFFLINE_SHOP_SYSTEM
	if (ch->GetExchange() || ch->GetViewingShopOwner() || ch->IsOpenSafebox() || ch->IsCubeOpen())
#else
	if (ch->GetExchange() || ch->GetMyShop() || ch->GetShopOwner() || ch->IsOpenSafebox() || ch->IsCubeOpen())
#endif
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("A¢¥A¢âA¢¬A¡Í A¡ÆA¡¦A¡¤A¢®AA?(A?A¡ËA¡ÆA¡©,A¡¾A©øA?A?,A¡íA©øAA¢®)A¢¯A¢®A¢¥A? A¡ÆA©øA¢æAŽA¡íA©øAA¢®A¢æA¡í A¡íA¡×A¢¯A¡ìA¢ÔA¡¯ A¨ùA¢Ò A¨úA¢¬A¨öA¢æA¢¥AA¢¥A¢â."));
		return;
	}

	int sans =  BiyologSistemi[ch->GetQuestFlag("bio.durum")][3];
	int toplam =  BiyologSistemi[ch->GetQuestFlag("bio.durum")][1];
	int level =  ch->GetLevel();

	int affectvnum =  BiyologSistemi[ch->GetQuestFlag("bio.durum")][6];
	int affectvalue =  BiyologSistemi[ch->GetQuestFlag("bio.durum")][7];
	int affectvnum2 =  BiyologSistemi[ch->GetQuestFlag("bio.durum")][8];
	int affectvalue2 =  BiyologSistemi[ch->GetQuestFlag("bio.durum")][9];
	int affectvnum3 =  BiyologSistemi[ch->GetQuestFlag("bio.durum")][10];
	int affectvalue3 =  BiyologSistemi[ch->GetQuestFlag("bio.durum")][11];
	int affectvnum4 =  BiyologSistemi[ch->GetQuestFlag("bio.durum")][12];
	int affectvalue4 =  BiyologSistemi[ch->GetQuestFlag("bio.durum")][13];
	int unlimited = 60*60*60*365;

	if(level < 30)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biolevelyok"));
		return;
	}

	if(ch->GetQuestFlag("bio.durum") > 10)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biyologbitmis"));
		return;
	}

	DWORD dwVnum = 0;
	DWORD dwVnum2 = 0;
	str_to_number(dwVnum, arg2);
	str_to_number(dwVnum2, arg3);

	const std::string& strArg1 = std::string(arg1);
	if(strArg1 == "request")
	{
		LPITEM item = ch->GetItem(TItemPos(INVENTORY, dwVnum));
		if (item != NULL)
		{
			if (item->GetVnum() == 70022)
			{
				if(ch->GetQuestFlag("bio.durum") > 10)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biyologbitmis"));
					return;
				}

				if (ch->CountSpecifyItem(70022) < 1)
				{
					return;
				}

				if(int(ch->GetQuestFlag("bio.sure")) == 1)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biosurezatenaktif"));
				}
				else if(ch->GetQuestFlag("bio.ruhtasi") == 2)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioruhdayapamazsin"));
				}
				else
				{
					item->SetCount(item->GetCount() - 1);
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biosureverildi"));
					ch->SetQuestFlag("bio.sure",1);
					ch->SetQuestFlag("bio.kalan",get_global_time()+0);
					ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog_update %d %d %d %d %d", ch->GetQuestFlag("bio.durum"), ch->GetQuestFlag("bio.ruhtasi"), ch->GetQuestFlag("bio.verilen"), BiyologSistemi[ch->GetQuestFlag("bio.durum")][1], ch->GetQuestFlag("bio.kalan") - get_global_time());
				}
			}
		}

		LPITEM item2 = ch->GetItem(TItemPos(INVENTORY, dwVnum2));
		if (item2 != NULL)
		{
			if(ch->GetQuestFlag("bio.durum") > 10)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biyologbitmis"));
				return;
			}

			int SANS_ITEMLER[3] = 
			{
				71035,
				76020,
				39023,
			};

			for (int it = 0; it <= 3; it++)
			{
				if (item2->GetVnum() == SANS_ITEMLER[it])
				{
					if (ch->CountSpecifyItem(SANS_ITEMLER[it]) < 1)
					{
						return;
					}

					if(int(ch->GetQuestFlag("bio.sans")) == 1)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biosanszatenaktif"));
					}
					else if(ch->GetQuestFlag("bio.ruhtasi") == 2)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioruhdayapamazsin"));
					}
					else
					{
						item2->SetCount(item2->GetCount() - 1);
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biosansverildi"));
						ch->SetQuestFlag("bio.sans", 1);
					}
				}
			}
		}

		if(ch->GetQuestFlag("bio.kalan") > get_global_time())
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biosuredolmadi"));
			return;
		}

		if(ch->GetQuestFlag("bio.durum") == 1 || ch->GetQuestFlag("bio.durum") == 2 || ch->GetQuestFlag("bio.durum") == 3 || ch->GetQuestFlag("bio.durum") == 4 || ch->GetQuestFlag("bio.durum") == 5 || ch->GetQuestFlag("bio.durum") == 6 || ch->GetQuestFlag("bio.durum") == 7 || ch->GetQuestFlag("bio.durum") == 8 || ch->GetQuestFlag("bio.durum") == 9 || ch->GetQuestFlag("bio.durum") == 10)
		{
			if(ch->GetQuestFlag("bio.durum") > 10)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biyologbitmis"));
				return;
			}

			if (ch->CountSpecifyItem(BiyologSistemi[ch->GetQuestFlag("bio.durum")][0]) < 1)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioitemyok"));
				return;
			}
			else
			{
				int prob = number(1,100);
				if(ch->GetQuestFlag("bio.sans") == 1)
				{
					sans = sans +100;
				}
				if(ch->GetQuestFlag("bio.sure") == 1)
				{
					ch->SetQuestFlag("bio.sure",0);
				}

				if(sans >= prob)
				{
					if (ch->GetQuestFlag("bio.verilen") >= BiyologSistemi[ch->GetQuestFlag("bio.durum")][1])
					{
						return;
					}

					ch->SetQuestFlag("bio.verilen",ch->GetQuestFlag("bio.verilen")+1);
					if(ch->GetQuestFlag("bio.sans") == 1)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioozutgitti"));
						ch->SetQuestFlag("bio.sans",0);
					}

					if(ch->GetQuestFlag("bio.verilen") == toplam)
					{
						if (ch->GetQuestFlag("bio.durum") == 9)
						{
							ch->SetQuestFlag("bio.ruhtasi",3);
							ch->SetQuestFlag("bio.odulvakti",1);
						}
						else
						{
							TItemTable* pTable = ITEM_MANAGER::instance().GetTable(BiyologSistemi[ch->GetQuestFlag("bio.durum")][4]);
							ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biotoplamabittiruhtasibul %s"), pTable->szLocaleName);
							ch->SetQuestFlag("bio.ruhtasi",2);
						}
					}
					else
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogecti %d"), (toplam-ch->GetQuestFlag("bio.verilen")));
						ch->SetQuestFlag("bio.kalan",get_global_time()+(BiyologSistemi[ch->GetQuestFlag("bio.durum")][2]*60));
					}			
				}
				else
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biobasarisiz"));
					ch->SetQuestFlag("bio.kalan",get_global_time()+(BiyologSistemi[ch->GetQuestFlag("bio.durum")][2]*60));
				}
				ch->RemoveSpecifyItem(BiyologSistemi[ch->GetQuestFlag("bio.durum")][0],1);
			}
		}

		ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog_update %d %d %d %d %d", ch->GetQuestFlag("bio.durum"), ch->GetQuestFlag("bio.ruhtasi"), ch->GetQuestFlag("bio.verilen"), BiyologSistemi[ch->GetQuestFlag("bio.durum")][1], ch->GetQuestFlag("bio.kalan") - get_global_time());
		return;
	}

	if(strArg1 == "stone")
	{
		if(ch->GetQuestFlag("bio.durum") > 10)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biyologbitmis"));
			return;
		}

		if(ch->GetQuestFlag("bio.durum") == 1 || ch->GetQuestFlag("bio.durum") == 2 || ch->GetQuestFlag("bio.durum") == 3 || ch->GetQuestFlag("bio.durum") == 4 || ch->GetQuestFlag("bio.durum") == 5 || ch->GetQuestFlag("bio.durum") == 6 || ch->GetQuestFlag("bio.durum") == 7 || ch->GetQuestFlag("bio.durum") == 8 || ch->GetQuestFlag("bio.durum") == 9 || ch->GetQuestFlag("bio.durum") == 10)
		{
			if (ch->GetQuestFlag("bio.verilen") >= BiyologSistemi[ch->GetQuestFlag("bio.durum")][1] && ch->GetQuestFlag("bio.ruhtasi") == 2)
			{
				if (ch->CountSpecifyItem(BiyologSistemi[ch->GetQuestFlag("bio.durum")][4]) < 1)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioruhtasiyok"));
					return;
				}
				else
				{
					ch->RemoveSpecifyItem(BiyologSistemi[ch->GetQuestFlag("bio.durum")][4],1);

					if(ch->GetQuestFlag("bio.durum") == 9 || ch->GetQuestFlag("bio.durum") == 10)
					{
						ch->SetQuestFlag("bio.ruhtasi",3);
						ch->SetQuestFlag("bio.odulvakti",1);
					}
					else
					{
						ch->SetQuestFlag("bio.ruhtasi",3);
					}
				}
			}
		}


		ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog_update %d %d %d %d %d", ch->GetQuestFlag("bio.durum"), ch->GetQuestFlag("bio.ruhtasi"), ch->GetQuestFlag("bio.verilen"), BiyologSistemi[ch->GetQuestFlag("bio.durum")][1], ch->GetQuestFlag("bio.kalan") - get_global_time());
		return;
	}

	if(strArg1 == "complate")
	{
		if(ch->GetQuestFlag("bio.durum") > 10)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biyologbitmis"));
			return;
		}


		if(ch->GetQuestFlag("bio.durum") == 1)
		{
			if (ch->GetQuestFlag("bio.verilen") >= (int)BiyologSistemi[ch->GetQuestFlag("bio.durum")][1] && ch->GetQuestFlag("bio.ruhtasi") == 3)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogorevtamamlandi"));
				ch->AutoGiveItem(BiyologSistemi[ch->GetQuestFlag("bio.durum")][5], 1);
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioyenigorev"));

				ch->AddAffect(AFFECT_BIO, aApplyInfo[affectvnum].bPointType, affectvalue, 0, 60*60*24*365*60, 0, false);

				ch->SetQuestFlag("bio.durum",ch->GetQuestFlag("bio.durum")+1);
				ch->SetQuestFlag("bio.verilen",0);
				ch->SetQuestFlag("bio.kalan",get_global_time()+0);
				ch->SetQuestFlag("bio.ruhtasi",1);
				ch->SetQuestFlag("bio.30",1);
				ch->SetQuestFlag("bio.bildiri",ch->GetQuestFlag("bio.bildiri")+1);
				ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog_update %d %d %d %d %d", ch->GetQuestFlag("bio.durum"), ch->GetQuestFlag("bio.ruhtasi"), ch->GetQuestFlag("bio.verilen"), BiyologSistemi[ch->GetQuestFlag("bio.durum")][1], ch->GetQuestFlag("bio.kalan") - get_global_time());
				return;
			}
		}

		if(ch->GetQuestFlag("bio.durum") == 2)
		{
			if (ch->GetQuestFlag("bio.verilen") >= (int)BiyologSistemi[ch->GetQuestFlag("bio.durum")][1] && ch->GetQuestFlag("bio.ruhtasi") == 3)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogorevtamamlandi"));
				ch->AutoGiveItem(BiyologSistemi[ch->GetQuestFlag("bio.durum")][5], 1);
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioyenigorev"));

				ch->AddAffect(AFFECT_BIO, aApplyInfo[affectvnum].bPointType, affectvalue, 0, 60*60*24*365*60, 0, false);

				ch->SetQuestFlag("bio.durum",ch->GetQuestFlag("bio.durum")+1);
				ch->SetQuestFlag("bio.verilen",0);
				ch->SetQuestFlag("bio.kalan",get_global_time()+0);
				ch->SetQuestFlag("bio.ruhtasi",1);
				ch->SetQuestFlag("bio.40",1);
				ch->SetQuestFlag("bio.bildiri",ch->GetQuestFlag("bio.bildiri")+1);
				ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog_update %d %d %d %d %d", ch->GetQuestFlag("bio.durum"), ch->GetQuestFlag("bio.ruhtasi"), ch->GetQuestFlag("bio.verilen"), BiyologSistemi[ch->GetQuestFlag("bio.durum")][1], ch->GetQuestFlag("bio.kalan") - get_global_time());
				return;
			}
		}

		if(ch->GetQuestFlag("bio.durum") == 3)
		{
			if (ch->GetQuestFlag("bio.verilen") >= (int)BiyologSistemi[ch->GetQuestFlag("bio.durum")][1] && ch->GetQuestFlag("bio.ruhtasi") == 3)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogorevtamamlandi"));
				ch->AutoGiveItem(BiyologSistemi[ch->GetQuestFlag("bio.durum")][5], 1);
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioyenigorev"));

				ch->AddAffect(AFFECT_BIO, aApplyInfo[affectvnum].bPointType, affectvalue, 0, 60*60*24*365*60, 0, false);

				ch->SetQuestFlag("bio.durum",ch->GetQuestFlag("bio.durum")+1);
				ch->SetQuestFlag("bio.verilen",0);
				ch->SetQuestFlag("bio.kalan",get_global_time()+0);
				ch->SetQuestFlag("bio.ruhtasi",1);
				ch->SetQuestFlag("bio.50",1);
				ch->SetQuestFlag("bio.bildiri",ch->GetQuestFlag("bio.bildiri")+1);
				ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog_update %d %d %d %d %d", ch->GetQuestFlag("bio.durum"), ch->GetQuestFlag("bio.ruhtasi"), ch->GetQuestFlag("bio.verilen"), BiyologSistemi[ch->GetQuestFlag("bio.durum")][1], ch->GetQuestFlag("bio.kalan") - get_global_time());
				return;
			}
		}

		if(ch->GetQuestFlag("bio.durum") == 4)
		{
			if (ch->GetQuestFlag("bio.verilen") >= (int)BiyologSistemi[ch->GetQuestFlag("bio.durum")][1] && ch->GetQuestFlag("bio.ruhtasi") == 3)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogorevtamamlandi"));
				ch->AutoGiveItem(BiyologSistemi[ch->GetQuestFlag("bio.durum")][5], 1);
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioyenigorev"));

				ch->AddAffect(AFFECT_BIO, aApplyInfo[affectvnum].bPointType, affectvalue, 0, 60*60*24*365*60, 0, false);

				ch->SetQuestFlag("bio.durum",ch->GetQuestFlag("bio.durum")+1);
				ch->SetQuestFlag("bio.verilen",0);
				ch->SetQuestFlag("bio.kalan",get_global_time()+0);
				ch->SetQuestFlag("bio.ruhtasi",1);
				ch->SetQuestFlag("bio.60",1);
				ch->SetQuestFlag("bio.bildiri",ch->GetQuestFlag("bio.bildiri")+1);
				ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog_update %d %d %d %d %d", ch->GetQuestFlag("bio.durum"), ch->GetQuestFlag("bio.ruhtasi"), ch->GetQuestFlag("bio.verilen"), BiyologSistemi[ch->GetQuestFlag("bio.durum")][1], ch->GetQuestFlag("bio.kalan") - get_global_time());
				return;
			}
		}

		if(ch->GetQuestFlag("bio.durum") == 5)
		{
			if (ch->GetQuestFlag("bio.verilen") >= (int)BiyologSistemi[ch->GetQuestFlag("bio.durum")][1] && ch->GetQuestFlag("bio.ruhtasi") == 3)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogorevtamamlandi"));
				ch->AutoGiveItem(BiyologSistemi[ch->GetQuestFlag("bio.durum")][5], 1);
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioyenigorev"));

				ch->AddAffect(AFFECT_BIO, aApplyInfo[affectvnum].bPointType, affectvalue, 0, 60*60*24*365*60, 0, false);
				ch->AddAffect(AFFECT_BIO, aApplyInfo[affectvnum2].bPointType, affectvalue2, 0, 60*60*24*365*60, 0, false);

				ch->SetQuestFlag("bio.durum",ch->GetQuestFlag("bio.durum")+1);
				ch->SetQuestFlag("bio.verilen",0);
				ch->SetQuestFlag("bio.kalan",get_global_time()+0);
				ch->SetQuestFlag("bio.ruhtasi",1);
				ch->SetQuestFlag("bio.70",1);
				ch->SetQuestFlag("bio.bildiri",ch->GetQuestFlag("bio.bildiri")+1);
				ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog_update %d %d %d %d %d", ch->GetQuestFlag("bio.durum"), ch->GetQuestFlag("bio.ruhtasi"), ch->GetQuestFlag("bio.verilen"), BiyologSistemi[ch->GetQuestFlag("bio.durum")][1], ch->GetQuestFlag("bio.kalan") - get_global_time());
				return;
			}
		}

		if(ch->GetQuestFlag("bio.durum") == 6)
		{
			if (ch->GetQuestFlag("bio.verilen") >= (int)BiyologSistemi[ch->GetQuestFlag("bio.durum")][1] && ch->GetQuestFlag("bio.ruhtasi") == 3)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogorevtamamlandi"));
				ch->AutoGiveItem(BiyologSistemi[ch->GetQuestFlag("bio.durum")][5], 1);
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioyenigorev"));

				ch->AddAffect(AFFECT_BIO, aApplyInfo[affectvnum].bPointType, affectvalue, 0, 60*60*24*365*60, 0, false);
				ch->AddAffect(AFFECT_BIO, aApplyInfo[affectvnum2].bPointType, affectvalue2, 0, 60*60*24*365*60, 0, false);

				ch->SetQuestFlag("bio.durum",ch->GetQuestFlag("bio.durum")+1);
				ch->SetQuestFlag("bio.verilen",0);
				ch->SetQuestFlag("bio.kalan",get_global_time()+0);
				ch->SetQuestFlag("bio.ruhtasi",1);
				ch->SetQuestFlag("bio.80",1);
				ch->SetQuestFlag("bio.bildiri",ch->GetQuestFlag("bio.bildiri")+1);
				ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog_update %d %d %d %d %d", ch->GetQuestFlag("bio.durum"), ch->GetQuestFlag("bio.ruhtasi"), ch->GetQuestFlag("bio.verilen"), BiyologSistemi[ch->GetQuestFlag("bio.durum")][1], ch->GetQuestFlag("bio.kalan") - get_global_time());
				return;
			}
		}

		if(ch->GetQuestFlag("bio.durum") == 7)
		{
			if (ch->GetQuestFlag("bio.verilen") >= (int)BiyologSistemi[ch->GetQuestFlag("bio.durum")][1] && ch->GetQuestFlag("bio.ruhtasi") == 3)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogorevtamamlandi"));
				ch->AutoGiveItem(BiyologSistemi[ch->GetQuestFlag("bio.durum")][5], 1);
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioyenigorev"));

				ch->AddAffect(AFFECT_BIO, aApplyInfo[affectvnum].bPointType, affectvalue, 0, 60*60*24*365*60, 0, false);
				ch->AddAffect(AFFECT_BIO, aApplyInfo[affectvnum2].bPointType, affectvalue2, 0, 60*60*24*365*60, 0, false);
				ch->AddAffect(AFFECT_BIO, aApplyInfo[affectvnum3].bPointType, affectvalue3, 0, 60*60*24*365*60, 0, false);
				ch->AddAffect(AFFECT_BIO, aApplyInfo[affectvnum4].bPointType, affectvalue4, 0, 60*60*24*365*60, 0, false);

				ch->SetQuestFlag("bio.durum",ch->GetQuestFlag("bio.durum")+1);
				ch->SetQuestFlag("bio.verilen",0);
				ch->SetQuestFlag("bio.kalan",get_global_time()+0);
				ch->SetQuestFlag("bio.ruhtasi",1);
				ch->SetQuestFlag("bio.85",1);
				ch->SetQuestFlag("bio.bildiri",ch->GetQuestFlag("bio.bildiri")+1);
				ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog_update %d %d %d %d %d", ch->GetQuestFlag("bio.durum"), ch->GetQuestFlag("bio.ruhtasi"), ch->GetQuestFlag("bio.verilen"), BiyologSistemi[ch->GetQuestFlag("bio.durum")][1], ch->GetQuestFlag("bio.kalan") - get_global_time());
				return;
			}
		}

		if(ch->GetQuestFlag("bio.durum") == 8)
		{
			if (ch->GetQuestFlag("bio.verilen") >= (int)BiyologSistemi[ch->GetQuestFlag("bio.durum")][1] && ch->GetQuestFlag("bio.ruhtasi") == 3)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogorevtamamlandi"));
				ch->AutoGiveItem(BiyologSistemi[ch->GetQuestFlag("bio.durum")][5], 1);
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioyenigorev"));

				ch->AddAffect(AFFECT_BIO, aApplyInfo[affectvnum].bPointType, affectvalue, 0, 60*60*24*365*60, 0, false);

				ch->SetQuestFlag("bio.durum",ch->GetQuestFlag("bio.durum")+1);
				ch->SetQuestFlag("bio.verilen",0);
				ch->SetQuestFlag("bio.kalan",get_global_time()+0);
				ch->SetQuestFlag("bio.ruhtasi",1);
				ch->SetQuestFlag("bio.90",1);
				ch->SetQuestFlag("bio.bildiri",ch->GetQuestFlag("bio.bildiri")+1);
				ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog_update %d %d %d %d %d", ch->GetQuestFlag("bio.durum"), ch->GetQuestFlag("bio.ruhtasi"), ch->GetQuestFlag("bio.verilen"), BiyologSistemi[ch->GetQuestFlag("bio.durum")][1], ch->GetQuestFlag("bio.kalan") - get_global_time());
				return;
			}
		}

		if(ch->GetQuestFlag("bio.durum") == 9)
		{
			if (ch->GetQuestFlag("bio.odulvakti") == 0)
			{
				return;
			}

			if (ch->GetQuestFlag("bio.odulvakti") == 1 and level >= 92)
            {
				if (ch->GetQuestFlag("bio.verilen") >= BiyologSistemi[ch->GetQuestFlag("bio.durum")][1] && ch->GetQuestFlag("bio.ruhtasi") == 3)
				{
					if(dwVnum == 1)
					{
						ch->AddAffect(AFFECT_BIO_92, aApplyInfo[affectvnum].bPointType, affectvalue, 0, unlimited, 0, false);
						ch->SetQuestFlag("bio.durum",10);
						ch->SetQuestFlag("bio.92",1);
						ch->SetQuestFlag("bio.verilen",0);
						ch->SetQuestFlag("bio.kalan",get_global_time()+0);
						ch->SetQuestFlag("bio.ruhtasi",1);
						ch->SetQuestFlag("bio.odulvakti",0);
						ch->SetQuestFlag("bio.bildiri",ch->GetQuestFlag("bio.bildiri")+1);
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogorevtamamlandi"));
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioyenigorev"));
						ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog_update %d %d %d %d %d", ch->GetQuestFlag("bio.durum"), ch->GetQuestFlag("bio.ruhtasi"), ch->GetQuestFlag("bio.verilen"), BiyologSistemi[ch->GetQuestFlag("bio.durum")][1], ch->GetQuestFlag("bio.kalan") - get_global_time());
						return;
					}

					if(dwVnum == 2)
					{
						ch->AddAffect(AFFECT_BIO_92, aApplyInfo[affectvnum2].bPointType, affectvalue2, 0, unlimited, 0, false);
						ch->SetQuestFlag("bio.durum",10);
						ch->SetQuestFlag("bio.92",1);
						ch->SetQuestFlag("bio.verilen",0);
						ch->SetQuestFlag("bio.kalan",get_global_time()+0);
						ch->SetQuestFlag("bio.ruhtasi",1);
						ch->SetQuestFlag("bio.odulvakti",0);
						ch->SetQuestFlag("bio.bildiri",ch->GetQuestFlag("bio.bildiri")+1);
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogorevtamamlandi"));
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioyenigorev"));
						ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog_update %d %d %d %d %d", ch->GetQuestFlag("bio.durum"), ch->GetQuestFlag("bio.ruhtasi"), ch->GetQuestFlag("bio.verilen"), BiyologSistemi[ch->GetQuestFlag("bio.durum")][1], ch->GetQuestFlag("bio.kalan") - get_global_time());
						return;
					}

					if(dwVnum == 3)
					{
						ch->AddAffect(AFFECT_BIO_92, aApplyInfo[affectvnum3].bPointType, affectvalue3, 0, unlimited, 0, false);
						ch->SetQuestFlag("bio.durum",10);
						ch->SetQuestFlag("bio.92",1);
						ch->SetQuestFlag("bio.verilen",0);
						ch->SetQuestFlag("bio.kalan",get_global_time()+0);
						ch->SetQuestFlag("bio.ruhtasi",1);
						ch->SetQuestFlag("bio.odulvakti",0);
						ch->SetQuestFlag("bio.bildiri",ch->GetQuestFlag("bio.bildiri")+1);
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogorevtamamlandi"));
						ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog_update %d %d %d %d %d", ch->GetQuestFlag("bio.durum"), ch->GetQuestFlag("bio.ruhtasi"), ch->GetQuestFlag("bio.verilen"), BiyologSistemi[ch->GetQuestFlag("bio.durum")][1], ch->GetQuestFlag("bio.kalan") - get_global_time());
						return;
					}
				}
            }
		}

		if(ch->GetQuestFlag("bio.durum") == 10)
		{
			if (ch->GetQuestFlag("bio.odulvakti") == 0)
			{
				return;
			}

			if (ch->GetQuestFlag("bio.odulvakti") == 1 and level >= 94)
            {
				if (ch->GetQuestFlag("bio.verilen") >= BiyologSistemi[ch->GetQuestFlag("bio.durum")][1] && ch->GetQuestFlag("bio.ruhtasi") == 3)
				{
					if(dwVnum == 1)
					{
						ch->AddAffect(AFFECT_BIO_94, aApplyInfo[affectvnum].bPointType, affectvalue, 0, unlimited, 0, false);
						ch->SetQuestFlag("bio.durum",11);
						ch->SetQuestFlag("bio.94",1);
						ch->SetQuestFlag("bio.verilen",0);
						ch->SetQuestFlag("bio.kalan",get_global_time()+0);
						ch->SetQuestFlag("bio.ruhtasi",1);
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogorevtamamlandi"));
						ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog_update %d %d %d %d %d", ch->GetQuestFlag("bio.durum"), ch->GetQuestFlag("bio.ruhtasi"), ch->GetQuestFlag("bio.verilen"), BiyologSistemi[ch->GetQuestFlag("bio.durum")][1], ch->GetQuestFlag("bio.kalan") - get_global_time());
						return;
					}

					if(dwVnum == 2)
					{
						ch->AddAffect(AFFECT_BIO_94, aApplyInfo[affectvnum2].bPointType, affectvalue2, 0, unlimited, 0, false);
						ch->SetQuestFlag("bio.durum",11);
						ch->SetQuestFlag("bio.94",1);
						ch->SetQuestFlag("bio.verilen",0);
						ch->SetQuestFlag("bio.kalan",get_global_time()+0);
						ch->SetQuestFlag("bio.ruhtasi",1);
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogorevtamamlandi"));
						ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog_update %d %d %d %d %d", ch->GetQuestFlag("bio.durum"), ch->GetQuestFlag("bio.ruhtasi"), ch->GetQuestFlag("bio.verilen"), BiyologSistemi[ch->GetQuestFlag("bio.durum")][1], ch->GetQuestFlag("bio.kalan") - get_global_time());
						return;
					}

					if(dwVnum == 3)
					{
						ch->AddAffect(AFFECT_BIO_94, aApplyInfo[affectvnum3].bPointType, affectvalue3, 0, unlimited, 0, false);
						ch->SetQuestFlag("bio.durum",11);
						ch->SetQuestFlag("bio.94",1);
						ch->SetQuestFlag("bio.verilen",0);
						ch->SetQuestFlag("bio.kalan",get_global_time()+0);
						ch->SetQuestFlag("bio.ruhtasi",1);
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogorevtamamlandi"));
						ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog_update %d %d %d %d %d", ch->GetQuestFlag("bio.durum"), ch->GetQuestFlag("bio.ruhtasi"), ch->GetQuestFlag("bio.verilen"), BiyologSistemi[ch->GetQuestFlag("bio.durum")][1], ch->GetQuestFlag("bio.kalan") - get_global_time());
						return;
					}
				}
            }
		}
	}

	if(strArg1 == "all")
	{
		if (quest::CQuestManager::instance().GetEventFlag("biyolog_hizli") == 1)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "Sistem suan icin devre disi!");
			return;
		}

		if (biyologtimer)
		{
			event_cancel(&biyologtimer);
		}

		TMainEventInfo5* info = AllocEventInfo<TMainEventInfo5>();

		info->kim = ch;
		info->deger = toplam;
		info->itemim1 = dwVnum;
		info->itemim2 = dwVnum2;
		biyologtimer = event_create(biyolog_event, info, PASSES_PER_SEC(1));
	}

	return;
}
#endif
