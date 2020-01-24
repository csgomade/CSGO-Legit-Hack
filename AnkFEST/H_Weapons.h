#include "H_Include.h"

#define WEAPON_NONE 0
#define WEAPON_KNIFE 42
#define WEAPON_KNIFEGG 41
#define WEAPON_TASER 31
#define WEAPON_GLOCK 4
#define WEAPON_ELITE 2
#define WEAPON_P250 36
#define WEAPON_TEC9 30
#define WEAPON_DEAGLE 1
#define WEAPON_HKP2000 32
#define WEAPON_USP 61
#define WEAPON_FIVESEVEN 3
#define WEAPON_CZ75 63
#define WEAPON_NOVA 35
#define WEAPON_XM1014 25
#define WEAPON_SAWEDOFF 29
#define WEAPON_MAG7 27
#define WEAPON_MAC10 17
#define WEAPON_MP7 33
#define WEAPON_UMP45 24
#define WEAPON_P90 19
#define WEAPON_MP9 34
#define WEAPON_BIZON 26
#define WEAPON_GALILAR 13
#define WEAPON_AK47 7
#define WEAPON_SG553 39
#define WEAPON_M4A4 16
#define WEAPON_M4A1 60
#define WEAPON_FAMAS 10
#define WEAPON_AUG 8
#define WEAPON_SSG08 40
#define WEAPON_AWP 9
#define WEAPON_G3SG1 11
#define WEAPON_SCAR20 38
#define WEAPON_M249 14
#define WEAPON_NEGEV 28
#define WEAPON_HE 44
#define WEAPON_FLASH 43
#define WEAPON_SMOKE 45
#define WEAPON_MOLOTOV 46
#define WEAPON_DECOY 47
#define WEAPON_INCGRENADE 48
#define WEAPON_C4 49

class C_Nospread
{
public:
	bool IsBadWeapon(int weaponid)
	{
		return(weaponid == WEAPON_KNIFEGG || weaponid == WEAPON_KNIFE || weaponid == WEAPON_NONE || weaponid == WEAPON_FLASH || weaponid == WEAPON_SMOKE || weaponid == WEAPON_MOLOTOV || weaponid == WEAPON_DECOY || weaponid == WEAPON_INCGRENADE || weaponid == WEAPON_C4);
	}

	bool IsNonRcs(int weaponid)
	{
		return(weaponid == WEAPON_DEAGLE || weaponid == WEAPON_ELITE || weaponid == WEAPON_FIVESEVEN || weaponid == WEAPON_GLOCK || weaponid == WEAPON_P250 || weaponid == WEAPON_HKP2000 || weaponid == WEAPON_USP || weaponid == WEAPON_TEC9);
	}
}; extern C_Nospread g_Nospread;
