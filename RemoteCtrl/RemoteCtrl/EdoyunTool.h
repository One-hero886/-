#pragma once
class CEdoyunTool
{
public:
    static void Dump(BYTE* pData, size_t nSize) {
        std::string strOut;
        for (size_t i = 0; i < nSize; i++) {
            char buf[8] = "";
            if (i > 0 && (i % 16 == 0))strOut += "\n";
            snprintf(buf, sizeof(buf), "%02X ", pData[i] & 0xFF);
            strOut += buf;

        }
        strOut += "\n";
        OutputDebugStringA(strOut.c_str());
    }
    static bool IsAdmin() {
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
        printf("length of tokeninformation is %d\r\n", len);
        return false;
    }
    static bool RunAsAdmin() {
        //本地策略组 开启Administrator账户 禁止空密码只能登录本地控制台
       //STARTUPINFOW si = {0};
       //PROCESS_INFORMATION pi = { 0 };
       //TCHAR sPath[MAX_PATH] = _T("");
       // GetModuleFileNameW(NULL, sPath, MAX_PATH);
       ////ret=CreateProcessWithTokenW(hToken,LOGON_WITH_PROFILE,NULL, (LPWSTR)(LPCWSTR)strCmd,
       //    //CREATE_UNICODE_ENVIRONMENT,NULL,NULL,&si,&pi);
       //ret = CreateProcessWithLogonW(L"Administrator", NULL,NULL,LOGON_WITH_PROFILE, NULL, sPath,
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
        int ret = CreateProcessWithLogonW(
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
        if (!ret) {
            ShowError();
            MessageBox(NULL, _T("创建进程失败"), _T("程序错误"), 0);
            return false;
        }
        WaitForSingleObject(pi.hProcess, INFINITE);

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    static void ShowError() {
        LPWSTR lpMessageBuf = NULL;
        //strerror(errno);//标准C语言库
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
            NULL, GetLastError(),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPWSTR)&lpMessageBuf, 0, NULL);
        OutputDebugStringW(lpMessageBuf);
        //MessageBoxW(NULL, lpMessageBuf, L"fasheng错误", 0);
        LocalFree(lpMessageBuf);

    }
    static bool WriteStartupDir(const CString& strPath) {
        //通过修改开机启动文件夹来实现开机启动
        TCHAR sPath[MAX_PATH] = _T("");
        GetModuleFileName(NULL, sPath, MAX_PATH);
        return  CopyFile(sPath, strPath, FALSE);
    }
    //开机启动的时候，程序的权限是过呢随启动用的
    //如果两者权限不一致，会导致程序启动失败
    //开机启动对环境变量的影响，如果依赖dll，则可能启动失败
    //解决方法:
    //[复制这些dll到system32下面或者sysWOW64下面]
    //system32下面，多是64位程序 stsWOW64下面多为32位程序
    //【使用静态库而非动态库】
    static bool WtiteRegisterTable(const CString& strPath) {
        //通过修改注册表来实现开机启动
        CString strSubKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
        TCHAR sPath[MAX_PATH] = _T("");
        GetModuleFileName(NULL, sPath, MAX_PATH);
        bool ret = CopyFile(sPath, strPath, FALSE);
        if (ret == FALSE) {
            MessageBox(NULL, _T("复制文件失败，是否权限不足？\r\n"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
            return false;
        }
        HKEY hKey = NULL;
        ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey);
        if (ret != ERROR_SUCCESS) {
            RegCloseKey(hKey);
            MessageBox(NULL, _T("设置自动开机启动失败！是否权限不足？\r\n程序启动失败！"), _T("错误"), MB_ICONERROR
                | MB_TOPMOST);
            return false;
        }
        //CString strPath = CString(_T("%windir%\\system32\\RemoteCtrl.exe"));
        RegSetValueEx(hKey, _T("RemoteCtrl"), 0, REG_SZ, (BYTE*)(LPCTSTR)strPath, strPath.GetLength());
        //RegSetValueEx(hKey, _T("RemoteCtrl"), 0, REG_SZ, (BYTE*)(LPCTSTR)strPath, strPath.GetLength()*sizeof(TCHAR));
        if (ret != ERROR_SUCCESS) {
            RegCloseKey(hKey);
            MessageBox(NULL, _T("设置自动开机启动失败！是否权限不足？\r\n程序启动失败！"), _T("错误"), MB_ICONERROR
                | MB_TOPMOST);
            return false;
        }
        RegCloseKey(hKey);
        return true;
    }
    static bool Init() {
        //用于MFC命令行项目初始化
        HMODULE hModule = ::GetModuleHandle(nullptr);
        if (hModule == nullptr)
        {
            wprintf(L"错误: GetModuleHandle失败\n");
            return false;
        }
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            wprintf(L"错误: MFC 初始化失败\n");
            return false;
        }
        return true;
    }
};

