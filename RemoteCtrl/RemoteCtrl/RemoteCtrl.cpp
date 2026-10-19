// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "RemoteCtrl.h"
#include "ServerSocket.h"
#include"Command.h"  
#include "framework.h"
#include<conio.h>
#include "CEdoyunQueue.h"
#include <MSWSock.h>
#include"EdoyunServer.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
//#define INVOKE_PATH _T(" C:\\Windows\\SysWOW64\\RemoteCtrl.exe")
#define INVOKE_PATH _T("C:\\Users\\TMC\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\RemoteCtrl.exe")

// 唯一的应用程序对象123

CWinApp theApp;

using namespace std;

//业务和通用
bool ChooseAutoInvoke(const CString& strPath) {
    TCHAR wcsSystem[MAX_PATH] = _T("");
    if (PathFileExists(strPath)) {
        return true;
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
        if (!CEdoyunTool::WriteStartupDir(strPath)){
          MessageBox(NULL, _T("复制文件失败，是否权限不足？\r\n"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
          return false;
        }
    }
    else if (ret==IDCANCEL) {
        return false;
    }
    return true;
}

class COverlapped {
public:
    OVERLAPPED m_overlapped;
    DWORD m_operator;
    char m_buffer[4096];
    COverlapped() {
        m_operator = 0;
        memset(&m_overlapped, 0, sizeof(m_overlapped));
        memset(m_buffer, 0, sizeof(m_buffer));
    }
};
void iocp() {
    EdoyunServer server;
    server.StartServer();
    getchar();
    //SOCKET sock = WSASocket(AF_INET,SOCK_STREAM,0,NULL,0,WSA_FLAG_OVERLAPPED);
    //if (sock == INVALID_SOCKET) {
    //    CEdoyunTool::ShowError();
    //    return;
    //}
    //HANDLE hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,sock,4);
    //SOCKET client = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    //CreateIoCompletionPort((HANDLE)sock,hIOCP,0,0);
    //sockaddr_in addr;
    //addr.sin_family = PF_INET;
    //addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    //addr.sin_port = htons(8080);
    //bind(sock,(sockaddr*)&addr,sizeof(addr));
    //listen(sock, 5);

    //COverlapped overlapped;
    //overlapped.m_operator = 1;//accpet

    //memset(&overlapped, 0, sizeof(OVERLAPPED));
    //char buffer[4096] = "";
    //DWORD received = 0;
    //if (AcceptEx(sock, client, buffer, 0, sizeof(sockaddr_in) + 16,
    //    sizeof(sockaddr_in) + 16, &received, &overlapped.m_overlapped) == FALSE) {
    //    CEdoyunTool::ShowError();
    //}

    ////开启线程
    //while (true) {//代表一个线程
    //    LPOVERLAPPED pOverlapped = NULL;
    //    DWORD transferred = 0;
    //    DWORD key = 0;
    //    if(GetQueuedCompletionStatus(hIOCP,&transferred, &key, &pOverlapped, INFINITY)) {
    //         
    //        switch () {
    //        case 0:
    //           

    //        
    //        }

    //    }
    //}
   
}

#include"ESocket.h"
void udp_server();
void udp_client(bool ishost=true);
void initsock() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
}
void clearsock() {
    WSACleanup();
}
int main(int argc,char* argv[])
{
    //if (!CEdoyunTool::Init())return 1;
    //initsock();
    //if (argc == 1) {
    //    char wstrDir[MAX_PATH];
    //    GetCurrentDirectoryA(MAX_PATH, wstrDir);
    //    STARTUPINFOA si;
    //    PROCESS_INFORMATION pi;
    //    memset(&si,0,sizeof(si));
    //    memset(&pi, 0, sizeof(pi));
    //    string strCmd = argv[0];
    //    strCmd += " 1";
    //    bool bRet=CreateProcessA(NULL,(LPSTR)strCmd.c_str(), NULL, NULL, FALSE, 0, NULL, wstrDir, &si, &pi);
    //    if (bRet) {
    //        CloseHandle(pi.hThread);
    //        CloseHandle(pi.hProcess);
    //        TRACE("进程ID：%d\r\n",pi.dwProcessId);
    //        TRACE("线程ID：%d\r\n", pi.dwThreadId);
    //        strCmd += " 2";
    //        bool bRet = CreateProcessA(NULL,(LPSTR)strCmd.c_str(), NULL, NULL, FALSE, 0, NULL, wstrDir, &si, &pi);
    //        if (bRet) {
    //            CloseHandle(pi.hThread);
    //            CloseHandle(pi.hProcess);
    //            TRACE("进程ID：%d\r\n", pi.dwProcessId);
    //            TRACE("线程ID：%d\r\n", pi.dwThreadId);
    //            udp_server();//服务器代码
    //        }
    //    }
    //}
    //else if (argc == 2) {//主客户端
    //    udp_client();
    //}
    //else {//从客户端
    //    udp_client(false);
    //}
    //clearsock();

    iocp();
    
    if (CEdoyunTool::IsAdmin()) {
        if (!CEdoyunTool::Init())return 1;
        if (ChooseAutoInvoke(INVOKE_PATH)) {
            CCommand cmd;
            int ret = CServerSocket::getInstance()->Run(CCommand::RunCommand, &cmd);
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
    else {
        if (CEdoyunTool::RunAsAdmin() == false) {
            CEdoyunTool::ShowError();
            return -1;
        }
    }
    
    return 0;
}
#include"ENetwork.h"
int RecvFromCB(void* arg, const EBuffer& buffer, ESockaddrIn& addr) {
    EServer* server = (EServer*)arg;
    return server->Sendto(addr,buffer);
}
int SendToCB(void* arg, const ESockaddrIn& addr, int ret) {
    EServer* server = (EServer*)arg;
    printf("sendto done!%p\r\n", server);
    return 0;
}
void udp_server() {
    EServerParameter param(
        "127.0.0.1", 20000, ETYPE::ETypeUDP, NULL, NULL, NULL, RecvFromCB, SendToCB
    );
    EServer server(param);
    server.Invoke(&server);
    printf("%s(%d):%s\r\n",__FILE__,__LINE__,__FUNCTION__);
    getchar();
    ESOCKET sock(new ESocket(ETYPE::ETypeUDP));
    if(*sock == INVALID_SOCKET) {
        printf("%s(%d):%sERROR!!!\r\n", __FILE__, __LINE__, __FUNCTION__);
        return;
    }
    std::list<ESockaddrIn> lstclients;
    ESockaddrIn client;
    if (sock->bind("127.0.0.1",20000) == -1) {
        printf("%s(%d):%sERROR(%d)!!!\r\n", __FILE__, __LINE__, __FUNCTION__,WSAGetLastError());
        closesocket(*sock);
        return;
    }
    EBuffer buf(1024 * 256);
    int len = sizeof(client);
    int ret = 0;
    while(!_kbhit()) {
        ret = sock->recvfrom(buf,client);
        if (ret > 0) {
            if (lstclients.size()<=0) {
                lstclients.push_back(client);
                printf("%s(%d):%s ip %s port %d\r\n", __FILE__, __LINE__, __FUNCTION__,
                    client.Get_IP(), client.GetPort());
                ret = sock->sendto(buf, client);
                printf("%s(%d):%s\r\n", __FILE__, __LINE__, __FUNCTION__);
            }
            else {
                buf.Update((void*)&lstclients.front(), lstclients.front().size());
                ret = sock->sendto(buf, client);
                printf("%s(%d):%s\r\n", __FILE__, __LINE__, __FUNCTION__);
            }
        }
        else {
            printf("%s(%d):%sERROR(%d)!!!ret=%d\r\n", __FILE__, __LINE__, __FUNCTION__, 
                WSAGetLastError(),ret);
        }
    }
    Sleep(1);
    
    printf("%s(%d):%s\r\n", __FILE__, __LINE__, __FUNCTION__);
    // CEdoyunTool::Dump((BYTE*)buf, ret);
}
void udp_client(bool ishost) {
    Sleep(2000);
    sockaddr_in server,client;
    int len = sizeof(client);
    server.sin_family = AF_INET;
    server.sin_port = htons(20000);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    SOCKET sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("%s(%d):%sERROR!!!\r\n", __FILE__, __LINE__, __FUNCTION__);
        return;
    }
    if (ishost) {//主客户端代码
        printf("%s(%d):%s\r\n", __FILE__, __LINE__, __FUNCTION__);
         EBuffer msg="hello world!\n";
        int ret = sendto(sock, msg.c_str(), msg.size(), 0, (sockaddr*)&server, sizeof(server));
        if (ret > 0) {
            msg.resize(1024);
            memset((char*)msg.c_str(), 0, msg.size());
            ret = recvfrom(sock, (char*)msg.c_str(), msg.size(), 0, (sockaddr*)&client, &len);
            printf("host %s(%d):%sERROR(%d)!!!ret==%d\r\n", __FILE__, __LINE__, __FUNCTION__, WSAGetLastError(),ret);
            if (ret > 0) {
                printf("%s(%d):%s ip %08x port %d\r\n", __FILE__, __LINE__, __FUNCTION__,
                    client.sin_addr.s_addr, ntohs(client.sin_port));
                printf("%s(%d):%s msg=%d\r\n", __FILE__, __LINE__, __FUNCTION__, msg.size());
            }
            ret = recvfrom(sock, (char*)msg.c_str(), msg.size(), 0, (sockaddr*)&client, &len);
            printf("host %s(%d):%sERROR(%d)!!!ret==%d\r\n", __FILE__, __LINE__, __FUNCTION__, WSAGetLastError(), ret);
            if (ret > 0) {
                printf("%s(%d):%s ip %08x port %d\r\n", __FILE__, __LINE__, __FUNCTION__,
                    client.sin_addr.s_addr, ntohs(client.sin_port));
                printf("%s(%d):%s msg=%s\r\n", __FILE__, __LINE__, __FUNCTION__, msg.c_str());
            }
        }
    }
    else {//从客户端代码
        printf("%s(%d):%s\r\n", __FILE__, __LINE__, __FUNCTION__);
        std::string msg = "hello world!\n";
        int ret = sendto(sock, msg.c_str(), msg.size(), 0, (sockaddr*)&server, sizeof(server));
        if (ret > 0) {
            msg.resize(1024);
            memset((char*)msg.c_str(), 0, msg.size());
            ret = recvfrom(sock, (char*)msg.c_str(), msg.size(), 0, (sockaddr*)&client, &len);
            printf("host %s(%d):%sERROR(%d)!!!ret==%d\r\n", __FILE__, __LINE__, __FUNCTION__, WSAGetLastError(), ret);
            if (ret > 0) {
                sockaddr_in addr;
                memcpy(&addr, msg.c_str(), sizeof(addr));
                sockaddr_in* paddr = (sockaddr_in*)msg.c_str();
                printf("%s(%d):%s ip %08x port %d\r\n", __FILE__, __LINE__, __FUNCTION__,
                    client.sin_addr.s_addr, ntohs(client.sin_port));
                printf("%s(%d):%s msg=%d\r\n", __FILE__, __LINE__, __FUNCTION__, msg.size());
                printf("%s(%d):%s ip %08x port %d\r\n", __FILE__, __LINE__, __FUNCTION__,
                    paddr->sin_addr.s_addr, ntohs(paddr->sin_port));
                msg = "hello,i am client\r\n";
               ret = sendto(sock, (char*)msg.c_str(), msg.size(), 0, (sockaddr*)&addr, sizeof(sockaddr_in));
               printf("host %s(%d):%sERROR(%d)!!!ret==%d\r\n", __FILE__, __LINE__, __FUNCTION__, WSAGetLastError(), ret);
            }
        }
    }
    closesocket(sock);
}