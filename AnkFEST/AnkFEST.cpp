#include "H_Include.h"

#define MPI 3.14159265358979323846
#define DegToRad MPI / 180

C_GlobalVars g_Vars;
C_Module g_Module;
C_Math g_Math;
C_Nospread g_Nospread;

int UPD = 100;
int UPDSET = 50;

bool GeneralPanic = false;

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

HWND m_hWnd;

bool InitializeSettings()
{
	GetModuleFileName((HINSTANCE)&__ImageBase, g_Vars.ini, _countof(g_Vars.ini));

	for (int i = 0; i < (int)strlen(g_Vars.ini); i++)
	{
		if (g_Vars.ini[strlen(g_Vars.ini) - i] == '\\')
		{
			g_Vars.ini[(strlen(g_Vars.ini) - i) + 1] = '\0';
			strcat(g_Vars.ini, ".\\Config.cfg");
			break;
		}
	}

	g_Vars.Aimbot.Key = g_Module.GetPrivateProfileFloat("Horcrux", "aimbot.key", 0, g_Vars.ini);
	g_Vars.EndKey = g_Module.GetPrivateProfileFloat("Horcrux", "aimbot.end.key", 0, g_Vars.ini);
	g_Vars.Triggerbot.Key = g_Module.GetPrivateProfileFloat("Horcrux", "triggerbot.key", 0, g_Vars.ini);
	g_Vars.ReloadKey = g_Module.GetPrivateProfileFloat("Horcrux", "cfg.reload.key", 0, g_Vars.ini);
	g_Vars.Aimbot.Enabled = 1;
	g_Vars.Aimbot.FOV = 0;
	g_Vars.Aimbot.Smooth = 0;
	g_Vars.Aimbot.Bone = 0;
	g_Vars.Aimbot.RCS = 0;
	g_Vars.Triggerbot.Enabled = 1;

	return true;
}

typedef struct C_LocalBase
{
	PBYTE LocalBase;
	PBYTE AngBase;
	PBYTE CWeaponBase;

	float flAngle[3];
	float i1[3];
	float MeFov[3];
	float recoil[3];
	float vPosition[3];

	int MyTeam;
	int	m_WeaponHandle;
	int	m_WeaponIDFirst;

	void Clear()
	{
		this->LocalBase = 0x0;
		this->AngBase = 0x0;
		this->CWeaponBase = 0x0;
	}

	void ReadStructure()
	{
		this->Clear();

		ReadProcessMemory(g_Vars.hProcess, g_Vars.dwClient + c_dwLocalBaseEntity, &LocalBase, sizeof(LocalBase), NULL);
		ReadProcessMemory(g_Vars.hProcess, g_Vars.dwEngine + ViewAng, &AngBase, sizeof(AngBase), NULL);
		ReadProcessMemory(g_Vars.hProcess, AngBase + ViewAngOff, &flAngle, sizeof(flAngle), NULL);
		ReadProcessMemory(g_Vars.hProcess, LocalBase + hitboxpos, &i1, sizeof(i1), NULL);
		ReadProcessMemory(g_Vars.hProcess, LocalBase + MyTeamAD, &MyTeam, sizeof(MyTeam), NULL);
		ReadProcessMemory(g_Vars.hProcess, AngBase + ViewAngOff, &MeFov, sizeof(MeFov), NULL);
		ReadProcessMemory(g_Vars.hProcess, LocalBase + Recoil_Offset + m_local, &recoil, sizeof(recoil), NULL);
		recoil[0] *= 2;
		recoil[1] *= 2;
		ReadProcessMemory(g_Vars.hProcess, g_Vars.dwEngine + c_dwEnginePosition, &vPosition, sizeof(vPosition), 0);
		ReadProcessMemory(g_Vars.hProcess, LocalBase + c_dwWeaponHandle, &m_WeaponHandle, 4, 0);
		m_WeaponIDFirst = m_WeaponHandle & 0xFFF;
		ReadProcessMemory(g_Vars.hProcess, g_Vars.dwClient + c_dwBaseEntity + ((0x10 * m_WeaponIDFirst) - 0x10), &CWeaponBase, sizeof(CWeaponBase), 0);
		ReadProcessMemory(g_Vars.hProcess, CWeaponBase + c_dwWeaponID, &g_Vars.m_iWeaponID, 4, NULL);

		//cout << g_Vars.m_iWeaponID << endl;
	}

} LocalBasePlayer;

