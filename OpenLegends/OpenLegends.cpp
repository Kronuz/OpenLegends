// OpenLegends.cpp: define el punto de entrada de la aplicación.
//

#include "stdafx.h"
#include "OpenLegends.h"

#include "ProjectFactory.h"
#include "GraphicsFactory.h"
#include "Shellapi.h"
//#region Declarations 

#define MAX_LOADSTRING 100

// Variables globales:
DWORD g_dwLastTick;
float g_rDelta;
IGraphics *g_pGraphicsI = NULL;
CMapGroup *g_pMapGroupI = NULL;

int g_nXScreenSize = 640;
int g_nYScreenSize = 480;

char g_szHomeDir[MAX_PATH] = "";
HWND g_hWnd = NULL;
HANDLE g_hRunScripts = NULL; // Thread handle
HANDLE g_hSemaphore = NULL; // Semaphore handle
bool g_bRunningScripts = false;
bool g_bClearToGo = true;
bool g_bDebug = false;
bool g_bStop = false;
static bool g_bFullScreen = false;

HINSTANCE hInst;								// Instancia actual
TCHAR szTitle[MAX_LOADSTRING];					// Texto de la barra de título
TCHAR *szTitleInfo;
TCHAR szWindowClass[MAX_LOADSTRING];			// nombre de clase de la ventana principal

// Declaraciones de funciones adelantadas incluidas en este módulo de código:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

HRESULT				Run();
HRESULT             LoadGame(LPCSTR szQuest);
void                Render();

void				OnSize();
float OnSizing(int nType, CRect *prcWindow);
//#endregion

//#region Window initialization and stuff 
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: colocar código aquí.
	MSG msg;
	HACCEL hAccelTable;

	// Inicializar cadenas globales
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_OPENZELDA, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	int argc;
	LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);

	// Debug version?
	int debugarg = 0;
	for(int i=1; i<argc; i++) {
		if(!_wcsicmp(argv[i], L"-d") || !_wcsicmp(argv[i], L"/d")) {
			g_bDebug = true;
			strcat(szTitle, " - [Debugging]");
			debugarg = i;
			break;
		}
	}
	szTitleInfo = szTitle + strlen(szTitle);

	// Realizar la inicialización de la aplicación:
	if (!InitInstance (hInstance, nCmdShow)) {
		return FALSE;
	}

	char szQuestFile[MAX_PATH] = "";
	for(int i=1; i<argc; i++) {
		if(i != debugarg) {
			if(!*g_szHomeDir) {
				WideCharToMultiByte(CP_ACP, 0, argv[i], -1, g_szHomeDir, sizeof(g_szHomeDir), NULL, NULL);
			} else if(!*szQuestFile) {
				WideCharToMultiByte(CP_ACP, 0, argv[i], -1, szQuestFile, sizeof(szQuestFile), NULL, NULL);
			} else break;
		}
	}

	if(g_bDebug) {
		if(FAILED(LoadGame(szQuestFile))) PostMessage(g_hWnd, WM_CLOSE, 0, 0);
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_OPENZELDA);
	// Create a semaphore for painting (start ready to paint):
	g_hSemaphore = CreateSemaphore( NULL, 0, 1, NULL );

	// Main Message Loop:
	ZeroMemory(&msg, sizeof(MSG));
	while(msg.message != WM_QUIT) {
		// Look for messages, if none are found then 
		// update the state and display it
		BOOL bRet;
		if(!g_pGraphicsI || !g_pMapGroupI) bRet = GetMessage(&msg, NULL, 0, 0);
		else bRet = PeekMessage( &msg, NULL, 0, 0, PM_REMOVE );
		if(bRet) {
			// Translate and dispatch the message
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		} else {
			// Handle the next frame
			Run();
			if(g_bStop) PostMessage(g_hWnd, WM_CLOSE, 0, 0);
		}
	}

	CloseHandle(g_hSemaphore);
	return (int) msg.wParam;
}

