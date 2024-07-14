#include <WinSock2.h>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <thread>
#include <Windows.h>

#pragma comment (lib, "ws2_32.lib")

#define PORT	8080
#define IP		"127.0.0.1"

int count = 0;
SOCKET		hSocket;

using namespace std;

string loginProcess();
string menuListRequest();
void menuSelect();
void showError(const char* msg);

int state = 0;
string account_id, account_passwd, store_nm, table_id;

void proc_recv() {
	char buffer[10000] = {};
	string data;
	while (!WSAGetLastError()) {
		ZeroMemory(&buffer, 10000);
		recv(hSocket, buffer, 10000, 0);
		cout << "받은 메세지 : " << buffer << endl;
		data = buffer;
		data = data.substr(data.find_first_of('{'));

		if (data.find("ACCOUNT_LOGIN")) {
			stringstream datalist;
			string tmp, msgCD, msg;
			datalist.clear();
			datalist << data;
			char delimiter = ',';
			state = 2;
			Sleep(1000);
			system("cls");
			Sleep(1000);
		}
		if (data.find("MENU_LIST_REQUEST")) {
			stringstream datalist;
			string tmp, msgCD, msg;
			data = data.substr(data.find_first_of('{'));
			datalist.clear();
			datalist << data;
			char delimiter = ',';
			while (getline(datalist, tmp, delimiter)) {
				cout << tmp << endl;
				
			}

			state = 3;
		}
	}
}

int main(int argc, char** argv) {
	WSADATA		wsaData;
	SOCKADDR_IN	servAddr;
	int			strLen;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		showError("WSAStartup() error!");
	}

	hSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (hSocket == INVALID_SOCKET) {
		showError("hSocket() error!");
	}

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr(IP);
	servAddr.sin_port = htons(PORT);

	if (connect(hSocket, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR) {
		showError("connect() error!");
	}

	thread proc1(proc_recv);


	string sendData;
	while (!WSAGetLastError()) {
		if (state == 1) {
			sendData = loginProcess();
			send(hSocket, sendData.c_str(), sendData.length(), 0);
		}
		else if (state == 2) {
			sendData = menuListRequest();
			cout << sendData << endl;
			send(hSocket, sendData.c_str(), sendData.length(), 0);
		}
		else if (state == 3) {
			menuSelect();
		}
	}

	closesocket(hSocket);
	WSACleanup();
	return 0;
}


string loginProcess() {
	stringstream stream;
	string json;

	cout << "\n해당 서비스를 이용하기 위해서는 로그인을 하셔야 합니다.\n" << endl;
	cout << "ACCOUNT ID : ";
	cin >> account_id;
	
	cout << "=================================" << endl;
	cout << "PASSWORD : ";
	cin >> account_passwd;
	
	cout << "=================================" << endl;
	cout << "매장명 : ";
	cin >> store_nm;
	
	cout << "=================================" << endl;
	cout << "테이블 ID : ";
	cin >> table_id;
		
	stream << "GET / HTTP/1.1\r\n\r\n";
	stream << "Accept : */*\r\n";
	stream << "Content-type : application/json\r\n";
	stream << "{\"ACCOUNT_ID\":" << account_id << ",\"ACCOUNT_PASSWD\":" << account_passwd	<< ",\"STORE_NM\":" << store_nm << ",\"TABLE_ID\":" << table_id << "}";
	json = stream.str();
	state = 0;
	return json;
}

string menuListRequest() {
	stringstream stream;
	string json;

	stream << "GET / HTTP/1.0\r\n\r\n";
	stream << "Accept : */*\r\n";
	stream << "Content-type : application/json\r\n";
	stream << "{\"STORE_NM\":" << store_nm << "}";
	json = stream.str();
	state = 0;
	return json;
}

void menuSelect() {
	cout << "메뉴 리스트를 불러옵니다" << endl;
}

void showError(const char* msg) {
	cout << "에러 : " << msg << endl;
	exit(-1);
}