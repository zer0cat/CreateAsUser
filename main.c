#include <windows.h>
#include <windowsx.h>
#include "resource.h"

BOOL CALLBACK DlgProc(HWND ,UINT ,WPARAM ,LPARAM );
void DlgOnCommand(HWND ,int ,HWND,UINT);
BOOL DlgOnInitDialog(HWND ,HWND,LPARAM);


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
TOKEN_PRIVILEGES tp;

switch(id)
{
case IDOK:
	GetDlgItemTextW(hwnd,IDC_LOGIN,login,63);
	GetDlgItemTextW(hwnd,IDC_PASS,pass,63);
	GetDlgItemTextW(hwnd,IDC_DOMAIN,domain,63);

	if (lstrlenW(login) == 0)
	{
		MessageBoxW(NULL,L"Введите валидное имя юзера!",L"Error",MB_ICONERROR);
		return;
	}

	if (lstrlenW(pass) == 0 )
	{
		MessageBoxW(NULL,L"Пустой пароль,скорее всего войти не получится",L"Error",MB_ICONERROR);
	}

	
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES , &hToken))
	{
		wsprintfW(err,L"Err: %d",GetLastError());
		MessageBoxW(0,L"Error GetCurrentProcess",err,MB_ICONEXCLAMATION);
	return;
	}


	ZeroMemory(&tp, sizeof(tp));
	// устанавливаем привилегию для логина TCB Name
	if (!LookupPrivilegeValue(NULL,SE_TCB_NAME, &tp.Privileges[0].Luid)) 
	{
		wsprintfW(err,L"Err: %d",GetLastError());
		MessageBoxW(0,L"Error LookupPrivilegeValue",err,MB_ICONEXCLAMATION);
	return;
	}
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Attributes =  SE_PRIVILEGE_ENABLED;//SE_PRIVILEGE_ENABLED;
	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, 0, NULL, 0)) 
	{
		wsprintfW(err,L"Err: %d",GetLastError());
		MessageBoxW(0,L"Error AdjustTokenPrivileges",err,MB_ICONEXCLAMATION);
	return;
	}
	

	ZeroMemory(&s_i,sizeof(STARTUPINFOW));
	s_i.cb = sizeof(STARTUPINFOW);
	s_i.lpDesktop = 0;

	if (isVistaPlus())
	{
	bOk = CreateProcessWithLogonW(login,domain,(lstrlenW(pass) == 0) ? NULL : pass,LOGON_WITH_PROFILE,\
					L"C:\\WINDOWS\\system32\\cmd.exe",0,CREATE_NEW_CONSOLE,NULL,NULL,&s_i,&p_i);
	if (!bOk)
	{
		wsprintfW(err,L"Err: %d",GetLastError());
		MessageBoxW(0,L"Error CreateProcessWithLogonW",err,MB_ICONEXCLAMATION);
		return;
	}
	}
	else
	{
	
	bOk = LogonUserW(login,L".", (lstrlenW(pass) == 0) ? NULL : pass,LOGON32_LOGON_INTERACTIVE,LOGON32_PROVIDER_DEFAULT,&hToken);
	if (!bOk)
	{
	wsprintfW(err,L"Err: %d",GetLastError());
	MessageBoxW(0,L"Error logonuser",err,MB_ICONEXCLAMATION);
	return;
	}

	/*if(!ImpersonateLoggedOnUser(hToken))
	{
	wsprintfW(err,L"Err: %d",GetLastError());
	MessageBoxW(0,L"Error ImpersonateLoggedOnUser",err,MB_ICONEXCLAMATION);
	return;
	}*/

	bOk = CreateProcessAsUserW(hToken,L"C:\\WINDOWS\\system32\\cmd.exe",NULL,NULL,NULL,TRUE,CREATE_NEW_CONSOLE,NULL,NULL,&s_i,&p_i);
	}
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
