// AsyncSelect_NetDraw_Client.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>

#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#include <windowsx.h>

#pragma comment (lib, "Ws2_32.lib")

#include "framework.h"
#include "AsyncSelect_NetDraw_Client.h"
#include "c_Save_Log.h"
#include "C_Ring_Buffer.h"

#define MAX_LOADSTRING 100

constexpr int UM_NETWORK = (WM_USER + 1);

struct st_HEADER
{
    unsigned short Len;
};
struct st_DRAW_PACKET
{
    int iStartX;
    int iStartY;
    int iEndX;
    int iEndY;
};


// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.
HWND hWnd;

const char* DEFAULT_PORT = "25000";
u_short s_DEFAULT_PORT = 25000;
SOCKET Connect_Socket;
WSADATA wsa_Data;
wchar_t IP[256];

wchar_t Log[256];
struct sockaddr_in Server_Addr;

C_RING_BUFFER *Send_Buffer;
C_RING_BUFFER *Recv_Buffer;



// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK MyDialogBox(HWND hwndDig, UINT message, WPARAM wParam, LPARAM IParam);

void WriteEvent();
void ReadEvent();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.
    int i_Result;
    WSADATA wsaData;

    Send_Buffer = new C_RING_BUFFER;
    Recv_Buffer = new C_RING_BUFFER;

    i_Result = WSAStartup(MAKEWORD(2, 2), &wsa_Data);
    if (i_Result != 0)
    {
        swprintf_s(Log, L"WSAStartup failed with error : % d \n", i_Result);
        c_Save_Log.saveLog(Log);
        return false;
    }
    Connect_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (Connect_Socket == INVALID_SOCKET)
    {
        swprintf_s(Log, L"socket failed with error: %ld \n", WSAGetLastError());
        c_Save_Log.saveLog(Log);
        WSACleanup();
        return false;
    }
    //////////////////////////////////////////////////////////////////////////////////

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_ASYNCSELECTNETDRAWCLIENT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);


    // 여기서 윈도우를 생성한다.
    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    //i_Result = WSAAsyncSelect(Connect_Socket, hWnd, UM_NETWORK, FD_CONNECT | FD_CLOSE | FD_READ | FD_WRITE);
    //if (i_Result == SOCKET_ERROR)
    //{
    //    swprintf_s(Log, L"WSAAsyncSelect failed with error: %ld \n", WSAGetLastError());
    //    c_Save_Log.saveLog(Log);
    //    WSACleanup();
    //    return false;
    //}



    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ASYNCSELECTNETDRAWCLIENT));

    MSG msg;

    // 기본 메시지 루프입니다:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ASYNCSELECTNETDRAWCLIENT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_ASYNCSELECTNETDRAWCLIENT);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    int i_Result;

   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }




   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//