typedef struct C_BasePlayer
{
	PBYTE dw_BasePointer;
	PBYTE dw_BoneMatrix;

	int hp;
	int Team;

	float you[3];
	float PlayerPos[3];
	float VecView[3];

	void Clear()
	{
		this->dw_BasePointer = 0x0;
		this->dw_BoneMatrix = 0x0;
		this->hp = 0;
		this->Team = 0;
		this->you[0] = 0;
		this->you[1] = 0;
		this->you[2] = 0;
	}

	void ReadStructure(int i)
	{
		this->Clear();
		ReadProcessMemory(g_Vars.hProcess, g_Vars.dwClient + c_dwBaseEntity + ((i - 1) * 16), &dw_BasePointer, sizeof(dw_BasePointer), NULL);
		ReadProcessMemory(g_Vars.hProcess, dw_BasePointer + Healths, &hp, sizeof(hp), NULL);
		ReadProcessMemory(g_Vars.hProcess, dw_BasePointer + MyTeamAD, &Team, sizeof(Team), NULL);
		ReadProcessMemory(g_Vars.hProcess, dw_BasePointer + BoneOffset, &dw_BoneMatrix, sizeof(dw_BoneMatrix), NULL);
		ReadProcessMemory(g_Vars.hProcess, dw_BoneMatrix + ((48 * (g_Vars.Aimbot.Bone)) + 12), &you[0], sizeof(you[0]), NULL);
		ReadProcessMemory(g_Vars.hProcess, dw_BoneMatrix + ((48 * (g_Vars.Aimbot.Bone)) + 28), &you[1], sizeof(you[1]), NULL);
		ReadProcessMemory(g_Vars.hProcess, dw_BoneMatrix + ((48 * (g_Vars.Aimbot.Bone)) + 44), &you[2], sizeof(you[2]), NULL);
		ReadProcessMemory(g_Vars.hProcess, dw_BasePointer + 0x134, &PlayerPos, sizeof(PlayerPos), NULL);
		ReadProcessMemory(g_Vars.hProcess, dw_BasePointer + 0x104, &VecView, sizeof(VecView), NULL);
		VecView[0] = VecView[0] + PlayerPos[0];
		VecView[1] = VecView[1] + PlayerPos[1];
		VecView[2] = VecView[2] + PlayerPos[2];

		you[2] -= 65;
	}
} BasePlayer[64];

void DropTarget()
{
	g_Vars.vAimVectorGeneral[0] = 0;
	g_Vars.vAimVectorGeneral[1] = 0;
	g_Vars.vAimVectorGeneral[2] = 0;
	g_Vars.fNearestTarget = 99999.9f;
	g_Vars.iNearestTarget = -1;
}