//
//  FUNCIÓN: MyRegisterClass()
//
//  PROPÓSITO: registrar la clase de ventana.
//
//  COMENTARIOS:
//
//    Esta función y su uso son sólo necesarios si desea que el código
//    sea compatible con sistemas Win32 anteriores a la función 
//    'RegisterClassEx' que se agregó a Windows 95. Es importante llamar a esta función
//    para que la aplicación obtenga iconos pequeños bien formados
//    asociados a ella.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_OPENZELDA);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCTSTR)IDC_OPENZELDA;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_OPENZELDA);

	return RegisterClassEx(&wcex);
}

//
//   FUNCIÓN: InitInstance(HANDLE, int)
//
//   PROPÓSITO: guardar el identificador de instancia y crear la ventana principal
//
//   COMENTARIOS:
//
//        En esta función, se guarda el identificador de instancia en una variable común y
//        se crea y muestra la ventana principal del programa.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Almacenar identificador de instancia en una variable global
   if(g_bDebug) nCmdShow = SW_HIDE;
   g_hWnd = CreateWindowEx(g_bDebug?WS_EX_TOPMOST:0, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!g_hWnd) {
      return FALSE;
   }

   ShowWindow(g_hWnd, nCmdShow);
   UpdateWindow(g_hWnd);

   return TRUE;
}
// Controlador de mensajes del cuadro Acerca de.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

//#endregion

