// stdafx.cpp: archivo de c�digo fuente que contiene s�lo las inclusiones est�ndar
// OpenLegends.pch ser� el encabezado precompilado
// stdafx.obj contiene la informaci�n de tipos precompilada

#include "stdafx.h"

// disable exeption handling:
#include <exception>
namespace std {
    _Prhand _Raise_handler;
    void _cdecl _Throw(class std::exception const &) {};
}
//////////////////////////////////////////////////////////////////////