DWORD Aimbot(LPVOID pParam)
{
	while (true)
	{
		Sleep(5);

		if (g_Vars.Aimbot.Enabled == 1)
		{
			LocalBasePlayer gLocalBasePlayer;
			C_BasePlayer gBasePlayer[65];

			gLocalBasePlayer.ReadStructure();

			if (GetAsyncKeyState(g_Vars.Aimbot.Key))
			{
				DropTarget();

				for (int i = 0; i < 64; i++)
				{
					gBasePlayer[i].ReadStructure(i);

					if (gBasePlayer[i].hp > 0)
					{
						if (gBasePlayer[i].Team != gLocalBasePlayer.MyTeam)
						{
							g_Vars.Distance = sqrt(((gLocalBasePlayer.i1[0] - gBasePlayer[i].you[0]) * (gLocalBasePlayer.i1[0] - gBasePlayer[i].you[0])) + ((gLocalBasePlayer.i1[1] - gBasePlayer[i].you[1]) * (gLocalBasePlayer.i1[1] - gBasePlayer[i].you[1])) + ((gLocalBasePlayer.i1[2] - gBasePlayer[i].you[2]) * (gLocalBasePlayer.i1[2] - gBasePlayer[i].you[2])));
							g_Vars.Angle[0] = ((asin((gBasePlayer[i].you[2] - gLocalBasePlayer.i1[2]) / g_Vars.Distance) * 180 / M_PI) * -1);
							g_Vars.Angle[1] = (g_Math.ATAN2(gBasePlayer[i].you[0] - gLocalBasePlayer.i1[0], gBasePlayer[i].you[1] - gLocalBasePlayer.i1[1]) / M_PI * 180);

							if (g_Vars.Aimbot.RCS == 0)
							{
								g_Vars.Aimmin[0] = g_Vars.Angle[0] - (gLocalBasePlayer.MeFov[0]);
								g_Vars.Aimmin[1] = g_Vars.Angle[1] - (gLocalBasePlayer.MeFov[1]);
							}
							else
							{
								g_Vars.Aimmin[0] = g_Vars.Angle[0] - (gLocalBasePlayer.MeFov[0] + gLocalBasePlayer.recoil[0]);
								g_Vars.Aimmin[1] = g_Vars.Angle[1] - (gLocalBasePlayer.MeFov[1] + gLocalBasePlayer.recoil[1]);
							}

							if (g_Vars.Aimmin[0] > 180) { g_Vars.Aimmin[0] -= 360; }
							if (g_Vars.Aimmin[1] > 180) { g_Vars.Aimmin[1] -= 360; }
							if (g_Vars.Aimmin[0] < -180) { g_Vars.Aimmin[0] += 360; }
							if (g_Vars.Aimmin[1] < -180) { g_Vars.Aimmin[1] += 360; }

							g_Vars.vAimVectorGeneral[0] = gBasePlayer[i].you[0];
							g_Vars.vAimVectorGeneral[1] = gBasePlayer[i].you[1];
							g_Vars.vAimVectorGeneral[2] = gBasePlayer[i].you[2];

							g_Vars.GeneralAimbot[0] = gLocalBasePlayer.i1[0] - g_Vars.vAimVectorGeneral[0];
							g_Vars.GeneralAimbot[1] = gLocalBasePlayer.i1[1] - g_Vars.vAimVectorGeneral[1];
							g_Vars.GeneralAimbot[2] = gLocalBasePlayer.i1[2] - g_Vars.vAimVectorGeneral[2];

							g_Vars.fovdist = sqrt((g_Vars.Aimmin[0] * g_Vars.Aimmin[0]) + (g_Vars.Aimmin[1] * g_Vars.Aimmin[1]));
							g_Vars.TargetSelection = g_Math.VectorLength(g_Vars.GeneralAimbot);
							g_Vars.TargetSelection2 = g_Vars.fovdist;

							if (g_Vars.fovdist < g_Vars.Aimbot.FOV / 2)
							{
								if (g_Vars.TargetSelection < g_Vars.fNearestTarget && g_Vars.TargetSelection2 < g_Vars.fNearestTarget)
								{
									g_Vars.fNearestTarget = g_Vars.TargetSelection;
									g_Vars.iNearestTarget = i;

									if (g_Vars.Aimbot.RCS == 1 && !g_Nospread.IsNonRcs(g_Vars.m_iWeaponID))
									{
										g_Vars.Angle[0] -= gLocalBasePlayer.recoil[0];
										g_Vars.Angle[1] -= gLocalBasePlayer.recoil[1];
									}

									if (g_Vars.Aimbot.Smooth > 0.0)
									{
										float Diff[3];

										Diff[0] = g_Vars.Angle[0] - gLocalBasePlayer.flAngle[0];
										Diff[1] = g_Vars.Angle[1] - gLocalBasePlayer.flAngle[1];

										if (Diff[0] > 180)  Diff[0] -= 360;
										if (Diff[1] > 180)  Diff[1] -= 360;
										if (Diff[0] < -180) Diff[0] += 360;
										if (Diff[1] <-180) Diff[1] += 360;
										if (Diff[0] >   2 / g_Vars.Aimbot.Smooth) { g_Vars.Angle[0] = gLocalBasePlayer.flAngle[0] + 2 / g_Vars.Aimbot.Smooth; }
										else if (Diff[0] < -2 / g_Vars.Aimbot.Smooth) { g_Vars.Angle[0] = gLocalBasePlayer.flAngle[0] - 2 / g_Vars.Aimbot.Smooth; }
										if (Diff[1] >   2 / g_Vars.Aimbot.Smooth) { g_Vars.Angle[1] = gLocalBasePlayer.flAngle[1] + 2 / g_Vars.Aimbot.Smooth; }
										else if (Diff[1] < -2 / g_Vars.Aimbot.Smooth) { g_Vars.Angle[1] = gLocalBasePlayer.flAngle[1] - 2 / g_Vars.Aimbot.Smooth; }
										if (g_Vars.Angle[0] > 180)  g_Vars.Angle[0] -= 360;
										if (g_Vars.Angle[1] > 180)  g_Vars.Angle[1] -= 360;
										if (g_Vars.Angle[0] < -180) g_Vars.Angle[0] += 360;
										if (g_Vars.Angle[1] < -180) g_Vars.Angle[1] += 360;
									}

									if (g_Nospread.IsBadWeapon(g_Vars.m_iWeaponID))
										continue;

									g_Vars.Angle[2] = 0.0f;
									g_Math.NormalizeAngles(g_Vars.Angle);
									WriteProcessMemory(g_Vars.hProcess, gLocalBasePlayer.AngBase + ViewAngOff, &g_Vars.Angle, sizeof(g_Vars.Angle), 0);
								}
							}
						}
					}
				}
			}
		}
	}
}

