//--------------------------------------------------------------------
//
// File: xkeycodes.h
// Desc: Map of X11 Keysyms to Windows VK
// Auth: Brian Dodge
//
// (C)opyright 2003  - BSA and Brian Dodge
// ALL RIGHTS RESERVED
//
//--------------------------------------------------------------------

#ifndef _XKEYCODES_H_
#define _XKEYCODES_H_ 1


// X11 keycode mappings to windows keycodes
//

#define X11keyTableBase		0x50
#define keyIllegal			-1

int X11keyTranslations[256] = {

/* Cursor control & motion */

/* XK_Home			0xFF50 */	VK_HOME,			/* 00 */
/* XK_Left			0xFF51 */	VK_LEFT,
/* XK_Up			0xFF52 */	VK_UP,
/* XK_Right			0xFF53 */	VK_RIGHT,
/* XK_Down			0xFF54 */	VK_DOWN,
/* XK_Prior			0xFF55 */	VK_PRIOR,
/* XK_Next			0xFF56 */	VK_NEXT,
/* XK_End			0xFF57 */	VK_END,
/* XK_Begin			0xFF58 */	VK_HOME,

/*					0xFF59 */	keyIllegal, /* ??? */

/*					0xFF5A */	keyIllegal, /* ??? */
/*					0xFF5B */	keyIllegal, /* ??? */
/*					0xFF5C */	keyIllegal, /* ??? */
/*					0xFF5D */	keyIllegal, /* ??? */
/*					0xFF5E */	keyIllegal, /* ??? */
/*					0xFF5F */	keyIllegal, /* ??? */

/* Misc Functions */

/* XK_Select		0xFF60 */	VK_SELECT,				/* 10 */
/* XK_Print			0xFF61 */	VK_PRINT,
/* XK_Execute		0xFF62 */	VK_EXECUTE,
/* XK_Insert		0xFF63 */	VK_INSERT,

/*					0xFF64 */	keyIllegal, /* ??? */

/* XK_Undo			0xFF65 */	VK_BACK,
/* XK_Redo			0xFF66 */	keyIllegal,
/* XK_Menu			0xFF67 */	/*keyIllegal*/VK_MENU,
/* XK_Find			0xFF68 */	VK_APPS,
/* XK_Cancel		0xFF69 */	VK_CANCEL,
/* XK_Help			0xFF6A */	VK_HELP,		/* F1 on sun keyboards */
/* XK_Break			0xFF6B */	VK_CANCEL,

/*					0xFF6C */	keyIllegal, /* ??? */
/*					0xFF6D */	keyIllegal, /* ??? */
/*					0xFF6E */	keyIllegal, /* ??? */
/*					0xFF6F */	keyIllegal, /* ??? */
/*					0xFF70 */	keyIllegal, /* ??? */	/* 20 */
/*					0xFF71 */	keyIllegal, /* ??? */
/*					0xFF72 */	keyIllegal, /* ??? */
/*					0xFF73 */	keyIllegal, /* ??? */
/*					0xFF74 */	keyIllegal, /* ??? */
/*					0xFF75 */	keyIllegal, /* ??? */
/*					0xFF76 */	keyIllegal, /* ??? */
/*					0xFF77 */	keyIllegal, /* ??? */
/*					0xFF78 */	keyIllegal, /* ??? */
/*					0xFF79 */	keyIllegal, /* ??? */
/*					0xFF7A */	keyIllegal, /* ??? */
/*					0xFF7B */	keyIllegal, /* ??? */
/*					0xFF7C */	keyIllegal, /* ??? */
/*					0xFF7D */	keyIllegal, /* ??? */

/* XK_Mode_switch	0xFF7E */	keyIllegal,
/* XK_Num_Lock		0xFF7F */	VK_NUMLOCK,

/* Keypad Functions, keypad numbers cleverly chosen to map to ascii */

/* XK_KP_Space		0xFF80 */	VK_SPACE, /* space */	/* 30 */

/*					0xFF81 */	keyIllegal, /* ??? */
/*					0xFF82 */	keyIllegal, /* ??? */
/*					0xFF83 */	keyIllegal, /* ??? */
/*					0xFF84 */	keyIllegal, /* ??? */
/*					0xFF85 */	keyIllegal, /* ??? */
/*					0xFF86 */	keyIllegal, /* ??? */
/*					0xFF87 */	keyIllegal, /* ??? */
/*					0xFF88 */	keyIllegal, /* ??? */

/* XK_KP_Enter		0xFF89 */	VK_TAB,

/*					0xFF8A */	keyIllegal,
/*					0xFF8B */	keyIllegal,
/*					0xFF8C */	keyIllegal,

/* XK_KP_Tab		0xFF8D */	13,

/*					0xFF8E */	keyIllegal,
/*					0xFF8F */	keyIllegal,
/*					0xFF90 */	keyIllegal,				/* 40 */

/* XK_KP_F1			0xFF91 */	VK_F1,
/* XK_KP_F2			0xFF92 */	VK_F2,
/* XK_KP_F3			0xFF93 */	VK_F3,
/* XK_KP_F4			0xFF94 */	VK_F4,

/*					0xFF95 */	VK_HOME,
/*					0xFF96 */	VK_LEFT,
/*					0xFF97 */	VK_UP,
/*					0xFF98 */	VK_RIGHT,
/*					0xFF99 */	VK_DOWN,
/*					0xFF9A */	VK_PRIOR,
/*					0xFF9B */	VK_NEXT,
/*					0xFF9C */	VK_END,
/*					0xFF9D */	VK_HOME, /* ?? */
/*					0xFF9E */	VK_INSERT,
/*					0xFF9F */	VK_DELETE,
/*					0xFFA0 */	keyIllegal, /* ??? */	/* 50 */
/*					0xFFA1 */	keyIllegal, /* ??? */
/*					0xFFA2 */	keyIllegal, /* ??? */
/*					0xFFA3 */	keyIllegal, /* ??? */
/*					0xFFA4 */	keyIllegal, /* ??? */
/*					0xFFA5 */	keyIllegal, /* ??? */
/*					0xFFA6 */	keyIllegal, /* ??? */
/*					0xFFA7 */	keyIllegal, /* ??? */
/*					0xFFA8 */	keyIllegal, /* ??? */
/*					0xFFA9 */	keyIllegal, /* ??? */

/* XK_KP_Multiply	0xFFAA */	VK_MULTIPLY,
/* XK_KP_Add		0xFFAB */	VK_ADD,
/* XK_KP_Separator	0xFFAC */	',',
/* XK_KP_Subtract	0xFFAD */	VK_SUBTRACT,
/* XK_KP_Decimal	0xFFAE */	VK_DECIMAL,
/* XK_KP_Divide		0xFFAF */	VK_DIVIDE,

/* XK_KP_0			0xFFB0 */	VK_NUMPAD0,					/* 60 */
/* XK_KP_1			0xFFB1 */	VK_NUMPAD1,
/* XK_KP_2			0xFFB2 */	VK_NUMPAD2,
/* XK_KP_3			0xFFB3 */	VK_NUMPAD3,
/* XK_KP_4			0xFFB4 */	VK_NUMPAD4,
/* XK_KP_5			0xFFB5 */	VK_NUMPAD5,
/* XK_KP_6			0xFFB6 */	VK_NUMPAD6,
/* XK_KP_7			0xFFB7 */	VK_NUMPAD7,
/* XK_KP_8			0xFFB8 */	VK_NUMPAD8,
/* XK_KP_9			0xFFB9 */	VK_NUMPAD9,

/*					0xFFBA */	keyIllegal, /* ??? */
/*					0xFFBB */	keyIllegal, /* ??? */
/*					0xFFBC */	keyIllegal, /* ??? */

/* XK_KP_Equal		0xFFBD */	'"',


/*
 * Auxilliary Functions; note the duplicate definitions for left and right
 * function keys;  Sun keyboards and a few other manufactures have such
 * function key groups on the left and/or right sides of the keyboard.
 * We've not found a keyboard with more than 35 function keys total.
 */

/* XK_F1			0xFFBE */	VK_F1,
/* XK_F2			0xFFBF */	VK_F2,
/* XK_F3			0xFFC0 */	VK_F3,					/* 70 */
/* XK_F4			0xFFC1 */	VK_F4,
/* XK_F5			0xFFC2 */	VK_F5,
/* XK_F6			0xFFC3 */	VK_F6,
/* XK_F7			0xFFC4 */	VK_F7,
/* XK_F8			0xFFC5 */	VK_F8,
/* XK_F9			0xFFC6 */	VK_F9,
/* XK_F10			0xFFC7 */	VK_F10,
/* XK_F11			0xFFC8 */	VK_F11,
/* XK_F12			0xFFC9 */	VK_F12,
/* XK_F13			0xFFCA */	VK_F13,
/* XK_F14			0xFFCB */	VK_F14,
/* XK_F15			0xFFCC */	VK_F15,
/* XK_F16			0xFFCD */	VK_F16,
/* XK_F17			0xFFCE */	VK_F17,
/* XK_F18			0xFFCF */	VK_F18,
/* XK_F19			0xFFD0 */	VK_F19,					/* 80 */
/* XK_F20			0xFFD1 */	VK_F20,
/* XK_F21			0xFFD2 */	VK_F21,
/* XK_F22			0xFFD3 */	VK_F22,
/* XK_F23			0xFFD4 */	VK_F23,
/* XK_F24			0xFFD5 */	VK_F24,
/* XK_F25			0xFFD6 */	keyIllegal,
/* XK_F26			0xFFD7 */	keyIllegal,
/* XK_F27			0xFFD8 */	keyIllegal,
/* XK_F28			0xFFD9 */	keyIllegal,
/* XK_F29			0xFFDA */	VK_PRIOR,	/* on Sun IPC */
/* XK_F30			0xFFDB */	keyIllegal,
/* XK_F31			0xFFDC */	keyIllegal,
/* XK_F32			0xFFDD */	keyIllegal,
/* XK_F33			0xFFDE */	keyIllegal,
/* XK_F34			0xFFDF */	keyIllegal,

/* XK_F35			0xFFE0 */	VK_NEXT,	/* on Sun IPC *//* 90 */
/*      			0xFFE1 */	VK_SHIFT,
/*      			0xFFE2 */	VK_SHIFT,
/*      			0xFFE3 */	VK_CONTROL,
/*      			0xFFE4 */	VK_CONTROL,
/*      			0xFFE5 */	VK_CAPITAL,
/*      			0xFFE6 */	VK_SHIFT,
/*      			0xFFE7 */	VK_MENU,
/*      			0xFFE8 */	VK_MENU,
/*      			0xFFE9 */	VK_MENU,
/*      			0xFFEA */	VK_MENU,
/*      			0xFFEB */	VK_MENU,
/*      			0xFFEC */	VK_MENU,
/*      			0xFFED */	VK_MENU,
/*      			0xFFEE */	VK_MENU,
/*      			0xFFEF */	keyIllegal,
/*      			0xFFF0 */	keyIllegal,
/*      			0xFFF1 */	keyIllegal,
/*      			0xFFF2 */	keyIllegal,
/*      			0xFFF3 */	keyIllegal,
/*      			0xFFF4 */	keyIllegal,
/*      			0xFFF5 */	keyIllegal,
/*      			0xFFF6 */	keyIllegal,
/*      			0xFFF7 */	keyIllegal,
/*      			0xFFF8 */	keyIllegal,
/*      			0xFFF9 */	keyIllegal,
/*      			0xFFFA */	keyIllegal,
/*      			0xFFFB */	keyIllegal,
/*      			0xFFFC */	keyIllegal,
/*      			0xFFFD */	keyIllegal,
/*      			0xFFFE */	keyIllegal,
/*      			0xFFFF */	VK_DELETE	

};

