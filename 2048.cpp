// 2048.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "dwmapi.h"
#include "2048.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

LRESULT CALLBACK WndChldProc(HWND, UINT, WPARAM, LPARAM);
void InitMainWindow(HWND);

const int numOfTilesX = 4;
const int numOfTilesY = 4;
HWND tiles[numOfTilesY][numOfTilesX];
const int tileSize = 60;
const int gapSize = 10;
HWND mainWindow;
int mainWindowHeight;
int mainWindowWidth;
int mainWindowClientAreaHeight;
int mainWindowClientAreaWidth;
HWND mirroredWindow;
const int mirroredWindowOffsetX = 500;
const int mirroredWindowOffsetY = 150;
int screenHeight;
int screenWidth;

ATOM                MyRegisterTileClass(HINSTANCE);
void                moveAndTransparency(HWND, HWND, HWND);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MY2048, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
    MyRegisterTileClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MY2048));

    MSG msg;

    // Main message loop:
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
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = CreateSolidBrush(RGB(250, 247, 238));
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDR_MENU1);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));

    return RegisterClassExW(&wcex);
}

ATOM MyRegisterTileClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndChldProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInst;
    wcex.hIcon = nullptr;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = CreateSolidBrush(RGB(204, 192, 174));
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"TILE";
    wcex.hIconSm = nullptr;

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   mainWindow = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME | WS_CLIPCHILDREN,
      100, 100, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!mainWindow)
   {
       return FALSE;
   }

   mirroredWindow = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION,
       100 + mirroredWindowOffsetX, 100 + mirroredWindowOffsetY, CW_USEDEFAULT, 0, mainWindow, nullptr, hInstance, nullptr);

   if (!mirroredWindow)
   {
       return FALSE;
   }

   mainWindowClientAreaHeight = numOfTilesY * (gapSize + tileSize) + gapSize;
   mainWindowClientAreaWidth = numOfTilesX * (gapSize + tileSize) + gapSize;

   RECT rc;
   SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
   screenHeight = rc.bottom - rc.top;
   screenWidth = rc.right - rc.left;

   SetWindowLong(mirroredWindow, GWL_EXSTYLE, GetWindowLong(mirroredWindow, GWL_EXSTYLE) | WS_EX_LAYERED);
   InitMainWindow(mirroredWindow);
   ShowWindow(mirroredWindow, nCmdShow);
   UpdateWindow(mirroredWindow);

   SetForegroundWindow(mainWindow);
   InitMainWindow(mainWindow);
   ShowWindow(mainWindow, nCmdShow);
   UpdateWindow(mainWindow);
   
   //BringWindowToTop(mainWindow);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_MOVE:
    {
        if (hWnd == mainWindow)
            moveAndTransparency(mainWindow, mirroredWindow, mirroredWindow);
        else
            moveAndTransparency(mirroredWindow, mainWindow, mirroredWindow);
    }
    break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
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
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
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

// Message handler for about box.
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

LRESULT CALLBACK WndChldProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
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

void InitMainWindow(HWND hWnd) {
    RECT rc;
    GetWindowRect(hWnd, &rc);
    rc.right = rc.left + mainWindowClientAreaWidth;
    rc.bottom = rc.top + mainWindowClientAreaHeight;
    AdjustWindowRectEx(&rc, (DWORD)GetWindowLong(hWnd, GWL_STYLE), GetMenu(hWnd) != NULL, 
        (DWORD)GetWindowLong(hWnd, GWL_EXSTYLE));
    mainWindowHeight = rc.bottom - rc.top;
    mainWindowWidth = rc.right - rc.left;
    MoveWindow(hWnd, rc.left, rc.top, mainWindowWidth, mainWindowHeight, TRUE);

    SetWindowText(hWnd, L"2048");
    for (int i = 0; i < numOfTilesY; ++i) {
        for (int j = 0; j < numOfTilesX; ++j) {
            tiles[i][j] = CreateWindowW(L"TILE", L"TILE", WS_CHILD | WS_VISIBLE,
                (j + 1) * gapSize + j * tileSize, (i + 1) * gapSize + i * tileSize,
                tileSize, tileSize, hWnd, nullptr, hInst, nullptr);
        }
    }
}

void moveAndTransparency(HWND hWnd1, HWND hWnd2, HWND hWnd3) {
    RECT rc1, rc2;
    GetWindowRect(hWnd1, &rc1);
    GetWindowRect(hWnd2, &rc2);

    rc2.left = screenWidth - rc1.left - mainWindowWidth;
    rc2.right = screenWidth - rc1.left;
    rc2.top = screenHeight - rc1.top - mainWindowHeight;
    rc2.bottom = screenHeight - rc1.top;

    MoveWindow(hWnd2, rc2.left, rc2.top, mainWindowWidth, mainWindowHeight, TRUE);

    if ((rc1.left >= rc2.left && rc1.left <= rc2.right || rc1.right >= rc2.left && rc1.right <= rc2.right) &&
        (rc1.top >= rc2.top && rc1.top <= rc2.bottom || rc1.bottom >= rc2.top && rc1.bottom <= rc2.bottom)) {
        SetLayeredWindowAttributes(hWnd3, 0, (255 * 50) / 100, LWA_ALPHA);
    }
    else
    {
        SetLayeredWindowAttributes(hWnd3, 0, 255, LWA_ALPHA);
    }
    UpdateWindow(hWnd3);
}