#include <windows.h>
#include <windowsx.h>
#include "resource.h"

BOOL CALLBACK DlgProc(HWND ,UINT ,WPARAM ,LPARAM );
void DlgOnCommand(HWND ,int ,HWND,UINT);
BOOL DlgOnInitDialog(HWND ,HWND,LPARAM);


BOOL EnableWindowsPrivilege(WCHAR* Privilege)
{
LUID luid = {0};
TOKEN_PRIVILEGES tp;
HANDLE currentToken,currentProcess = GetCurrentProcess();

tp.PrivilegeCount = 1;
tp.Privileges[0].Luid = luid;
tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (!LookupPrivilegeValueW(NULL, Privilege, &luid)) 
		{
		wprintf(L"LookupPrivilegeValue failed %d\n",GetLastError());
		return FALSE;
		}
	if (!OpenProcessToken(currentProcess, TOKEN_ALL_ACCESS, &currentToken)) 		
		{
		wprintf(L"OpenProcessToken for priv8 failed %d\n",GetLastError());
		return FALSE;
		}
	if (!AdjustTokenPrivileges(currentToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL)) 
		{
		wprintf(L"AdjustTokenPrivileges failed %d\n",GetLastError());		
		return FALSE;
		}
	return TRUE;
}



BOOL isVistaPlus(void)
{
OSVERSIONINFOW osv;
osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
GetVersionExW(&osv);
if (osv.dwMajorVersion >= 6) //return (osv.dwMajorVersion >= 6)
	return TRUE;
else 
	return FALSE;
}



int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd )
{


	DialogBox(hInstance,(char*)IDD_DIALOG1,NULL,DlgProc);
	return 0;
}

BOOL CALLBACK DlgProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
		HANDLE_MSG(hwnd,WM_INITDIALOG,DlgOnInitDialog);
		HANDLE_MSG(hwnd,WM_COMMAND,DlgOnCommand);
	default:
		return FALSE;
	}
}



void DlgOnCommand(HWND hwnd,int id,HWND h1,UINT uid1)
{
WCHAR login[64];
WCHAR pass[64];
WCHAR domain[64];
WCHAR err[32];
HANDLE hToken;
BOOL bOk;
STARTUPINFOW s_i;
PROCESS_INFORMATION p_i;

switch(id)
{
case IDOK:
	GetDlgItemTextW(hwnd,IDC_LOGIN,login,63);
	GetDlgItemTextW(hwnd,IDC_PASS,pass,63);
	GetDlgItemTextW(hwnd,IDC_DOMAIN,domain,63);

	if (lstrlenW(login) == 0)
	{
		MessageBoxW(NULL,L"¬ведите валидное им¤ юзера!",L"Error",MB_ICONERROR);
		return;
	}

	if (lstrlenW(pass) == 0 )
	{
		MessageBoxW(NULL,L"ѕустой пароль,скорее всего войти не получитс¤",L"Error",MB_ICONERROR);
	}

	// устанавливаем привилегию дл¤ логина TCB Name
	if (!EnableWindowsPrivilege(L"SeTcbPrivilege"))
		{
		wsprintfW(err,L"Err: %d",GetLastError());
		MessageBoxW(0,L"Error Setting TcbPrivilege!",err,MB_ICONEXCLAMATION);
		return;
		}



	ZeroMemory(&s_i,sizeof(STARTUPINFOW));
	s_i.cb = sizeof(STARTUPINFOW);
	s_i.lpDesktop = 0;

goto X;	

	bOk = CreateProcessWithLogonW(login,domain,(lstrlenW(pass) == 0) ? NULL : pass,LOGON_WITH_PROFILE,\
					L"C:\\WINDOWS\\system32\\cmd.exe",0,CREATE_NEW_CONSOLE,NULL,NULL,&s_i,&p_i);
		if (!bOk)
		{
			wsprintfW(err,L"Err: %d",GetLastError());
			MessageBoxW(0,L"Error CreateProcessWithLogonW",err,MB_ICONEXCLAMATION);
			return;
		}

X:
/* этот код  работает сугубо от системы, потому что не хватает какой-то привилегии. Еще можно юзать CreateProcessWithTokenW*/
	bOk = LogonUserW(login,domain, (lstrlenW(pass) == 0) ? NULL : pass,LOGON32_LOGON_INTERACTIVE,LOGON32_PROVIDER_DEFAULT,&hToken);
	if (!bOk)
		{
		wsprintfW(err,L"Err: %d",GetLastError());
		MessageBoxW(0,L"Error logonuser",err,MB_ICONEXCLAMATION);
		return;
		}


	bOk = CreateProcessAsUserW(hToken,L"C:\\WINDOWS\\system32\\cmd.exe",NULL,NULL,NULL,TRUE,CREATE_NEW_CONSOLE,NULL,NULL,&s_i,&p_i);

	if (!bOk)
	{
		wsprintfW(err,L"Err: %d",GetLastError());
		MessageBoxW(0,L"Error CreateProcessAsUserW",err,MB_ICONEXCLAMATION);
		
		return;
	}
	break;

case IDCANCEL:
	EndDialog(hwnd,0);
}

}


BOOL DlgOnInitDialog(HWND hwnd,HWND h1,LPARAM lParam)
{

	//MessageBoxW(0,L"Start",L"Hello!",MB_ICONEXCLAMATION);

	return TRUE;
}
