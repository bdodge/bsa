//--------------------------------------------------------------------
//
// File: mackeycodes.h
// Desc: Map of Mac/OSX Keysyms to Windows VK
// Auth: Brian Dodge
//
// (C)opyright 2015  - BSA and Brian Dodge
// ALL RIGHTS RESERVED
//
//--------------------------------------------------------------------

#ifndef _MACKEYCODES_H_
#define _MACKEYCODES_H_ 1


// MAC keycode mappings to windows keycodes
//

#define keyIllegal			-1

unsigned MACkeyTranslations[256] = {
	'A', 		// 0
	'S', 		// 1
	'D', 		// 2
	'F', 		// 3
	'H', 		// 4
	'G', 		// 5
	'Z', 		// 6
	'X', 		// 7
	'C', 		// 8
	'V', 		// 9
	keyIllegal,	// 10
	'B', 		// 11
	'Q', 		// 12
	'W', 		// 13
	'E', 		// 14
	'R', 		// 15
	'Y', 		// 16
	'T', 		// 17
	'1', 		// 18
	'2', 		// 19
	'3', 		// 20
	'4', 		// 21
	'6', 		// 22
	'5', 		// 23
	'=', 		// 24
	'9', 		// 25
	'7', 		// 26
	'-', 		// 27
	'8', 		// 28
	'0', 		// 29
	']', 		// 30
	'O', 		// 31
	'U', 		// 32
	'[', 		// 33
	'I', 		// 34
	'P', 		// 35
	VK_RETURN | 0x1000, 		// 36
	'L', 		// 37
	'J', 		// 38
	'\'', 		// 39
	'K', 		// 40
	';', 		// 41
	'\\', 		// 42
	',', 		// 43
	'/', 		// 44
	'N', 		// 45
	'M', 		// 46
	'.', 		// 47
	VK_TAB | 0x1000,	// 48
	' ',		// 49
	'`', 		// 50
	VK_BACK | 0x1000,	// 51
	keyIllegal,	// 52
	27, 		// 53
	keyIllegal,	// 54
	keyIllegal, //VK_COMMAND, 		// 55
	VK_SHIFT | 0x1000, 		// 56
	VK_CAPITAL | 0x1000,		// 57
	keyIllegal,	//VK_OPTION, 		// 58
	VK_CONTROL | 0x1000, 		// 59
	keyIllegal,	// 60
	keyIllegal,
	keyIllegal,
	keyIllegal,
	keyIllegal,
	VK_DECIMAL | 0x1000,
	keyIllegal,
	VK_MULTIPLY | 0x1000,
	keyIllegal,
	VK_ADD | 0x1000,
	keyIllegal,	// 70
	VK_NUMLOCK | 0x1000,
	keyIllegal,
	keyIllegal,
	keyIllegal,
	VK_DIVIDE | 0x1000,
	VK_RETURN | 0x1000,
	keyIllegal,
	VK_SUBTRACT | 0x1000,
	keyIllegal,
	keyIllegal,	// 80
	'=',
	VK_NUMPAD0 | 0x1000,
	VK_NUMPAD1 | 0x1000,
	VK_NUMPAD2 | 0x1000,
	VK_NUMPAD3 | 0x1000,
	VK_NUMPAD4 | 0x1000,
	VK_NUMPAD5 | 0x1000,
	VK_NUMPAD6 | 0x1000,
	VK_NUMPAD7 | 0x1000,
	keyIllegal,	// 90
	VK_NUMPAD8 | 0x1000,
	VK_NUMPAD9 | 0x1000,
	keyIllegal,
	keyIllegal,
	keyIllegal,
	VK_F5 | 0x1000,
	VK_F6 | 0x1000,
	VK_F7 | 0x1000,
	VK_F3 | 0x1000,
	VK_F8 | 0x1000,	// 100
	VK_F9 | 0x1000,
	keyIllegal,
	VK_F11 | 0x1000,
	keyIllegal,
	VK_F13 | 0x1000,
	keyIllegal,
	VK_F14 | 0x1000,
	keyIllegal,
	keyIllegal,
	keyIllegal,	// 110
	VK_F12 | 0x1000,
	keyIllegal,
	VK_F15 | 0x1000,
	VK_HELP | 0x1000,
	VK_HOME | 0x1000,
	VK_PRIOR | 0x1000,
	VK_DELETE | 0x1000,
	VK_F4 | 0x1000,
	VK_END | 0x1000,
	VK_F2 | 0x1000,	// 120
	VK_NEXT | 0x1000,
	VK_F1 | 0x1000,
	
	VK_LEFT	| 0x1000, 		// 123
	VK_RIGHT | 0x1000, 		// 124				
	VK_DOWN | 0x1000, 		// 125
	VK_UP	| 0x1000, 		// 126

	keyIllegal,
	keyIllegal,
	keyIllegal,		
	keyIllegal
};
#endif