static int oldX;
static int oldY;
static bool click;
static bool b_Flag = false;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int i_Result;


    switch (message)
    {
    case UM_NETWORK:
    {
        i_Result = WSAGETASYNCERROR(lParam);
        if (i_Result == SOCKET_ERROR)
        {
            swprintf_s(Log, L"WSAGETASYNCERROR failed with error: %ld \n", WSAGetLastError());
            c_Save_Log.saveLog(Log);
            closesocket(Connect_Socket);
            WSACleanup();
            PostMessage(hWnd, WM_DESTROY, 0, 0);
            break;
        }

        switch (WSAGETSELECTEVENT(lParam))
        {
        case FD_CONNECT:
        {
            break;
        }
        case FD_CLOSE:
        {
            swprintf_s(Log, L"연결이 종료되었습니다.  \n");
            c_Save_Log.saveLog(Log);

            closesocket(Connect_Socket);
            WSACleanup();

            delete Send_Buffer;
            delete Recv_Buffer;

            PostMessage(hWnd, WM_DESTROY, 0, 0);
            break;
        }
        case FD_WRITE:
        {
            b_Flag = true;
            WriteEvent();
            break;
        }
        case FD_READ:
        {
            ReadEvent();
            break;
        }
        default:
            break;
        }
    }
    break;
    case WM_CREATE:
    {
        int i_Result = WSAAsyncSelect(Connect_Socket, hWnd, UM_NETWORK, FD_CONNECT | FD_CLOSE | FD_READ | FD_WRITE);
        if (i_Result == SOCKET_ERROR)
        {
            swprintf_s(Log, L"WSAAsyncSelect failed with error: %ld \n", WSAGetLastError());
            c_Save_Log.saveLog(Log);
            WSACleanup();
            return false;
        }

        DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, (DLGPROC)MyDialogBox);
    }
    break;

    case WM_LBUTTONDOWN:
    {
        click = true;
    }
    break;
    case WM_LBUTTONUP:
    {
        click = false;
    }
    break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_MOUSEMOVE:
    {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        if (click)
        {
            st_HEADER header;
            st_DRAW_PACKET packet;

            header.Len = sizeof(packet);
            packet.iStartX = oldX;
            packet.iStartY = oldY;
            packet.iEndX = x;
            packet.iEndY = y;

            i_Result = Send_Buffer->Enqueue((const char*)&header, sizeof(header));
            if (i_Result == 0)
            {
                swprintf_s(Log, L"SendBuff가 다 차서 종료되었습니다.  \n");
                c_Save_Log.saveLog(Log);

                closesocket(Connect_Socket);
                WSACleanup();

                delete Send_Buffer;
                delete Recv_Buffer;

                PostMessage(hWnd, WM_DESTROY, 0, 0);
                break;
            }

            i_Result = Send_Buffer->Enqueue((const char*)&packet, sizeof(packet));
            if (i_Result == 0)
            {
                swprintf_s(Log, L"SendBuff가 다 차서 종료되었습니다.  \n");
                c_Save_Log.saveLog(Log);

                closesocket(Connect_Socket);
                WSACleanup();

                delete Send_Buffer;
                delete Recv_Buffer;

                PostMessage(hWnd, WM_DESTROY, 0, 0);
                break;
            }

            WriteEvent();
        }

        oldY = y;
        oldX = x;
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

BOOL CALLBACK MyDialogBox(HWND hwndDig, UINT message, WPARAM wParam, LPARAM IParam)
{
    int i_Result;

    switch (message)
    {
        case WM_INITDIALOG:
		{
			// Edit Control에 기본 텍스트 설정
            SetDlgItemText(hwndDig, IDC_EDIT1, L"127.0.0.1");
			return (INT_PTR)TRUE;
		}
        case WM_COMMAND:
        {
			int wmId = LOWORD(wParam);
			// 메뉴 선택을 구문 분석합니다:
			switch (wmId)
			{
                case IDOK:
				{
                    GetDlgItemText(hwndDig, IDC_EDIT1, IP, sizeof(IP));

                    Server_Addr.sin_family = AF_INET;
                    Server_Addr.sin_port = htons(s_DEFAULT_PORT);
                    InetPton(AF_INET, IP, &Server_Addr.sin_addr);

                    i_Result = connect(Connect_Socket, (struct sockaddr*)&Server_Addr, sizeof(Server_Addr));
                    if (i_Result == SOCKET_ERROR)
                    {
                        if (WSAGetLastError() == WSAEWOULDBLOCK)
                        {
                            swprintf_s(Log, L"WSAEWOULDBLOCK to server  %ld \n", WSAGetLastError());
                            c_Save_Log.saveLog(Log);
                        }
                        else if (WSAGetLastError() != WSAEWOULDBLOCK)
                        {
                            swprintf_s(Log, L"Unable to connect to server  %ld \n", WSAGetLastError());
                            c_Save_Log.saveLog(Log);

                            closesocket(Connect_Socket);
                            WSACleanup();
                            EndDialog(hwndDig, 0);
                            PostQuitMessage(0);
                            return TRUE;
                        }
                    }

                    EndDialog(hwndDig, 0);
				}
                break;
				case IDCANCEL:
				{
					EndDialog(hwndDig, 0);
                    PostQuitMessage(0);
					return TRUE;
				}
				break;
	
			}
            InvalidateRect(hwndDig, NULL, TRUE);
            UpdateWindow(hwndDig);
		}
		    break;
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwndDig, &ps);
            EndPaint(hwndDig, &ps);
        }
    }

    return FALSE;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void WriteEvent()
{
    int Send_Size;

    if (b_Flag == false) return;

    while (1)
    {
        // 링 버퍼에 있는 것을 전부 보내야 한다. 
        if (Send_Buffer->GetUseSize() == 0)
            return;
        
        // 링버퍼는 전부 차지 않는 다는 가정으로 만들어졌다. 고로 끊어진 메시지는 있을 수 없다. 
         Send_Size = send(Connect_Socket, Send_Buffer->GetFrontBufferPtr(), Send_Buffer->DirectDequeueSize(), 0);

        if (Send_Size == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
            {
                swprintf_s(Log, L"send failed with error: %ld \n", WSAGetLastError());
                c_Save_Log.saveLog(Log);

                closesocket(Connect_Socket);
                WSACleanup();
                PostMessage(hWnd, WM_DESTROY, 0, 0);
            }

            b_Flag = false;
            break;
        }
        Send_Buffer->MoveFront(Send_Size);
    }
}
void ReadEvent()
{
    // 버퍼의 모든 데이터를 링버퍼로 가져온다. 
    // Recv버퍼는 다 차지 않는다. 

    int Recv_Size;
    int Peek_Result;
    int Dq_Result;
    st_HEADER header;
    st_DRAW_PACKET packet;

    Recv_Size = recv(Connect_Socket, Recv_Buffer->GetRearBufferPtr(), Recv_Buffer->DirectEnqueueSize(), 0);
    if (Recv_Size == SOCKET_ERROR)
    {
        if (WSAGetLastError() == WSAEWOULDBLOCK)
        {
            swprintf_s(Log, L"recv WSAEWOULDBLOCK failed with error: %ld \n", WSAGetLastError());
            c_Save_Log.saveLog(Log);
            
            // 안뜰거 같은데
            __debugbreak();
            return;
        }

        else if (WSAGetLastError() != WSAEWOULDBLOCK)
        {
            swprintf_s(Log, L"recv failed with error: %ld \n", WSAGetLastError());
            c_Save_Log.saveLog(Log);

            closesocket(Connect_Socket);
            WSACleanup();
            PostMessage(hWnd, WM_DESTROY, 0, 0);
            return;
        }
    }

    Recv_Buffer->MoveRear(Recv_Size);


    // Recv버퍼에 받은 내용을 바로 그린다.
    HDC hdc = GetDC(hWnd);
    while (1)
    {
        // 헤더도 완성하지 못했다면 반환한다. 
        if (Recv_Buffer->GetUseSize() < sizeof(st_DRAW_PACKET)) break;

        // 헤더를 열어서 사이즈를 확인한다. 
        Peek_Result = Recv_Buffer->Peek((char*)&header, sizeof(st_HEADER), true);
        if (Peek_Result == 0) break;

        // (헤더 + 메시지) 를 완성하지 못했다면 반환한다. 
        if (Recv_Buffer->GetUseSize() < sizeof(st_HEADER) + header.Len) break;

        // (헤더 + 메시지) 크기가 링버퍼의 최대 크기보다 크다면 연결을 끊는다. 
        if (header.Len + sizeof(header) > Recv_Buffer->GetBufferSize())
        {
            ReleaseDC(hWnd, hdc);
            swprintf_s(Log, L"recv(헤더 + 메시지)의 크기가 버퍼 총 사이즈보다 크다. with error. \n");
            c_Save_Log.saveLog(Log);

            closesocket(Connect_Socket);
            WSACleanup();
            PostMessage(hWnd, WM_DESTROY, 0, 0);
            return;
        }

        //---------------------------------------------
        // 메시지 완성됨 == 메시지를 꺼내서 그린다. 
        Recv_Buffer->MoveFront(sizeof(st_HEADER));

        Dq_Result = Recv_Buffer->Dequeue((char*)&packet, header.Len);
        // 혹시 모를 에러체크, 싱글 스레드에서는 일어날 일이 없다.
        // 근데 Dequeue로 이미 뺏는데 뻑나면, 뭐함???? 이미 망했는데???
        if (Dq_Result < header.Len)
        {
            swprintf_s(Log, L"recv DQ failed with error \n");
            c_Save_Log.saveLog(Log);

            closesocket(Connect_Socket);
            WSACleanup();
            PostMessage(hWnd, WM_DESTROY, 0, 0);
        }

        MoveToEx(hdc, packet.iStartX, packet.iStartY, NULL);
        LineTo(hdc, packet.iEndX, packet.iEndY);

    }
    ReleaseDC(hWnd, hdc);
}
