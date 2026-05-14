#include <stdio.h>
#include <string.h>
#include "vip_tag.h"

VIPTag g_VIPTag;

IVIPApi* g_pVIPCore;

IVEngineServer2* engine = nullptr;
CGameEntitySystem* g_pGameEntitySystem = nullptr;
CEntitySystem* g_pEntitySystem = nullptr;

#define VIP_TAG_COOKIE "vip_tag_display"

PLUGIN_EXPOSE(VIPTag, g_VIPTag);
bool VIPTag::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();
	GET_V_IFACE_ANY(GetEngineFactory, g_pSchemaSystem, ISchemaSystem, SCHEMASYSTEM_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer2, SOURCE2ENGINETOSERVER_INTERFACE_VERSION);
	g_SMAPI->AddListener( this, this );
	return true;
}

bool VIPTag::Unload(char *error, size_t maxlen)
{
	delete g_pVIPCore;
	return true;
}

static bool IsVIPTagEnabled(int iSlot)
{
	if (!g_pVIPCore) return true;
	const char* c = g_pVIPCore->VIP_GetClientCookie(iSlot, VIP_TAG_COOKIE);
	return !(c && !strcmp(c, "off"));
}

static void SetVIPTagEnabled(int iSlot, bool on)
{
	if (!g_pVIPCore) return;
	g_pVIPCore->VIP_SetClientCookie(iSlot, VIP_TAG_COOKIE, on ? "on" : "off");
}

static void ApplyVIPTagNow(int iSlot)
{
	CCSPlayerController* pc = CCSPlayerController::FromSlot(iSlot);
	if (!pc) return;
	const char* szClan = g_pVIPCore->VIP_GetClientFeatureString(iSlot, "clantag");
	if (szClan && *szClan)
		pc->m_szClan() = CUtlSymbolLarge(szClan);
	else
		pc->m_szClan() = CUtlSymbolLarge("\0");
}

static void ClearVIPTagNow(int iSlot)
{
	CCSPlayerController* pc = CCSPlayerController::FromSlot(iSlot);
	if (!pc) return;
	pc->m_szClan() = CUtlSymbolLarge("\0");
}

static bool VIP_TagToggleCallback(int iSlot, const char* /*szFeature*/, VIP_ToggleState /*eOld*/, VIP_ToggleState& eNew)
{
	if (eNew == ENABLED) {
		SetVIPTagEnabled(iSlot, true);
		ApplyVIPTagNow(iSlot);
	} else if (eNew == DISABLED) {
		SetVIPTagEnabled(iSlot, false);
		ClearVIPTagNow(iSlot);
	}
	return true;
}

void VIP_OnPlayerSpawn(int iSlot, int iTeam, bool bIsVIP)
{
	if(!bIsVIP) return;
	if(!IsVIPTagEnabled(iSlot)) return;

	CCSPlayerController* pPlayerController = CCSPlayerController::FromSlot(iSlot);
	if(!pPlayerController) return;
	const char* szClan = g_pVIPCore->VIP_GetClientFeatureString(iSlot, "clantag");
	if(szClan && strlen(szClan) > 0)
		pPlayerController->m_szClan() = CUtlSymbolLarge(szClan);
	else
		pPlayerController->m_szClan() = CUtlSymbolLarge("\0");
}

void VIP_OnVIPClientRemoved(int iSlot, int iReason)
{
	CCSPlayerController* pPlayerController = CCSPlayerController::FromSlot(iSlot);
	if(!pPlayerController) return;
	pPlayerController->m_szClan() = CUtlSymbolLarge("\0");
}

CGameEntitySystem* GameEntitySystem()
{
    return g_pVIPCore->VIP_GetEntitySystem();
};

void VIP_OnVIPLoaded()
{
	g_pGameEntitySystem = GameEntitySystem();
	g_pEntitySystem = g_pGameEntitySystem;
	g_pVIPCore->VIP_OnPlayerSpawn(VIP_OnPlayerSpawn);
	g_pVIPCore->VIP_OnVIPClientRemoved(VIP_OnVIPClientRemoved);
}

void VIPTag::AllPluginsLoaded()
{
	int ret;
	g_pVIPCore = (IVIPApi*)g_SMAPI->MetaFactory(VIP_INTERFACE, &ret, NULL);

	if (ret == META_IFACE_FAILED)
	{
		char error[64];
		V_strncpy(error, "Failed to lookup vip core. Aborting", 64);
		ConColorMsg(Color(255, 0, 0, 255), "[%s] %s\n", GetLogTag(), error);
		std::string sBuffer = "meta unload "+std::to_string(g_PLID);
		engine->ServerCommand(sBuffer.c_str());
		return;
	}
	g_pVIPCore->VIP_OnVIPLoaded(VIP_OnVIPLoaded);
	g_pVIPCore->VIP_RegisterFeature("clantag", VIP_STRING, TOGGLABLE, nullptr, VIP_TagToggleCallback);
}

const char *VIPTag::GetLicense()
{
	return "Public";
}

const char *VIPTag::GetVersion()
{
	return "1.1";
}

const char *VIPTag::GetDate()
{
	return __DATE__;
}

const char *VIPTag::GetLogTag()
{
	return "[VIP-TAG]";
}

const char *VIPTag::GetAuthor()
{
	return "Pisex";
}

const char *VIPTag::GetDescription()
{
	return "Puts a vip tag on the player";
}

const char *VIPTag::GetName()
{
	return "[VIP] Tag";
}

const char *VIPTag::GetURL()
{
	return "https://discord.com/invite/g798xERK5Y";
}
