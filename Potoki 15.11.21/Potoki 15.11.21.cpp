// Potoki 15.11.21.cpp : Определяет точку входа для приложения.
//

#include "framework.h"
#include "Potoki 15.11.21.h"
#include<Windows.h>
#include<WindowsX.h>
#include <commctrl.h>
#include <commdlg.h>
#include <WinUser.h>
#include <string.h>
#include <psapi.h>
#include <strsafe.h>


#define MAX_LOADSTRING 100
#define IDC_LIST 2001
#define IDC_BUTTON1 2002
#define IDC_LIST2 2003 
#define IDC_BUTTON2 2004
#define IDC_BUTTON3 2005
#define IDC_BUTTON4 2006
#define IDC_LIST3 2007
#define IDC_TEXTBOX1 2008

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
HWND hList;
HWND hList2;
HWND hList3;
HWND mB;
HWND hEdit;
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна
HANDLE SelectedProcess;
int countproc = 0;

// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_POTOKI151121, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_POTOKI151121));

    MSG msg;

    // Цикл основного сообщения:
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
//  ФУНКЦИЯ: MyRegisterClass()
//
//  ЦЕЛЬ: Регистрирует класс окна.
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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_POTOKI151121));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_POTOKI151121);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}
void LoadProcessesToListBox(HWND hList) //Все открытые процессы
{
    ListBox_ResetContent(hList);

    DWORD aProcessIdc[1024], cbNeeded = 0;
    BOOL bRet = EnumProcesses(aProcessIdc, sizeof(aProcessIdc), &cbNeeded);

    if (FALSE != bRet)
    {
        TCHAR szName[MAX_PATH], szBuffer[300];

        for (DWORD i = 0, n = cbNeeded / sizeof(DWORD); i < n; ++i)
        {
            DWORD dwProcessId = aProcessIdc[i], cch = 0;
            if (0 == dwProcessId) continue;

            HANDLE hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, dwProcessId);

            if (NULL != hProcess)
            {
                cch = GetModuleBaseName(hProcess, NULL, szName, MAX_PATH);
                CloseHandle(hProcess);
            }
            if (0 == cch)
                StringCchCopy(szName, MAX_PATH, L"Незвестный процесс");
            StringCchPrintf(szBuffer, _countof(szBuffer), L"%s (PID: %u)", szName, dwProcessId);

            int iItem = ListBox_AddString(hList, szBuffer);

            ListBox_SetItemData(hList, iItem, dwProcessId);
            SelectedProcess = hProcess;
        }
    }
}
void LoadModulesToListBox(HWND hList, DWORD dwProcessId) //Модули процесса
{
    ListBox_ResetContent(hList);

    HANDLE hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, dwProcessId);

    if (NULL != hProcess)
    {
        DWORD cb = 0;
        EnumProcessModulesEx(hProcess, NULL, 0, &cb, LIST_MODULES_ALL);

        DWORD nCount = cb / sizeof(HMODULE);

        HMODULE* hModule = new HMODULE[nCount];

        cb = nCount * sizeof(HMODULE);

        BOOL bRet = EnumProcessModulesEx(hProcess, hModule, cb, &cb, LIST_MODULES_ALL);

        if (FALSE != bRet)
        {
            TCHAR szFileName[MAX_PATH];

            for (DWORD i = 0; i < nCount; ++i)
            {
                bRet = GetModuleFileNameEx(hProcess, hModule[i], szFileName, MAX_PATH);

                if (FALSE != bRet) ListBox_AddString(hList, szFileName);
            }
        }
        delete[]hModule;

        CloseHandle(hProcess);

    }
}

