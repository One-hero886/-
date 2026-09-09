// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "RemoteCtrl.h"
#include "ServerSocket.h"
#include"Command.h"  
#include "framework.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif



// 唯一的应用程序对象123

CWinApp theApp;

using namespace std;
void WtiteRegisterTable(const CString& strPath) {
    CString strSubKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
    char sPath[MAX_PATH] = "";
    char sSys[MAX_PATH] = "";
    std::string strExe = "\\RemoteCtrl.exe ";
    GetCurrentDirectoryA(MAX_PATH, sPath);
    GetSystemDirectoryA(sSys, sizeof(sSys));
    std::string strCmd = "mklink " + std::string(sSys) + strExe + std::string(sPath) + strExe;
    system("strCmd.c_str()");
    HKEY hKey = NULL;
    int ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey);
    if (ret != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        MessageBox(NULL, _T("设置自动开机启动失败！是否权限不足？\r\n程序启动失败！"), _T("错误"), MB_ICONERROR
            | MB_TOPMOST);
        ::exit(0);
    }

    //CString strPath = CString(_T("%windir%\\system32\\RemoteCtrl.exe"));
    RegSetValueEx(hKey, _T("RemoteCtrl"), 0, REG_SZ, (BYTE*)(LPCTSTR)strPath, strPath.GetLength());
    //RegSetValueEx(hKey, _T("RemoteCtrl"), 0, REG_SZ, (BYTE*)(LPCTSTR)strPath, strPath.GetLength()*sizeof(TCHAR));
    if (ret != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        MessageBox(NULL, _T("设置自动开机启动失败！是否权限不足？\r\n程序启动失败！"), _T("错误"), MB_ICONERROR
            | MB_TOPMOST);
        ::exit(0);
    }
    RegCloseKey(hKey);

}
void WriteStartupDir(const CString& strPath) {

    
    CString strCmd = GetCommandLine();
    strCmd.Replace(_T("\""),_T(""));
    bool ret=CopyFile(strCmd, strPath, FALSE);
    if (ret == FALSE) {
        MessageBox(NULL, _T("复制文件失败，是否权限不足？\r\n"),_T("错误"), MB_ICONERROR | MB_TOPMOST);
        ::exit(0);
    }

}
void ChooseAutoInvoke() {
    TCHAR wcsSystem[MAX_PATH] = _T("");
    //CString strPath = CString(_T(" C:\\Windows\\SysWOW64\\RemoteCtrl.exe"));
    CString strPath = _T("C:\\Users\\TMC\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\RemoteCtrl.exe");
    if (PathFileExists(strPath)) {
        return;
    }
    CString strSubKey=_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
    CString strInfo = _T("该程序只允许合法的用途！\n");
    strInfo += _T("继续运行该程序，将使得这台机器处于被监控状态！\n");
    strInfo += _T("如果你不希望这样，请按”取消“按钮，退出程序。\n");
    strInfo += _T("按下“是”按钮，该程序将被复制到你的机器上，并随系统启动而自动运行！\n");
    strInfo += _T("按下“否”按钮，程序只运行一次，不会在系统内留下任何东西！\n");
    int ret=MessageBox(NULL,strInfo,_T("警告"),MB_YESNOCANCEL|MB_ICONWARNING|MB_TOPMOST);
    if (ret == IDYES) {
        //WtiteRegisterTable(strPath);
        WriteStartupDir(strPath);
    }
    else if (ret==IDCANCEL) {
        ::exit(0);
    }
    return;
}
void ShowError() {
    LPWSTR lpMessageBuf=NULL;
    //strerror(errno);//标准C语言库
    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        NULL,GetLastError(),
        MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
        (LPWSTR)&lpMessageBuf,0,NULL);
    OutputDebugStringW(lpMessageBuf);
    //MessageBoxW(NULL, lpMessageBuf, L"fasheng错误", 0);
    LocalFree(lpMessageBuf);
    
}
bool IsAdmin() {
    HANDLE hToken = NULL; 
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        ShowError();
        return false;
    }
    TOKEN_ELEVATION eve;
    DWORD len = 0;
    if (GetTokenInformation(hToken, TokenElevation,
        &eve, sizeof(eve), &len) == FALSE) {//令牌是否已提升（Elevation 状态）
        ShowError();
        return false;
    }
     CloseHandle(hToken);
    if (len == sizeof(eve)) {
        return eve.TokenIsElevated;
    }
    printf("length of tokeninformation is %d\r\n",len);
    return false;
}
void RunAsAdmin() {
   HANDLE hToken = NULL;
   bool ret= LogonUser("Administrator",NULL,NULL,LOGON32_LOGON_INTERACTIVE,LOGON32_PROVIDER_DEFAULT,&hToken);
   if (!ret) {
       ShowError();
       MessageBox(NULL,_T("登陆错误！"),_T("程序错误"),0);
       ::exit(0);
   }

   OutputDebugString("Logon administrator success!\r\n");
   //STARTUPINFOW si = {0};
   //PROCESS_INFORMATION pi = { 0 };
   //TCHAR sPath[MAX_PATH] = _T("");
   //GetCurrentDirectory(MAX_PATH, sPath);
   //CString strCmd = sPath;
   //strCmd += _T("\\RemoteCtrl.exe");
   ////ret=CreateProcessWithTokenW(hToken,LOGON_WITH_PROFILE,NULL, (LPWSTR)(LPCWSTR)strCmd,
   //    //CREATE_UNICODE_ENVIRONMENT,NULL,NULL,&si,&pi);
   //ret = CreateProcessWithLogonW(L"Administrator", NULL,NULL,LOGON_WITH_PROFILE, NULL, (LPWSTR)(LPCWSTR)strCmd.GetBuffer(),
   //    CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &si, &pi);
   STARTUPINFOW si = { sizeof(si) };
   PROCESS_INFORMATION pi = { 0 };

   // 1. 获取当前程序自身的绝对路径（避免工作目录不一致的问题）
   WCHAR szModulePath[MAX_PATH] = { 0 };
   GetModuleFileNameW(NULL, szModulePath, MAX_PATH);

   // 去掉自身文件名，截取出真实目录
   WCHAR* pSlash = wcsrchr(szModulePath, L'\\');
   if (pSlash) {
       *pSlash = L'\0';
   }

   // 2. 强制使用 CStringW（确保每个字符都是 2 字节的宽字符）
   CStringW strCmd;
   strCmd.Format(L"\"%s\\RemoteCtrl.exe\"", szModulePath); // 加上双引号防止路径中包含空格导致解析中断

   // 3. 传入宽字符缓冲区
   ret = CreateProcessWithLogonW(
       L"Administrator",
       NULL,
       NULL,
       LOGON_WITH_PROFILE,
       NULL,
       strCmd.GetBuffer(), // CStringW 的 GetBuffer() 返回合法的 LPWSTR
       CREATE_UNICODE_ENVIRONMENT,
       NULL,
       szModulePath,       // 显式指定子进程的工作目录为 exe 所在目录
       &si,
       &pi
   );
   strCmd.ReleaseBuffer();
   CloseHandle(hToken); 
   if (!ret) {
       ShowError();
       MessageBox(NULL,_T("创建进程失败"),_T("程序错误"),0);
       ::exit(0);
   }
   WaitForSingleObject(pi.hProcess, INFINITE);

   CloseHandle(pi.hProcess);
   CloseHandle(pi.hThread);
}
int main()
 {


     int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // 初始化 MFC 并在失败时显示错误
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: 在此处为应用程序的行为编写代码。
            wprintf(L"错误: MFC 初始化失败\n");
            nRetCode = 1;
        }
        else
        {
            if (IsAdmin()) {
                OutputDebugString("current is run as administrator!\r\n");
                //MessageBox(NULL, _T("管理员"), _T("用户状态"), 0);
            }
            else {
                OutputDebugString("current is run as normal user!\r\n");
                RunAsAdmin();
                //MessageBox(NULL, _T("普通用户"), _T("用户状态"), 0);
                return nRetCode;
            }

            CCommand cmd;
            ChooseAutoInvoke();
			int ret= CServerSocket::getInstance()->Run(CCommand::RunCommand,&cmd);
            switch (ret) {
            case -1:
                MessageBox(NULL, _T("网络初始化异常，未能成功"),
                    _T("网络初始化失败"), MB_OK | MB_ICONERROR);
                exit(0);
                break;
            case 2:
                MessageBox(NULL, _T("多次无法正常接入用户，结束程序！"),
                    _T("接入用户失败"), MB_OK | MB_ICONERROR);
                exit(0);
                break;
            }
        }
    }
    else
    {
        // TODO: 更改错误代码以符合需要
        wprintf(L"错误: GetModuleHandle 失败\n");
        nRetCode = 1;
    }

    return nRetCode;
}
