// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "ServerSocket.h"
#include <stdio.h>
#include <io.h>
#include<list>
typedef struct file_info{
    file_info() {
        IsInvalid = false;
        IsDirectory = -1;
        HasNext = true;
        memset(szFileName, 0, sizeof(szFileName));
    }
    bool IsInvalid;//是否有效
    bool IsDirectory;//是否为目录
    bool HasNext;//是否还有后续
    char szFileName[256];//文件名
}FILEINFO,*PFILEINFO;
#include"direct.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif



// 唯一的应用程序对象123

CWinApp theApp;

using namespace std;

int MakeDriverInfo() {
    std::string result;

    for (int i = 1; i <= 26; i++) {
        if (_chdrive(i) == 0) {
            if (result.size() > 0) {
                result += ',';
                result += 'A' + i - 1;
            }
        }
    }
    CPacket pack(1, (BYTE*)result.c_str(), result.size());
    CServerSocket::getInstance()->Send(pack);
    return 0;
}

int MakeDirectoryInfo() {
    
    std::string strPath;
    //std::list<FILEINFO> lstFileInfos;
    if (CServerSocket::getInstance()->GetFilePath(strPath) == false) {
        OutputDebugString(_T("当前的命令，不是获取文件列表，命令解析错误"));
        return -1;
    }
    if (_chdir(strPath.c_str()) != 0) {
        FILEINFO finfo;
        finfo.IsInvalid = true;
        finfo.IsDirectory = true;
        finfo.HasNext = false;
        memcpy(finfo.szFileName,strPath.c_str(),strPath.size());
        CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
        CServerSocket::getInstance()->Send(pack);
        //lstFileInfos.push_back(finfo);
        OutputDebugString(_T("没有权限，访问目录"));
        return -2;
    }
    _finddata_t fdata;
    int hfind = 0;
    if ((hfind=_findfirst("*", &fdata)) == -1) {
        OutputDebugString(_T("没有找到任何文件！！"));
        return -3;
    }
    do {
        FILEINFO finfo;
        finfo.IsDirectory = (fdata.attrib & _A_SUBDIR) != 0;
        memcpy(finfo.szFileName,fdata.name,strlen(fdata.name));
        CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
        CServerSocket::getInstance()->Send(pack);
        //lstFileInfos.push_back(finfo);
    } while (!_findnext(hfind, &fdata));

    FILEINFO finfo;
    finfo.HasNext = false;
    CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
    CServerSocket::getInstance()->Send(pack);
    return 0;
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
            // TODO: 在此处为应用程序的行为编写代码。

            //CServerSocket* pserver = CServerSocket::getInstance();
            //int count = 0;
            //    if (pserver->InitSocket() == false) {
            //        MessageBox(NULL, _T("网络初始化异常，未能成功"),
            //            _T("网络初始化失败"), MB_OK | MB_ICONERROR);
            //        exit(0);
            //    }
            //while (CServerSocket::getInstance()!=NULL) {
            //    if (pserver->AcceptClient() == false) {
            //        if (count >= 3) {
            //            MessageBox(NULL, _T("多次无法正常接入用户，结束程序！"),
            //                _T("接入用户失败"), MB_OK | MB_ICONERROR);
            //            exit(0);
            //        }
            //        MessageBox(NULL, _T("无法连接用户"), _T("接入用户失败"), MB_OK | MB_ICONERROR);
            //        count++;
            //    }

            //    int ret = pserver->DealCommand();

            //}

            int nCmd = 1;

            switch (nCmd) {
            case 1://查看磁盘分区
                MakeDriverInfo();
                break;

            case 2://查看指定目录下的文件
                MakeDirectoryInfo();
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
