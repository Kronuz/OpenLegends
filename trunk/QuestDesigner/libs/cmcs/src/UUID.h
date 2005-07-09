/////////////////////////////////////////////////////////////////////////////
//
// UUID.h : UUIDs used by the project
//
//  Copyright © 2000-2003  Nathan Lewis <nlewis@programmer.net>
//
// This source code can be modified and distributed freely, so long as this
// copyright notice is not altered or removed.
//
/////////////////////////////////////////////////////////////////////////////

//***************************************************************************
//***************************************************************************
//
// IMPORTANT NOTE!!!
//
// If you make modifications to the CodeMax source code, you should do the
// following:
//
//		1. Change the name of the OCX or DLL to avoid conflicts with the
//		   "official" release.
//
//		2. Change *all* of the UUIDs defined below - again, to avoid
//		   conflicts with the "official" release.
//
//		3. Go through all of the *.RGS files and update them with the new
//		   Coclass UUIDs defined below.
//
//		4. Update the EDITX.RGS file with the new LIBRARY_UUID defined below.
//
//***************************************************************************
//***************************************************************************

#ifndef _UUID_H__INCLUDED_
#define _UUID_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// UUIDs
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Library Information ------------------------------------------------------
#define LIBRARY_UUID					665BF2B8-F41F-4EF4-A8D0-303FBFFC475E
#define LIBRARY_VERSION					2.0
#define LIBRARY_HELPSTRING				"CodeSense 2.0 Edit Control"
#define LIBRARY_HELPFILE				"CodeSense.chm"


// Enumeration UUIDs --------------------------------------------------------
#define UUID_cmLimit					14E6D0A4-E47F-4258-AFDA-44A2541F31DD
#define UUID_cmCommand					557889FE-B10F-4F52-AB32-2F9FF2489582
#define UUID_cmCommandErr				2D87F27F-2FF0-4103-A40B-2E1A02117103
#define UUID_cmColorItem				F60231AF-73B1-48B4-A8E5-3F6B5967EF1C
#define UUID_cmFontStyleItem			C9E73222-0563-4F5F-830A-C7CF4D69C7E5
#define UUID_cmFontStyle				CDE370EC-78A9-4113-93D2-53859605B116
#define UUID_cmAutoIndentMode			7D101078-AFD2-4B81-B22F-468E96E7A04D
#define UUID_cmHitTestCode				186844B4-72BC-4B16-B812-E703784B00D6
#define UUID_cmLineNumStyle				A046736C-F875-4412-A368-8B2F1F8DC1CC
#define UUID_cmPrintFlags				BAC39AFF-E501-409C-ABD1-84BA18E40915
#define UUID_cmLangStyle				A514F7BC-69FD-43F3-88CE-9AF05AE1BC69
#define UUID_cmBorderStyle				977C37DA-22DD-4831-BB06-2FDB66378E6E
#define UUID_cmTokenType				A4267B0A-37A9-42EC-BCF0-E6E1EDF8F189
#define UUID_cmToolTipType				BA2345A4-0DB8-46F3-9377-5C1C9720F9C8
#define UUID_cmCodeListSortStyle		E652A11E-9E6A-4869-A2E9-CD8ACFC2DC63


// Interface UUIDs ----------------------------------------------------------
#define UUID_IRange						6CB7A54F-E406-4074-BB5E-6DEB17AC0010
#define UUID_IPosition					9BF3BD89-D8AD-4D43-9BFE-5FDDF07CD558
#define UUID_ILanguage					DAFFE8A2-925B-45D4-B034-F8E9D093135A
#define UUID_IHotKey					BCBDC4F8-EDAB-490D-A110-02D3D71A54DE
#define UUID_IGlobals					11065D3D-3A32-4170-962A-8A0F168DB117
#define UUID_IRect						3EFDC073-CA43-4781-9DA9-91328099A120
#define UUID_ICodeList					BA4A1EBD-AF16-407E-93A8-C440ABDAC9B5
#define UUID_ICodeTip					20CCBF7B-FC8D-42E7-9F22-99100C76AA7E
#define UUID_ICodeTipHighlight			1523D89A-5D87-479C-914E-A3070F37441B
#define UUID_ICodeTipFuncHighlight		BD846BB3-25EA-4566-82D3-4C1F790D6747
#define UUID_ICodeTipMultiFunc			9708C966-49FD-44C1-90C7-3615BADB1215
#define UUID_ICodeSense					515DB298-D15B-44BA-91C7-0134233EC305


// Dispinterface UUIDs ------------------------------------------------------
//
// NOTE: If you change the UUID below, be sure to update the corresponding
// declaration of IID_ICodeSenseEvents in CEDITX.CPP
//
#define UUID_ICodeSenseEvents			A29D57AD-8492-47AD-A6C1-9B68677B182F


// Coclass UUIDs ------------------------------------------------------------
#define UUID_CodeSense					A0F69707-2D87-4B20-9471-FBC003427134
#define UUID_Range						94D8D56F-73D3-4F3A-B1C0-F77EF1B2CB63
#define UUID_Position					96621C91-00F2-424A-B62F-C426B7FD9BD0
#define UUID_Language					5C5486F3-CE41-4EC6-B658-0155CE8836A1
#define UUID_HotKey						BE2A73A9-BE2C-420E-86A4-818E48E3F29C
#define UUID_Globals					5DFD0E0D-A13D-4B57-9A3C-FBB7B4072BE1
#define UUID_Rect						7F4BD1FA-4142-4C14-BCEC-5563E7F66E9B
#define UUID_CodeList					8DE7E400-050C-4B69-9EE7-2E9B0CCFD943
#define UUID_CodeTip					9FCF6BF7-2185-4A42-AD34-FF65FB4910B2
#define UUID_CodeTipHighlight			739908FB-2BE8-40BE-9DDD-B486289BF04B
#define UUID_CodeTipFuncHighlight		B1F5AB74-65B5-49FB-8A7C-221987BF09A4
#define UUID_CodeTipMultiFunc			0458139D-32E9-4C43-876B-73B0A21A4E9C


/////////////////////////////////////////////////////////////////////////////

#endif//#ifndef _UUID_H__INCLUDED_
