// 2048.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "dwmapi.h"
#include "2048.h"
#include <string>
#include <fstream>
#include <cstdio>

#define MAX_LOADSTRING 100

using namespace std;

const int numOfTilesX = 4;
const int numOfTilesY = 4;
const int tileSize = 70;
const int gapSize = 5;

struct TILEDATA {
    HWND window;
    COLORREF color;
    int number;
    int size;
}typedef TILEDATA;

struct WINDOWDATA {
    TILEDATA tiles[numOfTilesY][numOfTilesX];
    TILEDATA scoreTile;
    HWND window;
}typedef WINDOWDATA;

struct GAMEDATA {
    WINDOWDATA mainWindow;
    WINDOWDATA mirroredWindow;
    int windowHeight;
    int windowWidth;
    int windowClientAreaHeight;
    int windowClientAreaWidth;
    int screenHeight;
    int screenWidth;
    int endValue;
    bool started;
}typedef GAMEDATA;

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

GAMEDATA game;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

LRESULT             CALLBACK WndChldProc(HWND, UINT, WPARAM, LPARAM);
void                InitMainWindow(WINDOWDATA*);
ATOM                MyRegisterTileClass(HINSTANCE);
void                moveAndTransparency(HWND, HWND, HWND);
TILEDATA*           getTileData(HWND);
void                resetTiles();
void                modifyTile(TILEDATA*, int, int);
void                uncheckAllCheckOne(int);
void                spawnTwo();
bool                moveTilesUp();
bool                moveTilesLeft();
bool                moveTilesDown();
bool                moveTilesRight();
void                pushTile(int, int, int, int);
void                mergeTile(int, int, int, int);
void                checkForVictory(int, int);
void                saveGameState();
void                loadGameState();
void                spawnAnim(TILEDATA*);
void                mergeAnim(TILEDATA*);
void                dispEndMess(bool win);
void                checkForLoss();

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

    saveGameState();

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
    wcex.hbrBackground = CreateSolidBrush(RGB(250, 247, 238));
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

   game.mainWindow.window = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME | WS_CLIPCHILDREN,
       0, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!game.mainWindow.window)
   {
       return FALSE;
   }

   game.mirroredWindow.window = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION,
       700, 200, CW_USEDEFAULT, 0, game.mainWindow.window, nullptr, hInstance, nullptr);

   if (!game.mirroredWindow.window)
   {
       return FALSE;
   }

   game.windowClientAreaHeight = (numOfTilesY + 1) * tileSize + 2 * gapSize;
   game.windowClientAreaWidth = numOfTilesX * tileSize + 2 * gapSize;
   game.started = false;

   RECT rc;
   SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
   game.screenHeight = rc.bottom - rc.top;
   game.screenWidth = rc.right - rc.left;

   SetWindowLong(game.mirroredWindow.window, GWL_EXSTYLE, GetWindowLong(game.mirroredWindow.window, GWL_EXSTYLE) | WS_EX_LAYERED);
   InitMainWindow(&game.mirroredWindow);
   ShowWindow(game.mirroredWindow.window, nCmdShow);
   

   SetForegroundWindow(game.mainWindow.window);
   InitMainWindow(&game.mainWindow);
   ShowWindow(game.mainWindow.window, nCmdShow);
   
   
   loadGameState();
   UpdateWindow(game.mirroredWindow.window);
   UpdateWindow(game.mainWindow.window);

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
        if (hWnd == game.mainWindow.window)
            moveAndTransparency(game.mainWindow.window, game.mirroredWindow.window, game.mirroredWindow.window);
        else
            moveAndTransparency(game.mirroredWindow.window, game.mainWindow.window, game.mirroredWindow.window);
    }
    break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case ID_GAME_NEWGAME:
            {
                resetTiles();
                spawnTwo();
                game.started = true;
            }
                break;
            case ID_GOAL_8:
                uncheckAllCheckOne(8);
                break;
            case ID_GOAL_16:
                uncheckAllCheckOne(16);
                break;
            case ID_GOAL_64:
                uncheckAllCheckOne(64);
                break;
            case ID_GOAL_2048:
                uncheckAllCheckOne(2048);
                break;
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
    case WM_CHAR:
    {
        if (game.started) {
            switch ((char)wParam)
            {
            case 'w':
            {
                if (moveTilesUp()) spawnTwo();
                else checkForLoss();
            }
            break;
            case 'a':
            {
                if (moveTilesLeft()) spawnTwo();
                else checkForLoss();
            }
            break;
            case 's':
            {
                if (moveTilesDown()) spawnTwo();
                else checkForLoss();
            }
            break;
            case 'd':
            {
                if (moveTilesRight()) spawnTwo();
                else checkForLoss();
            }
            break;
            default:
                break;
            }
        }
    }
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
    case WM_PAINT:
    {
        RECT rc;
        GetClientRect(hWnd, &rc);
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        TILEDATA* tile = getTileData(hWnd);
        HPEN pen = CreatePen(PS_SOLID, 2, tile->color);
        HPEN oldPen = (HPEN)SelectObject(hdc, pen);
        HBRUSH brush = CreateSolidBrush(tile->color);
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
        HFONT font = CreateFont(20, 10, 0, 0, FW_BOLD, false, FALSE, 0, EASTEUROPE_CHARSET, 
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Verdana"));
        HFONT oldFont = (HFONT)SelectObject(hdc, font);
        RoundRect(hdc, rc.left + gapSize + tile->size, rc.top + gapSize + tile->size, rc.right - gapSize - tile->size, rc.bottom - gapSize - tile->size, tileSize / 4, tileSize / 4);
        SetTextColor(hdc, RGB(255, 255, 255));
        SetBkMode(hdc, TRANSPARENT);
        if (tile->number >= 0) {
            string str = to_string(tile->number);
            TCHAR* s = new TCHAR[str.size() + 1];
            s[str.size()] = 0;
            copy(str.begin(), str.end(), s);
            DrawText(hdc, s, (int)_tcslen(s), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
        else {
            DrawText(hdc, _T(""), 0, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
        SelectObject(hdc, oldPen);
        DeleteObject(pen);
        SelectObject(hdc, oldBrush);
        DeleteObject(brush);
        SelectObject(hdc, oldFont);
        DeleteObject(font);
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_TIMER:
    {
        TILEDATA* tile = getTileData(hWnd);
        if (wParam == 1) {
            if (tile->size > 0) {
                tile->size -= 3;
                InvalidateRect(hWnd, NULL, TRUE);
            }
            else {
                KillTimer(hWnd, 1);
            }
        }
        else if (wParam == 2) {
            if (tile->size < 0 && tile->size > -5) {
                tile->size--;
                InvalidateRect(hWnd, NULL, TRUE);
            }
            else {
                KillTimer(hWnd, 2);
                SetTimer(hWnd, 3, 1, NULL);
            }
        }
        else if (wParam == 3) {
            if (tile->size < 0) {
                tile->size++;
                InvalidateRect(hWnd, NULL, TRUE);
            }
            else {
                KillTimer(hWnd, 3);
            }
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

void InitMainWindow(WINDOWDATA *wData) {
    RECT rc;
    GetWindowRect(wData->window, &rc);
    rc.right = rc.left + game.windowClientAreaWidth;
    rc.bottom = rc.top + game.windowClientAreaHeight;
    AdjustWindowRectEx(&rc, (DWORD)GetWindowLong(wData->window, GWL_STYLE), GetMenu(wData->window) != NULL,
        (DWORD)GetWindowLong(wData->window, GWL_EXSTYLE));
    game.windowHeight = rc.bottom - rc.top;
    game.windowWidth = rc.right - rc.left;
    MoveWindow(wData->window, rc.left, rc.top, game.windowWidth, game.windowHeight, TRUE);

    SetWindowText(wData->window, L"2048");
    for (int i = 0; i < numOfTilesY; ++i) {
        for (int j = 0; j < numOfTilesX; ++j) {
            wData->tiles[i][j].window = CreateWindowW(L"TILE", L"TILE", WS_CHILD | WS_VISIBLE,
                j * tileSize + gapSize, (i + 1) * tileSize + gapSize,
                tileSize, tileSize, wData->window, nullptr, hInst, nullptr);
            wData->tiles[i][j].size = 0;
        }
    }
    wData->scoreTile.window = CreateWindow(L"TILE", L"TILE", WS_CHILD | WS_VISIBLE, gapSize, gapSize,
        numOfTilesX * tileSize, tileSize, wData->window, nullptr, hInst, nullptr);
    wData->scoreTile.size = 0;
    resetTiles();
}

void moveAndTransparency(HWND hWnd1, HWND hWnd2, HWND hWnd3) {
    RECT rc1, rc2;
    GetWindowRect(hWnd1, &rc1);
    GetWindowRect(hWnd2, &rc2);

    rc2.left = game.screenWidth - rc1.left - game.windowWidth;
    rc2.right = game.screenWidth - rc1.left;
    rc2.top = game.screenHeight - rc1.top - game.windowHeight;
    rc2.bottom = game.screenHeight - rc1.top;

    MoveWindow(hWnd2, rc2.left, rc2.top, game.windowWidth, game.windowHeight, TRUE);

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

TILEDATA* getTileData(HWND hWnd) {
    for (int i = 0; i < numOfTilesY; ++i) {
        for (int j = 0; j < numOfTilesX; ++j) {
            if (hWnd == game.mainWindow.tiles[i][j].window) return &game.mainWindow.tiles[i][j];
            if (hWnd == game.mirroredWindow.tiles[i][j].window) return &game.mirroredWindow.tiles[i][j];
        }
    }
    if (hWnd == game.mainWindow.scoreTile.window) return &game.mainWindow.scoreTile;
    return &game.mainWindow.scoreTile;
}

void resetTiles() {
    for (int i = 0; i < numOfTilesY; ++i) {
        for (int j = 0; j < numOfTilesX; ++j) {
            game.mainWindow.tiles[i][j].number = -1;
            game.mainWindow.tiles[i][j].color = RGB(204, 192, 174);
            InvalidateRect(game.mainWindow.tiles[i][j].window, NULL, TRUE);
            game.mirroredWindow.tiles[i][j].number = -1;
            game.mirroredWindow.tiles[i][j].color = RGB(204, 192, 174);
            InvalidateRect(game.mirroredWindow.tiles[i][j].window, NULL, TRUE);
        }
    }
    game.mainWindow.scoreTile.number = 0;
    game.mainWindow.scoreTile.color = RGB(204, 192, 174);
    InvalidateRect(game.mainWindow.scoreTile.window, NULL, TRUE);
    game.mirroredWindow.scoreTile.number = 0;
    game.mirroredWindow.scoreTile.color = RGB(204, 192, 174);
    InvalidateRect(game.mirroredWindow.scoreTile.window, NULL, TRUE);
}

void modifyTile(TILEDATA *tile, int number, int anim) {
    tile->number = number;
    switch (number)
    {
    case 0:
        tile->number--;
        tile->color = RGB(204, 192, 174);
        break;
    case 2:
        tile->color = RGB(238, 228, 198);
        break;
    case 4:
        tile->color = RGB(239, 225, 218);
        break;
    case 8:
        tile->color = RGB(243, 179, 124);
        break;
    case 16:
        tile->color = RGB(246, 153, 100);
        break;
    case 32:
        tile->color = RGB(246, 125, 98);
        break;
    case 64:
        tile->color = RGB(247, 93, 60);
        break;
    case 128:
        tile->color = RGB(237, 206, 116);
        break;
    case 256:
        tile->color = RGB(239, 204, 98);
        break;
    case 512:
        tile->color = RGB(243, 201, 85);
        break;
    case 1024:
        tile->color = RGB(238, 200, 72);
        break;
    case 2048:
        tile->color = RGB(239, 192, 47);
        break;
    default:
        break;
    }
    if (anim == 1) {
        spawnAnim(tile);
        return;
    }
    if (anim == 2) mergeAnim(tile);
    InvalidateRect(tile->window, NULL, TRUE);
}

void uncheckAllCheckOne(int endVal) {
    HMENU mn = GetMenu(game.mainWindow.window);
    CheckMenuItem(mn, ID_GOAL_8, MF_UNCHECKED);
    CheckMenuItem(mn, ID_GOAL_16, MF_UNCHECKED);
    CheckMenuItem(mn, ID_GOAL_64, MF_UNCHECKED);
    CheckMenuItem(mn, ID_GOAL_2048, MF_UNCHECKED);
    switch (endVal)
    {
    case 8:
        CheckMenuItem(mn, ID_GOAL_8, MF_CHECKED);
        break;
    case 16:
        CheckMenuItem(mn, ID_GOAL_16, MF_CHECKED);
        break;
    case 64:
        CheckMenuItem(mn, ID_GOAL_64, MF_CHECKED);
        break;
    case 2048:
        CheckMenuItem(mn, ID_GOAL_2048, MF_CHECKED);
        break;
    default:
        break;
    }
    game.endValue = endVal;
}

void spawnTwo() {
    int x = rand() % numOfTilesX, y = rand() % numOfTilesY;
    for (int i = 0; i < numOfTilesX; ++i) {
        for (int j = 0; j < numOfTilesY; ++j) {
            if (game.mainWindow.tiles[(y + j) % numOfTilesY][(x + i) % numOfTilesX].number == -1) {
                modifyTile(&game.mainWindow.tiles[(y + j) % numOfTilesY][(x + i) % numOfTilesX], 2, 1);
                modifyTile(&game.mirroredWindow.tiles[(y + j) % numOfTilesY][(x + i) % numOfTilesX], 2, 1);
                return;
            }
        }
    }
    game.started = false;
    dispEndMess(false);
}

bool moveTilesUp() {
    bool moved = false;
    for (int j = 0; j < numOfTilesY; ++j) {
        bool alreadyMerged[numOfTilesX] = { false };
        for (int i = numOfTilesX - 1; i > 0; --i) {
            if (game.mainWindow.tiles[i][j].number > 0) {
                if (game.mainWindow.tiles[i - 1][j].number == -1) {
                    pushTile(j, i, j, i - 1);
                    alreadyMerged[i - 1] = alreadyMerged[i];
                }
                else if (!alreadyMerged[i] && game.mainWindow.tiles[i - 1][j].number == game.mainWindow.tiles[i][j].number) {
                    mergeTile(j, i, j, i - 1);
                    alreadyMerged[i - 1] = true;
                    checkForVictory(j, i - 1);
                }
                else {
                    continue;
                }
                for (int k = i + 1; k < numOfTilesX; ++k) {
                    pushTile(j, k, j, k - 1);
                    alreadyMerged[k - 1] = alreadyMerged[k];
                }
                moved = true;
            }
        }
    }
    if (!game.started) dispEndMess(true);
    return moved;
}

bool moveTilesLeft() {
    bool moved = false;
    for (int i = 0; i < numOfTilesX; ++i) {
        bool alreadyMerged[numOfTilesY] = { false };
        for (int j = numOfTilesY - 1; j > 0; --j) {
            if (game.mainWindow.tiles[i][j].number > 0) {
                if (game.mainWindow.tiles[i][j - 1].number == -1) {
                    pushTile(j, i, j - 1, i);
                    alreadyMerged[j - 1] = alreadyMerged[j];
                }
                else if (!alreadyMerged[j] && game.mainWindow.tiles[i][j - 1].number == game.mainWindow.tiles[i][j].number) {
                    mergeTile(j, i, j - 1, i);
                    alreadyMerged[j - 1] = true;
                    checkForVictory(j - 1, i);
                }
                else {
                    continue;
                }
                for (int k = j + 1; k < numOfTilesY; ++k) {
                    pushTile(k, i, k - 1, i);
                    alreadyMerged[k - 1] = alreadyMerged[k];
                }
                moved = true;
            }
        }
    }
    if (!game.started) dispEndMess(true);
    return moved;
}

bool moveTilesDown() {
    bool moved = false;
    for (int j = 0; j < numOfTilesY; ++j) {
        bool alreadyMerged[numOfTilesX] = { false };
        for (int i = 0; i < numOfTilesX - 1; ++i) {
            if (game.mainWindow.tiles[i][j].number > 0) {
                if (game.mainWindow.tiles[i + 1][j].number == -1) {
                    pushTile(j, i, j, i + 1);
                    alreadyMerged[i + 1] = alreadyMerged[i];
                }
                else if (!alreadyMerged[i] && game.mainWindow.tiles[i + 1][j].number == game.mainWindow.tiles[i][j].number) {
                    mergeTile(j, i, j, i + 1);
                    alreadyMerged[i + 1] = true;
                    checkForVictory(j, i + 1);
                }
                else {
                    continue;
                }
                for (int k = i - 1; k >= 0; --k) {
                    pushTile(j, k, j, k + 1);
                    alreadyMerged[k + 1] = alreadyMerged[k];
                } moved = true;
            }
        }
    }
    if (!game.started) dispEndMess(true);
    return moved;
}

bool moveTilesRight() {
    bool moved = false;
    for (int i = 0; i < numOfTilesX; ++i) {
        bool alreadyMerged[numOfTilesY] = { false };
        for (int j = 0; j < numOfTilesY - 1; ++j) {
            if (game.mainWindow.tiles[i][j].number > 0) {
                if (game.mainWindow.tiles[i][j + 1].number == -1) {
                    pushTile(j, i, j + 1, i);
                    alreadyMerged[j + 1] = alreadyMerged[j];
                }
                else if (!alreadyMerged[j] && game.mainWindow.tiles[i][j + 1].number == game.mainWindow.tiles[i][j].number) {
                    mergeTile(j, i, j + 1, i);
                    alreadyMerged[j + 1] = true;
                    checkForVictory(j + 1, i);
                }
                else {
                    continue;
                }
                for (int k = j - 1; k >= 0; --k) {
                    pushTile(k, i, k + 1, i);
                    alreadyMerged[k + 1] = alreadyMerged[k];
                }
                moved = true;
            }
        }
    }
    if (!game.started) dispEndMess(true);
    return moved;
}

void pushTile(int fromX, int fromY, int toX, int toY) {
    modifyTile(&game.mainWindow.tiles[toY][toX], game.mainWindow.tiles[fromY][fromX].number, 0);
    modifyTile(&game.mainWindow.tiles[fromY][fromX], 0, 0);
    modifyTile(&game.mirroredWindow.tiles[toY][toX], game.mirroredWindow.tiles[fromY][fromX].number, 0);
    modifyTile(&game.mirroredWindow.tiles[fromY][fromX], 0, 0);
}

void mergeTile(int fromX, int fromY, int toX, int toY) {
    int sum = game.mainWindow.tiles[toY][toX].number + game.mainWindow.tiles[fromY][fromX].number;
    modifyTile(&game.mainWindow.tiles[toY][toX], sum, 2);
    modifyTile(&game.mainWindow.tiles[fromY][fromX], 0, 0);
    game.mainWindow.scoreTile.number += sum;
    InvalidateRect(game.mainWindow.scoreTile.window, NULL, TRUE);
    modifyTile(&game.mirroredWindow.tiles[toY][toX], sum, 2);
    modifyTile(&game.mirroredWindow.tiles[fromY][fromX], 0, 0);
    game.mirroredWindow.scoreTile.number += sum;
    InvalidateRect(game.mirroredWindow.scoreTile.window, NULL, TRUE);
}

void checkForVictory(int x, int y) {
    if (game.mainWindow.tiles[y][x].number >= game.endValue) {
        game.started = false;
    }
}

void saveGameState() {
    ofstream file;
    file.open("2048.ini");
    file << "[GAME]" << endl;
    file << "STATUS=";
    if (game.started)
        file << 1 << endl;
    else
        file << 1 << endl;
    file << "SCORE=" << game.mainWindow.scoreTile.number << endl;
    file << "GOAL=" << game.endValue << endl;
    file << "BOARD=";
    for (int i = 0; i < numOfTilesY; ++i) {
        for (int j = 0; j < numOfTilesX; ++j) {
            if (game.mainWindow.tiles[i][j].number > 0)
                file << game.mainWindow.tiles[i][j].number << ";";
            else
                file << "0;";
        }
    }
    file.close();
}

void loadGameState() {
    ifstream file;
    file.open("2048.ini");
    if (file.good()) {
        string s;
        int n;
        // header
        file >> s;
        // game start indicator
        file >> s;
        game.started = (bool)stoi(s.substr(s.find("=") + 1, s.length() - 1));
        // score
        file >> s;
        n = stoi(s.substr(s.find("=") + 1, s.length() - 1));
        game.mainWindow.scoreTile.number = n;
        InvalidateRect(game.mainWindow.scoreTile.window, NULL, TRUE);
        game.mirroredWindow.scoreTile.number = n;
        InvalidateRect(game.mirroredWindow.scoreTile.window, NULL, TRUE);
        // goal
        file >> s;
        n = stoi(s.substr(s.find("=") + 1, s.length() - 1));
        uncheckAllCheckOne(n);
        // board values
        file >> s;
        s = s.substr(s.find("=") + 1, s.length() - 1);
        int end;
        for (int i = 0; i < numOfTilesY; ++i) {
            for (int j = 0; j < numOfTilesX; ++j) {
                end = s.find(";");
                n = stoi(s.substr(0, end));
                modifyTile(&game.mainWindow.tiles[i][j], n, 0);
                modifyTile(&game.mirroredWindow.tiles[i][j], n, 0);
                s.erase(0, end + 1);
            }
        }
    }
    else {
        resetTiles();
        uncheckAllCheckOne(2048);
    }
    DrawMenuBar(game.mainWindow.window);
    DrawMenuBar(game.mirroredWindow.window);

    file.close();
}

void spawnAnim(TILEDATA *tile) {
    tile->size = 30;
    SetTimer(tile->window, 1, 1, NULL);
}

void mergeAnim(TILEDATA *tile) {
    tile->size = -1;
    SetTimer(tile->window, 2, 1, NULL);
}

void dispEndMess(bool win) {
    TCHAR buf[15];
    _stprintf_s(buf, 15, _T("Score: %d"), game.mainWindow.scoreTile.number);
    if (win) {
        MessageBox(NULL, buf, L"You win!", MB_OK);
    }
    else {
        MessageBox(NULL, buf, L"Game over!", MB_OK);
    }
}

void checkForLoss() {
    for (int i = 0; i < numOfTilesY; ++i) {
        for (int j = 0; j < numOfTilesX; ++j) {
            if (game.mainWindow.tiles[i][j].number == -1) return;
        }
    }
    game.started = false;
    dispEndMess(false);
}