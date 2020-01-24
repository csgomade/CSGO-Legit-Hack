#include "H_Include.h"

class C_GlobalVars
{

public:
	PBYTE dwBoneBase;
	bool bBehind;
	int Window_x;
	int Window_y;
	int Window_w;
	int Window_h;
	int Screen_Width;
	int Screen_Height;
	PBYTE dwEngine;
	PBYTE dwClient;
	HANDLE hProcess;
	HANDLE hSnapshot;
	DWORD hPID;
	HWND HandleWindow;
	float vAimVectorGeneral[3];
	float Delta[3];
	float Diff[3];
	float GeneralAimbot[3];
	float m_vecVelocity[3];
	float angle1[3];
	float angle2[3];
	float angle3[3];
	float angle4[3];
	float angle5[3];
	float youpos[3];
	float Angle[3];
	float Aimmin[3];
	float TargetSelection;
	float TargetSelection2;
	float fNearestTarget;
	float Distance;
    float fovdist;
	int iNearestTarget;
	int m_iWeaponID;
	std::string pfad;
	char ini[260];

	struct Aimbot
	{
		int Enabled;
		int Key;
		int FOV;
		float Smooth;
		int Bone;
		int RCS;
	}Aimbot;

	struct Triggerbot
	{
		int Enabled;
		int Key;
		int Sleep1;
		int Sleep2;
	}Triggerbot;

	struct Radar
	{
		int Enabled;
		int Distance;
	}Radar;

	struct GESP
	{
		int Enabled;
	}GESP;

	int PanicKey;
	int EndKey;
	int ReloadKey;
}; extern C_GlobalVars g_Vars;