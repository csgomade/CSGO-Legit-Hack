#include "H_Include.h"

class C_Module
{
public:
	PBYTE GetModule(HANDLE Snapshot, string Module)
	{
		PBYTE           ModuleAddy;
		string          Compare;
		MODULEENTRY32   ME32;

		if (Snapshot == INVALID_HANDLE_VALUE)
			return (PBYTE)ERROR_INVALID_HANDLE;
		else
		{
			ME32.dwSize = sizeof(MODULEENTRY32);
			if (!Module32First(Snapshot, &ME32))
				return (PBYTE)GetLastError();
		}

		while (Compare != Module)
		{
			if (!Module32Next(Snapshot, &ME32))
				return (PBYTE)GetLastError();
			else
				Compare = string(ME32.szModule);
		}
		ModuleAddy = ME32.modBaseAddr;

		return ModuleAddy;
	}

	int GetProcID(string ProcName)
	{
		PROCESSENTRY32 PE32;
		HANDLE ProcSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		if (ProcSnapshot == INVALID_HANDLE_VALUE)
			return 0;

		PE32.dwSize = sizeof(PROCESSENTRY32);

		if (!Process32First(ProcSnapshot, &PE32))
		{
			CloseHandle(ProcSnapshot);
			return 0;
		}
		else
		if (PE32.szExeFile == ProcName)
		{
			CloseHandle(ProcSnapshot);
			return PE32.th32ProcessID;
		}
		else
		{
			do
			{
				if (PE32.szExeFile == ProcName)
				{
					CloseHandle(ProcSnapshot);
					return PE32.th32ProcessID;
				}

			} while (Process32Next(ProcSnapshot, &PE32));
			CloseHandle(ProcSnapshot);
			return 0;
		}
	}

	bool SetDebugPrivilege()
	{
		HANDLE hProcess = GetCurrentProcess(), hToken;
		TOKEN_PRIVILEGES priv;
		LUID luid;

		OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES, &hToken);

		LookupPrivilegeValue(0, "seDebugPrivilege", &luid);

		priv.PrivilegeCount = 1;
		priv.Privileges[0].Luid = luid;
		priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		AdjustTokenPrivileges(hToken, false, &priv, 0, 0, 0);
		CloseHandle(hToken);
		CloseHandle(hProcess);
		return true;

	}

	float GetPrivateProfileFloat(const char* section, const char* key, float def_value, const char* filename)
	{
		char buffer[64];
		if (::GetPrivateProfileString(section, key, "", buffer, sizeof(buffer), filename))
			return static_cast<float>(atof(buffer));
		return static_cast<float>(def_value);
	}

}; extern C_Module g_Module;