#include <WinSock2.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <sql.h>
#include <sqltypes.h>
#include <sqlext.h>
#include <thread>
#include <sstream>

#pragma comment (lib, "ws2_32.lib")

using namespace std;

#define PORT 8080

SOCKET          hServSock;
SOCKET          hClntSock;

SQLHENV henv;
SQLRETURN ret;
SQLHDBC hdbc;
SQLWCHAR* connectionString;
SQLHSTMT hstmt;

void showError(const char* msg)
{
    cout << "에러 : " << msg << endl;
    exit(-1);
}

void proc_recvs() {
    char buffer[1024] = { 0 };
    string data;
    while (!WSAGetLastError()) {
        ZeroMemory(&buffer, 1024);
        recv(hClntSock, buffer, 1024, 0);
        cout << "받은 메세지 : " << buffer << endl;
        data = buffer;
        data = data.substr(data.find('{'));

        stringstream datalist;

        datalist.clear();
        datalist << data;
        char delimiter = ',';
        if (data.find("ACCOUNT_ID")) {
            string tmp, account, passwd, storeNm, tableID;
            while (getline(datalist, tmp, delimiter)) {
                cout << tmp << endl;
                if (tmp.find("ACCOUNT_ID") != string::npos) {
                    account = tmp.substr(tmp.find(":")+1);
                }
                else if (tmp.find("ACCOUNT_PASSWD") != string::npos) {
                    passwd = tmp.substr(tmp.find(':')+1);
                }
                else if (tmp.find("STORE_NM") != string::npos) {
                    storeNm = tmp.substr(tmp.find(':')+1);
                }
                else if (tmp.find("TABLE_ID") != string::npos) {
                    tableID = tmp.substr(tmp.find(':')+1);
                    tableID.erase(tableID.end()-1);
                }
            }
            cout << account << "/" << passwd << "/" << storeNm << "/" << tableID << endl;
            stringstream reply;
            string json;
            reply << "{\"MSG_CD\":0000" << ",\"MSG\":ACCOUNT_LOGIN SUCCESS}";
            json = reply.str();
            send(hClntSock, json.c_str(), json.length(), 0);
        }
        else if (data.find("STORE_NM")) {
            string tmp, storeNm;
            while (getline(datalist, tmp, delimiter)) {
                cout << tmp << endl;
                if (tmp.find("STORE_NM") != string::npos) {
                    storeNm = tmp.substr(tmp.find(':') + 1);
                }
            }


        }
        
        
    }
}

int main(int argc, char** argv)
{
    
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

    ret = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);

    
    ret = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

    connectionString = (SQLWCHAR*)L"DRIVER={SQLServer};SERVER=serverIP;DATABASE=DataBaseName;UID=username;PWD=passwd";

    ret = SQLDriverConnect(hdbc, NULL, connectionString, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);
    //쿼리 실행을 위한 문장 핸들 생성
    ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    

    WSADATA         wsaData;
    SOCKADDR_IN     servAddr;
    SOCKADDR_IN     clntAddr;

    long valread;

    int szClntAddr;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        showError("WSAStartup() error!");
    }

    hServSock = socket(PF_INET, SOCK_STREAM, 0);
    if (hServSock == INVALID_SOCKET) {
        showError("socket() error!");
    }

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(PORT);

    if (bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR) {
        showError("bind() error!");
    }
    
    if (listen(hServSock, 5) == SOCKET_ERROR) {
        showError("listen() error!");
    }


    cout << "클라이언트를 기다립니다...." << endl;
    cout << "-----------------------------\n" << endl;
    szClntAddr = sizeof(clntAddr);
    hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &szClntAddr);
    if (hClntSock == INVALID_SOCKET) {
        showError("accept() error!");
    }
    if (!WSAGetLastError()) {
        cout << "연결 완료" << endl;
        cout << "Client IP : " << inet_ntoa(clntAddr.sin_addr) << endl;
        cout << "Port : " << ntohs(clntAddr.sin_port) << endl;
    }

    thread proc(proc_recvs);

    char buffer[10000] = { 0 };
    while (!WSAGetLastError()) {
        
    }
    
    proc.join();

    closesocket(hClntSock);
    closesocket(hServSock);
    ::WSACleanup();
    return 0;
}