DWORD Triggerbot(LPVOID pParam)
{
	while (true)
	{
		Sleep(5);

		if (g_Vars.Triggerbot.Enabled == 1)
		{
			if (GetAsyncKeyState(g_Vars.Triggerbot.Key))
			{
				PBYTE LOCALBASE;
				PBYTE BASEPOINTER;

				byte ID;
				byte HP;
				byte TEAM;
				byte MYTEAM;

				ReadProcessMemory(g_Vars.hProcess, g_Vars.dwClient + c_dwLocalBaseEntity, &LOCALBASE, sizeof(LOCALBASE), NULL);
				ReadProcessMemory(g_Vars.hProcess, LOCALBASE + OFFSET_CROSSHAIRID, &ID, sizeof(ID), NULL);

				if (ID)
				{
					ReadProcessMemory(g_Vars.hProcess, g_Vars.dwClient + c_dwBaseEntity + ((ID - 1) * 16), &BASEPOINTER, sizeof(BASEPOINTER), NULL);
					ReadProcessMemory(g_Vars.hProcess, BASEPOINTER + Healths, &HP, sizeof(HP), NULL);

					if (HP > 0)
					{
						ReadProcessMemory(g_Vars.hProcess, BASEPOINTER + MyTeamAD, &TEAM, sizeof(TEAM), NULL);
						ReadProcessMemory(g_Vars.hProcess, LOCALBASE + MyTeamAD, &MYTEAM, sizeof(MYTEAM), NULL);

						if (TEAM != MYTEAM)
						{
							PostMessage(g_Vars.HandleWindow, WM_LBUTTONDOWN, MK_LBUTTON, 0);
							Sleep(g_Vars.Triggerbot.Sleep1);
							PostMessage(g_Vars.HandleWindow, WM_LBUTTONUP, MK_LBUTTON, 0);
							Sleep(g_Vars.Triggerbot.Sleep2);
						}
					}
				}
			}
		}
	}
}

