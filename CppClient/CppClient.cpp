// CppClient.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "framework.h"
#include "CppClient.h"
#include "mutex"
#include "../MsgServer/Msg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

mutex mut;

void ProcessMessages()
{
    while (true)
    {
        Message m = Message::Send(M_BROKER, M_GETDATA);
        switch (m.m_Header.m_Type)
        {
        case M_DATA:
        {
            mut.lock();
            cout << m.m_Data << endl;
            mut.unlock();
            break;
        }
        default:
        {
            Sleep(100);
            break;
        }
        }
    }
}

// The one and only application object
void Client()
{
    AfxSocketInit();
    Message m = Message::Send(M_BROKER, M_INIT);
    thread t(ProcessMessages);
    t.detach();
    while (1) {
        cout << "Send Msg - 0;  Exit - 1: ";
        bool check;
        cin >> check;
        switch (check)
        {
        case 0:
        {
            mut.lock();
            cout << "Write Receiver: ";
            cin >> m.m_Header.m_To;
            cout << "Write Message: ";
            cin.ignore();
            getline(cin, m.m_Data);
            Message::Send(m.m_Header.m_To, M_DATA, m.m_Data);
            mut.unlock();
            break;
        }
        case 1:
        {
            Message::Send(M_BROKER, M_EXIT);
            return;
        }
        }
    }
}

CWinApp theApp;

using namespace std;

int main()
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // initialize MFC and print and error on failure
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: code your application's behavior here.
            wprintf(L"Fatal Error: MFC initialization failed\n");
            nRetCode = 1;
        }
        else
        {
            Client();
        }
    }
    else
    {
        // TODO: change error code to suit your needs
        wprintf(L"Fatal Error: GetModuleHandle failed\n");
        nRetCode = 1;
    }

    return nRetCode;
}