//#region Main message handler 
//
//  FUNCIÓN: WndProc(HWND, unsigned, WORD, LONG)
//
//  PROPÓSITO: procesar mensajes de la ventana principal.
//
//  WM_COMMAND	: procesar el menú de aplicación
//  WM_PAINT	: pintar la ventana principal
//  WM_DESTROY	: enviar un mensaje de finalización y volver
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int nScreenX, nScreenY;
	static CRect rcWindow, rcClip;
	static float fZoom;

	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message) 
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam); 
		wmEvent = HIWORD(wParam); 
		// Analizar las selecciones de menú:
		switch (wmId)
		{
		case ID_LOADQUEST:
			LoadGame(NULL);
			break;
		case IDM_ABOUT:
			DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
			break;
		case IDM_TOGGLEFULLSCREEN:
			if(g_pGraphicsI && g_pMapGroupI) {
				if(g_bFullScreen == true) {
					g_pGraphicsI->SetMode(g_hWnd);
					SetWindowPos(
						g_hWnd, 
						HWND_NOTOPMOST, 
						rcWindow.left, rcWindow.top, 
						rcWindow.Width(), rcWindow.Height(), 
						SWP_SHOWWINDOW );
					OnSize();
					ShowCursor(TRUE);
					g_bFullScreen = false;
				} else {
					GetWindowRect(g_hWnd, &rcWindow);
					g_pGraphicsI->SetMode(NULL, false, g_nXScreenSize, g_nYScreenSize);
					OnSize();
					ShowCursor(FALSE);
					g_bFullScreen = true;
				}
			}
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_ERASEBKGND:
		if(!g_pMapGroupI) 
			return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		Render();
		break;
	case WM_DESTROY:
		g_bDebug = false;
		ReleaseSemaphore(g_hSemaphore, 1, NULL);
		CProjectFactory::Interface(g_hWnd)->StopWaiting();
		if(g_hRunScripts) {
			if(WaitForSingleObject(g_hRunScripts, 1000) == WAIT_TIMEOUT) {
				MessageBeep((UINT)-1);
				TerminateThread(g_hRunScripts, 1);
			}
			ASSERT(WaitForSingleObject(g_hRunScripts, 2000) == WAIT_OBJECT_0);
			CloseHandle(g_hRunScripts);
			g_hRunScripts = NULL;
		}
		PostQuitMessage(0);
		break;
	case WM_SIZE:
		OnSize();
		break;
	case WM_SIZING: {
		CRect *pRect = (CRect*)lParam; 
		CRect rcWindow = *pRect;
		switch(wParam) {
			case WMSZ_BOTTOM:
				OnSizing(1, &rcWindow);
				pRect->bottom = pRect->top + rcWindow.Height();
				pRect->right = pRect->left + rcWindow.Width();
				break;
			case WMSZ_BOTTOMLEFT:
				OnSizing(3, &rcWindow);
				pRect->bottom = pRect->top + rcWindow.Height();
				pRect->left = pRect->right - rcWindow.Width();
				break;
			case WMSZ_BOTTOMRIGHT:
				OnSizing(3, &rcWindow);
				pRect->bottom = pRect->top + rcWindow.Height();
				pRect->right = pRect->left + rcWindow.Width();
				break;
			case WMSZ_LEFT:
				OnSizing(0, &rcWindow);
				pRect->bottom = pRect->top + rcWindow.Height();
				pRect->left = pRect->right - rcWindow.Width();
				break;
			case WMSZ_RIGHT:
				OnSizing(0, &rcWindow);
				pRect->bottom = pRect->top + rcWindow.Height();
				pRect->right = pRect->left + rcWindow.Width();
				break;
			case WMSZ_TOP:
				OnSizing(1, &rcWindow);
				pRect->top = pRect->bottom - rcWindow.Height();
				pRect->right = pRect->left + rcWindow.Width();
				break;
			case WMSZ_TOPLEFT:
				OnSizing(3, &rcWindow);
				pRect->top = pRect->bottom - rcWindow.Height();
				pRect->left = pRect->right - rcWindow.Width();
				break;
			case WMSZ_TOPRIGHT:
				OnSizing(3, &rcWindow);
				pRect->top = pRect->bottom - rcWindow.Height();
				pRect->right = pRect->left + rcWindow.Width();
				break;
		}
		return TRUE;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
//#endregion

//#region Game loading stuff 
float OnSizing(int nType, CRect *prcWindow)
{
	CRect rcClient, rcWindow;
	GetClientRect(g_hWnd, &rcClient);

	GetWindowRect(g_hWnd, &rcWindow);
	int Xdif = rcWindow.Width() - rcClient.Width();
	int Ydif = rcWindow.Height() - rcClient.Height();
	if(g_bFullScreen) Ydif = Xdif = 0;

	if(prcWindow) rcWindow = *prcWindow;

	float fXFactor = (float)(rcWindow.Width()-Xdif) / (float)g_nXScreenSize;
	float fYFactor = (float)(rcWindow.Height()-Ydif) / (float)g_nYScreenSize;

	if((rcWindow.Width()-Xdif) < g_nXScreenSize/2) 
		fXFactor = (float)(g_nXScreenSize/2) / (float)g_nXScreenSize;
	if((rcWindow.Height()-Ydif) < g_nYScreenSize/2) 
		fYFactor = (float)(g_nYScreenSize/2) / (float)g_nYScreenSize;

	float fZoom = 1.0f;
	if(g_bDebug && nType == -1) {
		fZoom = 0.5f;
		rcWindow.left =
			GetSystemMetrics(SM_CXSCREEN) - ((int)((float)g_nXScreenSize * fZoom + 0.5f) + Xdif) - 50;
		rcWindow.top = 50;
	}

	switch(nType) {
		case 0: fZoom = fXFactor; break;
		case 1: fZoom = fYFactor; break;
		case 2: fZoom = min(fXFactor, fYFactor); break;
		case 3: fZoom = max(fXFactor, fYFactor); break;
	}

	// Snap the resizing:
	int nGran = 4;
	float fSnap = 0.02f;
	for(int iq1=0; iq1<=nGran; iq1++) {
		for(int iq2=0; iq2<=nGran; iq2++) {
			float fTmp = (float)iq1 + ((float)iq2 / (float)nGran);
			if(fZoom > fTmp - fSnap && fZoom < fTmp + fSnap) {
				fZoom = fTmp;
				iq1 = iq2 = nGran + 1;
			}
		}
	}

	// Set the new zoom level:
	sprintf(szTitleInfo, "   (%d%%)", (int)(fZoom * 100.0f));
	SetWindowText(g_hWnd, szTitle);

	if(prcWindow) {
		rcWindow.right = rcWindow.left + (int)((float)g_nXScreenSize * fZoom + 0.5f) + Xdif;
		rcWindow.bottom = rcWindow.top + (int)((float)g_nYScreenSize * fZoom + 0.5f) + Ydif;

		if(prcWindow) *prcWindow = rcWindow;
	}
	return fZoom;
}

void OnSize()
{
	if(!g_pMapGroupI) return;

	float fZoom = OnSizing(3, NULL);

	CRect rcClip, rcWindow, rcVisible;
	if(g_bFullScreen) GetWindowRect(g_hWnd, &rcWindow);
	else GetClientRect(g_hWnd, &rcWindow);

	CPoint Point(0, 0);
	g_pGraphicsI->ViewToWorld(&Point);

	g_pGraphicsI->GetWorldRect(&rcClip);
	g_pGraphicsI->SetWindowView(g_hWnd, fZoom, &rcWindow, &rcClip);

	g_pGraphicsI->SetWorldPosition(&Point);
}

HRESULT LoadGame(LPCSTR szQuest)
{
	if(!szQuest) return E_FAIL;
	if(!*g_szHomeDir || !*szQuest) return E_FAIL;
	if(!g_pGraphicsI) {
		if(FAILED(CGraphicsFactory::New(&g_pGraphicsI, "GraphicsD3D9.dll"))) {
			MessageBox(g_hWnd, "Couldn't load graphics plugin, check plugin version.", "Open Legends", MB_OK);
			return E_FAIL;
		}

		g_pGraphicsI->Initialize(g_hWnd);
		CProjectFactory::Interface(g_hWnd)->Configure(&g_pGraphicsI, g_bDebug);
	}

	if(!CProjectFactory::Interface(g_hWnd)->Load(g_szHomeDir)) return E_FAIL;

	if(!CProjectFactory::Interface()->LoadWorld(szQuest)) return E_FAIL;
	if((g_pMapGroupI = CProjectFactory::Interface()->FindMapGroup(2, 2))==NULL) return E_FAIL;

	if(!g_pMapGroupI->Load()) return E_FAIL;

	CSize szMap;
	g_pMapGroupI->GetSize(szMap);

	// We need to recalculate the window's size and position:
	CRect rcWindow;
	GetWindowRect(g_hWnd, &rcWindow);
	float fZoom = OnSizing(-1, &rcWindow);

	CRect rcClip, rcClient;
	rcClient.SetRect(0, 0, (int)((float)g_nXScreenSize * fZoom), (int)((float)g_nYScreenSize * fZoom));
	rcClip.SetRect(0, 0, szMap.cx, szMap.cy);
	g_pGraphicsI->SetWindowView(g_hWnd, fZoom, &rcClient, &rcClip);

	MoveWindow(g_hWnd, rcWindow.left, rcWindow.top, rcWindow.Width(), rcWindow.Height(), TRUE);
	if(g_bDebug) ShowWindow(g_hWnd, SW_SHOW);

	return S_OK;
}
//#endregion

// Thread but also standard function for script debugging/non-debugging.
DWORD WINAPI RunScripts(LPVOID lpParameter)
{
	RUNACTION action;
	action.m_Ptr = lpParameter;
	action.bJustWait = false;
	IGame *pGameI = (IGame *)lpParameter;
	do {
		if(g_bDebug) {
			WaitForSingleObject(g_hSemaphore, INFINITE);
			if(!g_bDebug) return 0;
			g_bClearToGo = false;
			g_bRunningScripts = true;
			action.hSemaphore = g_hSemaphore;
		}

		// run the scripts for the next frame:
		g_pMapGroupI->Run(action);
		// wait for all the scripts to finish:
		g_bStop = pGameI->WaitScripts();

		if(g_bDebug) {
			g_bRunningScripts = false;
			ReleaseSemaphore(g_hSemaphore, 1, NULL);
		}
	} while(g_bDebug && !g_bStop);

	return 0;
}

void Render()
{
	if(g_bStop) return;
	if(!g_pGraphicsI || !g_pMapGroupI) return;

	static int nTimeLeft = 10000;
	static DWORD dwStarting = GetTickCount() + 10000;
	
	IGame *pGameI = CProjectFactory::Interface(g_hWnd);
	// Update timings and stuff for the animations
	float fps = pGameI->UpdateFPS(60);
	if(fps != -1.0f) {
		if(g_bDebug) ReleaseSemaphore(g_hSemaphore, 1, NULL);
		///////////////////////////////////////////////////////////////////////////
		// 1. RUN THE SCRIPTS
		if(!g_bDebug) RunScripts((LPVOID)pGameI);
		else if(!g_bRunningScripts && g_bClearToGo) {
			if(dwStarting) { // Give time to the debugger to start...
				nTimeLeft = dwStarting - GetTickCount();
				if(nTimeLeft <= 0) dwStarting = 0;
			} else {
				// Run the scripts in a separated thread (for debugging capabilities also):
				// check if the RunScripts thread is still active, if not create one
				DWORD dwAux = 0;
				if(g_hRunScripts) GetExitCodeThread(g_hRunScripts, &dwAux);
				if(dwAux != STILL_ACTIVE) {
					if(g_hRunScripts) CloseHandle(g_hRunScripts);
					g_hRunScripts = ::CreateThread(NULL, 0, RunScripts, (LPVOID)pGameI, 0, &dwAux);
				}
			}
		}

		///////////////////////////////////////////////////////////////////////////
		// 2. DRAW THE WORLD
		// wait until we are allowed to paint something (during debug or after running all the scripts):
		if(g_bDebug) WaitForSingleObject(g_hSemaphore, INFINITE);
		
		// **** This should be set by the user in the scripts:
		g_pGraphicsI->SetFilterBkColor(COLOR_RGB(0,0,0));

		// Select the background color of the map (the first map when wiping):
		g_pGraphicsI->SetClearColor(g_pMapGroupI->GetBkColor());

		// Begin Painting:
		if(g_pGraphicsI->BeginPaint()) {
			// Draw the first map:
			g_pMapGroupI->Draw(g_pGraphicsI);

			// flush the first map or filters to the back buffer:
			g_pGraphicsI->FlushFilters(true);

			//If wiping, set the filters for the second map, 
			//select the background color of the second map, and draw the second map:
			//g_pGraphicsI->SetFilter();
			//g_pGraphicsI->SetClearColor(g_pSecondMapGroupI->GetBkColor());
			//g_pSecondMapGroupI->Draw(g_pGraphicsI);

			bool bFilters = g_pGraphicsI->SetFilter(EnableFilters, (void*)false);
			// Draw information about the frame rate and other things:
			g_pGraphicsI->DrawText(CPoint(10,10), COLOR_ARGB(255,255,255,255), "%4.1f fps", fps);
			if(g_bDebug) {
				if(g_bRunningScripts) {
					g_pGraphicsI->DrawText(CPoint(10,25), COLOR_ARGB(255,255,225,128), "Debugging...");
				} else if(dwStarting) {
					if(!pGameI->isDebugging()) dwStarting = 0;
					g_pGraphicsI->DrawText(CPoint(10,25), COLOR_ARGB(255,255,225,128), "Start the debugger now! (%d seconds left)", nTimeLeft/1000);
				}
			}
			g_pGraphicsI->SetFilter(EnableFilters, (void*)bFilters);

			// End painting:
			g_pGraphicsI->EndPaint();
		}

		///////////////////////////////////////////////////////////////////////////
		// 3. CLEAN STUFF
		// check if a compete frame has been drawn (without debugging in the middle):
		if(!g_bRunningScripts) {
			g_bClearToGo = true;

			// if so, delete temporary stuff, such as primitives, temporary sprites, 
			// and marked-as-deleted sprites.
			g_pMapGroupI->CleanTemp();
		}
	}
}

//-----------------------------------------------------------------------------
// Name: Run()
// Desc: Called whenever the program is idle
//
//-----------------------------------------------------------------------------
HRESULT Run()
{
	Render();
	return S_OK;
}