#define X119995keyTableBase		0

int X119995keyTranslations[256] = {

/* ISO 9995 Function and Modifier Keys */

/*									0xFE00	*/	keyIllegal,

/*	XK_ISO_Lock						0xFE01	*/	keyIllegal,
/*	XK_ISO_Level2_Latch				0xFE02	*/	keyIllegal,
/*	XK_ISO_Level3_Shift				0xFE03	*/	keyIllegal,
/*	XK_ISO_Level3_Latch				0xFE04	*/	keyIllegal,
/*	XK_ISO_Level3_Lock				0xFE05	*/	keyIllegal,

/*	XK_ISO_Group_Latch				0xFE06	*/	keyIllegal,
/*	XK_ISO_Group_Lock				0xFE07	*/	keyIllegal,
/*	XK_ISO_Next_Group				0xFE08	*/	keyIllegal,
/*	XK_ISO_Next_Group_Lock			0xFE09	*/	keyIllegal,
/*	XK_ISO_Prev_Group				0xFE0A	*/	keyIllegal,
/*	XK_ISO_Prev_Group_Lock			0xFE0B	*/	keyIllegal,
/*	XK_ISO_First_Group				0xFE0C	*/	keyIllegal,
/*	XK_ISO_First_Group_Lock			0xFE0D	*/	keyIllegal,
/*	XK_ISO_Last_Group				0xFE0E	*/	keyIllegal,
/*	XK_ISO_Last_Group_Lock			0xFE0F	*/	keyIllegal,

/*									0xFE10	*/	keyIllegal,
/*									0xFE11	*/	keyIllegal,
/*									0xFE12	*/	keyIllegal,
/*									0xFE13	*/	keyIllegal,
/*									0xFE14	*/	keyIllegal,
/*									0xFE15	*/	keyIllegal,
/*									0xFE16	*/	keyIllegal,
/*									0xFE17	*/	keyIllegal,
/*									0xFE18	*/	keyIllegal,
/*									0xFE19	*/	keyIllegal,
/*									0xFE1A	*/	keyIllegal,
/*									0xFE1B	*/	keyIllegal,
/*									0xFE1C	*/	keyIllegal,
/*									0xFE1D	*/	keyIllegal,
/*									0xFE1E	*/	keyIllegal,
/*									0xFE1F	*/	keyIllegal,

/*	XK_ISO_Left_Tab					0xFE20	*/	VK_TAB,
/*	XK_ISO_Move_Line_Up				0xFE21	*/	VK_UP,
/*	XK_ISO_Move_Line_Down			0xFE22	*/	VK_DOWN,
/*	XK_ISO_Partial_Line_Up			0xFE23	*/	VK_UP,
/*	XK_ISO_Partial_Line_Down		0xFE24	*/	VK_DOWN,
/*	XK_ISO_Partial_Space_Left		0xFE25	*/	VK_LEFT,
/*	XK_ISO_Partial_Space_Right		0xFE26	*/	VK_RIGHT,

/*  XK_Set_Margin_Left				0xFE27	*/	keyIllegal,

/*	XK_Set_Margin_Right				0xFE28	*/	keyIllegal,
/*	XK_ISO_Release_Margin_Left		0xFE29	*/	keyIllegal,
/*	XK_ISO_Release_Margin_Right		0xFE2A	*/	keyIllegal,
/*	XK_ISO_Release_Both_Margins		0xFE2B	*/	keyIllegal,
/*	XK_ISO_Fast_Cursor_Left			0xFE2C	*/	VK_LEFT,
/*	XK_ISO_Fast_Cursor_Right		0xFE2D	*/	VK_RIGHT,
/*	XK_ISO_Fast_Cursor_Up			0xFE2E	*/	VK_UP,
/*	XK_ISO_Fast_Cursor_Down			0xFE2F	*/	VK_DOWN,
/*	XK_ISO_Continuous_Underline		0xFE30	*/	keyIllegal,
/*	XK_ISO_Discontinuous_Underline	0xFE31	*/	keyIllegal,
/*	XK_ISO_Emphasize				0xFE32	*/	keyIllegal,
/*	XK_ISO_Center_Object			0xFE33	*/	keyIllegal,
/*	XK_ISO_Enter					0xFE34	*/	VK_RETURN,

/*									0xFE35	*/	keyIllegal,
/*									0xFE36	*/	keyIllegal,
/*									0xFE37	*/	keyIllegal,
/*									0xFE38	*/	keyIllegal,
/*									0xFE39	*/	keyIllegal,
/*									0xFE3A	*/	keyIllegal,
/*									0xFE3B	*/	keyIllegal,
/*									0xFE3C	*/	keyIllegal,
/*									0xFE3D	*/	keyIllegal,
/*									0xFE3E	*/	keyIllegal,
/*									0xFE3F	*/	keyIllegal,

/*									0xFE40	*/	keyIllegal,
/*									0xFE41	*/	keyIllegal,
/*									0xFE42	*/	keyIllegal,
/*									0xFE43	*/	keyIllegal,
/*									0xFE44	*/	keyIllegal,
/*									0xFE45	*/	keyIllegal,
/*									0xFE46	*/	keyIllegal,
/*									0xFE47	*/	keyIllegal,
/*									0xFE48	*/	keyIllegal,
/*									0xFE49	*/	keyIllegal,
/*									0xFE4A	*/	keyIllegal,
/*									0xFE4B	*/	keyIllegal,
/*									0xFE4C	*/	keyIllegal,
/*									0xFE4D	*/	keyIllegal,
/*									0xFE4E	*/	keyIllegal,
/*									0xFE4F	*/	keyIllegal,

/*	XK_dead_grave					0xFE50	*/	keyIllegal,
/*	XK_dead_acute					0xFE51	*/	keyIllegal,
/*	XK_dead_circumflex				0xFE52	*/	keyIllegal,
/*	XK_dead_tilde					0xFE53	*/	keyIllegal,
/*	XK_dead_macron					0xFE54	*/	keyIllegal,
/*	XK_dead_breve					0xFE55	*/	keyIllegal,
/*	XK_dead_abovedot				0xFE56	*/	keyIllegal,
/*	XK_dead_diaeresis				0xFE57	*/	keyIllegal,
/*	XK_dead_abovering				0xFE58	*/	keyIllegal,
/*	XK_dead_doubleacute				0xFE59	*/	keyIllegal,
/*	XK_dead_caron					0xFE5A	*/	keyIllegal,
/*	XK_dead_cedilla					0xFE5B	*/	keyIllegal,
/*	XK_dead_ogonek					0xFE5C	*/	keyIllegal,
/*	XK_dead_iota					0xFE5D	*/	keyIllegal,
/*	XK_dead_voiced_sound			0xFE5E	*/	keyIllegal,
/*	XK_dead_semivoiced_sound		0xFE5F	*/	keyIllegal,
/*	XK_dead_belowdot				0xFE60	*/	keyIllegal,
/*  XK_dead_hook					0xFE61	*/	keyIllegal,
/*  XK_dead_horn					0xFE62	*/	keyIllegal,

/*									0xFE63	*/	keyIllegal,
/*									0xFE64	*/	keyIllegal,
/*									0xFE65	*/	keyIllegal,
/*									0xFE66	*/	keyIllegal,
/*									0xFE67	*/	keyIllegal,
/*									0xFE68	*/	keyIllegal,
/*									0xFE69	*/	keyIllegal,
/*									0xFE6A	*/	keyIllegal,
/*									0xFE6B	*/	keyIllegal,
/*									0xFE6C	*/	keyIllegal,
/*									0xFE6D	*/	keyIllegal,
/*									0xFE6E	*/	keyIllegal,
/*									0xFE6F	*/	keyIllegal,

/*	XK_AccessX_Enable				0xFE70	*/	keyIllegal,
/*	XK_AccessX_Feedback_Enable		0xFE71	*/	keyIllegal,
/*	XK_RepeatKeys_Enable			0xFE72	*/	keyIllegal,
/*	XK_SlowKeys_Enable				0xFE73	*/	keyIllegal,
/*	XK_BounceKeys_Enable			0xFE74	*/	keyIllegal,
/*	XK_StickyKeys_Enable			0xFE75	*/	keyIllegal,
/*	XK_MouseKeys_Enable				0xFE76	*/	keyIllegal,
/*	XK_MouseKeys_Accel_Enable		0xFE77	*/	keyIllegal,
/*	XK_Overlay1_Enable				0xFE78	*/	keyIllegal,
/*	XK_Overlay2_Enable				0xFE79	*/	keyIllegal,
/*	XK_AudibleBell_Enable			0xFE7A	*/	keyIllegal,

/*									0xFE7B	*/	keyIllegal,
/*									0xFE7C	*/	keyIllegal,
/*									0xFE7D	*/	keyIllegal,
/*									0xFE7E	*/	keyIllegal,
/*									0xFE7F	*/	keyIllegal,

/*									0xFE80	*/	keyIllegal,
/*									0xFE81	*/	keyIllegal,
/*									0xFE82	*/	keyIllegal,
/*									0xFE83	*/	keyIllegal,
/*									0xFE84	*/	keyIllegal,
/*									0xFE85	*/	keyIllegal,
/*									0xFE86	*/	keyIllegal,
/*									0xFE87	*/	keyIllegal,
/*									0xFE88	*/	keyIllegal,
/*									0xFE89	*/	keyIllegal,
/*									0xFE8A	*/	keyIllegal,
/*									0xFE8B	*/	keyIllegal,
/*									0xFE8C	*/	keyIllegal,
/*									0xFE8D	*/	keyIllegal,
/*									0xFE8E	*/	keyIllegal,
/*									0xFE8F	*/	keyIllegal,

/*									0xFE90	*/	keyIllegal,
/*									0xFE91	*/	keyIllegal,
/*									0xFE92	*/	keyIllegal,
/*									0xFE93	*/	keyIllegal,
/*									0xFE94	*/	keyIllegal,
/*									0xFE95	*/	keyIllegal,
/*									0xFE96	*/	keyIllegal,
/*									0xFE97	*/	keyIllegal,
/*									0xFE98	*/	keyIllegal,
/*									0xFE99	*/	keyIllegal,
/*									0xFE9A	*/	keyIllegal,
/*									0xFE9B	*/	keyIllegal,
/*									0xFE9C	*/	keyIllegal,
/*									0xFE9D	*/	keyIllegal,
/*									0xFE9E	*/	keyIllegal,
/*									0xFE9F	*/	keyIllegal,

/*									0xFEA0	*/	keyIllegal,
/*									0xFEA1	*/	keyIllegal,
/*									0xFEA2	*/	keyIllegal,
/*									0xFEA3	*/	keyIllegal,
/*									0xFEA4	*/	keyIllegal,
/*									0xFEA5	*/	keyIllegal,
/*									0xFEA6	*/	keyIllegal,
/*									0xFEA7	*/	keyIllegal,
/*									0xFEA8	*/	keyIllegal,
/*									0xFEA9	*/	keyIllegal,
/*									0xFEAA	*/	keyIllegal,
/*									0xFEAB	*/	keyIllegal,
/*									0xFEAC	*/	keyIllegal,
/*									0xFEAD	*/	keyIllegal,
/*									0xFEAE	*/	keyIllegal,
/*									0xFEAF	*/	keyIllegal,

/*									0xFEB0	*/	keyIllegal,
/*									0xFEB1	*/	keyIllegal,
/*									0xFEB2	*/	keyIllegal,
/*									0xFEB3	*/	keyIllegal,
/*									0xFEB4	*/	keyIllegal,
/*									0xFEB5	*/	keyIllegal,
/*									0xFEB6	*/	keyIllegal,
/*									0xFEB7	*/	keyIllegal,
/*									0xFEB8	*/	keyIllegal,
/*									0xFEB9	*/	keyIllegal,
/*									0xFEBA	*/	keyIllegal,
/*									0xFEBB	*/	keyIllegal,
/*									0xFEBC	*/	keyIllegal,
/*									0xFEBD	*/	keyIllegal,
/*									0xFEBE	*/	keyIllegal,
/*									0xFEBF	*/	keyIllegal,

/*									0xFEC0	*/	keyIllegal,
/*									0xFEC1	*/	keyIllegal,
/*									0xFEC2	*/	keyIllegal,
/*									0xFEC3	*/	keyIllegal,
/*									0xFEC4	*/	keyIllegal,
/*									0xFEC5	*/	keyIllegal,
/*									0xFEC6	*/	keyIllegal,
/*									0xFEC7	*/	keyIllegal,
/*									0xFEC8	*/	keyIllegal,
/*									0xFEC9	*/	keyIllegal,
/*									0xFECA	*/	keyIllegal,
/*									0xFECB	*/	keyIllegal,
/*									0xFECC	*/	keyIllegal,
/*									0xFECD	*/	keyIllegal,
/*									0xFECE	*/	keyIllegal,
/*									0xFECF	*/	keyIllegal,

/*	XK_First_Virtual_Screen			0xFED0	*/	keyIllegal,
/*	XK_Prev_Virtual_Screen			0xFED1	*/	keyIllegal,
/*	XK_Next_Virtual_Screen			0xFED2	*/	keyIllegal,
/*	XK_Last_Virtual_Screen			0xFED4	*/	keyIllegal,
/*	XK_Terminate_Server				0xFED5	*/	keyIllegal,

/*									0xFED6	*/	keyIllegal,
/*									0xFED7	*/	keyIllegal,
/*									0xFED8	*/	keyIllegal,
/*									0xFED9	*/	keyIllegal,
/*									0xFEDA	*/	keyIllegal,
/*									0xFEDB	*/	keyIllegal,
/*									0xFEDC	*/	keyIllegal,
/*									0xFEDD	*/	keyIllegal,
/*									0xFEDE	*/	keyIllegal,
/*									0xFEDF	*/	keyIllegal,

/*	XK_Pointer_Left					0xFEE0	*/	keyIllegal,
/*	XK_Pointer_Right				0xFEE1	*/	keyIllegal,
/*	XK_Pointer_Up					0xFEE2	*/	keyIllegal,
/*	XK_Pointer_Down					0xFEE3	*/	keyIllegal,
/*	XK_Pointer_UpLeft				0xFEE4	*/	keyIllegal,
/*	XK_Pointer_UpRight				0xFEE5	*/	keyIllegal,
/*	XK_Pointer_DownLeft				0xFEE6	*/	keyIllegal,
/*	XK_Pointer_DownRight			0xFEE7	*/	keyIllegal,
/*	XK_Pointer_Button_Dflt			0xFEE8	*/	keyIllegal,
/*	XK_Pointer_Button1				0xFEE9	*/	keyIllegal,
/*	XK_Pointer_Button2				0xFEEA	*/	keyIllegal,
/*	XK_Pointer_Button3				0xFEEB	*/	keyIllegal,
/*	XK_Pointer_Button4				0xFEEC	*/	keyIllegal,
/*	XK_Pointer_Button5				0xFEED	*/	keyIllegal,
/*	XK_Pointer_DblClick_Dflt		0xFEEE	*/	keyIllegal,
/*	XK_Pointer_DblClick1			0xFEEF	*/	keyIllegal,
/*	XK_Pointer_DblClick2			0xFEF0	*/	keyIllegal,
/*	XK_Pointer_DblClick3			0xFEF1	*/	keyIllegal,
/*	XK_Pointer_DblClick4			0xFEF2	*/	keyIllegal,
/*	XK_Pointer_DblClick5			0xFEF3	*/	keyIllegal,
/*	XK_Pointer_Drag_Dflt			0xFEF4	*/	keyIllegal,
/*	XK_Pointer_Drag1				0xFEF5	*/	keyIllegal,
/*	XK_Pointer_Drag2				0xFEF6	*/	keyIllegal,
/*	XK_Pointer_Drag3				0xFEF7	*/	keyIllegal,
/*	XK_Pointer_Drag4				0xFEF8	*/	keyIllegal,
/*	XK_Pointer_EnableKeys			0xFEF9	*/	keyIllegal,
/*	XK_Pointer_Accelerate			0xFEFA	*/	keyIllegal,
/*	XK_Pointer_DfltBtnNext			0xFEFB	*/	keyIllegal,
/*	XK_Pointer_DfltBtnPrev			0xFEFC	*/	keyIllegal
/*	XK_Pointer_Drag5				0xFEFD	*/	keyIllegal,

/*									0xFEFE	*/	keyIllegal,
/*									0xFEFF	*/	keyIllegal,
};

#endif /* XWINDOWS -> WIN32 key mapping table */

