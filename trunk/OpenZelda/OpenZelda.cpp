// OpenZelda.cpp: define el punto de entrada de la aplicación.
//

#include "stdafx.h"
#include "OpenZelda.h"

#include "ProjectFactory.h"
#include "GraphicsFactory.h"

//#region Declarations 

#define MAX_LOADSTRING 100

// Variables globales:
DWORD g_dwLastTick;
float g_rDelta;
IGraphics *g_pGraphicsI;
CMapGroup *g_pMapGroupI;

CBString g_sHomeDir;
HWND g_hWnd;

HINSTANCE hInst;								// Instancia actual
TCHAR szTitle[MAX_LOADSTRING];					// Texto de la barra de título
TCHAR szWindowClass[MAX_LOADSTRING];			// nombre de clase de la ventana principal

// Declaraciones de funciones adelantadas incluidas en este módulo de código:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

HRESULT				ProcessNextFrame();
HRESULT             LoadGame();
void                Render();
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

	// Realizar la inicialización de la aplicación:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_OPENZELDA);

	// Main Message Loop:
	ZeroMemory(&msg, sizeof(MSG));
	while(msg.message != WM_QUIT) {
		// Look for messages, if none are found then 
		// update the state and display it
		if(PeekMessage( &msg, NULL, 0, 0, PM_REMOVE )) {
			// Translate and dispatch the message
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		} else {
			// Handle the next frame
			ProcessNextFrame();
		}
	}

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

   g_hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!g_hWnd)
   {
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
	static bool bFullScreen = false;

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
			LoadGame();
			break;
		case IDM_ABOUT:
			DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_KEYDOWN:
		if(g_pGraphicsI) {
			if(bFullScreen == true) {
				//g_pGraphicsI->SetMode(g_hWnd);
				//bFullScreen = false;
			} else {
				g_pGraphicsI->SetMode(g_hWnd, false, 640, 480);
				bFullScreen = true;
			}
		}
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
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
//#endregion

HRESULT LoadGame()
{
	if(!g_pGraphicsI) {
		if(FAILED(CGraphicsFactory::New(&g_pGraphicsI, "GraphicsD3D8.dll"))) {
			MessageBox(g_hWnd, "Couldn't load graphics plugin, check plugin version.", "Open Zelda", MB_OK);
			return E_FAIL;
		}

		g_pGraphicsI->Initialize(g_hWnd);
	}

	g_sHomeDir = "C:\\qd\\Quest Designer 2.1.4\\";
	if(!CProjectFactory::Interface(g_hWnd)->Load(g_sHomeDir)) return E_FAIL;

	if(!CProjectFactory::Interface()->LoadWorld("C:\\qd\\Quest Designer 2.1.4\\save\\kakariko.qss")) return E_FAIL;
	if((g_pMapGroupI = CProjectFactory::Interface()->FindMapGroup(2, 2))==NULL) return E_FAIL;

	CSize szMap;
	if(!g_pMapGroupI->Load()) return E_FAIL;;
	g_pMapGroupI->GetSize(szMap);

	// We need to recalculate the window's size and position:
	CRect rcClient, rcClip;
	GetClientRect(g_hWnd, &rcClient);

	rcClip.SetRect(0, 0, szMap.cx, szMap.cy);
	g_pGraphicsI->SetWindowView(g_hWnd, rcClient, rcClip);

	return S_OK;
}

void Render()
{
	if(!g_pMapGroupI) return;

	// Update timings and stuff for the animations
	float fps = CProjectFactory::Interface(g_hWnd)->UpdateFPS(50);
	if(fps != -1.0f) {
		RUNACTION action;
		action.m_Ptr = (LPVOID)g_pMapGroupI;

		// run the scripts for the next frame:
		action.bJustWait = false;
		g_pMapGroupI->Run(action);
		CProjectFactory::Interface(g_hWnd)->WaitScripts();
		
		if(g_pGraphicsI->BeginPaint()) {
			g_pMapGroupI->Draw(g_pGraphicsI);
			g_pGraphicsI->DrawText(CPoint(10,10), COLOR_ARGB(255,255,255,255), "%4.1f fps", fps);
			g_pGraphicsI->EndPaint();
		}
	}
}

//-----------------------------------------------------------------------------
// Name: ProcessNextFrame()
// Desc: Called whenever the program is idle
//
//-----------------------------------------------------------------------------
HRESULT ProcessNextFrame()
{
	Render();
	return S_OK;
}
