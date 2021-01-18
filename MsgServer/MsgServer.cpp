// MsgServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "framework.h"
#include "MsgServer.h"
#include "Msg.h"
#include "Session.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int gMaxID = M_USER;
map<int, shared_ptr<Session>> gSessions;

void ProcessClient(SOCKET hSock)
{
    CSocket s;
    s.Attach(hSock);
    Message m;

//    while (true)
    {
        int nCode = m.Receive(s);
        switch (nCode)
        {
        case M_INIT:
        {
            auto pSession = make_shared<Session>(++gMaxID, m.m_Data);
            gSessions[pSession->m_ID] = pSession;
            Message::Send(s, pSession->m_ID, M_BROKER, M_INIT);
            cout << "Client " << pSession->m_ID << " connects" << endl;
            break;
        }
        case M_EXIT:
        {
            cout << "Client " << m.m_Header.m_From << " disconnects" << endl;
            gSessions.erase(m.m_Header.m_From);
            Message::Send(s, m.m_Header.m_From, M_BROKER, M_CONFIRM);
            return;
        }
        case M_GETDATA:
        {
            if (gSessions.find(m.m_Header.m_From) != gSessions.end())
            {
                gSessions[m.m_Header.m_From]->timer = time(0);
                gSessions[m.m_Header.m_From]->Send(s);
            }
            break;
        }
        default:
        {
            if (gSessions.find(m.m_Header.m_From) != gSessions.end())
            {
                gSessions[m.m_Header.m_From]->timer = time(0);
                if (gSessions.find(m.m_Header.m_To) != gSessions.end())
                {
                    gSessions[m.m_Header.m_To]->Add(m);
                }
                else if (m.m_Header.m_To == M_ALL)
                {
                    for (auto& [id, Session] : gSessions)
                    {
                        if (id != m.m_Header.m_From)
                            Session->Add(m);
                    }
                }
                Message::Send(s, m.m_Header.m_From, M_BROKER, M_CONFIRM);
            }
            break;
        }
        }
    }
}

void Timeout() {
    while (true) {
        Sleep(1000);
        for (auto& [id, Session] : gSessions) {
            if ((time(0) - Session->timer) > 10) {
                cout << "Client " << id << " is disconnected" << endl;
                gSessions.erase(id);
                break;
            }
        }
    }
}

void Server()
{
    AfxSocketInit();

    CSocket Server;
    Server.Create(12345);
    thread t1(Timeout);
    t1.detach();

    while (true)
    {
        if (!Server.Listen())
            break;
        CSocket s;
        Server.Accept(s);
        thread t(ProcessClient, s.Detach());
        t.detach();
    }
}


// The one and only application object

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
            Server();
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