bool CrtProcess(LPCTSTR cmdLine, bool assignToJob, HANDLE hJob) 
{
    DWORD aProcessIdc[1024];
    DWORD dwProcessId = countproc;
    TCHAR szBuffer[360];


    BOOL inJob = FALSE;
    IsProcessInJob(GetCurrentProcess(), NULL, &inJob);
    TCHAR cmdstr[MAX_PATH];
    StringCchCopy(cmdstr, MAX_PATH, cmdLine);
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi;
    BOOL res = CreateProcess(NULL, cmdstr, NULL, NULL, FALSE, CREATE_SUSPENDED | CREATE_BREAKAWAY_FROM_JOB, NULL, NULL, &si, &pi);
    if (res != NULL) {
        AssignProcessToJobObject(hJob, pi.hProcess);
        ResumeThread(pi.hThread);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
    int iItem = ListBox_AddString(hList3, szBuffer);
    countproc++;
    return true;
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }
    
   CreateWindowEx(0,
       TEXT("BUTTON"), TEXT("Ждать закрытие процесса"),
       BS_CENTER | WS_CHILD | WS_VISIBLE,
       5, 440, 200, 25,
       hWnd, (HMENU)IDC_BUTTON1, hInstance, NULL);

   CreateWindowEx(0,
       TEXT("BUTTON"), TEXT("Закрыть процесс"),
       BS_CENTER | WS_CHILD | WS_VISIBLE,
       215, 440, 200, 25,
       hWnd, (HMENU)IDC_BUTTON2, hInstance, NULL);

   CreateWindowEx(0,
       TEXT("BUTTON"), TEXT("Обновить лист"),
       BS_CENTER | WS_CHILD | WS_VISIBLE,
       425, 440, 200, 25,
       hWnd, (HMENU)IDC_BUTTON3, hInstance, NULL);

   CreateWindowEx(0,
       TEXT("BUTTON"), TEXT("Создать процесс"),
       BS_CENTER | WS_CHILD | WS_VISIBLE,
       840,30, 150, 25,
       hWnd, (HMENU)IDC_BUTTON4, hInstance, NULL);

       hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"Edit", L"Название процесса:",
       WS_BORDER | WS_CHILD | WS_VISIBLE
       | NULL | NULL, 840, 5, 150, 20,
       hWnd, (HMENU)IDC_TEXTBOX1, hInst, 0);

    CreateWindowEx(0, TEXT("Static"), TEXT("Список процессов"), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER 
        , 5, 5, 260, 24, hWnd, 0, 0, 0);

    CreateWindowEx(0, TEXT("Static"), TEXT("Список модулей процесса"), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER
        , 285, 5, 260, 24, hWnd, 0, 0, 0);

    CreateWindowEx(0, TEXT("Static"), TEXT("Процессы в задании"), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER
        , 565, 5, 260, 24, hWnd, 0, 0, 0);

   hList = CreateWindowEx(WS_EX_CLIENTEDGE, L"listbox", L"", WS_CHILD
       | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL
       | LBS_STANDARD, 5, 25, 260, 420, hWnd,
       (HMENU)IDC_LIST, 0, 0);
   LoadProcessesToListBox(hList);

   hList2 = CreateWindowEx(WS_EX_CLIENTEDGE, L"listbox", L"", WS_CHILD
       | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL
       | LBS_STANDARD, 285, 25, 260, 420, hWnd,
       (HMENU)IDC_LIST2, 0, 0);

   hList3 = CreateWindowEx(WS_EX_CLIENTEDGE, L"listbox", L"", WS_CHILD
       | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL
       | LBS_STANDARD, 565, 25, 260, 420, hWnd,
       (HMENU)IDC_LIST3, 0, 0);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
        if (wParam == IDC_BUTTON1) //Ждать закрытие процесса
        {
            DWORD cursel2 = SendMessage(hList, LB_GETCURSEL, 0, 0L);
            DWORD ID = ListBox_GetItemData(hList, cursel2);

            HANDLE proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ID);
            WaitForSingleObject(proc, INFINITE);
            LoadProcessesToListBox(hList);
        }
        if (wParam == IDC_BUTTON2) //Закрытие процесса
        {
            DWORD cursel3 = SendMessage(hList, LB_GETCURSEL, 0, 0L);
            DWORD ID2 = ListBox_GetItemData(hList, cursel3);

            HANDLE proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ID2);
            TerminateProcess(proc,0);
            LoadProcessesToListBox(hList);
        }
        if (wParam == IDC_BUTTON3) LoadProcessesToListBox(hList); //Обновление листа

        if (wParam == IDC_BUTTON4)
        {
            wchar_t str[80];
            SendMessage(hEdit, WM_GETTEXT, 80, LPARAM(str));

            STARTUPINFO si = { sizeof(si) };
            PROCESS_INFORMATION pi;
            CreateProcess(NULL, str, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

            HANDLE hJob = CreateJobObject(NULL, L"Job");
            CrtProcess(str, TRUE, hJob);



            LoadProcessesToListBox(hList);
        }
        switch (LOWORD(wParam))
        {
        case IDC_LIST:
            if (HIWORD(wParam) == LBN_DBLCLK)
            {
                DWORD cursel = SendMessage(hList, LB_GETCURSEL, 0, 0L);
                LoadModulesToListBox(hList2, ListBox_GetItemData(hList, cursel));
            }
        }

            int wmId = LOWORD(wParam);
            // Разобрать выбор в меню:
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
            // TODO: Добавьте сюда любой код прорисовки, использующий HDC...
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

// Обработчик сообщений для окна "О программе".
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
