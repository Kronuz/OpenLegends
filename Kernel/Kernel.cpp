// Kernel.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "Kernel.h"

#include "GameManager.h"

//////////////////////////////////////////////////////////////////////////////
// Interface Version Definition:
const WORD IGame::Version = 0x0401;

//////////////////////////////////////////////////////////////////////////////
// Exported functions:

HRESULT QueryGameInterface(WORD Version, IGame **pInterface, IConsole *Output)
{
	if( HIBYTE(Version) != HIBYTE(IGame::Version) ) return E_FAIL;
	if(	LOBYTE(Version) > LOBYTE(IGame::Version) ) return E_FAIL;

	if(!*pInterface) {
		// Convert our interface pointer to a CGraphicsD3D8 object
		*pInterface = CGameManager::Instance();
		(*pInterface)->SetConsole(Output);
		return S_OK;
	}

	return E_FAIL;
}
// Release our Handle to the class
HRESULT ReleaseGameInterface(IGame **pInterface)
{
	if(!*pInterface) {
		return E_FAIL;
	}

	// Not our device:
	if((*pInterface)->GetModuleID() != MODULE_ID) {
		return E_FAIL;
	}

	delete *pInterface;
	*pInterface = NULL;
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
// Entry point:
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}