bool GetWeaponInfos()
{

	switch (g_Vars.m_iWeaponID)
	{

	case WEAPON_GLOCK:

		g_Vars.Aimbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.glock.enabled", 0, g_Vars.ini);
		g_Vars.Aimbot.FOV = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.glock.fov", 0, g_Vars.ini);
		g_Vars.Aimbot.Smooth = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.glock.smooth", 0, g_Vars.ini);
		g_Vars.Aimbot.Bone = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.glock.bone", 0, g_Vars.ini);
		g_Vars.Triggerbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.glock.enabled", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep1 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.glock.sleep.1", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep2 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.glock.sleep.2", 0, g_Vars.ini);

		break;

	case WEAPON_ELITE:

		g_Vars.Aimbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.elite.enabled", 0, g_Vars.ini);
		g_Vars.Aimbot.FOV = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.elite.fov", 0, g_Vars.ini);
		g_Vars.Aimbot.Smooth = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.elite.smooth", 0, g_Vars.ini);
		g_Vars.Aimbot.Bone = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.elite.bone", 0, g_Vars.ini);
		g_Vars.Triggerbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.elite.enabled", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep1 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.elite.sleep.1", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep2 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.elite.sleep.2", 0, g_Vars.ini);

		break;

	case WEAPON_P250:

		g_Vars.Aimbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.p250.enabled", 0, g_Vars.ini);
		g_Vars.Aimbot.FOV = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.p250.fov", 0, g_Vars.ini);
		g_Vars.Aimbot.Smooth = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.p250.smooth", 0, g_Vars.ini);
		g_Vars.Aimbot.Bone = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.p250.bone", 0, g_Vars.ini);
		g_Vars.Triggerbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.p250.enabled", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep1 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.p250.sleep.1", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep2 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.p250.sleep.2", 0, g_Vars.ini);

		break;

	case WEAPON_TEC9:

		g_Vars.Aimbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.tec9.enabled", 0, g_Vars.ini);
		g_Vars.Aimbot.FOV = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.tec9.fov", 0, g_Vars.ini);
		g_Vars.Aimbot.Smooth = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.tec9.smooth", 0, g_Vars.ini);
		g_Vars.Aimbot.Bone = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.tec9.bone", 0, g_Vars.ini);
		g_Vars.Triggerbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.tec9.enabled", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep1 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.tec9.sleep.1", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep2 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.tec9.sleep.2", 0, g_Vars.ini);

		break;

	case WEAPON_DEAGLE:

		g_Vars.Aimbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.deagle.enabled", 0, g_Vars.ini);
		g_Vars.Aimbot.FOV = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.deagle.fov", 0, g_Vars.ini);
		g_Vars.Aimbot.Smooth = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.deagle.smooth", 0, g_Vars.ini);
		g_Vars.Aimbot.Bone = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.deagle.bone", 0, g_Vars.ini);
		g_Vars.Triggerbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.deagle.enabled", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep1 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.deagle.sleep.1", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep2 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.deagle.sleep.2", 0, g_Vars.ini);

		break;

	case WEAPON_HKP2000:

		g_Vars.Aimbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.hkp2000.enabled", 0, g_Vars.ini);
		g_Vars.Aimbot.FOV = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.hkp2000.fov", 0, g_Vars.ini);
		g_Vars.Aimbot.Smooth = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.hkp2000.smooth", 0, g_Vars.ini);
		g_Vars.Aimbot.Bone = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.hkp2000.bone", 0, g_Vars.ini);
		g_Vars.Triggerbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.hkp2000.enabled", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep1 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.hkp2000.sleep.1", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep2 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.hkp2000.sleep.2", 0, g_Vars.ini);

		break;

	case WEAPON_USP:

		g_Vars.Aimbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.usp.enabled", 0, g_Vars.ini);
		g_Vars.Aimbot.FOV = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.usp.fov", 0, g_Vars.ini);
		g_Vars.Aimbot.Smooth = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.usp.smooth", 0, g_Vars.ini);
		g_Vars.Aimbot.Bone = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.usp.bone", 0, g_Vars.ini);
		g_Vars.Triggerbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.usp.enabled", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep1 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.usp.sleep.1", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep2 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.usp.sleep.2", 0, g_Vars.ini);

		break;

	case WEAPON_FIVESEVEN:

		g_Vars.Aimbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.fiveseven.enabled", 0, g_Vars.ini);
		g_Vars.Aimbot.FOV = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.fiveseven.fov", 0, g_Vars.ini);
		g_Vars.Aimbot.Smooth = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.fiveseven.smooth", 0, g_Vars.ini);
		g_Vars.Aimbot.Bone = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.fiveseven.bone", 0, g_Vars.ini);
		g_Vars.Triggerbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.fiveseven.enabled", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep1 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.fiveseven.sleep.1", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep2 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.fiveseven.sleep.2", 0, g_Vars.ini);

		break;

	case WEAPON_CZ75:

		g_Vars.Aimbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.cz75.enabled", 0, g_Vars.ini);
		g_Vars.Aimbot.FOV = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.cz75.fov", 0, g_Vars.ini);
		g_Vars.Aimbot.Smooth = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.cz75.smooth", 0, g_Vars.ini);
		g_Vars.Aimbot.Bone = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.cz75.bone", 0, g_Vars.ini);
		g_Vars.Triggerbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.cz75.enabled", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep1 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.cz75.sleep.1", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep2 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.cz75.sleep.2", 0, g_Vars.ini);

		break;

	case WEAPON_NOVA:

		g_Vars.Aimbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.nova.enabled", 0, g_Vars.ini);
		g_Vars.Aimbot.FOV = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.nova.fov", 0, g_Vars.ini);
		g_Vars.Aimbot.Smooth = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.nova.smooth", 0, g_Vars.ini);
		g_Vars.Aimbot.Bone = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.nova.bone", 0, g_Vars.ini);
		g_Vars.Triggerbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.nova.enabled", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep1 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.nova.sleep.1", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep2 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.nova.sleep.2", 0, g_Vars.ini);

		break;

	case WEAPON_XM1014:

		g_Vars.Aimbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.xm1014.enabled", 0, g_Vars.ini);
		g_Vars.Aimbot.FOV = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.xm1014.fov", 0, g_Vars.ini);
		g_Vars.Aimbot.Smooth = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.xm1014.smooth", 0, g_Vars.ini);
		g_Vars.Aimbot.Bone = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.xm1014.bone", 0, g_Vars.ini);
		g_Vars.Triggerbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.xm1014.enabled", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep1 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.xm1014.sleep.1", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep2 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.xm1014.sleep.2", 0, g_Vars.ini);

		break;

	case WEAPON_SAWEDOFF:

		g_Vars.Aimbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.swadeoff.enabled", 0, g_Vars.ini);
		g_Vars.Aimbot.FOV = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.swadeoff.fov", 0, g_Vars.ini);
		g_Vars.Aimbot.Smooth = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.swadeoff.smooth", 0, g_Vars.ini);
		g_Vars.Aimbot.Bone = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.swadeoff.bone", 0, g_Vars.ini);
		g_Vars.Triggerbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.swadeoff.enabled", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep1 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.swadeoff.sleep.1", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep2 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.swadeoff.sleep.2", 0, g_Vars.ini);

		break;

	case WEAPON_MAG7:

		g_Vars.Aimbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.mag7.enabled", 0, g_Vars.ini);
		g_Vars.Aimbot.FOV = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.mag7.fov", 0, g_Vars.ini);
		g_Vars.Aimbot.Smooth = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.mag7.smooth", 0, g_Vars.ini);
		g_Vars.Aimbot.Bone = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.mag7.bone", 0, g_Vars.ini);
		g_Vars.Triggerbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.mag7.enabled", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep1 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.mag7.sleep.1", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep2 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.mag7.sleep.2", 0, g_Vars.ini);

		break;

	case WEAPON_MAC10:

		g_Vars.Aimbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.mac10.enabled", 0, g_Vars.ini);
		g_Vars.Aimbot.FOV = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.mac10.fov", 0, g_Vars.ini);
		g_Vars.Aimbot.Smooth = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.mac10.smooth", 0, g_Vars.ini);
		g_Vars.Aimbot.Bone = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.mac10.bone", 0, g_Vars.ini);
		g_Vars.Aimbot.RCS = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.mac10.rcs", 0, g_Vars.ini);
		g_Vars.Triggerbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.mac10.enabled", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep1 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.mac10.sleep.1", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep2 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.mac10.sleep.2", 0, g_Vars.ini);

		break;

	case WEAPON_MP7:

		g_Vars.Aimbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.mp7.enabled", 0, g_Vars.ini);
		g_Vars.Aimbot.FOV = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.mp7.fov", 0, g_Vars.ini);
		g_Vars.Aimbot.Smooth = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.mp7.smooth", 0, g_Vars.ini);
		g_Vars.Aimbot.Bone = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.mp7.bone", 0, g_Vars.ini);
		g_Vars.Aimbot.RCS = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.mp7.rcs", 0, g_Vars.ini);
		g_Vars.Triggerbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.mp7.enabled", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep1 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.mp7.sleep.1", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep2 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.mp7.sleep.2", 0, g_Vars.ini);

		break;

	case WEAPON_UMP45:

		g_Vars.Aimbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.ump45.enabled", 0, g_Vars.ini);
		g_Vars.Aimbot.FOV = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.ump45.fov", 0, g_Vars.ini);
		g_Vars.Aimbot.Smooth = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.ump45.smooth", 0, g_Vars.ini);
		g_Vars.Aimbot.Bone = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.ump45.bone", 0, g_Vars.ini);
		g_Vars.Aimbot.RCS = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.ump45.rcs", 0, g_Vars.ini);
		g_Vars.Triggerbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.ump45.enabled", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep1 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.ump45.sleep.1", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep2 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.ump45.sleep.2", 0, g_Vars.ini);

		break;

	case WEAPON_P90:

		g_Vars.Aimbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.p90.enabled", 0, g_Vars.ini);
		g_Vars.Aimbot.FOV = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.p90.fov", 0, g_Vars.ini);
		g_Vars.Aimbot.Smooth = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.p90.smooth", 0, g_Vars.ini);
		g_Vars.Aimbot.Bone = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.p90.bone", 0, g_Vars.ini);
		g_Vars.Aimbot.RCS = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.p90.rcs", 0, g_Vars.ini);
		g_Vars.Triggerbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.p90.enabled", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep1 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.p90.sleep.1", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep2 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.p90.sleep.2", 0, g_Vars.ini);

		break;

	case WEAPON_MP9:

		g_Vars.Aimbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.mp9.enabled", 0, g_Vars.ini);
		g_Vars.Aimbot.FOV = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.mp9.fov", 0, g_Vars.ini);
		g_Vars.Aimbot.Smooth = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.mp9.smooth", 0, g_Vars.ini);
		g_Vars.Aimbot.Bone = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.mp9.bone", 0, g_Vars.ini);
		g_Vars.Aimbot.RCS = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.mp9.rcs", 0, g_Vars.ini);
		g_Vars.Triggerbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.mp9.enabled", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep1 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.mp9.sleep.1", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep2 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.mp9.sleep.2", 0, g_Vars.ini);

		break;

	case WEAPON_BIZON:

		g_Vars.Aimbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.bizon.enabled", 0, g_Vars.ini);
		g_Vars.Aimbot.FOV = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.bizon.fov", 0, g_Vars.ini);
		g_Vars.Aimbot.Smooth = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.bizon.smooth", 0, g_Vars.ini);
		g_Vars.Aimbot.Bone = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.bizon.bone", 0, g_Vars.ini);
		g_Vars.Aimbot.RCS = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.bizon.rcs", 0, g_Vars.ini);
		g_Vars.Triggerbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.bizon.enabled", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep1 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.bizon.sleep.1", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep2 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.bizon.sleep.2", 0, g_Vars.ini);

		break;

	case WEAPON_GALILAR:

		g_Vars.Aimbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.galil.enabled", 0, g_Vars.ini);
		g_Vars.Aimbot.FOV = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.galil.fov", 0, g_Vars.ini);
		g_Vars.Aimbot.Smooth = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.galil.smooth", 0, g_Vars.ini);
		g_Vars.Aimbot.Bone = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.galil.bone", 0, g_Vars.ini);
		g_Vars.Aimbot.RCS = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.galil.rcs", 0, g_Vars.ini);
		g_Vars.Triggerbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.galil.enabled", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep1 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.galil.sleep.1", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep2 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.galil.sleep.2", 0, g_Vars.ini);

		break;

	case WEAPON_AK47:

		g_Vars.Aimbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.ak.enabled", 0, g_Vars.ini);
		g_Vars.Aimbot.FOV = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.ak.fov", 0, g_Vars.ini);
		g_Vars.Aimbot.Smooth = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.ak.smooth", 0, g_Vars.ini);
		g_Vars.Aimbot.Bone = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.ak.bone", 0, g_Vars.ini);
		g_Vars.Aimbot.RCS = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.ak.rcs", 0, g_Vars.ini);
		g_Vars.Triggerbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.ak.enabled", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep1 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.ak.sleep.1", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep2 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.ak.sleep.2", 0, g_Vars.ini);

		break;

	case WEAPON_SG553:

		g_Vars.Aimbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.sg553.enabled", 0, g_Vars.ini);
		g_Vars.Aimbot.FOV = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.sg553.fov", 0, g_Vars.ini);
		g_Vars.Aimbot.Smooth = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.sg553.smooth", 0, g_Vars.ini);
		g_Vars.Aimbot.Bone = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.sg553.bone", 0, g_Vars.ini);
		g_Vars.Aimbot.RCS = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.sg553.rcs", 0, g_Vars.ini);
		g_Vars.Triggerbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.sg553.enabled", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep1 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.sg553.sleep.1", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep2 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.sg553.sleep.2", 0, g_Vars.ini);

		break;

	case WEAPON_M4A4:

		g_Vars.Aimbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.m4a4.enabled", 0, g_Vars.ini);
		g_Vars.Aimbot.FOV = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.m4a4.fov", 0, g_Vars.ini);
		g_Vars.Aimbot.Smooth = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.m4a4.smooth", 0, g_Vars.ini);
		g_Vars.Aimbot.Bone = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.m4a4.bone", 0, g_Vars.ini);
		g_Vars.Aimbot.RCS = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.m4a4.rcs", 0, g_Vars.ini);
		g_Vars.Triggerbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.m4a4.enabled", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep1 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.m4a4.sleep.1", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep2 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.m4a4.sleep.2", 0, g_Vars.ini);

		break;

	case WEAPON_M4A1:

		g_Vars.Aimbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.m4a1.enabled", 0, g_Vars.ini);
		g_Vars.Aimbot.FOV = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.m4a1.fov", 0, g_Vars.ini);
		g_Vars.Aimbot.Smooth = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.m4a1.smooth", 0, g_Vars.ini);
		g_Vars.Aimbot.Bone = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.m4a1.bone", 0, g_Vars.ini);
		g_Vars.Aimbot.RCS = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.m4a1.rcs", 0, g_Vars.ini);
		g_Vars.Triggerbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.m4a1.enabled", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep1 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.m4a1.sleep.1", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep2 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.m4a1.sleep.2", 0, g_Vars.ini);

		break;

	case WEAPON_FAMAS:

		g_Vars.Aimbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.famas.enabled", 0, g_Vars.ini);
		g_Vars.Aimbot.FOV = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.famas.fov", 0, g_Vars.ini);
		g_Vars.Aimbot.Smooth = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.famas.smooth", 0, g_Vars.ini);
		g_Vars.Aimbot.Bone = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.famas.bone", 0, g_Vars.ini);
		g_Vars.Aimbot.RCS = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.famas.rcs", 0, g_Vars.ini);
		g_Vars.Triggerbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.famas.enabled", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep1 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.famas.sleep.1", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep2 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.famas.sleep.2", 0, g_Vars.ini);

		break;

	case WEAPON_AUG:

		g_Vars.Aimbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.aug.enabled", 0, g_Vars.ini);
		g_Vars.Aimbot.FOV = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.aug.fov", 0, g_Vars.ini);
		g_Vars.Aimbot.Smooth = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.aug.smooth", 0, g_Vars.ini);
		g_Vars.Aimbot.Bone = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.aug.bone", 0, g_Vars.ini);
		g_Vars.Aimbot.RCS = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.aug.rcs", 0, g_Vars.ini);
		g_Vars.Triggerbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.aug.enabled", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep1 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.aug.sleep.1", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep2 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.aug.sleep.2", 0, g_Vars.ini);

		break;

	case WEAPON_SSG08:

		g_Vars.Aimbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.scout.enabled", 0, g_Vars.ini);
		g_Vars.Aimbot.FOV = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.scout.fov", 0, g_Vars.ini);
		g_Vars.Aimbot.Smooth = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.scout.smooth", 0, g_Vars.ini);
		g_Vars.Aimbot.Bone = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.scout.bone", 0, g_Vars.ini);
		g_Vars.Triggerbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.scout.enabled", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep1 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.scout.sleep.1", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep2 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.scout.sleep.2", 0, g_Vars.ini);

		break;

	case WEAPON_AWP:

		g_Vars.Aimbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.awp.enabled", 0, g_Vars.ini);
		g_Vars.Aimbot.FOV = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.awp.fov", 0, g_Vars.ini);
		g_Vars.Aimbot.Smooth = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.awp.smooth", 0, g_Vars.ini);
		g_Vars.Aimbot.Bone = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.awp.bone", 0, g_Vars.ini);
		g_Vars.Triggerbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.awp.enabled", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep1 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.awp.sleep.1", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep2 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.awp.sleep.2", 0, g_Vars.ini);

		break;

	case WEAPON_G3SG1:

		g_Vars.Aimbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.g3sg1.enabled", 0, g_Vars.ini);
		g_Vars.Aimbot.FOV = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.g3sg1.fov", 0, g_Vars.ini);
		g_Vars.Aimbot.Smooth = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.g3sg1.smooth", 0, g_Vars.ini);
		g_Vars.Aimbot.Bone = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.g3sg1.bone", 0, g_Vars.ini);
		g_Vars.Triggerbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.g3sg1.enabled", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep1 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.g3sg1.sleep.1", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep2 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.g3sg1.sleep.2", 0, g_Vars.ini);

		break;

	case WEAPON_SCAR20:

		g_Vars.Aimbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.scar.enabled", 0, g_Vars.ini);
		g_Vars.Aimbot.FOV = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.scar.fov", 0, g_Vars.ini);
		g_Vars.Aimbot.Smooth = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.scar.smooth", 0, g_Vars.ini);
		g_Vars.Aimbot.Bone = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.scar.bone", 0, g_Vars.ini);
		g_Vars.Triggerbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.scar.enabled", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep1 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.scar.sleep.1", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep2 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.scar.sleep.2", 0, g_Vars.ini);

		break;

	case WEAPON_M249:

		g_Vars.Aimbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.m249.enabled", 0, g_Vars.ini);
		g_Vars.Aimbot.FOV = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.m249.fov", 0, g_Vars.ini);
		g_Vars.Aimbot.Smooth = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.m249.smooth", 0, g_Vars.ini);
		g_Vars.Aimbot.Bone = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.m249.bone", 0, g_Vars.ini);
		g_Vars.Aimbot.RCS = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.m249.rcs", 0, g_Vars.ini);
		g_Vars.Triggerbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.m249.enabled", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep1 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.m249.sleep.1", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep2 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.m249.sleep.2", 0, g_Vars.ini);

		break;

	case WEAPON_NEGEV:

		g_Vars.Aimbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.negev.enabled", 0, g_Vars.ini);
		g_Vars.Aimbot.FOV = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.negev.fov", 0, g_Vars.ini);
		g_Vars.Aimbot.Smooth = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.negev.smooth", 0, g_Vars.ini);
		g_Vars.Aimbot.Bone = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.negev.bone", 0, g_Vars.ini);
		g_Vars.Aimbot.RCS = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.negev.rcs", 0, g_Vars.ini);
		g_Vars.Triggerbot.Enabled = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.negev.enabled", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep1 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.negev.sleep.1", 0, g_Vars.ini);
		g_Vars.Triggerbot.Sleep2 = g_Module.GetPrivateProfileFloat("Horcrux", "weapon.triggerbot.negev.sleep.2", 0, g_Vars.ini);

		break;
	}

	return true;
}

bool Info()
{
	if (UPD == 100)
	{
		InitializeSettings();
		GetWeaponInfos();

		g_Vars.hPID = g_Module.GetProcID("csgo.exe");
		g_Vars.HandleWindow = FindWindowA("Valve001", 0);
		g_Vars.hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, g_Vars.hPID);
		g_Vars.hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, g_Vars.hPID);
		g_Vars.dwClient = g_Module.GetModule(g_Vars.hSnapshot, "client_panorama.dll");
		g_Vars.dwEngine = g_Module.GetModule(g_Vars.hSnapshot, "engine.dll");

		UPD = 0;
	}
	UPD = UPD + 1;
	return true;
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	if (!g_Module.SetDebugPrivilege())
	{
		MessageBox(0, "Failed to alloc privileges.", "AnkFEST Aimbot", MB_OK | MB_ICONERROR);
		ExitProcess(0);
	}

	if (!InitializeSettings())
	{
		MessageBox(0, "Failed to parse settings.", "AnkFEST Aimbot", MB_OK | MB_ICONERROR);
		ExitProcess(0);
	}

	if (!GetWeaponInfos())
	{
		MessageBox(0, "Failed to get weapon information.", "AnkFEST Aimbot", MB_OK | MB_ICONERROR);
		ExitProcess(0);
	}

	Info();

	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Aimbot, 0, 0, 0);
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Triggerbot, 0, 0, 0);

	if (GetAsyncKeyState(g_Vars.ReloadKey))
	{
		InitializeSettings();

		// are settings updated?
	}

	while (1)
	{
		if (!Info())
		{
			MessageBox(0, "Failed to load game thread.", "AnkFEST Aimbot", MB_OK | MB_ICONERROR);
			ExitProcess(0);
		}
		Sleep(5);
	}
}