
/*****************************************************************************
 * Synopsis:    void SetVerPosition(pos_piu, rel, row)
 * Description: Set the vertical position of the cursor.
 * Argument s:   (Sint32) pos_piu -- positive, whole value in PIU.
 *              (ValRel) rel -- determine relative, absolute, error
 *                                   in reading the signs of the value.
 *              (Sint8) row -- ~0 if setting position by row, in which
 *                              case a movement off the page causes
 *                              a form feed.
 * Return:      (void)
 * Uses:
 * Sets:
 * Changes:
 * Heap Use:    none.
 *****************************************************************************/

static void SetVerPosition(Sint32 pos_piu, ValRel rel, Sint8 row)
{
    Sint32 vCursor_piu;

    if (!SOMETHINGONPAGE) {
	parse.firstInk = NOINKMOVED;
	HPCheckPageChar();
    }

    ASSERT(rel != REL_ERROR);

    if (rel != REL_NOTREL) {
	vCursor_piu = verCursor_piu + ((rel == REL_POS) ? pos_piu : -pos_piu);
    } else {
	vCursor_piu = printer.tMargin_piu + pos_piu;
    }

    if (vCursor_piu < 0) {
	vCursor_piu = 0;
    } else if (vCursor_piu > printer.bLimit_piu) {
       /*
	*   Relative motion off the page by ROWS causes a formfeed
	*   to the next page.  Otherwise, just constrain to page length.
	*/
	if (rel && row) {
	    HPFormFeed(FFNOCR, FFNOEOJ);
	    vCursor_piu -= printer.bLimit_piu;
	    /*
	     * If the relative motion goes past yet another page (or more
	     * than one more page) do another formfeed and position the
	     * cursor at the top of that page.
	     */
	    if ( vCursor_piu >= printer.bLimit_piu ) {
		HPFormFeed(FFNOCR, FFNOEOJ);
		vCursor_piu = verCursor_piu;
	    }
	} 
    }

    HPSetAbsY(vCursor_piu);
    verCursor_piu = vCursor_piu;
    PDIgclose();            /* close open rasters on Y move */
}



/*****************************************************************************
 * Synopsis:    void SetHorPosition(pos_piu, rel)
 * Description: Set the cursor horizontal position.
 * Arguments:   (Sint32) pos_piu -- horizontal position to set.
 *              (Sint8) rel -- absolute, relative or error in sign parsing.
 * Return:      (void)
 * Uses:
 * Sets:
 * Changes:
 * Heap Use:    none
 *****************************************************************************/

static void SetHorPosition(Sint32 pos_piu, ValRel rel)
{
    Sint32 relMotion_piu;
    Sint32 curPos_piu;
    Sint32 newPos_piu;

    if (!SOMETHINGONPAGE) {
	parse.firstInk = NOINKMOVED;
	HPCheckPageChar();
    }

    ASSERT(rel != REL_ERROR);

    curPos_piu = HPGetAbsX();

    /*
     *  Calculate the new position (by relative or absolute means)
     */
    if (rel != REL_NOTREL) {
	if (rel == REL_NEG) {
    /*  If beyond the right limit, adjust to the right limit plus one PCL UNIT (UOM pel) */
	    if (curPos_piu > printer.rLimit_piu)
		newPos_piu = printer.rLimit_piu - pos_piu + printer.one_uom_pel_in_piux;
	    else 
		newPos_piu = curPos_piu - pos_piu;
	/* do not let neg rel move actually go negative */
	if (newPos_piu < 0)
	    newPos_piu = 0;
	} else {
	    newPos_piu = curPos_piu + pos_piu;
	}
    } else {
	newPos_piu = pos_piu;
    }
    /* 
     *  Constrain the newPos_piu to a reasonable limit to avoid 
     *  math problems
     */
    if (newPos_piu > 2* printer.rLimit_piu)
	newPos_piu = 2 * printer.rLimit_piu;

    /*
     *  Determine the relative motion of the new position.
     */
    relMotion_piu = newPos_piu - curPos_piu;

    /*
     *  Determine if a relative motion needs be done either because
     *  of a positive motion with underlining or just because we
     *  are doing relative motion (make these actions exclusive!).
     */
     if (relMotion_piu > 0 && options.underline) {
	/*
	 * Before doing the relative motion, make sure the current font
	 * is up-to-date for floating underlines.
	 */
	if (options.primary) {
	    HPMAYBECHOOSEFONT(&primFont);
	}
	else {
	    HPMAYBECHOOSEFONT(&secFont);
	}
	PDIfatspace(relMotion_piu);
	parse.firstInk  = INKONPAPER;
    } else if (rel != REL_NOTREL) {
	PDIrelmove(relMotion_piu);
    }

    /*
     *  Determine if we have absolute addressing and move to that
     *  absolute position.  Notice that we may have even moved
     *  to it relatively with the PDIfatspace() call above, but
     *  it is important to make the absolute call as well to
     *  get rid of error terms accumulated by PDI on relative motion.
     */
    if (rel == REL_NOTREL) {
	HPSetAbsX(newPos_piu);
    }
    if (newPos_piu > printer.rMargin_piu )
	printer.wrLimit_piu = printer.rLimit_piu + printer.one_uom_pel_in_piux;
    else {
	printer.wrLimit_piu = printer.rMargin_piu;
	if (printer.rMargin_piu == printer.rLimit_piu)
	    printer.wrLimit_piu += printer.one_uom_pel_in_piux;
    }

}


/*****************************************************************************
 * Synopsis:    void setHPlot()
 * Description: Set horizontal plot size (from Esc*c#K command) for HPGL2
 * Arguments:   none.
 * Return:      (void)
 * Uses:        plotSizeWidth_piu, HPGLargs.framewd
 * Sets:        HPGLargs.plotwd
 * Changes:
 * Heap Use:    none.
 *****************************************************************************/
static int plotSizeWidth_piu;
static Boolean defaultHPlot;
static int plotSizeHgt_piu;
static Boolean defaultVPlot;
static Boolean newVp,newHp;

static void setHPlot(void)
{
    if (defaultHPlot && newHp) {
		/*
		 *  This defaults to the current frame width.
		 */
		plotSizeWidth_piu = HPGLargs.framewd;
    }
    HPGLargs.plotwd = plotSizeWidth_piu;

    if (!defaultHPlot)
		newHp = FALSE;
}

/*****************************************************************************
 * Synopsis:    void setVPlot()
 * Description: Set vertical plot size (from Esc*c#L command) for HPGL2
 * Arguments:   none.
 * Return:      (void)
 * Uses:        plotSizeHgt_piu, HPGLargs.frameht
 * Sets:        OMGLargs.plotht
 * Changes:
 * Heap Use:    none.
 *****************************************************************************/
static void setVPlot(void)
{
    if (defaultVPlot && newVp) {
		/*
		 *  This defaults to the current frame height.
		 */
		plotSizeHgt_piu = HPGLargs.frameht;
    }
    HPGLargs.plotht = plotSizeHgt_piu;

    if (!defaultVPlot)
		newVp = FALSE;
}

/*****************************************************************************
 * Synopsis:    void resetHVPlot()
 * Description: reset setHPlot and setVPlot to default state
 * Arguments:   none.
 * Return:      (void)
 * Uses:
 * Sets:        defaultHPlot, defaultVPlot
 * Changes:
 * Heap Use:    none.
 *****************************************************************************/

void resetHVPlot(void)
{
    defaultHPlot = TRUE;
    defaultVPlot = TRUE;
    newHp = TRUE;
    newVp = TRUE;
}


/*****************************************************************************
 * Synopsis:    void DotMovement()
 * Description: Do action from <esc>*p#Z where Z is decoded.
 * Arguments:   none.
 * Return:      (void)
 * Uses:
 * Sets:
 * Changes:
 * Heap Use:    none.
 *****************************************************************************/

static void DotMovement(void)
{
    Uint8   ch;

    if (parse.mac.macroDef || parse.relative == REL_ERROR) {
	return;
    }

    ch = TOUPPER(parse.ch);

#ifdef PDI1_USING_COLOR
    if (ch == 'P') {
	if (IS_PDI1_USING_COLOR(CurrentDevice.colorFormat))
	{
	    parse.colorUsed = TRUE;   
	    if (! parse.rastXfer)			/* ignore in raster mode */
		HPPushPopPalette(parse.wholeVal);	/* <Esc>*p#P */
	    return;
	}
    }
#endif

    DoEndRasterGraphics();
    if (ch == 'Y') {
	SetVerPosition(UOMY_TO_PIUY * parse.wholeVal, parse.relative, 0);
    } else if (ch == 'X') {
	SetHorPosition(UOMX_TO_PIUX * parse.wholeVal, parse.relative);
    }
    else if (ch == 'R') {
	    /* Set the user fill pattern reference */
	if (parse.wholeVal > 1) {
	    return;
	}
	printer.anchorx_piu = HPGetAbsX();
	printer.anchory_piu = verCursor_piu;
	printer.rotatePat = parse.wholeVal;
/* Set the new anchor corner here, so hatches and udfp's will work */
	PDIUanchor (printer.anchorx_piu, printer.anchory_piu,
		    printer.rLimit_piu, printer.bLimit_piu,
		    printer.orient,  printer.printDir, printer.rotatePat);
	if (printer.rotatePat == 0 ) {
	   /*  rotate patterns with print direction */
	   PDIurotate(printer.printDir);
	   HPBRotate(printer.printDir);
	}
	else {
	   PDIurotate(RotNone);
	   HPBRotate(RotNone);
	}
    }
}


/*****************************************************************************
 * Synopsis:    void SwapMarginsAndVerCursor(oldPDir, newPDir)
 * Description: Swap the margins, based on the old and new print direction.
 *              Also change the 'verCursor_piu' value for the new coords.
 *              Also change the anchorx,y
 * Arguments:   (Sint32) oldPDir -- [0 .. 3] old orthogonal print direction.
 *              (Sint32) newPDir -- [0 .. 3] new     "        "       "
 * Return:      (void)
 * Uses:        HPGetAbsX().
 * Sets:
 * Changes:     printer.[lrtb]Margin, printer.[rb]Limit
 * Heap Use:
 *****************************************************************************/

void SwapMarginsAndVerCursor(Sint32 oldPDir, Sint32 newPDir)
{
    Sint32  oLMargin;
    Sint32  oRMargin;
    Sint32  oBMargin;
    Sint32  oTMargin;
    Sint32  oRLimit;
    Sint32  oBLimit;
    Sint32  tmpanch;
    Sint32  key;

    ASSERT(oldPDir >= 0 && oldPDir <= 3 && newPDir >= 0 && newPDir <= 3);

    if (oldPDir != newPDir) {

	/*
	 *  There a three possible swaps of margins because one can
	 *  only swap from the current print direction to one of the
	 *  other 3 print directions.
	 */
	oLMargin = printer.lMargin_piu;
	oTMargin = printer.tMargin_piu;
	oRMargin = printer.rMargin_piu;
	oBMargin = printer.bMargin_piu;
	oRLimit =  printer.rLimit_piu;
	oBLimit =  printer.bLimit_piu;

	key = newPDir - oldPDir;
	key = (key + 4) % 4;

	switch (key) {

	case 1:         /* 90 degree swap */
	    PDIgclose();
	    verCursor_piu = HPGetAbsX();
	    printer.tMargin_piu = oLMargin;
	    printer.lMargin_piu = oBLimit - oBMargin;
	    printer.bMargin_piu = oRMargin;
	    printer.rMargin_piu = oBLimit - oTMargin;
	    printer.rLimit_piu = oBLimit;
	    printer.bLimit_piu = oRLimit;
	    tmpanch = printer.anchorx_piu;
	    printer.anchorx_piu = oBLimit - printer.anchory_piu;
	    printer.anchory_piu = tmpanch;
	    break;

	case 2:         /* 180 degree swap */
	    PDIgclose();
	    verCursor_piu = oBLimit - verCursor_piu;
	    printer.tMargin_piu = oBLimit - oBMargin;
	    printer.lMargin_piu = oRLimit - oRMargin;
	    printer.bMargin_piu = oBLimit - oTMargin;
	    printer.rMargin_piu = oRLimit - oLMargin;
	    printer.anchory_piu = oBLimit - printer.anchory_piu;
	    printer.anchorx_piu = oRLimit - printer.anchorx_piu;
	    break;

	case 3:         /* 270 degree swap */
	    PDIgclose();
	    verCursor_piu = oRLimit  - HPGetAbsX(); 
	    printer.tMargin_piu = oRLimit - oRMargin;
	    printer.lMargin_piu = oTMargin;
	    printer.bMargin_piu = oRLimit - oLMargin;
	    printer.rMargin_piu = oBMargin;
	    printer.rLimit_piu = oBLimit;
	    printer.bLimit_piu = oRLimit;
	    tmpanch = printer.anchory_piu;
	    printer.anchory_piu = oRLimit - printer.anchorx_piu;
	    printer.anchorx_piu = tmpanch;
	    break;

# ifdef DEBUG
	default:
	    /* should never get here */
	    DBGFATAL();
	    break;
# endif
	}
	printer.wrLimit_piu = printer.rMargin_piu;
	if (printer.rMargin_piu == printer.rLimit_piu)
	    printer.wrLimit_piu += printer.one_uom_pel_in_piux;

	/*
	 *  Close off raster saving.  (We might just have done
	 *  this already, but extra calls are harmless.  -GAD)
	 *  KLUDGE ALERT!  May need to turn off raster mode.
	 */
	PDIgclose();
    }
}




/*****************************************************************************
 * Synopsis:    void CursorMargin()
 * Description: Peform cursor movements and margin settings.
 * Arguments:   none.
 * Return:      (void)
 * Uses:
 * Sets:
 * Changes:
 * Heap Use:    none.
 *
 *  Executes the escape sequence of the form <esc>&a#Z, where the value of
 *  Z is decoded. Handles left and right margin settings.  Also does cursor
 *  movements.
 *****************************************************************************/

static void CursorMargin(void)
{
    Sint32 tmp, tmp2;
    Uint8 ch;
    EFONT_NUM fn;

    if (parse.mac.macroDef || parse.relative == REL_ERROR) {
	/*
	 *  Either defining a macro or got too many signs in parsing.
	 */
	return;
    }

    ch = TOUPPER(parse.ch);

    switch (ch) {

    case 'L':           /* Esc&a#L set left margin */
	tmp2 = HPGetPitch(CHKFONT);
	tmp = tmp2 * parse.wholeVal;
	if (printer.rMargin_piu == printer.rLimit_piu) {
	    if (tmp <= printer.rLimit_piu) {
		printer.lMargin_piu = MAX(tmp, 0);
		if (printer.lMargin_piu > HPGetAbsX()) {
		    HPSetAbsX(printer.lMargin_piu);
		}
	    }
	}else if (tmp <= printer.wrLimit_piu) {
	    printer.lMargin_piu = MAX(tmp, 0);
	    if (printer.lMargin_piu > HPGetAbsX()) {
		HPSetAbsX(printer.lMargin_piu);
	    }
	}
	break;

    case 'M':           /* Esc&a#M set right margin */
	tmp = HPGetPitch(CHKFONT) * (parse.wholeVal + 1);
	if (tmp > printer.lMargin_piu) {
	    printer.rMargin_piu = MIN(tmp, printer.rLimit_piu);

	    if (HPGetAbsX() > printer.rMargin_piu) {
		HPSetAbsX(printer.rMargin_piu + printer.one_uom_pel_in_piux);
	    }
	    printer.wrLimit_piu = printer.rMargin_piu;
	    if (printer.rMargin_piu == printer.rLimit_piu)
		printer.wrLimit_piu += printer.one_uom_pel_in_piux;
	}
	break;

    case 'P':           /* Esc&a#P set print direction */
	tmp = ((Sint32) parse.wholeVal / 90) & 3;
	if (parse.wholeVal % 90 == 0) {
	    /* will only change 'printdir' if a multiple of 90 */
	    if (tmp != printer.printDir) {
		/*
		 *  The order of calling this below is important because
		 *  the SwapMargins...() function calls HPGetAbsX() which
		 *  depends upon 'printer.printDir' but we don't set
		 *  'printDir' until after the SwapMargins...() call.
		 *  PCL and spaghetti: two great tastes that go great together.
		 */
		if (printer.printDir & 1)
		    printer.rMargin_piu -= printer.one_uom_pel_in_piux;
		SwapMarginsAndVerCursor(printer.printDir, tmp);
		if (tmp & 1) {
		    printer.rMargin_piu += printer.one_uom_pel_in_piux;
		    printer.wrLimit_piu = printer.rMargin_piu;
		}
		printer.printDir = tmp;

		/*  The fact that the value has actually changed
		 *  will guarantee that PDI1 will update its internal state,
		 *  which will in turn allow HPSearchFont() to do what we want.
		 */
		HPTRotate(printer.printDir);

		/* Builtin patterns: */
		if (printer.rotatePat == 0) {
		    /* rotate patterns with print direction */
		    PDIurotate(printer.printDir);
		    HPBRotate(printer.printDir);
		}
		else {
		    PDIurotate(RotNone);
		    HPBRotate(RotNone);
		}
  
		/*  What we want is merely to see if the new orientation
		 *  might cause the font search algorithm to change its
		 *  tiny mind about the best font, now that its mind has
		 *  been enlightened about orientation issues.
		 *  Since these are only called if chooseFont is already
		 *  true the only net effect is to clear the mysterious
		 *  savePitch flag.
		 */
		if (primFont.chooseFont) {
		    fn = HPSearchFont(&primFont.desc, TRUE, TRUE);
		    if (fn != primFont.fontNum)
			HPSetChooseFont(&primFont, FALSE);
		}
		else if (secFont.chooseFont) {
		    fn = HPSearchFont(&secFont.desc, TRUE, TRUE);
		    if (fn != secFont.fontNum)
			HPSetChooseFont(&secFont, FALSE);
		}

		if (!PDIf->twobyte) {
		    /* or should we set chooseFont? */
		    PDIcharacterset(PDI1charset, PDI1cset); 
		}

		if (RotatedRaster) {
		    HPGRotate((printer.printDir + 3) % 4);
		} else {
		    HPGRotate(printer.printDir);
		}
	    }
	    /* set anchor and fill pattern for character here in case the
	     * very next thing is some printable characters (bug 16446)
	     */
	    PDIUanchor (printer.anchorx_piu, printer.anchory_piu,
		    printer.rLimit_piu, printer.bLimit_piu,
		    printer.orient,  printer.printDir, printer.rotatePat);
	    PDIcharpattern(options.modelPat);
	}
	break;

    case 'R':           /* Esc&a#R cursor move row */
			/* 4 decimal places */
	tmp = (parse.fractVal * printer.vmi_piu) / FRACTMUX;
	tmp += parse.wholeVal * printer.vmi_piu;
	if (parse.relative == REL_NOTREL) {
	    /*
	     *  Apparently, if not moving a relative # of columns we
	     *  make relative to the starting Y position at the
	     *  top of the page rather from the top of the logical
	     *  page.
	     */
	    tmp += (TOPVMIPERCENT * printer.vmi_piu) / 100;
	}
	SetVerPosition(tmp, parse.relative, 1);
	break;

    case 'V':           /* Esc&a#V cursor move vertical deci point */
			/* 2 decimal places */
	tmp = (100 * parse.fractVal) / FRACTMUX;
	tmp *= ONE_DPT_IN_PIUY;
	tmp /= 100;
	tmp += ONE_DPT_IN_PIUY * parse.wholeVal;
	/* disable units of measure during decipoint positioning otherwise
	 * it will attempt to round the position to the closest "unit".
	 */
	SetVerPosition(tmp, parse.relative, 0);
	break;

    case 'C':           /* Esc&a#C cursor move column */
			/* 4 decimal places */
    {
	FontState  *pState;
	
	/*
	 *  Make sure that the font is selected if any font
	 *  changes were made just prior to the command.
	 */
	if (options.primary) {
	pState = &primFont;
	} else {
	pState = &secFont;
	}
	HPMAYBECHOOSEFONT(pState);
	if ( printer.hmi_piu < 0  || !PDIf->fixed )
	{
	    tmp2 = HPGetPitch(CHKFONT);
	    tmp = (parse.fractVal * tmp2) / FRACTMUX;
	    tmp += parse.wholeVal * tmp2;
	}
	else
	{
	    tmp = (parse.fractVal * printer.hmi_piu) / FRACTMUX;
	    tmp += parse.wholeVal * printer.hmi_piu;
	}
	
	/* Turn on UOM rounding for movements by columns */
	PDI1EnableUnitsOfMeasure( TRUE );
	SetHorPosition(tmp, parse.relative);
	PDI1EnableUnitsOfMeasure( FALSE );
	break;
    }
    case 'H':           /* Esc&a#H cursor move horizontal deci point */
			/* 2 decimal places */
	tmp = (100 * parse.fractVal) / FRACTMUX;
	tmp *= ONE_DPT_IN_PIUX;
	tmp /= 100;
	tmp += ONE_DPT_IN_PIUX * parse.wholeVal;
	SetHorPosition(tmp, parse.relative);
	break;

    case 'G':        /* Esc & a # G duplex side selection (but you know */
		     /* the illegal ESC *p<n>G would get here too) -- */
	{            /* all models, but not always the same effect. */
#ifdef DUPLEXING
	Boolean dup_present;
#endif
	DUPLEX_SIDE side = (DUPLEX_SIDE) parse.wholeVal;

	if (side == SIDEnext || side == SIDEfront || side == SIDEback)
	    {
	/*  Even on a printer without a duplexer, this command always
	 *  does a form feed, which in turn may require the overlay macro.
	 *  (It turns out to also want a carriage return.)
	 *  Only if there really is a duplexer do we make the PDI call.
	 */
	    if (SOMETHINGONPAGE) {
		HPFormFeed(FFYESCR, FFNOEOJ);
		parse.firstInk = NOINKFF;
		}
#ifdef DUPLEXING
	    dup_present = OMGetCurrentBool(OMDUPLEXERPRESENT, OMSYSTEM);

	    if (dup_present)
		PDIduplexside(side);
#endif
	    }
	/* else ignore the illegal parameter */
	}
	break;

    case 'W':           /* Esc&a<n>W -- undefined W command */
	UndefCommand();
	break;

    } /* switch */
}


/*****************************************************************************
 * Synopsis:    void UnderlineOnOff()
 * Description: Turns underlining on and off.
 * Arguments:   none.
 * Return:      (void)
 * Uses:
 * Sets:
 * Changes:
 * Heap Use:    none.
 *
 *  Executes the escape sequence of the form <esc>&d@ and <esc>&d#D
 *  Determines if underline should be on or off.
 *****************************************************************************/

static void UnderlineOnOff(void)
{
    Uint8   ch;

    if (parse.mac.macroDef) {
	return;
    }

    ch = TOUPPER(parse.ch);

    if (ch == '@')  {
	PDIunderspace(0);           /* underline off */
	PDIunderline(UNDERnone);
	options.underline = 0;
    } /* else if (ch == 'D' && !options.underline) { */
      else if (ch == 'D' ) {
      switch(parse.wholeVal)
	{

	 case 2 :
	 case -2:
	    PDIunderspace(1);
	    PDIunderline(UNDERdoublefixed);
	    options.underline = UNDERdoublefixed; 
	    break;
	 case 3 :
	 case -3:
	    PDIunderspace(1);
	    PDIunderline(UNDERfloat);
	    options.underline = UNDERfloat; 
	    break;
	 case 4 :
	 case -4:
	    PDIunderspace(1);
	    PDIunderline(UNDERdoublefloat);
	    options.underline = UNDERdoublefloat; 
	    break;
	default :

            /* (parse.wholeVal should == 0, but any value is OK)  */
	    /* fixed underline, always 5 dots below baseline and 3 dots wide */
	    PDIunderspace(1);
	    PDIunderline(UNDERsingle);
	    options.underline = UNDERsingle;
	    break;
	}
    }
}


/*****************************************************************************
 * Cursor Stack Routines
 *****************************************************************************/


/*****************************************************************************
 * Synopsis:    void PushCursor()
 * Description: Push cursor position on the stack.
 * Arguments:
 * Return:
 * Uses:
 * Sets:
 * Changes:
 * Heap Use:    none.
 *****************************************************************************/

static void PushCursor(void)
{
    if (parse.curStkPos < CURSTKSIZ) {
	parse.verCurStk[parse.curStkPos] = verCursor_piu;
	parse.horCurStk[parse.curStkPos] = HPGetAbsX();
	parse.printDirStk[parse.curStkPos] = printer.printDir;
	parse.curStkPos += 1;
    }
}


/*****************************************************************************
 * Synopsis:    void PopCursor()
 * Description: Pop old cursor position from the stack.
 * Arguments:   none.
 * Return:      (void)
 * Uses:
 * Sets:
 * Changes:
 * Heap Use:    none.
 *****************************************************************************/

static void PopCursor(void)
{
    Sint32 xpos_piu;
    Sint32 SavedPrintDir;
    Sint32 CurPrintDir;

    if (parse.curStkPos > 0) {
	parse.curStkPos -= 1;
	/*
	 * Not only is the cursor location important, but the print direction
	 * that it was saved under must be used to get the absolute 
	 * location correct
	 */
	CurPrintDir = printer.printDir;
	SavedPrintDir = parse.printDirStk[parse.curStkPos];
	SwapMarginsAndVerCursor(CurPrintDir,SavedPrintDir);
	printer.printDir = SavedPrintDir;
	xpos_piu = parse.horCurStk[parse.curStkPos];
	verCursor_piu = parse.verCurStk[parse.curStkPos];
	HPSetAbsY(verCursor_piu);
	HPSetAbsX(xpos_piu);
	SwapMarginsAndVerCursor(SavedPrintDir,CurPrintDir);
	printer.printDir = CurPrintDir;
	PDIgclose();

	if (!SOMETHINGONPAGE) {
	    parse.firstInk = NOINKMOVED;
	    HPCheckPageChar();
	}
	/* the right limit test printer.wrLimit_piu depends upon whether the
	 * the x position is beyond the right margin or not so
	 * characters beyond the right margin will print
	 */
	if (xpos_piu > printer.rMargin_piu)
	    printer.wrLimit_piu = printer.rLimit_piu + printer.one_uom_pel_in_piux;
	else {
	    printer.wrLimit_piu = printer.rMargin_piu;
	    if (printer.rMargin_piu == printer.rLimit_piu)
		printer.wrLimit_piu += printer.one_uom_pel_in_piux;
	}
	/* if the position is out of page, move it to the appropriate limit */
	if (verCursor_piu > printer.bLimit_piu) {
	    verCursor_piu = printer.bLimit_piu + printer.one_uom_pel_in_piuy;
	    HPSetAbsY(verCursor_piu);
	}
	if (xpos_piu > printer.rLimit_piu) {
	    xpos_piu = printer.rLimit_piu + printer.one_uom_pel_in_piux;
	    HPSetAbsX(xpos_piu);
	}
    }
}


/*****************************************************************************
 * Synopsis:    void PushPopMacro()
 * Description: Interpret and perform push/pop cursor stack.
 * Arguments:   none.
 * Return:      (void)
 * Uses:
 * Sets:
 * Changes:
 * Heap Use:    none.
 *
 *  Executes the escape sequence of the form <esc>&f#Z, where Z is decoded.
 *  Performs pushes and pops of the cursor and also does macro commands.
 *
 *  The only command below allowable during macro definition is the
 *  end-macro command.
 *
 *  Many of the commands are not allowed during macro invocation.
 *****************************************************************************/

static void PushPopMacro(void)
{
    Uint8 ch = TOUPPER(parse.ch);

    if (ch == 'S') {
	if (!parse.mac.macroDef) {
	    if (parse.wholeVal == 0) {
		PushCursor();
	    } else if (parse.wholeVal == 1) {
		PopCursor();
	    }
	}
    } else if (ch == 'Y') {
	if (!parse.mac.macroDef) {
	    if (options.macroID.idType == STRINGID &&
	    options.macroID.name.stringID != NULL) {
		PDI1FREE(options.macroID.name.stringID);
		options.macroID.name.stringID = NULL;
	    }
	    parse.wholeVal = MIN(parse.wholeVal, 32767);
	    options.macroID.idType = NUMERICID;
	    options.macroID.name.numericID = parse.wholeVal;
	}
    } else if (ch == 'X') {

	switch (parse.wholeVal) {

	case 0:         /* Esc&f0X start macro definition */
	    if (!parse.mac.macroDef && !parse.mac.macroLevel) {
		/* tell if this is middle of combined command */
		HPDefineMacro(options.macroID, islower(parse.ch) );
	    }
	    break;

	case 1:         /* Esc&f1X end macro definition or execution */
	    if (parse.mac.macroDef || parse.mac.macroLevel > 0) {
		HPEndMacro();
	    }
	    break;

	case 2:         /* Esc&f2X exec macro */
	    if (!parse.mac.macroDef) {
		HPReadMacro(options.macroID, MACROEXEC);
	    }
	    break;

	case 3:         /* Esc&f3X call macro save options */
	    if (!parse.mac.macroDef) {
		HPReadMacro(options.macroID, MACROCALL);
	    }
	    break;

	case 4:         /* Esc&f4X enable the overlay macro */
	    if (!parse.mac.macroDef && !parse.mac.macroLevel) {
		HPMacroHead *pList;
		/* Check if macro is defined */
		/* the 2nd argument: TRUE ==> compare with associated ID if this macro
		 * is defined using Alphanumeric command (originalStringID!=NULL).  
		 * Otherwise, the 2nd argument is ignored. */
		pList = HPMacFind(options.macroID, TRUE);
		if (pList)
		    parse.mac.overlayID = options.macroID;
		else
		    parse.mac.overlayID.idType = INVALID_ID;/*disable for bad selection*/
	    }
	    break;

	case 5:         /* Esc&f5X disable the overlay macro */
	    if (!parse.mac.macroDef && !parse.mac.macroLevel) {
	if( parse.mac.overlayID.idType != INVALID_ID)
	    HPMacSetOvly(parse.mac.overlayID, FALSE);
	parse.mac.overlayID.idType = INVALID_ID;
	    }
	    break;

	case 6:         /* Esc&f6X delete all macros */
	    if (!parse.mac.macroDef && !parse.mac.macroLevel) {
		HPMacDelThem(All_DL_Macros);
	    }
	    break;

	case 7:         /* Esc&f7X delete all temporary macros */
	    if (!parse.mac.macroDef && !parse.mac.macroLevel) {
		HPMacDelThem(Temp_DL_Macros);
	    }
	    break;

	case 8:         /* Esc&f8X delete macro */
	    if (!parse.mac.macroDef && !parse.mac.macroLevel) {
		HPMacDelete(options.macroID);
	    }
	    break;

	case 9:         /* Esc&f9X make macro temporary */
	    if (!parse.mac.macroDef && !parse.mac.macroLevel) {
		HPMacMkTemp(options.macroID);
	    }
	    break;

	case 10:        /* Esc&f10X make macro permanent */
	    if (!parse.mac.macroDef && !parse.mac.macroLevel) {
		HPMacMkPerm(options.macroID);
	    }
	    break;

	} /* switch */
    } else if (ch == 'W') {    /* swallow ESC & f <n> W */
	UndefCommand();
    }
}

/*****************************************************************************
 * Synopsis:    void HPSelectPitch()
 * Description: Select fixed pitch for fixed pitch fonts
 * Arguments:   0 == 10 char /in
 *              2 == 16.66 char/in
 *              4 == 12 char/in  (from HP PCL-5 Comparison Guide pA-5)
 * Return:      (void)
 * Uses:
 * Sets:
 * Changes:
 * Heap Use:
 *
 *  Executes the escape sequence of the form <esc>&k#Z, where Z is decoded.
 *  Determines the line termination type, HMI, and whether to use compressed
 *  or non compressed mode.
 *****************************************************************************/

void HPSelectPitch(int ptch)
{
    if (ptch == 0) {
	/*
	 *  10 chars / in.
	 */
	primFont.desc.pitch = secFont.desc.pitch = HIGH_RES/10;
    } else if (ptch == 2) {
	/*
	 *  16.66 char / in.
	 */
	primFont.desc.pitch = secFont.desc.pitch = (3 * HIGH_RES) / 50;
    } else if (ptch == 4) {
	/*
	 *  12 char / in.
	 */
	primFont.desc.pitch = secFont.desc.pitch = HIGH_RES/12;
    }

    /* AlphanumericID command. */
    primFont.descSave.pitch = primFont.desc.pitch;

    if (ptch==0 || ptch==2 || ptch==4) {
	HPSetChooseFont(&primFont, 0);
	HPSetChooseFont(&secFont, 0);
	HPResetBackspace();
    }
}



/*****************************************************************************
 * Synopsis:    void TermPitchHmi()
 * Description: Interprets and sets line termination mode, HMI, and whether or
 *              not to use compressed mode.
 * Arguments:   none.
 * Return:      (void)
 * Uses:
 * Sets:
 * Changes:
 * Heap Use:
 *
 *  Executes the escape sequence of the form <esc>&k#Z, where Z is decoded.
 *  Determines the line termination type, HMI, and whether to use compressed
 *  or non compressed mode.
 *****************************************************************************/

static void TermPitchHmi(void)
{
    Sint32      tmp;
    Uint8       ch;

    if (parse.mac.macroDef) {
	return;
    }

    ch = TOUPPER(parse.ch);

    switch (ch) {

    case 'G':           /* Esc&k#G line termination */
	if (parse.wholeVal >= 0 && parse.wholeVal <= 3) {
	    options.lineTerm = (Sint8) parse.wholeVal;
	}
	break;

    case 'S':           /* Esc&k#S pitch selection */
	HPSelectPitch(parse.wholeVal);
	break;


    case 'H':           /* Esc&k#H set hmi */
	if (parse.wholeVal < 32767) {
	    FontState  *pState;

	    /*
	     *  Make sure that the font is selected if any font
	     *  changes were made just prior to the HMI command.
	     */
	    if (options.primary) {
		pState = &primFont;
	    } else {
		pState = &secFont;
	    }
	    HPMAYBECHOOSEFONT(pState);

	    /*
	     *  Set HMI in PIU_XDPI coords.
	     */
	    tmp = parse.wholeVal * PIU_XDPI;
	    tmp += ((parse.fractVal * PIU_XDPI) + (FRACTMUX / 2)) / FRACTMUX;
	    tmp = (tmp + (120 / 2)) / 120;
	    printer.hmi_piu = tmp;    /* Save the hmi */
	    if ( tmp < 0 )
		tmp = -tmp;

	PDIspacingmode(FIXEDSPACE, tmp);
        printer.keepHMI = 1;    /* GZ keep this HMI at EndMacro after font reselection */ 

	    /* Because this is how the target seems to work */
	    /* When the font is proportional, further changes to HMI    */
	    /* are not reflected in the default backspace, but changes  */
	    /* made before calling for a proportional font remain in    */
	    /* place.  The useDefaultBksp flag is of minimal importance */
	    /* in the fixed case, because whether its the default or a  */
	    /* real character, they have the same width, and they are   */
	    /* treated the same for backspacing and overstriking.       */
	    if (PDIf->fixed)
		HPResetBackspace();
	}
	break;

    case 'W':           /* Esc&k<n>W -- undefined W command */
	UndefCommand();
	break;
    }
}


/*****************************************************************************
 * Synopsis:    void HPNewOrient(newOrient)
 * Description: Change to a new orientation.
 * Arguments:   (Sint32) newOrient -- orientation to change to.
 * Return:      (void)
 * Uses:
 * Sets:
 * Changes:
 * Heap Use:    none.
 *****************************************************************************/

void HPNewOrient(Sint32 newOrient)
{
    EFONT_NUM fn;
    int        vmi_piu;

    if (SOMETHINGONPAGE) {
	HPFormFeed(FFNOCR, FFNOEOJ);

	/* The HP LJ3 does not treat this as a full-fledged form-feed.
	 * The top margin, vmi / line-spacing are NOT reset immediately.
	 * The LJ3 state is similar to NOINKMOVED, not NOINKFF.  -BAM, WPC
	 */
	parse.firstInk = NOINKMOVED;
    }

    /*
     *  Get the VMI value from the default value kept in 1200 dpi.
     */
    OMgetInt(omPCLVMI, OMCURRENT, &vmi_piu);
    printer.vmi_piu = ONE_INCH_IN_PIUY/1200 * vmi_piu;

    printer.orient = newOrient;

    /* Befor "zeroing" print direction, ALWAYS call PDIcharacterset to restore 
     * character set and empty cache shortcut table.
     * Setting print direction to zero here is not really good, becuase it will
     * show a "no print direction change" if the print direction is set to 0
     * using the escape sequence: <Esc>&a0P following ANY orientation command.
     * (This fixes 15456 and 15457).
     */
    PDIcharacterset(PDI1charset, PDI1cset);
    printer.printDir = 0;

    PDIorient(newOrient);
    if (newOrient % 2 && options.graphicMode3) {
	HPGRotate(3);
    } else {
	HPGRotate(0);
    }
    PDItrotate(RotNone);
    PDIbrotate(RotNone);

    if (primFont.chooseFont) {
	fn = HPSearchFont(&primFont.desc, TRUE, TRUE);
	if (fn != primFont.fontNum)
	    HPSetChooseFont(&primFont, FALSE);
    }
    else if (secFont.chooseFont) {
	fn = HPSearchFont(&secFont.desc, TRUE, TRUE);
	if (fn != secFont.fontNum)
	    HPSetChooseFont(&secFont, FALSE);
    }

    PDIspacingmode(PROPORTIONAL, -1);

    parse.mac.overlayID.idType = INVALID_ID;
    HPResetPageChar(0,FALSE);
    /* The anchor corner default is reset when we change orientation */
    printer.anchorx_piu = 0;
    printer.anchory_piu = 0;
}



/*****************************************************************************
 * Synopsis:    void LpiLenMargin()
 * Description: Commands for setting up page parameters.
 * Arguments:   none.
 * Return:      (void)
 * Uses:
 * Sets:
 * Changes:
 * Heap Use:    ?
 *
 *  Executes the escape sequence of the form <esc>&l#Z, where Z is decoded.
 *****************************************************************************/
static void LpiLenMargin(void)
{
    Sint32  tmp,tmp1;
    Sint32  newbLimit;
    Uint8   ch;
    PAPERINFO (* papers)[];
    PAPERINFO * paperInfo;
    PAPERCODE size;
    PAPERSOURCE tray;

#if 0 /* GZ */
#ifdef DUPLEXING
    Boolean dup_present;
    Boolean dup_mode;
    e_BindingEdge  binding;
    DUPLEX_MODE mode = DUPsimplex;
#endif
#endif

    if (parse.mac.macroDef || parse.relative == REL_ERROR) {
	return;
    }

    ch = TOUPPER(parse.ch);

    switch (ch) {

    case 'A':       /* Esc&l#A select page size */

	/*
	 *  Do a form feed, regardless of whether or not the page
	 *  size requested is the same as current or even completely bogus.
	 */
	if (SOMETHINGONPAGE) {
	    HPFormFeed(FFNOCR, FFNOEOJ);
	    parse.firstInk = NOINKFF;
	}

	if (parse.wholeVal == 0)        /* stay with current paper size */
	    return;

#ifndef NEW_PCL_TO_PR
	/* Convert the PCL command value to an OM enum paper code
	*/
	size = HPPclToPapers(parse.wholeVal);
	if (size)
	  HPSetPaperSize(size);
#endif

/* GZ paper size change does not mean to read the duplex default values */
#if 0
#ifdef DUPLEXING
	OMgetBool(omDUPLEXERPRESENT, OMCURRENT, &dup_present);
	OMgetEnum(omBINDINGEDGE, OMCURRENT, &binding);
	OMgetBool(omDUPLEX, OMCURRENT, &dup_mode);

	if (!PDIpaper.noduplex) { /* if not "noduplex", set correct duplex mode */
	    if (dup_present) {
		if (dup_mode)
		    mode = (binding == eShortEdge) ? DUPshort : DUPlong;
		/* else it remains DUPsimplex */
	    }
	}

	PDIsetduplex(mode); 
#endif
#endif

	/*
	 *  When paper size changes, even if to the same size
	 *  use the current print direction and orientation as new
	 *  orientation, and reset print direction 
	 */
	tmp = (printer.orient + printer.printDir) % 4;
	if (printer.orient != tmp) {
	    HPNewOrient(tmp);
	    }
	HPResetPageChar(0,FALSE); /* GZ do not allow default duplex values with TRUE */
	parse.mac.overlayID.idType = INVALID_ID;
	break;

    case 'C':       /* Esc&l#C select VMI */
	/*
	 *  4 decimal places in PIU_YDPI units; wholeVal in 1/48in.
	 */
	tmp = parse.wholeVal * PIU_YDPI;
	tmp += (parse.fractVal * PIU_YDPI) / FRACTMUX;
	tmp /= 48;

	/* Rounding the fractional part for accurracy to match HP's target: 
	 * see bug# 26146 */
	tmp1 = (parse.wholeVal * PIU_YDPI * FRACTMUX) + 
	            parse.fractVal * PIU_YDPI - tmp*FRACTMUX*48;
	tmp1 /= 48; 
	 
	if (2*tmp1 >= FRACTMUX)
	    tmp += 1;

    if (tmp <= printer.bLimit_piu) {
          tmp1 = printer.vmi_piu;
          printer.vmi_piu = tmp;
          /* Fix for #34664 and #17460 */
          if (( tmp < tmp1) &&
            (!SOMETHINGONPAGE && parse.firstInk != NOINKMOVED )) {
             HPCursorFirstLine();
          }   
    }
	break;

    case 'D':       /* Esc&l#D select lpi */
	switch (parse.wholeVal) {

	case 0:
	    /*
	     *  12 lines / in.
	     */
	    parse.wholeVal = 12;
	    /* NOTE! NO BREAK HERE INTENTIONALLY! */
	case 1:
	case 2:
	case 3:
	case 4:
	case 6:
	case 8:
	case 12:
	case 16:
	case 24:
	case 48:
	    printer.vmi_piu = ONE_INCH_IN_PIUY / parse.wholeVal;
	    break;
	}  /* switch */

	if (parse.firstInk == NOINKESCE)
	    HPCursorFirstLine();
	break;

    case 'E':               /* Esc&l#E set top margin */
	if (printer.vmi_piu != 0) {
	    tmp = parse.wholeVal * printer.vmi_piu;
	    if (tmp <= printer.bLimit_piu) {
		printer.tMargin_piu = tmp;
		printer.bMargin_piu = printer.bLimit_piu - ONE_INCH_IN_PIUY/2;
		if (printer.bMargin_piu <= printer.tMargin_piu)
		    /* The margin needs to be deleted */
		    printer.bMargin_piu = printer.bLimit_piu;
		if (parse.firstInk == NOINKESCE ){
		    HPCursorFirstLine();
		}
	    }
	}
	break;

    case 'F':               /* Esc&l#F set text length */
	if (parse.wholeVal == 0) {
	    /*
	     *  Set to default text length.  **** NOTE: not in LJ manual ***.
	     */
	    printer.bMargin_piu = printer.bLimit_piu - ONE_INCH_IN_PIUY/2;
	} else {
	    /*
	     *  Set new text length if not larger than physical page.
	     */
	    tmp = parse.wholeVal;       /* count of lines of text */
	    if (tmp < 0)
		tmp = - tmp; /* Treat negative text length as positive
			      * *** Not in LJ Manual either ***
			      */
	    tmp = tmp * printer.vmi_piu;
	    if (tmp>0 && tmp <= printer.bLimit_piu + printer.one_uom_pel_in_piuy 
			    - printer.tMargin_piu) {
		printer.bMargin_piu = printer.tMargin_piu + tmp - printer.one_uom_pel_in_piuy;
	    }
	}
	break;

    case 'G':           /* ESC & l <n> G -- LJIIISi output bin selection
			   1 = upper, 2 = lower;  someday more? */
	PDIsetbin(parse.wholeVal - 1);
	break;

    case 'H':           /* Esc&l#H change paper source, eject page
			 * Here we largely ignore the parameter, and
			 * ask Printer State to distingush among these
			 * possible values:     0 - eject
			 *                      1 - upper tray
			 *                      2 - manual feed
			 *                      3 - manual feed envelope
			 * (IIID, IIISi, IIIP)  4 - lower tray
			 * (optional paper)     5 - paper deck
			 * (IIID, IIISi)        6 - envelope feeder
			 */
	if (SOMETHINGONPAGE) {
	    HPFormFeed(FFNOCR, FFNOEOJ);
	}
	HPCarriageReturn();
	HPCursorFirstLine();
	HPCheckPageChar();

#ifdef DUPLEXING
    /*  This command is supposed to force the
     *  next page to go to the front side.
     */
	PDIsetduplex(PDIgetduplexmode());
#endif

	switch (parse.wholeVal) {
	    case 1: tray = eTray1; break;
	    case 2: tray = eTrayManualFeed; break;
	    case 0: tray = eEject; break;    
	    case 3: tray = eTrayManualFeedEnvelope; break;
#ifdef LJ4MVK
	    case 4: tray = eTrayManualFeed; break;
	    case 5: tray = eTrayOptional; break;
#endif

	/*  case 6: tray = eEnvelopeTray; break; */
	    default: tray = 0; break;
	}
	if (tray) {
	  HPSetPaperSource(tray);
	}
	
	break;

    case 'L':           /* Esc&l#L set perforation skip mode */
	if (parse.wholeVal == 0 || parse.wholeVal == 1) {
	    options.perfSkip = parse.wholeVal;
	}
	break;

    case 'O':           /* Esc&l#O set orientation */
	if (parse.wholeVal >= 0 && parse.wholeVal <= 3 &&
				   parse.wholeVal != printer.orient) {
	    HPNewOrient(parse.wholeVal);
	}
	break;

    case 'P':           /* Esc&l#P set page length */
	/*  Executing the overlay macro will annihilate parse.wholeVal,
	 *  so start to compute length from line count and VMI.
	 */
	/*  This line was brought out of the following "if" body so that
       	we can add a test of its value to the "if" statement. There was a
       	test file with a few Esc&l0P's resulting in a negative newbLimit,
       	so we didn't find the proper size for it and ended up changing to
       	default size. HP seems to ignore this command and do not change
       	the size. Now we match this behavior.
   	*/
	newbLimit = parse.wholeVal * printer.vmi_piu - printer.one_uom_pel_in_piuy;

	if (/* printer.vmi_piu &&        ...not really needed.               */
		parse.relative != REL_NEG /* ...we don't take a negative value   */
		&& newbLimit > 0          /* ...added test. HPPclCodeFromLength()
						won't use a smaller size anyway  */
		) {
	    /*
	    * Do a formfeed here before resetting the overlay Macro
	    * Do it here first because the paper change can effect the
	    * orientation and therefore what the overlay macro will print
	    */
	    if (SOMETHINGONPAGE) {
		HPFormFeed(FFNOCR, FFNOEOJ);
	    }
	    parse.mac.overlayID.idType = INVALID_ID;

	    /*
	     *  When paper length changes, even if to the same length
	     *  use the combination print direction and orientation as new
	     *  orientation, and reset print direction , This must be done
	     *  before the paper code check.
	     */
	    tmp = (printer.orient + printer.printDir) % 4;
	    if (printer.orient != tmp) {
		HPNewOrient(tmp);
		}

	    /* return the paper code from the new length
	     * if the new length is bad, it will return PCLPAPERNOT
	    */
	    tmp1 = HPPclCodeFromLength(newbLimit);
	    /*
	     * Set the paper code to the default size if bad length
KLUGE ALERT!!!!!!!!  Who says the default is the first entry?
	      */
	    if (tmp1 == PCLPAPERNOT) {
		tmp = PRpapertable(&papers, PDI1res.x, PDI1res.y);
		paperInfo = &(*papers)[0];
		size  = paperInfo->PCLpaperID;
	    }
	    else {
		size = tmp1;
	    }

#ifndef NEW_PCL_TO_PR
	    /* Convert the PCL command value to an OM enum paper code
	     */
	    size = HPPclToPapers(size);
	    if (size)
	      HPSetPaperSize(size);
#endif

	    if (tmp1 == PCLPAPERNOT)
		HPResetPageChar(0,FALSE); /* GZ do not allow default duplex values with TRUE */
	    else
		HPResetPageChar(newbLimit,FALSE); /* GZ do not allow default duplex values with TRUE */

	    /*
	     * have to special case the default limit here 
	    */
	    if (newbLimit == printer.bLimit_piu)
		printer.bMargin_piu = printer.bLimit_piu - ONE_INCH_IN_PIUY/2;
	}
	/*  A form feed may have to be done here for VMI == 0 or 
	 *  negative page length
	 */
	 if (SOMETHINGONPAGE) {
	    HPFormFeed(FFNOCR, FFNOEOJ);
	}
	break;

    case 'S':           /* Esc & l # S duplex mode command -- */
	{               /* all models, but not always the same effect. */
#ifdef DUPLEXING
	Boolean Dup_present;
	Boolean Dup_mode;
	e_BindingEdge Binding;
#endif
	DUPLEX_MODE Mode = (DUPLEX_MODE) parse.wholeVal;
	if (Mode == DUPsimplex || Mode == DUPlong || Mode == DUPshort)
	    {
	    if (SOMETHINGONPAGE)
		{
	    /*  Never mind whether there's actually a duplexer or not,
	     *  this command always ejects the current page.  It also
	     *  implicitly and unconditionally does a carriage return.
	     */
		HPFormFeed(FFYESCR, FFNOEOJ);    /* do the overlay macro */
		parse.firstInk = NOINKFF;
		}
#ifdef DUPLEXING
	    Dup_present = OMGetCurrentBool(OMDUPLEXERPRESENT, OMSYSTEM);
#endif
	/*  If there is no duplexer installed, our duty is done.
	 *  If there is, then we have not only to tell PDI about it,
	 *  but we have to parse the command value into the two
	 *  separate options.  (Having separate options helps to
	 *  maintain sanity in PostScript.)  These are set in
	 *  the temporary values, but not actually in NVRAM.
	 *
	 *  If this paper type is not eligible for duplexing,
	 *  tell PDI to do simplex, but leave the options alone.
	 */
#ifdef DUPLEXING
	    if (Dup_present) {
		if (Mode == DUPsimplex)
		    Dup_mode = FALSE;
		else {
		    Dup_mode = TRUE;
		    if (Mode == DUPshort)
			Binding = eShortEdge;
		    else
			Binding = eLongEdge;

		    /* OMSetCurrentEnum(OMBINDINGEDGE, OMSYSTEM, Binding); GZ */
		    }
		/* OMSetCurrentBool(OMDUPLEX, OMSYSTEM, Dup_mode); GZ */

		if (PDIpaper.noduplex)
		    Mode = DUPsimplex;

		PDIsetduplex(Mode);
		}
#endif
	    }
	/* else ignore the illegal parameter */
	}
	break;

#if 0   /* Disabled pr Tom Todd */
    case 'T':           /* ESC & l 1 T -- mechanical job separator */
	if (parse.wholeVal == 1)
	    PDIjog();
	break;
#endif

    case 'U':           /* Esc&l#U left offset registration in decipts */
    case 'Z':           /* Esc&l#Z top offset registration in decipts */
	tmp = parse.fractVal * 100;
	tmp /= FRACTMUX;
	tmp += parse.wholeVal * 100;
	if (parse.relative == REL_NEG) {
	    tmp = -tmp;
	}
	if (ch == 'U') {
	    tmp /= ONE_DPT_IN_PIUX;
	    PDIsetXoffset(tmp);
	} else {
	    tmp /= ONE_DPT_IN_PIUY;
	    PDIsetYoffset(tmp);
	}
	break;

    case 'X':           /* Esc&l#X set number of copies */
	if (parse.wholeVal > 0 && parse.wholeVal <= MAXCOPIES
	    && parse.wholeVal != options.nCopies) {
	    int copies;
	    copies = options.nCopies = parse.wholeVal;
	    PDIsetcopycount(options.nCopies);
	}
	break;

	/* PDI1Collation = {1,-1}, 1 = off, -1  = on */
	/* This is Xionics proprietary for testing mfp collation */
	/* This is not involved with PDIcollation or SOFTCOLLATION */
    case 'Y':           /* Esc&l<n>Y -- collation: n = 0 means off, else on */
	{
	    extern int PDI1Collation;
	    if (parse.wholeVal == 0) PDI1Collation = 1;
	    else PDI1Collation = -1;
	}
	break;

    case 'W':           /* Esc&l<n>W -- undefined W command */
	UndefCommand();
	break;

    } /* switch */
}


/*****************************************************************************
 * Synopsis:    void TransparentPrint()
 * Description: Interprets and flags that transparent data is about to come.
 * Arguments:   none.
 * Return:      (void)
 * Uses:
 * Sets:
 * Changes:
 * Heap Use:
 *
 *  Executes the escape sequence of the form <esc>&p#Z, where Z is decoded.
 *  The state variable is changed to indicate that transparent data is to
 *  follow.
 *****************************************************************************/

static void TransparentPrint(void)
{
    Uint8   ch;

    ch = TOUPPER(parse.ch);

    if (ch == 'X' && parse.wholeVal > 0) {
	parse.state = Trans;
	parse.stateSet = 1;
	parse.nDataBytes = parse.nBytesToGo = parse.wholeVal;
    }

#ifdef PDI1_USING_COLOR
    else if (!IS_PDI1_USING_COLOR(CurrentDevice.colorFormat)) return;

    else { 
	if (parse.mac.macroDef || parse.relative == REL_ERROR) { 
	    return; 
	}    
	switch (ch) {
	case 'C':   /* <Esc>&p#C */
	    parse.colorUsed = TRUE;
	    /* The command is to be ignored for invalid palette ids */
	    if ((parse.wholeVal !=2 && parse.wholeVal != 6) /* these don't use ID */
		|| options.PalControlID != INVALID_PALID)
		HPPaletteControl(parse.wholeVal);
	    break; 
	case 'I':   /* <Esc>&p#I */
	    parse.colorUsed = TRUE;
	    if (parse.relative != REL_NEG
		&& (parse.wholeVal >= 0 && parse.wholeVal <= 32767))
		options.PalControlID = parse.wholeVal;
	    else   
		options.PalControlID = INVALID_PALID;
	break;   
	case 'S':   /* <Esc>&p#S */
	    parse.colorUsed = TRUE;
	    if (parse.relative != REL_NEG
		&& (parse.wholeVal >= 0 && parse.wholeVal <= 32767))
		HPSelectPalette((Sint16)parse.wholeVal);
	break;   
	}    
    }
#endif

}


/*****************************************************************************
 * Synopsis:    void FlushPages()
 * Description: Interprets and executes the Flush Pages command
 * Arguments:   none.
 * Return:      (void)
 * Uses:
 * Sets:
 * Changes:
 * Heap Use:
 *
 *  Executes the escape sequence of the form <esc>&r#Z, where Z is decoded.
 *****************************************************************************/

static void FlushPages(void)
{
    Uint8 ch = TOUPPER(parse.ch);

    if (parse.mac.macroDef)
	return;

    if (ch == 'F') {
	if (parse.wholeVal == 1)
	    {
	    /* flush all pages */
	    if (SOMETHINGONPAGE) {
		HPFormFeed(FFYESCR, FFNOEOJ);
		parse.firstInk = NOINKFF;
#ifdef DUPLEXING
		if (PDIneedbackside())
		    HPFormFeed(FFYESCR, FFNOEOJ);
#endif
	    }
#ifdef OLD_PCL_DISPLAYLIST_CODE
	    /* Wait till all display lists printed */
	    DLsync();
#endif
		} else if (parse.wholeVal == 0){
#ifdef OLD_PCL_DISPLAYLIST_CODE
	    /* Wait till all display lists printed */
	    DLwait();
#endif
		}
    }
     else       /* just to swallow Esc&r<n>W -- undefined W command */
	 UndefCommand();
}


/*****************************************************************************
 * Synopsis:    void OnOffLineWrap()
 * Description: Turns line wrap on or off.
 * Arguments:   none.
 * Return:      (void)
 * Uses:
 * Sets:
 * Changes:
 * Heap Use:
 *
 *  Executes the escape sequence of the form <esc>&s#C
 *  Turns line wrap on or off.
 *****************************************************************************/

static void OnOffLineWrap(void)
{
    Uint8   ch;

    if (parse.mac.macroDef) {
	return;
    }

    ch = TOUPPER(parse.ch);

    if (ch == 'C') {
	if (parse.wholeVal == 0 || parse.wholeVal == 1) {
	    options.lineWrap = !parse.wholeVal;
	}
    printer.wrLimit_piu = printer.rMargin_piu;
    if (printer.rMargin_piu == printer.rLimit_piu)
	printer.wrLimit_piu += printer.one_uom_pel_in_piux;
    }
    else        /* just to swallow Esc&s<n>W -- undefined W command */
	UndefCommand();
}



/*****************************************************************************
 * Synopsis:    void FontManagement()
 * Description: Executes the font management commands.
 * Arguments:
 * Return:
 * Uses:
 * Sets:
 * Changes:
 * Heap Use:
 *
 *  Executes the escape sequence of the form <esc>(s#Z or )s#Z, where Z is
 *  decoded.  Executes the font management commands.
 *****************************************************************************/

static void FontManagement(void)
{
    int  tmp;
    Sint32  fontChange;
    Sint32  savePitch;
    Uint8   ch;

    if (parse.mac.macroDef && TOUPPER(parse.ch) != 'W') {
	return;
    }

    /*
     *  The flags 'fontChange' and 'savePitch' are only changed below
     *  when they are set.
     */
    fontChange = 0;
    savePitch  = 0;

    ch = TOUPPER(parse.ch);

    switch (ch) {

    case 'B':               /* Esc(s#B set stroke weight */
	tmp = MIN(parse.wholeVal, 7);
	if (parse.relative == REL_NEG) {
	    tmp = -tmp;
	}
	fontChange = 1;
#if 0
	savePitch = 1;    /* how PCL4 used to be */
#endif
        if (parse.primFont) {
            primFont.desc.weight = tmp;
	    primFont.descSave.weight = tmp;  /* AlphanumericID command. */
        } else {
            secFont.desc.weight = tmp;
	    secFont.descSave.weight = tmp;   /* AlphanumericID command. */
        }
        break;

    case 'H':               /* Esc(s#H set font pitch */
			    /* 2 decimal places, in HIGH_RES units */
	tmp = (parse.wholeVal * 100) + ((parse.fractVal * 100) / FRACTMUX);
	tmp = MIN(57600, MAX(10, tmp));

#if 0
	/* This messes up PJL's setting of Pitch and overwrites the value 
	   that is used in resets */
	OMSetCurrentInt(OMFONTPITCH, OMHPPCL4, tmp);
#endif
        if (tmp) {
            tmp = HIGH_RES*100 / tmp;
        }
        fontChange = 1;
        if (parse.primFont) {
            /* Don't change unless we have to -- we have to know this */
            /* because only if we actually change do we clear justBS  */
            if (primFont.desc.pitch != tmp) {
                primFont.desc.pitch = tmp;
                if (PDIf->fixed)
                    HPClearJustBackspaced();  /* after file bug_sg */
            }
	    primFont.descSave.pitch = tmp;    /* AlphanumericID command. */
        } else {
            if (secFont.desc.pitch != tmp) {
                secFont.desc.pitch = tmp;
                if (PDIf->fixed)
                    HPClearJustBackspaced();
            }
	    secFont.descSave.pitch = tmp;     /* AlphanumericID command. */
        }
        break;

    case 'T':               /* Esc(s#T set type face */

	    tmp = MIN(parse.wholeVal, 65535);
#if 0
		savePitch = 1;    /* how PCL4 used to be */
#endif
        fontChange = 1;
        if (parse.primFont) {
            primFont.desc.typeface = tmp;
	    primFont.descSave.typeface = tmp;    /* AlphanumericID command. */
        } else {
            secFont.desc.typeface = tmp;
	    secFont.descSave.typeface = tmp;     /* AlphanumericID command. */
        }
        break;


    case 'V':               /* Esc(s#V set font point size */
			    /* 2 decimal places, in HIGH_RES units */
			    /* parse.wholeVal is in 1/72 in units */
	tmp = ( parse.wholeVal * 100) + 
	      ((parse.fractVal * 100  + FRACTMUX/2) / FRACTMUX);
	tmp = MIN(99975, MAX(25, tmp));

        tmp = PIU_TO_HRU * tmp;
        fontChange = 1;
        if (parse.primFont) {
            primFont.desc.point = tmp;
	    primFont.descSave.point = tmp;    /* AlphanumericID command. */
        } else {
            secFont.desc.point = tmp;
	    secFont.descSave.point = tmp;     /* AlphanumericID command. */
        }
        break;


    case 'W':               /* Esc(s#W download char or font definition */
	if (parse.wholeVal > 0) {

	    parse.nDataBytes = parse.nBytesToGo = parse.wholeVal;

		/* For this command, '(' or ')' doesn't distinguish */
		/* primary or secondary, but rather whether this is */
		/* a character or a font definition.  cf Symbolset  */
		/* definition, Esc(f#W, below.                      */
	    if (parse.mac.macroDef) {
		parse.state = Count;
	    } else if (parse.primFont) {
		parse.state = CharDL;
	    } else {
		parse.state = FontDL;
		BZERO(&fontHd, sizeof(fontHd));
		/*
		 * If this font ID is already used, delete the font using it.
		 */
		HPDeleteFont();
	    }
	    parse.stateSet = 1;
	}
	break;


    case 'P':               /* Esc(s#P set font spacing */
        if (parse.wholeVal == 0 || parse.wholeVal == 1) {
            fontChange = 1;
            /* Don't change unless we have to -- we have to know this  */
            /* because only if we actually change, and we are changing */
            /* the currently active font, do we clear justBackSpaced   */
            if (parse.primFont) {
                if (primFont.desc.fixed != !parse.wholeVal) {
                    primFont.desc.fixed = !parse.wholeVal;
                    if (options.primary) {
                        HPClearJustBackspaced();  /* after file bug_sg */
                    }
                }
		primFont.descSave.fixed = !parse.wholeVal;  /* AlphanumericID command. */
            } else {
                if (secFont.desc.fixed != !parse.wholeVal) {
                    secFont.desc.fixed = !parse.wholeVal;
                    if (!options.primary) {
                        HPClearJustBackspaced();
                    }
                }
		secFont.descSave.fixed = !parse.wholeVal;  /* AlphanumericID command. */
            }
        }
        break;


    case 'S':               /* Esc(s#S set font style */
	tmp = MIN(parse.wholeVal, 32767);
	fontChange = 1;
#if 0
	savePitch = 1;        /* how PCL4 used to be */
#endif
        if (parse.primFont) {
            primFont.desc.style = tmp;
	    primFont.descSave.style = tmp;  /* AlphanumericID command. */
        } else {
            secFont.desc.style = tmp;
	    secFont.descSave.style = tmp;   /* AlphanumericID command. */
        }
        break;

    } /* switch */

    if (fontChange) {
	if (parse.primFont) {
	    HPSetChooseFont(&primFont, savePitch);
	} else {
	    HPSetChooseFont(&secFont, savePitch);
	}
	/* Even though the HMI goes back to its default above, the */
	/* the default backspace and its flag remain as they were  */
	/* if only the height changes; otherwise it does reset.    */
	/* Kluge Alert: Should test exactly which do and don't.    */
	/* Test: bug_sg (says we should), but change height if     */
	/*  then backspace and overstrike (says we shouldn't.)     */
	if (ch != 'V') 
	    HPResetBS = TRUE;
    }
}


/*****************************************************************************
 * Synopsis:    void SymSetManagement()
 * Description: Executes the symbol set management commands.
 * Arguments:
 * Return:
 * Uses:
 * Sets:
 * Changes:
 * Heap Use:
 *
 *  Executes the escape sequence of the form <esc>(f#W[data],
 *  the symbol set management command.
 *****************************************************************************/

static void SymSetManagement(void)
{
    Uint8   ch;

    if (parse.mac.macroDef && TOUPPER(parse.ch) != 'W') {
	return;
    }

    ch = TOUPPER(parse.ch);

    switch (ch) {
	/* The only valid value here is 'W' */
    case 'W':               /* Esc(f#W download symbol set definition */
	if (parse.wholeVal > 0) {
	    parse.wholeVal = MIN(parse.wholeVal, MAXSINT16);
	       /* Bug 14917, lj4cet 34_01. TRM p 10-4, limit is 32767. */
	       /*  Can only be 530 meaningful bytes -- 18 for header,  */
	       /*  2 bytes each for 256 characters.                    */
	    parse.nDataBytes = parse.nBytesToGo = parse.wholeVal;

	    if (parse.mac.macroDef ||(parse.wholeVal > MAXSINT16)) {
		parse.state = Count;
	    } else {
		parse.state = SymSetDL;

		/*  The handling for Symbol Set definition download will
		 * parallel that for Character and Font downloads.
		 * Both a structure (unioned with a buffer) for downloading, 
		 * and a linked list of them have been defined.  Here we
		 * BZERO out the buffer structure, and get to add another
		 * link to the list.
		 */

		/* If this symbolsetID exists, the SymSetDL case will */
		/* have it deleted before adding this symbol set.     */

		/* Get the new Character Set list element */
		/* After hppl.c fills in symSetHd, it will copy the data into
		 * the list, and add the Current element to the list,
		 * but for now use its buffer:
		 */
		BZERO(&symSetHd, sizeof(symSetHd));
		/* The next fields are not part of the HP buffer definition. */
		symSetHd.ssStruct.Temporary = TRUE;
		symSetHd.ssStruct.next = (HPSymSetHead *)NULL;
	    }
	    parse.stateSet = 1;
	}
	break;
    }
}


/*****************************************************************************
 * Synopsis:    void SymbolSetFontId()
 * Description: Set font ID and symbol set.
 * Arguments:
 * Return:
 * Uses:
 * Sets:
 * Changes:
 * Heap Use:
 *
 *  Executes the escape sequence of the form Esc)#Z or Esc(#Z.  Handles
 *  font id specification, symbol set selection.
 *****************************************************************************/

static void SymbolSetFontId(void)
{
    Sint32          symbolValue;
    int    fontnum;
    int     symset;
    int     opitch;
    int   pointsize_piu;
    FontState      *pState;
    FONT           *pFont;
    Uint8           ch;
    HPFontID        temp_fontID; /* AlphanumericID modifications */
    
    if (parse.mac.macroDef) {
	return;
    }

    ch = TOUPPER(parse.ch);

    if (ch == 'X') 
    {
	/* select font by font id number */	
        parse.wholeVal = MAX(0, parse.wholeVal);
        parse.wholeVal = MIN(32767, parse.wholeVal);

        temp_fontID.idType = NUMERICID;  /* AlphanumericID modifications */
        temp_fontID.name.numericID = parse.wholeVal;
        HPDesignateFont(temp_fontID, parse.primFont);

	/* Backspace:  
	*  If you try to overstrike characters from different fonts by:
	*       "A"  <backspace>   SELECTFONT(100);   "B"
	*  The left edge of the characters will be lined up (approximately).
	*  Since the backspace moves the cursor back, all we do here is forget
	*  that we backspaced. (fixes trin bug #02274, "WW" in test bs09)
	*
	*  Interestingly, switching the order:
	*       "A"   SELECTFONT(100);   <backspace>   "B"
	*  or ANY other method of switching fonts causes the second character
	*  to be centered over the first, and be treated as if it had the
	*  width of the first character, potentially overwritting adjacent
	*  characters.  This centering is controlled by 'justBackspaced' in
	*  hppl.c:PrintChar().                 -BAM, 12-18-92         */
	/* That's not entirely true.  It seems changing the spacing, or */
	/* if fixed then changing the pitch, also clears the flag, and  */
	/* maybe more, including some Shift-In/Shift-Out.  - DSC 1/6/93 */
	HPClearJustBackspaced();

    } else if (ch == '@') {

	if (parse.wholeVal == 3) {

	    /* check if parsing for primary font or secondary */
	    if (parse.primFont) {
		pState = &primFont;
	    } else {
		pState = &secFont;
	    }

	    OMgetInt(omPCLFONTNUM, OMDEFAULT, &fontnum);

	    /* check for valid font */
	    if (!PDIfonthead(fontnum)) {
		HPSetFactoryFont();
		OMgetInt(omPCLFONTNUM, OMDEFAULT, &fontnum);
	    }
	    /* check for bad status */
	    if (PDIfonthead(fontnum)->Status != StatUsed) {
		HPSetFactoryFont();
		OMgetInt(omPCLFONTNUM, OMDEFAULT, &fontnum);
	    }

	    /*  get the default font pitch from front panel options */
	    pFont = PDIfonthead(fontnum);
	    /* do this when the pitch is not meanless */
	    if (!((pFont->scalable)&&!(pFont->fixed))) {
		OMgetInt(omPCLFONTPITCH, OMDEFAULT, &opitch);
		if (opitch == 0) opitch = 10*100;
		pState->desc.pitch = HIGH_RES*100/opitch;
	    }

	    HPSetFontChar(fontnum, &pState->desc);
	    pState->fontNum = fontnum;
	    pState->chooseFont = 0;  /* But we're going to set it below! */
	    pState->savePitch = 0;

	    OMgetInt(omPCLSYMSET, OMDEFAULT, &symset);

	    /*
	     *  Set the default symbolset, but if doesn't exist
	     *  then set to our ol' pal ROMAN-8.
	     */
	    pState->desc.symbolSet = symset;
	    pState->desc.pdi1Set = HPtoPDI1Symbol(symset);
	    if (!pState->desc.pdi1Set) {
		primFont.desc.pdi1Set = (Sint16) SETroman8;
		primFont.desc.symbolSet = 277;
	    }

	    /* AlphanumericID command. */
	    pState->descSave.symbolSet = pState->desc.symbolSet; 
	    pState->descSave.pdi1Set   = pState->desc.pdi1Set;
	    
	    /* Get default point size, unless its not defined. 
	     *   Remember that it's stored in 1/100 of a point.
	     */
	    if(!( pFont->scalable && pFont->fixed ) ) {
		OMgetInt(omPCLFONTSIZE, OMDEFAULT, &pointsize_piu);
		pState->desc.point = pointsize_piu * PIU_TO_HRU;

		/* AlphanumericID command. */
		pState->descSave.point = pState->desc.point;
	    }

	    /*
	     *  Set the 'savePitch' flag if the new font is
	     *  proportionally spaced.
	     */
	    HPSetChooseFont(pState, 0);
	    /*** this is what we used to do for PCL4:  HPSetChooseFont(pState, !
	     *** pState->desc.fixed); ***/
	    
	    /* Even though the HMI goes back to its default above, the */
	    /* the default backspace and its flag remain as they were. */
	    /* (To test, overstrike differently-sized characters.)     */
	}

    /* if valid symbol set escape sequence termination character, then process it */
    /* Note: LJ4 accepts '\' and '^' in the symbol set ID selection values */
    /*       Most likely there are other acceptable letters */
    /*       Test 20.02, Test 10.4 teaches us ']'.  I suspect it's A to '`' */
    }

    /* The change below was made for Bug # 32817 report # 36245
     * 3/5/99 - MSR
     */

#ifdef USEFUL
 else if ( (ch >= 'A' && ch <= 'Z') 
	     || ch == '\\' || ch == '^' || ch == ']' ) {
#else

   else if ((ch>= 'A') && (ch <= '`')) {
#endif


	/* if valid symbol set escape sequence value field, then process it */
	if (parse.wholeVal <= 2047) {

	    /* convert symbol set code sequence to numerical value */
	    symbolValue = parse.wholeVal * 32 + ch - '@';

	    /* check if parsing for primary font or secondary */
	    if (parse.primFont) {
		pState = &primFont;
	    } else {
		pState = &secFont;
	    }

	    pState->desc.symbolSet = symbolValue;
	    pState->desc.pdi1Set = HPtoPDI1Symbol(symbolValue);
	    HPSetChooseFont(pState, 0);
	    /*** this is what we used to do for PCL4:  HPSetChooseFont(pState, 1); ***/
	    
	    /* AlphanumericID command. */
	    pState->descSave.symbolSet = pState->desc.symbolSet;
	    pState->descSave.pdi1Set   = pState->desc.pdi1Set;	    
	}
    }
}

/*****************************************************************************
 * Synopsis:    Boolean HPescProlog()
 * Description: Allocate resources used by hpesc.c
 * Arguments:   none.
 * Return:      FALSE if successful.
 *****************************************************************************/

Boolean HPescProlog(void)
{
    return(FALSE);
}


/*****************************************************************************
 * Synopsis:    void HPescEpilog()
 * Description: Free resources used by hpesc.c
 * Arguments:   none.
 * Return:      void
 *****************************************************************************/

void HPescEpilog(void)
{
}

/*****************************************************************************
 * Synopsis:    void StartEndGraphics()
 * Description: Execute the starting and ending of graphics reading sequences.
 * Arguments:   none.
 * Return:      (void)
 * Uses:
 * Sets:
 * Changes:
 * Heap Use:    none.
 *
 *  Executes the sequence "<esc>*r#Z", for starting and ending graphics.
 *****************************************************************************/

static void StartEndGraphics(void)

{
    Uint8 ch = TOUPPER(parse.ch);
    int limit;

    if (parse.mac.macroDef || parse.relative == REL_ERROR)
	return;

    switch (ch) {

    case 'A':           /* Esc*r#A start graphics, sets graph left margin */
	limit = 1;
#ifdef PDI1_USING_COLOR
	if (IS_PDI1_USING_COLOR(CurrentDevice.colorFormat)) limit = 3;
#endif
	if ((parse.wholeVal >= 0) && (parse.wholeVal <= limit)) {
	    DoStartRasterGraphics(parse.wholeVal);
	}
	break;

    case 'B':           /* Esc*r#B end graphics transfer */
	DoEndRasterGraphics();
	break;

    case 'C':          /* Esc*r#C end graphics IIIsi and IIIP only ( # ignored\)*/
	/* The following call is added to speed up processing of tests
	   from a certain DeskJet 1200C driver which contain rasters
	   using Source Raster Height = 1 and landscape orientation.
	   (al503iwf, gt106iwi)
 
	   If an End Raster command is followed immediately by a Start Raster
	   command, under some circumstances it's safe to ignore both. */

	if(ignoreEndStartRaster())
	    break;
		    
	DoEndRasterGraphics();
	options.grapLeftMar_piu = 0; /* reset left margin */
	options.compress = 0; /* reset compression mode */
	break;

       
    case 'F':           /* Esc*r#F graphics rotate mode */
	if (!parse.rastXfer) { /* ignore in raster mode */
	    if (parse.wholeVal == 3) {
		options.graphicMode3 = 1;
	    } else if (parse.wholeVal == 0) {
		options.graphicMode3 = 0;
	    }
	    if (RotatedRaster) {
		HPGRotate((printer.printDir + 3) % 4);
	    } else {
		HPGRotate(printer.printDir);
	    }
	}
	break;

    case 'S':           /* Esc*r#S raster width  */
	if (!parse.rastXfer) { /* ignore in raster mode */
	    options.rasterWidth_rig = parse.wholeVal;
	/* the raster width is only half when subsampling */
	}
	break;

    case 'T':           /* Esc*r#T raster height  */
	if (!parse.rastXfer) { /* ignore in raster mode */
	    options.rasterHgt_rig = parse.wholeVal;
	/* the raster height is only half when subsampling */
	}
	break;

#ifdef PDI1_USING_COLOR
    case 'U':           /* Esc*r#U Simple Color Mode  */
	if (!IS_PDI1_USING_COLOR(CurrentDevice.colorFormat)) break;

	parse.colorUsed = TRUE;
	if (!parse.rastXfer) { /* ignore in raster mode */
	    if (parse.wholeVal == 3 || parse.wholeVal == 1
			|| (parse.wholeVal == 4 && parse.relative == REL_NEG)) {
		Sint16 tmp;
		tmp = parse.wholeVal;
		if (parse.relative == REL_NEG)
		    tmp = -tmp;
		HPsetSimplePal(tmp);
	    }
	}
	break;
#endif

    case 'Y':           /* Esc*r#Y raster Y offset  */
	HPDownYLines();
	break;

    case 'W':           /* Esc*r<n>W -- undefined W command */
	UndefCommand();
	break;
    }
}

#define HICOMPRESS (9) /* Redefined from 5 for HP LJ5, 3 for HP LJ3p, 2 for LJ2 */

/*****************************************************************************
 * Synopsis:    void GraphXferCompress()
 * Description: Set compression mode or download a scanline of raster data.
 * Arguments:   none.
 * Return:      (void)
 * Uses:
 * Sets:
 * Changes:
 * Heap Use:
 *
 *  Executes the sequence "<esc>*b#Z" where 'Z' is the termination char.
 *****************************************************************************/

static void GraphXferCompress(void)
{
    Uint8 ch;

    if (parse.relative == REL_ERROR)
	return;

    ch = TOUPPER(parse.ch);

    switch (ch) {
	case 'V':           /* Esc*b#V Transfer Raster (Plane) */
#ifdef PDI1_USING_COLOR
	    if (IS_PDI1_USING_COLOR(CurrentDevice.colorFormat)) {
	        parse.colorUsed = TRUE;
	        HPXferRasterCmd(FALSE);
	    }
	    else
#endif
	      if (parse.wholeVal > 0) {   /* discard data */
		parse.nBytesToGo = parse.wholeVal;
		parse.state = Count;
		parse.stateSet = 1;
	      }
	break;

	case 'W':           /* Esc*b#W raster row or block download */
	    HPXferRasterCmd(TRUE);
	    break;

	case 'M':           /* Esc*b#M set compression mode */
	    if (!parse.mac.macroDef) {
		if ((parse.wholeVal >= 0
			&& parse.wholeVal <= HICOMPRESS)
			&& ((parse.wholeVal < 6 && parse.wholeVal != 4)
				|| parse.wholeVal > 8)) {
		    options.compress = parse.wholeVal;
		}
	    }
	    break;

	case 'X':           /* Esc*b#X raster X offset */
	    if (parse.wholeVal <= 32767) 
		/* Round down to the nearest multiple of 8. */
		options.x_offset = (Uint16) (parse.wholeVal / 8) * 8;
	    break;

	case 'Y':           /* Esc*b#Y raster Y offset */
	    HPDownYLines();
	    break;

	case 'S':           /* Esc*b#S seed row source */
	    HPSetSeedRowOffset(parse.wholeVal);
    } /* switch */
}


/*****************************************************************************
 * Synopsis:    void GraphicsResolution()
 * Description: Sets the graphics resolution, yes it do.
 * Arguments:   none.
 * Return:      (void)
 * Uses:
 * Sets:
 * Changes:
 * Heap Use:    none.
 *
 *  Executes the sequence "<esc>*t#R", which sets the graphics resolution.
 *****************************************************************************/

static void GraphicsResolution(void)
{
    Uint8   ch;

    if (parse.mac.macroDef) {
	return;
    }

    ch = TOUPPER(parse.ch);

    switch (ch) {

    case 'R':
	if (!parse.rastXfer) {
	    /*believe user's resolution for non 300 dpi or 600 dpi devices*/
	    if((PDI1res.x != 600) && (PDI1res.x != 300) && (PDI1res.x != 1200))
	    {
		if((parse.wholeVal>=1) && (parse.wholeVal<=1200))
		    options.resolution_rig = parse.wholeVal;
		else
		    options.resolution_rig = PDI1res.x;
	    }
	    /*otherwise do HP's rounding*/
	    /* Add checking for PDI1res.y for non-square pixel build */
	    else if (((PDI1res.x < 600) && (PDI1res.x > 299)) ||
		     ((PDI1res.y < 600) && (PDI1res.y > 299))) {
		/* allow resolution at the engine resolution
		   whatever that may be..
		 */     
		if (parse.wholeVal == PDI1res.x)
		    options.resolution_rig = parse.wholeVal;
		else
		    {
			/* handle illegal cases by rounding to next highest value */
			/* In color PCL 600 dpi rasters are scaled in PDI1 */
			/* instead of draft_mode_subsampling. */
			if(parse.wholeVal > 300) {
			    options.resolution_rig = 600;
			} else {
			    if(parse.wholeVal > 150) {
				options.resolution_rig = 300;
			    } else if(parse.wholeVal > 100) {
				options.resolution_rig = 150;
			    } else if(parse.wholeVal >  75) {
				options.resolution_rig = 100;
			    } else {
				options.resolution_rig =  75;
			    }
			}
		    }
	    } /* < 600 DPI */
	    else {  /* printer is at >= 600 DPI output mode */
		if(parse.wholeVal > 300) {
		    options.resolution_rig = 600;
		} else if(parse.wholeVal > 200) {
		    options.resolution_rig = 300;
		} else if(parse.wholeVal > 150) {
		    options.resolution_rig = 200;
		} else if(parse.wholeVal > 100) {
		    options.resolution_rig = 150;
		} else if(parse.wholeVal >  75) {
		    options.resolution_rig = 100;
		} else {
		    options.resolution_rig =  75;
		}
	    }
	}
	/* enable "draft-mode" (sub-sampling) if raster input is
	 * 600 DPI and printer output is 300 DPI.
	 * only do if switchable build otherwise buffers too small
	 ********* NOTE TO PORTING:  Keep an eye on this if your resolution
				     is higher!
	 */
	break; 

#ifdef PDI1_USING_COLOR
    /* The following commands are for PCL5 color printers
     * This could (should) be a switch but for now, this is easier.
     */
    case 'H':
	if (!IS_PDI1_USING_COLOR(CurrentDevice.colorFormat)) break;
	parse.colorUsed = TRUE;
	if(!parse.rastXfer) {
	    HPDestRasterWidthCmd();     /* <Esc>*t#H        */
	}
	break;
# ifndef NO_COLOR_ADJUSTMENTS
    case 'I':           /* <Esc>*t#I    Gamma Correction cmd */
	if (!IS_PDI1_USING_COLOR(CurrentDevice.colorFormat)) break;
	parse.colorUsed = TRUE;
	if (parse.relative != REL_NEG && !parse.rastXfer) {
	    float   gam;
	    gam = (float)parse.wholeVal;
	    if (parse.fractVal)
		gam += (float)((float)parse.fractVal/(float)FRACTMUX);
	    HPGammaCorrection(gam);
	}
	break;
# endif /* !NO_COLOR_ADJUSTMENTS */
    case 'J':
	if (!IS_PDI1_USING_COLOR(CurrentDevice.colorFormat)) break;
	/* Color LaserJet 4550 compatibility */
	/* 0  - Continuous tone detail (high lpi) (device best dither) */
	/* 3  - Device best dither */
	/* 15 - Continuous tone smooth (high lpi) */
	/* 18 - Continuous tone basic (low lpi) */
	parse.colorUsed = TRUE;
	if(!parse.rastXfer){
	    	if (parse.wholeVal == 15 || parse.wholeVal == 18)
			parse.wholeVal = 3;
		if (parse.wholeVal == 0 || parse.wholeVal == 3)
			HPChooseRendAlg(); /* <Esc>*t#J Render Algorithm Cmd */
	}
	break;
    case 'K':
	if (!IS_PDI1_USING_COLOR(CurrentDevice.colorFormat)) break;
	parse.colorUsed = TRUE;
	HPScaleAlgorithmCmd();  /* <Esc>*t#K        */
	break;
    case 'V':
	if (!IS_PDI1_USING_COLOR(CurrentDevice.colorFormat)) break;
	parse.colorUsed = TRUE;
	HPDestRasterHeightCmd();        /* <Esc>*t#V        */
	break;
#endif	/* PDI1_USING_COLOR */

    case 'W':          /* just to handle Esc*t<n>W -- undefined W command */
	UndefCommand();

    } /* switch */
}




/*****************************************************************************
 * Synopsis:    void FontGraphics()
 * Description: Font management and rectangle drawing.
 * Arguments:   none.
 * Return:      (void)
 * Uses:
 * Sets:
 * Changes:
 * Heap Use:    none.
 *
 *  Executes the escape sequence of the form <esc>*c#Z, where Z is decoded.
 *  Handles font management, rectangle drawing, symbol set management.
 *****************************************************************************/

static void FontGraphics(void)
{
    Sint32  tmp;
    Sint32  oldX_piu, oldY_piu;
    Sint32  pdiX_piu, pdiY_piu;
    Uint8   ch;
    Boolean force_pat_tx, prevSourceTx, prevPatTx;

    if (parse.mac.macroDef || parse.relative == REL_ERROR) {
    /* Use default values in case of REL_ERROR */
      if (parse.relative == REL_ERROR){
	ch = TOUPPER(parse.ch);
	switch (ch) {
	case 'A':
	    options.horRuleSize_piu =0;
	    break;
	case 'B':
	    options.verRuleSize_piu =0;
	    break;
	case 'G':
	    options.areaFillID =0;
	    break;
	    }
	}
    return;
    }

    ch = TOUPPER(parse.ch);

    switch (ch) {

    case 'A':           /* Esc*c#A, set horizontal rule/pattern */
			/* convert from units_of_measure to PIU */
	options.horRuleSize_piu = UOMX_TO_PIUX * parse.wholeVal;

# ifdef ROUNDINGUP
	/*  NOTE:   The old PL/8 rounded up to the next dot if the
	 *          fractional value was non-zero in the first 2
	 *          decimal places.  Dunno if this is correct. (AJP)
	 */
	tmp = FRACTMUX/100;
	if (parse.fractVal/tmp > 0) {
	    options.horRuleSize_piu += printer.one_eng_pel_in_piux;
	}
#endif
	if ( parse.wholeVal &&  ( options.horRuleSize_piu < printer.one_eng_pel_in_piux ) )
	    options.horRuleSize_piu = printer.one_eng_pel_in_piux;

	break;

    case 'B':           /* Esc*c#B, set vertical rule/pattern size in dots */
			/* convert from units-of-meas. to PIUY */

	options.verRuleSize_piu = UOMY_TO_PIUY * parse.wholeVal;

# ifdef ROUNDINGUP
	/*  NOTE:   The old PL/8 rounded up to the next dot if the
	 *          fractional value was non-zero in the first 2
	 *          decimal places.  Dunno if this is correct. (AJP)
	 */
	tmp = FRACTMUX/100;
	if (parse.fractVal/tmp > 0) {
	    options.verRuleSize_piu += printer.one_eng_pel_in_piuy;
	}
# endif
	if ( parse.wholeVal &&  ( options.verRuleSize_piu < printer.one_eng_pel_in_piuy ) )
	    options.verRuleSize_piu = printer.one_eng_pel_in_piuy;
	break;

    case 'H':           /* Esc*c#H, horizontal rule/pattern size in decipts */
			/* 4 decimal places, ROUNDED UP to 300 dpi */
	/*
	 *  This is a strange algorithm for rounding up the rectangle size
	 *  to the nearest 300dpi value but expressing that value in
	 *  PIU_XDPI units.  This algorithm only works when PIU_XDPI is
	 *  an even multiple of PCL_XUNITS_300DPI and of decpts.
	 *
	 *  Here's the basic mechanism:
	 *  1)  The whole portion of the input value (decpts) and the
	 *      fract portion of the input value (dpts/10000) are
	 *      both multiplied by DPT_TO_PIUX to put the values into
	 *      internal units (usually 7200dpi).
	 *  2)  After multiplying, the portion of the fractional value
	 *      above 10000 is added to the wholeVal, with the remainder
	 *      staying in the fract val.
	 *  3)  The whole val is modulo'ed by one engine pixel size in piu to truncate
	 *      to an even multiple of engine output pixels.  The remainder of this
	 *      modulo is put into tmp.
	 *  4)  If the value in tmp is nonzero, or if the value in the
	 *      fract val is nonzero, round up to the next engine output dpi multiple.
	 */
	parse.wholeVal *= DPT_TO_PIUX; 
	parse.fractVal *= DPT_TO_PIUX;
	parse.wholeVal += parse.fractVal/FRACTMUX;
	parse.fractVal %= FRACTMUX;

	tmp = parse.wholeVal % printer.one_eng_pel_in_piux;
	parse.wholeVal -= tmp;
	if (tmp || parse.fractVal) {
	    parse.wholeVal += printer.one_eng_pel_in_piux;
	}
	options.horRuleSize_piu = parse.wholeVal;
	break;

    case 'V':           /* Esc*c#V, vertical rule/pattern size in decipts */
	/*
	 *  Same algorithm described above.
	 */
	parse.wholeVal *= DPT_TO_PIUY;
	parse.fractVal *= DPT_TO_PIUY;
	parse.wholeVal += parse.fractVal/FRACTMUX;
	parse.fractVal %= FRACTMUX;
	tmp = parse.wholeVal % printer.one_eng_pel_in_piuy;
	parse.wholeVal -= tmp;
	if (tmp || parse.fractVal) {
	    parse.wholeVal += printer.one_eng_pel_in_piuy;
	}
	options.verRuleSize_piu = parse.wholeVal;
	break;

    case 'D':           /* Esc*c#D, specify font id */
	parse.wholeVal = MAX(0, parse.wholeVal);
	parse.wholeVal = MIN(32767, parse.wholeVal);
	if (options.fontID.idType == STRINGID &&
	    options.fontID.name.stringID != NULL)
	{
	    PDI1FREE(options.fontID.name.stringID);
	}

	options.fontID.idType = NUMERICID;
	options.fontID.name.numericID = parse.wholeVal; /* AlphanumericID */
	break;
    case 'E':           /* Esc*c#E, specify char id */
	options.charID = parse.wholeVal;
	break;

    case 'F':           /* Esc*c#F, font management */
	switch (parse.wholeVal) {

	case 0:             /* delete all downloaded fonts */
	    /* KLUDGE ALERT!  Need to see if a deleted font had continue char
			      currently downloaded. */

	    /* Force reselection in case we go and delete the font we've got
	     *   currently selected.
	     * In some cases this will be unecessary:
	     * TO DO: Does this cause an error if we're 
	     *     currently selected by ID?
	     */
	    HPSetChooseFont(&primFont, 0);
	    HPSetChooseFont(&secFont, 0);
	    (void) PDIfdelall();
	    break;

	case 1:             /* delete all temporary fonts */
	    /* KLUDGE ALERT!  Need to see if a deleted font had continue char
			      currently downloaded. */

	    if (PDIf->Temporary) {
		/* Force reselection in case we go and delete the font we've got
		 *   currently selected.
		 */
		HPSetChooseFont(&primFont, 0);
		HPSetChooseFont(&secFont, 0);
	    }
            PDIfdeltemps();
            break;

	case 2:             /* delete last id specified font */
	    /* KLUDGE ALERT!  Need to see if the deleted font had continue char
			      currently downloaded. */

	    /* It's not necessary to force reselection here
	     * (see TO DO note at case 0 above)
	     * since HPDeleteFont() will do it iff it's needed.
	     */
	    HPDeleteFont();
	    break;

	case 3:             /* delete last char id specified */
	    /* KLUDGE ALERT!  Need to see if the deleted char was continued. */
	    HPCharDelete(options.fontID, options.charID);
	    break;

	case 4:             /* make font temporary */
	    HPSetTempFlag(1);
	    break;

	case 5:             /* make font permanent */
	    HPSetTempFlag(0);
	    break;

	case 6:             /* copy assign last font id specified */
	    /*
	     * Do not allow cloned fonts which follow font selection
	     * commands to participate in the font selection process using
	     * those criteria.  If font selection commands have been
	     * received since the last font selection was run, run it now.
	     */
	    HPMAYBECHOOSEFONT(&primFont);
	    HPMAYBECHOOSEFONT(&secFont);
	    if (options.primary) {
		HPSelectFont(&primFont.desc, primFont.fontNum);
	    }
	    else {
		HPSelectFont(&secFont.desc, secFont.fontNum);
	    }
	    HPCopyAssign();
	    break;

	} /* switch */
	break;

    case 'G':           /* Esc*c#G, set area fill pattern id */
	options.areaFillID = parse.wholeVal;
	break;

    case 'Q':           /* Esc*c#Q, user fill pattern control */
	HPUpattcontrol(parse.wholeVal,options.areaFillID);
	break;

    case 'P':           /* Esc*c#P, draw rectangle */
	force_pat_tx = FALSE;
	switch (parse.wholeVal) {

	case 0:             /* solid rule pattern */
	    PDIfillpattern(FILLsolid);
	    break;

	case 1:
	    PDIfillpattern(FILLwhite);
	    /* On HP4M+ and CLJ, transparent is ignored for white
	       PCL rectangle.  Probably a bug, which we'll emulate. 
	       Flag the case here, and we'll force it down below. */
	    force_pat_tx = TRUE;
	    break;

	case 2:             /* gray scale pattern */
	    if (options.areaFillID <= 0 || options.areaFillID > 100) {
		return;
	    }
	    PDIUanchor (printer.anchorx_piu, printer.anchory_piu,
		    printer.rLimit_piu, printer.bLimit_piu,
		    printer.orient,  printer.printDir, printer.rotatePat);
	    options.rectPat = options.areaFillID;
	    PDIfillpattern(options.rectPat);
	    break;

	case 3:             /* HP-defined pattern */
	    if (options.areaFillID <= 0 || options.areaFillID > 6) {
		return;
	    }
	    PDIUanchor (printer.anchorx_piu, printer.anchory_piu,
			printer.rLimit_piu, printer.bLimit_piu,
			printer.orient,  printer.printDir, printer.rotatePat);
	    options.rectPat = options.areaFillID;
	    PDIfillpattern(-options.rectPat);
	    break;
	case 4:             /* User-defined fill patterns */
	    if (options.areaFillID < 0 || options.areaFillID > 32767 ) {
		return;
	    }
	    options.rectPat = HPUdfPatt(options.areaFillID);
	    if  (options.rectPat) {
		/* set the anchor corner for this pattern */
		PDIUanchor (printer.anchorx_piu, printer.anchory_piu,
		    printer.rLimit_piu, printer.bLimit_piu,
		    printer.orient,  printer.printDir, printer.rotatePat);
		PDIfillpattern(options.rectPat);
		break;
	    }
	    else {
		return;
	    }

	case 5:             /* current pattern */
	    options.rectPat = options.modelPat;
	    PDIfillpattern(options.rectPat);

	    if (options.rectPat == FILLwhite) {
		/* On HP4M+ and CLJ, transparent is ignored for white
		   PCL rectangle.  Probably a bug, which we'll emulate. 
		   Flag the case here, and we'll force it down below. */
		force_pat_tx = TRUE;
	    }
	    break;

	default:
	    return;
	} /* switch */

	/*
	 *  Determine the X and Y of the current point in PCL-based
	 *  coords.  This code used to round the current point to the
	 *  nearest unit of measure. This caused rectangles to be
	 *  incorrectly placed on the page when the units of measure
	 *  didn't match the device resolution.
	 */
	oldX_piu = HPGetAbsX();
	pdiX_piu = MIN(oldX_piu + options.horRuleSize_piu 
		- printer.one_eng_pel_in_piux, printer.rLimit_piu);
	oldY_piu = HPGetAbsY();
	pdiY_piu = MIN(oldY_piu + options.verRuleSize_piu 
		- printer.one_eng_pel_in_piuy, printer.bLimit_piu);
	/*
	 *  Now determine the PDI coords for the bottom-right corner
	 *  of the rectangle.  PDI will make a rectangle from the
	 *  the current point to the given bottom-right.
	 */
	if ((pdiX_piu - oldX_piu) >= 0 && (pdiY_piu - oldY_piu) >= 0) {
	    Uint8 color[5];
	    
	    HPtoPDICoords(&pdiX_piu, &pdiY_piu);
	    /* If we're doing a white rect, we want to force it to be 
	       opaque (see above). */
	    if (force_pat_tx) {
		prevSourceTx = PDIgetsrctransp();
		prevPatTx = PDIgetpattransp();
		PDItransparent(prevSourceTx, TRUE);
		
		/* Also save the foreground color (PDI1.color) because
		 * GetFillPat() will set it to white for the white fill,
		 * but we need to restore it after the white fill.
		 */
		for (tmp = 0; tmp < 5; tmp++) color[tmp] = PDI1.color[tmp];
	    }
	    PDIrectangle(pdiX_piu, pdiY_piu);
	    if (force_pat_tx) {
		PDItransparent(prevSourceTx, prevPatTx);
		/* Restore the foreground color */
		PDI1setcolor(color);
	    }
	}

	if (!SOMETHINGONPAGE) {
	    parse.firstInk = INKONPAPER;
	    HPCheckPageChar();
	}

	/*
	 *  We clobbered the print model imaging pattern by drawing a
	 *  filled rectangle.  Now we have to restore the imaging
	 *  pattern that we clobbered.
	 */
	PDIfillpattern(options.modelPat);
	break;

    /* 
     * The flags newHp newVp are to signal that new plot information has
     * been given. Scaling in the opposite direction (vertical or horizontal)
     * happens if the first direction does not have new plot size.
     * (I bet your'e confused!!)
     */

    case 'K':           /* Esc*c#K, specify horizontal plot size
			 *          (delaying binding until GL2 called;
			 *           test: VI00534a) */
	{
	    if (parse.wholeVal > 0 || parse.fractVal > 0) {
		/*
		 *  convert inches to piu (7200 units)
		 */
		defaultHPlot = FALSE;
		newHp = TRUE;
		parse.wholeVal *= PIU_XDPI;
		parse.fractVal *= PIU_XDPI;
		parse.wholeVal += parse.fractVal/FRACTMUX;
		if (parse.fractVal % FRACTMUX) {
		    parse.wholeVal += 1;
		}
		plotSizeWidth_piu = parse.wholeVal;
		if (parse.relative == REL_NEG) {
		    plotSizeWidth_piu = -plotSizeWidth_piu;
		}
	    } else {
		/*
		 *  This defaults to the current frame width.
		 */
		defaultHPlot = TRUE;
		newHp = TRUE;
	    }
	}
	break;

    case 'L':           /* Esc*c#L, specify vertical plot size
			 *          (delaying binding until GL2 called) */
	{
	    if (parse.wholeVal > 0 || parse.fractVal > 0) {
		/*
		 *  convert inches to piu (7200 units)
		 */

		defaultVPlot = FALSE;
		newVp = TRUE;
		parse.wholeVal *= PIU_YDPI;
		parse.fractVal *= PIU_YDPI;
		parse.wholeVal += parse.fractVal/FRACTMUX;
		if (parse.fractVal % FRACTMUX) {
		    parse.wholeVal += 1;
		}
		plotSizeHgt_piu = parse.wholeVal;
		if (parse.relative == REL_NEG) {
		    plotSizeHgt_piu = -plotSizeHgt_piu;
		}
	    } else {
		/*
		 *  This defaults to the current frame height.
		 */
		defaultVPlot = TRUE;
		newVp = TRUE;
	    }
	}
	break;

    case 'X':           /* Esc*c#X, set horizontal frame size */
	{
	    int  frameWidth_piu;
	    int   plotWidth_piu;
	    Boolean   resize;
	    Boolean   clrbuf;

	    if (parse.wholeVal > 0 || parse.fractVal > 0) {
		/*
		 *  Get decipoints to 4 decimal places but convert
		 *  to 7200 piu
		 */
		parse.wholeVal *= ONE_DPT_IN_PIUX;
		parse.fractVal *= ONE_DPT_IN_PIUX;
		parse.wholeVal += parse.fractVal/FRACTMUX;
		if (parse.fractVal % FRACTMUX) {
		    parse.wholeVal += 1;
		}
		frameWidth_piu = parse.wholeVal;
	    } else {
		/*
		 *  This defaults to the width of the default margins.
		 *  We have to look at print direction to determine if
		 *  we are using 'rLimit_piu' or 'bLimit_piu' to determine this.
		 */
		if (printer.printDir % 2) {
		    frameWidth_piu = printer.bLimit_piu;
		} else {
		    frameWidth_piu = printer.rLimit_piu;
		}
	    }
	    HPGLargs.framewd = frameWidth_piu;

	    plotWidth_piu = frameWidth_piu;
	    HPGLargs.plotwd = plotWidth_piu;

	    resize = 1;
	    HPGLargs.resize = resize;
	    clrbuf = 1;
	    HPGLargs.ClrPolyBuf = clrbuf;
	}
	break;

    case 'Y':           /* Esc*c#Y, set vertical frame size */
	{
	    int  frameHgt_piu;
	    int   plotHgt_piu;
	    Boolean   resize;

	    if (parse.wholeVal > 0 || parse.fractVal > 0) {
		/*
		 *  Get decipoints to 4 decimal places but convert
		 *  to 7200 piu
		 */
		parse.wholeVal *= ONE_DPT_IN_PIUY;
		parse.fractVal *= ONE_DPT_IN_PIUY;
		parse.wholeVal += parse.fractVal/FRACTMUX;
		if (parse.fractVal % FRACTMUX) {
		    parse.wholeVal += 1;
		}
		frameHgt_piu = parse.wholeVal;
	    } else {
		/*
		 *  This defaults to the height of the default margins.
		 *  We have to look at print direction to determine if
		 *  we are using 'rLimit_piu' or 'bLimit_piu' to determine this.
		 *  The default margins are 1 inch smaller than the
		 *  physical page size.
		 */
		if (printer.printDir % 2) {
		    frameHgt_piu = printer.rLimit_piu - ONE_INCH_IN_PIUX +
			printer.one_uom_pel_in_piux;
		} else {
		    frameHgt_piu = printer.bLimit_piu - ONE_INCH_IN_PIUY +
			printer.one_uom_pel_in_piux;
		}
	    }
	    HPGLargs.frameht = frameHgt_piu;

	    plotHgt_piu = frameHgt_piu;
	    HPGLargs.plotht = plotHgt_piu;

	    resize = 1;
	    HPGLargs.resize = resize;
	}
	break;

    case 'R':           /* Esc*c#R, Symbolset ID */
	if ((parse.wholeVal >= 0) && (parse.wholeVal <= 32767))
	    options.symbolsetID = parse.wholeVal;
	break;
    
    case 'S':           /* Esc*c#S, symbol set control */
	switch (parse.wholeVal) {
	    /* The information is all in the PDIdlCsetList list */
	    /*  "current" is stored right above, in options.symbolsetID    */
	case 0:         /* delete all user-defined symbol sets */
	    PDIdelSymSets(TRUE);
	    break;

	case 1:         /* delete all temporary user-defined symbol sets */
	    PDIdelSymSets(FALSE);
	    break;

	case 2:         /* delete current user-defined symbol set */
	    (void) PDIsymSetDelete(options.symbolsetID);
	    break;

	case 4:         /* make current user-defined symbol set temporary */
	    (void) PDIsymSetTemp(options.symbolsetID, TRUE);
	    break;

	case 5:         /* make current user-defined symbol set permanent */
	    (void) PDIsymSetTemp(options.symbolsetID, FALSE);
	    break;

	default:        /* Illegal value */
	    break;

	}

	if ((parse.wholeVal >= 0) && (parse.wholeVal <= 2))
	{
    /* We've just reduced the number of downloaded symbol sets in the world. */
    /* If PDI1charset thinks it's got SETdownloaded, it may have             */
    /*  another think coming.                                                */
    /* KLUGE ALERT: This is a bit of a sledge hammer approach, I'm not       */
    /*  really sure what PDI1charset is about, but if it's SETdownloaded,    */
    /*  that's us.                                                           */
    /* This may be overkill, but what the heck, this is a rare operation.    */
    /* If the parameter was 0 or 1, we don't know which sets were deleted.   */
    /* If it was 2, we could be a little more clever, but are thorough.      */
    /* Re-synchronize the .desc.pdi1Sets if they were downloaded, and the    */
    /* PDI1charset and PDI1cset.  (To Do: PDI1charset & PDI1cset should go). */
    /* The following ought to be enough to handle it...                      */
	    if (primFont.desc.pdi1Set == SETdownloaded)
	    {
	       if ((primFont.desc.pdi1Set = 
		    HPtoPDI1Symbol(primFont.desc.symbolSet)) != SETdownloaded)
		   HPSetChooseFont(&primFont,0);  /* Since value is new */
	    }
	    if (secFont.desc.pdi1Set == SETdownloaded)
	    {
	       if ((secFont.desc.pdi1Set = 
		    HPtoPDI1Symbol(secFont.desc.symbolSet)) != SETdownloaded)
		   HPSetChooseFont(&secFont,0);
	    }

	    if (PDI1charset == SETdownloaded)
	    {
		FontState *pState;

		if (options.primary)
		    pState = &primFont;
		else
		    pState = &secFont;

		PDI1charset = pState->desc.pdi1Set;
		PDI1cset = pState->desc.symbolSet;

		/* Force font reselection -- is that ever a problem here? */
		HPSetChooseFont(pState, 0);
	    }
	    /* Else we don't think we've got a downloaded set now, so */
	    /* let everything slide until the next font selection,    */
	    /* which won't find the downloaded set that's not there.  */
	}
	break;
    
    
    case 'T':           /* Esc*c#T, set picture frame anchor point */
	{
	    int    anchorX_piu;
	    int    anchorY_piu;
	    Boolean   resize;

	    if (parse.wholeVal == 0 && parse.fractVal == 0) {
		anchorX_piu = PDIgetabsx();
		anchorY_piu = PDIgetabsy();

		HPGLargs.anchx = anchorX_piu;
		HPGLargs.anchy = anchorY_piu;

		resize = 1;
		HPGLargs.resize = resize;
	    }
	}
	break;

    case 'W':           /* Esc*c#W  -- Load user fill pattern command */
    if (parse.wholeVal > 0) {
	    parse.nBytesToGo = parse.nDataBytes = parse.wholeVal;
	    parse.state = UdfpStart;
	    parse.stateSet = 1;
    }
	break;

    } /* switch */
}


/*****************************************************************************
 * Synopsis:    void PixelPlacement()
 * Description: Selection of pixel placement mode
 *      or selection of Logical Raster Opcode
 * Arguments:   none.
 * Return:      (void)
 * Uses:
 * Sets:
 * Changes:
 * Heap Use:    none.
 *
 *  Executes the escape sequence of the form <esc>*l#R
 *****************************************************************************/

static void PixelPlacement(void)
{
    Uint8 ch;

    if (parse.mac.macroDef || parse.relative == REL_ERROR) {
	return;
    }

    ch = TOUPPER(parse.ch);

    switch (ch) {

    case 'R':           /* ESC*l#R, selection of the pixel placement mode */
	if(parse.rastXfer) {
	    DoEndRasterGraphics();
	}
	PDI1PixPlace(parse.wholeVal);
	options.pix_placement = parse.wholeVal;
    break;
    case 'O':       /* ESC*l#O, select Logical Raster Opcode*/
	if((parse.wholeVal>=0) && (parse.wholeVal<=255))
	    {
		options.opcode=parse.wholeVal;
		if (!parse.rastXfer) /* else we set new opcode at EndRaster */ 
			PDIopcode(options.opcode);
	    }
	break;

    case 'W':       /* <Esc>*l#W    */
#ifdef PDI1_USING_COLOR
      if (IS_PDI1_USING_COLOR(CurrentDevice.colorFormat)) {
	if(parse.rastXfer) {
	    DoEndRasterGraphics();
	}
# ifdef NO_COLOR_ADJUSTMENTS
	/* Absorb Incoming Data */
	if (parse.wholeVal > 0) {
	  parse.nDataBytes = parse.nBytesToGo = parse.wholeVal;
	  parse.state = Count;
	} else {
	  parse.state = Start;
	}
	parse.stateSet = 1;
# else
	parse.colorUsed = TRUE;
	if(parse.wholeVal == 0){         /* Clear Lookup Table */
	    HPClearLookupTables();
	    parse.state = Start;
	    parse.stateSet = 1;
	}
	else if(parse.wholeVal == 770){  /* Accept Incoming Lookup Table */
	    parse.nDataBytes = parse.nBytesToGo = parse.wholeVal;
	    parse.state = ColorLUT;
	    parse.stateSet = 1;
	}
	else {                           /* Absorb Incoming Data */
	    parse.nDataBytes = parse.nBytesToGo = parse.wholeVal;
	    parse.state = Count;
	    parse.stateSet = 1;
	}
# endif /* !NO_COLOR_ADJUSTMENTS */
	break;
      }
      else
#endif /* defined (PDI1_USING_COLOR) */
      {
     	UndefCommand();
        break;
      }
    }
}


/*****************************************************************************
 * Synopsis:    void PrintModel()
 * Description: Selection of print model options.
 * Arguments:   none.
 * Return:      (void)
 * Uses:
 * Sets:
 * Changes:
 * Heap Use:    none.
 *
 *  Executes the escape sequence of the form <esc>*v#Z, where Z is decoded.
 *  Handles print model choice selection.
 *  Some commands are locked out in Raster Mode, some force an End Raster.
 *****************************************************************************/

static void PrintModel(void)
{
    Uint8 ch;

    if (parse.mac.macroDef || parse.relative == REL_ERROR) {
	return;
    }    
    ch = TOUPPER(parse.ch);

    switch (ch) {

    case 'T':           /* ESC*v#T, selection of the print model pattern */
	if (parse.rastXfer) {
	    DoEndRasterGraphics();
	}
	switch (parse.wholeVal) {

	case 0:             /* black rule pattern */
	    options.modelPat = FILLblack;
	    PDIfillpattern(FILLblack);
	    PDIcharpattern(FILLblack);
	    break;

	case 1:
	    options.modelPat = FILLwhite;
	    PDIfillpattern(FILLwhite);
	    PDIcharpattern(FILLwhite);
	    break;

	case 2:             /* gray scale pattern */
	    if (options.areaFillID >= 0 && options.areaFillID <= 100) {
		options.modelPat = options.areaFillID;
		/* must set the anchor corner for grays to come out right */
		PDIUanchor (printer.anchorx_piu, printer.anchory_piu,
		    printer.rLimit_piu, printer.bLimit_piu,
		    printer.orient,  printer.printDir, printer.rotatePat);
		PDIfillpattern(options.modelPat);
		PDIcharpattern(options.modelPat);
	    }
	    break;

	case 3:             /* HP-defined pattern */
	    if (options.areaFillID > 0 && options.areaFillID <= 6) {
		options.modelPat = -options.areaFillID;
		/* must set the anchor corner for hatches to come out right */
		PDIUanchor (printer.anchorx_piu, printer.anchory_piu,
		    printer.rLimit_piu, printer.bLimit_piu,
		    printer.orient,  printer.printDir, printer.rotatePat);
		PDIfillpattern(options.modelPat);
		PDIcharpattern(options.modelPat);
	    }
	    break;
	case 4:             /* User-defined pattern */
	    if ((options.areaFillID >= 0) &&( options.areaFillID <= 32767)) {
	int i;      /*UDFP pattern number*/
		i = HPUdfPatt(options.areaFillID);
	/*ignore undefined patterns*/
		if (i != 0) {   /*if pattern defined*/
		    options.modelPat = i;   /*save UDFP pattern number*/
		/* set the anchor corner for this pattern */
		PDIUanchor (printer.anchorx_piu, printer.anchory_piu,
		    printer.rLimit_piu, printer.bLimit_piu,
		    printer.orient,  printer.printDir, printer.rotatePat);
		    PDIfillpattern(options.modelPat);
		    PDIcharpattern(options.modelPat);
		}
	    }
	    break;
	}
	break;

    case 'N':           /* ESC*v#N, source transparency selection */
	if (parse.rastXfer) {
	    DoEndRasterGraphics();
	}
       if (parse.wholeVal == 0 || parse.wholeVal == 1) {
	    options.srcMode = parse.wholeVal;
	    PDItransparent(options.srcMode, options.patMode);
	}
	break;

    case 'O':           /* ESC*v#O, pattern transparency selection */
	if (parse.rastXfer) {
	    DoEndRasterGraphics();
	}
	if (parse.wholeVal == 0 || parse.wholeVal == 1) {
	    options.patMode = parse.wholeVal;
	    PDItransparent(options.srcMode, options.patMode);
	}
	break;

#ifdef PDI1_USING_COLOR
    case 'A':   /* <Esc>*v#A    color component 1 */
	if (!IS_PDI1_USING_COLOR(CurrentDevice.colorFormat)) break;
	parse.colorUsed = TRUE;
	/* just stash the value; it's used by assign color index cmd.
	   values may be positive or negative, integer or float */
	if (!parse.rastXfer){
	    options.colorComps[0] = (float)parse.wholeVal;
	    if (parse.relative == REL_NEG)
		options.colorComps[0] = -options.colorComps[0];
	    if (parse.fractVal) {
		if (parse.relative == REL_NEG)
		    parse.fractVal = -parse.fractVal;
		options.colorComps[0] += 
		    (float)((float)parse.fractVal/(float)FRACTMUX);
	    }
	}
    break;
    case 'B':   /* <Esc>*v#B    color component 2 */
	if (!IS_PDI1_USING_COLOR(CurrentDevice.colorFormat)) break;
	parse.colorUsed = TRUE;
	if (!parse.rastXfer) {
	    options.colorComps[1] = (float)parse.wholeVal;
	    if (parse.relative == REL_NEG)
		options.colorComps[1] = -options.colorComps[1];
	    if (parse.fractVal) {
		if (parse.relative == REL_NEG)
		    parse.fractVal = -parse.fractVal;
		options.colorComps[1] += 
		    (float)((float)parse.fractVal/(float)FRACTMUX);
	    }
	}
    break;
    case 'C':   /* <Esc>*v#C    color component 3 */
	if (!IS_PDI1_USING_COLOR(CurrentDevice.colorFormat)) break;
	parse.colorUsed = TRUE;
	if (!parse.rastXfer) {
	    options.colorComps[2] = (float)parse.wholeVal;
	    if (parse.relative == REL_NEG)
		options.colorComps[2] = -options.colorComps[2];
	    if (parse.fractVal) {
		if (parse.relative == REL_NEG)
		    parse.fractVal = -parse.fractVal;
		options.colorComps[2] += 
		    (float)((float)parse.fractVal/(float)FRACTMUX);
	    }
	}
    break;
    case 'I':   /*<Esc>*v#I*/
	if (!IS_PDI1_USING_COLOR(CurrentDevice.colorFormat)) break;
	parse.colorUsed = TRUE;
	if (!parse.rastXfer) {
	    HPAssignColorIndex(&options.colorComps[0],parse.wholeVal);
	    }
	break;
    case 'S':
	if (!IS_PDI1_USING_COLOR(CurrentDevice.colorFormat)) break;
	parse.colorUsed = TRUE;
	if (parse.relative != REL_NEG && !parse.rastXfer){
	    HPsetForegroundColor((Uint16)parse.wholeVal); /* <Esc>*v#S  */
	    
	    /* Setting FillType to SOLIDCOLOR makes GetFillPat(), case (retval == 8)
	       to reset foreground color, otherwise foreground color will be set
	       to default black.  */
           PDI1.solidblack = FALSE; 
	    
	    PDIfillpattern(FILLsolid); 
	    PDIcharpattern(FILLsolid);
	}
	break;
    case 'W':               /* <Esc>*v#W */
	if (!IS_PDI1_USING_COLOR(CurrentDevice.colorFormat)) {
	    UndefCommand();
	    break;
	    }
	parse.colorUsed = TRUE;
	parse.nDataBytes = parse.nBytesToGo = parse.wholeVal;
	if (!parse.rastXfer &&
		(parse.wholeVal == 6  || parse.wholeVal == 18 ||
		parse.wholeVal == 30 || parse.wholeVal == 86 ||
		parse.wholeVal == 122)) {
	    parse.state = PalCID;
	    parse.stateSet = 1;
	}
	else {
	    parse.state = Count;
	    parse.stateSet = 1;
	}
	break;
#else     /* !PDI1_USING_COLOR: commands undefined */
     case 'W':   /* <Esc>*v#W  */
     	UndefCommand();
     	break;
#endif /* PDI1_USING_COLOR */

    } /* switch */
}

/*****************************************************************************
 * Synopsis:    void HPDoPercent()
 * Description: Look for <esc>%#B to switch to GL/2 mode
 * Description: Look for <esc>%#X for UEL
 * Arguments:   none.
 * Return:      (void)
 * Uses:
 * Sets:        options manager:  CAPX/Y, GLARG, GLORIENT;  saves PDI state
 * Changes:
 * See Also:    gl/gldce.c - accepts values from here, enters GL mode
 *****************************************************************************/
static void HPDoPercent(void)
{
    
    if (TOUPPER(parse.ch) == 'B') {
	extern PDI1STATE *savedPDI1;
	int capX_piu;
	int capY_piu;
	int  glArg;
	int  glOrient;
	
	/*
	 *  Put the current cursor position into the options.
	 */
	capX_piu = PDIgetabsx();
	capY_piu = PDIgetabsy();
	
	HPGLargs.capx = capX_piu;
	HPGLargs.capy = capY_piu;
	
	/*
	 *  Put the even or odd argument into the options.  (test modesw11)
	 *  For color, -1 is a viable mode and needs to be distinguished from 1.
	 */
	glArg = parse.wholeVal;

#if 0 /* previously #ifdef COLOR_RENDERING */
	/* Fix for bug #34115, See the PCL5C Technical
	   Reference Manual p.7-2 for more details.
	*/
	if(parse.relative == REL_NEG)
		glArg *= -1;
#endif
	HPGLargs.arg = glArg;

	/*
	 *  Put the current rotation in for GL/2.
	 */
	glOrient = printer.orient;
	HPGLargs.orient = glOrient;
	
	/*
	 *  Set plot size (from Esc*c#K and Esc*c#L commands)
	 */
	setHPlot();
	setVPlot();
	
	/*
	 *  Set up the page characteristics if this is the first thing
	 *  to go on the page.
	 */
	if (!SOMETHINGONPAGE) {
	    HPCheckPageChar();
	}
	
	/*
	 *  Save current PDI1 for restoring after return from GL2.
	 */
	*savedPDI1 = PDI1;
	
	parse.state = Swtch;
	parse.stateSet = 1;
	
	/*
	 * If chooseFont is set, choose now, because when we return
	 * from GL2 we're going to trust the font number, and while
	 * the number may be temporarily be wrong (as when a font is
	 * deleted) better not try to select it, or you'll hit an ASSERT.
	 * (HPReturnFromGL2() no longer forces choosing, in case our current
	 * font number was selected by number, not attributes.  Either
	 * it or this, however, has to choose if choosing is necessary.)
	 */
	if (options.primary) {
	    HPMAYBECHOOSEFONT(&primFont);
	}
	else {
	    HPMAYBECHOOSEFONT(&secFont);
	}
    }
    
    /* is it UEL? */
    else if (parse.ch == 'X' && parse.wholeVal == 12345) {
	extern Sint32 pclNodeEOJ;

	PCLDEBUG(1,"HPDoPercent: UEL\n");

#ifdef EMPJL4
	UNGETCHAR();
	UNGETCHAR();
	UNGETCHAR();

	UNGETCHAR();
	UNGETCHAR();
	UNGETCHAR();

	UNGETCHAR();
	UNGETCHAR();
	UNGETCHAR();
#endif
	if (SOMETHINGONPAGE)
	    HPFormFeed(FFNOCR, FFEOJ);

	pclNodeEOJ = PCL_EOJ_NORMAL;
	parse.state = Exit;
	parse.stateSet = TRUE;
    }
    
    /* else unknown: ignore */
}


#if defined(DEBUG) & (defined(PCL_IMAGE_COMPRESS) | defined(PCL_FONT_CACHE_COMPRESS) | defined(PCL_BAND_COMPRESS) | defined(PCL_LOSSY_COMPRESS))
/*****************************************************************************
* Synopsis:    void HPCompression()
* Description: Pseudo-command for compression control:  accumulate value string.
* Arguments:   none.
* Return:      (void)
*
*     called on ESC | c <parameter> <terminator>
*****************************************************************************/

static void HPCompression(void)
{
    switch (TOUPPER(parse.ch)) {
    
#ifdef PCL_IMAGE_COMPRESS
	case 'E':
	case 'I':
	    PDI1imageComprActive = parse.wholeVal;
	    OMsetInt(omPCLIMAGECOMPRESS, OMDEFAULT, PDI1imageComprActive);
	    if (PDI1imageComprActive)
		 PCLDEBUG1(4, "PCL image compression enabled, method %d\n",
				    PDI1imageComprActive);
	    else
		PCLDEBUG(4, "PCL image compression disabled\n");
	    break;
#endif

#ifdef PCL_LOSSY_COMPRESS
	case 'L':
	    switch (parse.wholeVal) {
	    case 0:
	      PDI1lossyComprActive = FALSE;
	      break;

	    case 1:
	    case 2:
	      PDI1lossyComprActive = TRUE;
	      break;

	    default:
	      break;
	    }
	    OMsetBool(omPCLLOSSYCOMPRESS, OMDEFAULT, PDI1lossyComprActive);
	    PCLDEBUG1(4, "PCL lossy image compression %sabled\n",
					PDI1lossyComprActive ? "dis" : "en");
	    break;
#endif

#ifdef PCL_FONT_CACHE_COMPRESS
	case 'F':
	    PDI1fontComprActive = parse.wholeVal != 0;
	    /* THIS OPTION SHOULD HAVE BEEN BOOLEAN! */
	    OMsetInt(omPCLFONTCOMPRESS, OMDEFAULT, PDI1fontComprActive);
	    PCLDEBUG1(4, "PCL character compression %sabled\n",
			    PDI1fontComprActive ? "en" : "dis");
	    break;
#endif

#ifdef PCL_BAND_COMPRESS
	case 'B':
	    PDI1bandComprActive = parse.wholeVal;
	    OMsetInt(omPCLBANDCOMPRESS, OMDEFAULT, PDI1bandComprActive);
	    if (PDI1imageComprActive)
		 PCLDEBUG1(4, "PCL band compression enabled, method %d\n",
				    PDI1bandComprActive);
	    else
		PCLDEBUG(4, "PCL band compression disabled\n");
	    break;
#endif
    }
}
#endif /* PCL_..._COMPRESS */


#if (defined(DEBUG) & defined(PCL_FONT_SMOOTHING)) | defined(COMPLEXITY_METRICS)
/*****************************************************************************
* Synopsis:    void HPmiscellaneous()
* Description: Pseudo-commands for this 'n' that
* Arguments:   none.
* Return:      (void)
*
*     called on ESC | s <parameter> <terminator>
*****************************************************************************/

static void HPmiscellaneous(void)
{
#ifdef PCL_FONT_SMOOTHING
    extern Boolean PDI1fontsmoothing;             /* see pcl5strt.c */
#endif

#ifdef COMPLEXITY_METRICS
    extern Boolean PDI1_disable_monster;          /* see pdi1dlst.c */
#endif

#if defined(COMPLEXITY_METRICS) & defined(DEBUG)
    extern Uint8 PDI1complexity_metrics_active;   /* see pcl5strt.c */
#endif

    switch (TOUPPER(parse.ch))
      {
#ifdef PCL_FONT_SMOOTHING
      case 'F':
      case 'E':
	PDI1fontsmoothing = parse.wholeVal != 0;
	PCLDEBUG1(4, "PCL font smoothing %sabled\n",
			parse.wholeVal ? "en" : "dis");
	break;
#endif
#if defined(COMPLEXITY_METRICS) & defined(DEBUG)
      case 'M':
	PDI1complexity_metrics_active = parse.wholeVal != 0;
	PCLDEBUG1(4, "PCL complexity metrics %sactive\n",
			parse.wholeVal ? "" : "in");
	break;
#endif
#ifdef COMPLEXITY_METRICS
      case 'B':
	PDI1_disable_monster = parse.wholeVal != 0;
	PCLDEBUG1(4, "PCL monster bands %sabled\n",
			parse.wholeVal ? "dis" : "en");
	break;
#endif
    }
}
#endif


/*****************************************************************************
* Synopsis:    void HPUnitsOfMeasure()
* Description: Units of measure for PCL dot commands.
*              PCL format: ESC&u<value>D
*               <value> = 96,100,120,144,150,160,180,200,225,240,288,300
*                         360, 400,450,480,600,720,800,900,1200,1440,1800,
*                         2400,3600, and 7200
* Arguments:   none.
* Return:      (void)
*****************************************************************************/

static void HPUnitsOfMeasure(void)
{
    Uint16 lower_value, higher_value, iii;
    static CONST Uint16 uom_dpi[] = 
		    { 96, 100, 120, 144, 150, 160, 180, 200, 225, 240, 288, 
			  300, 360, 400, 450, 480, 600, 720, 800, 900,
			  1200, 1440, 1800, 2400, 3600, 7200 };


    if (parse.mac.macroDef) {
        return;
    }

    switch (parse.ch) 
	{
	  case 'D':
	  case 'd':
	    /* check that value is in range (96..7200) and that it is
	       an integral divisor of 7200 */
	    if ( parse.wholeVal < 96 )
		parse.wholeVal = 96;
	    else if ( parse.wholeVal > 7200 )
		parse.wholeVal = 7200;

	    for ( iii=0; ; iii++ )
		{
		    if ( parse.wholeVal <= uom_dpi[iii] )
			{
			    higher_value = uom_dpi[iii];
			    break;
			}
		}

	    if ( higher_value == parse.wholeVal )
		printer.units_of_measure_x_dpi
			  = printer.units_of_measure_y_dpi = parse.wholeVal;
	    else
		{
		    lower_value = uom_dpi[iii-1];
		    /* minimize the relative error:  the relative
		       error of a unit selection of 4801 is closer to
		       7200 than to 3600 tive error:  the relative
		       error of a unit selection of 4801 is closer to
		       7200 than to 3600 
		       ( (7200-4801)/7200 < (4801-3600)/3600 )
		       => (7200-4801)*3600 < (4801-3600)*7200 */
		    if ( ( higher_value - parse.wholeVal ) * lower_value 
			   <= ( parse.wholeVal - lower_value ) * higher_value )
			printer.units_of_measure_x_dpi = 
				printer.units_of_measure_y_dpi = higher_value;
		    else
			printer.units_of_measure_x_dpi = 
				printer.units_of_measure_y_dpi = lower_value;
		}

	    printer.one_uom_pel_in_piux = PIU_XDPI/printer.units_of_measure_x_dpi;
	    printer.one_uom_pel_in_piuy = PIU_YDPI/printer.units_of_measure_y_dpi;
	    PDI1SetUnitsOfMeasure( printer.units_of_measure_x_dpi,
				   printer.units_of_measure_y_dpi );
	    printer.hmi_piu = -1;    /* reset HMI to its default value */
	    PDIspacingmode(PROPORTIONAL, -1);
	    PCLDEBUG1(4,"Units of measure: %d\n", printer.units_of_measure_x_dpi );
	    HPCheckPageChar();
	    break;

	case 'W':           /* Esc&u<n>W -- undefined W command */
	    UndefCommand();
	    break;
	}
}


void HPUnitsOfMeasureReset(void)
{
    printer.units_of_measure_x_dpi = PCL_XUNITS_300DPI;
    printer.units_of_measure_y_dpi = PCL_YUNITS_300DPI;
    printer.one_uom_pel_in_piux = PIU_XDPI/printer.units_of_measure_x_dpi;
    printer.one_uom_pel_in_piuy = PIU_YDPI/printer.units_of_measure_y_dpi;
    PDI1SetUnitsOfMeasure( printer.units_of_measure_x_dpi,
			   printer.units_of_measure_y_dpi );
}

#ifdef PDI1_USING_COLOR
#if !defined(NO_COLOR_ADJUSTMENTS)
/*****************************************************************************
 * Synopsis:    void DriverConfiguration()
 * Description: On parsing the escape sequence of the form <esc>*o#W;
 *              set up to get data for driver configuration command
 *      
 * Arguments:   none.
 * Return:      (void)
 *****************************************************************************/

static void DriverConfiguration(void)
{
    Uint8 ch;

    if (parse.mac.macroDef || parse.relative == REL_ERROR) {
	return;
    }

    ch = TOUPPER(parse.ch);

    switch (ch) {

    case 'W':       /* <Esc>*o#W    */
	if (parse.wholeVal == 3) {	/* Accept Incoming info */
	    parse.nDataBytes = parse.nBytesToGo = parse.wholeVal;
	    parse.state = GetDriverConfig;
	    parse.stateSet = 1;
	}
	else {				/* Absorb Incoming Data */
	    parse.nDataBytes = parse.nBytesToGo = parse.wholeVal;
	    parse.state = Count;
	    parse.stateSet = 1;
	}
	break;

    }
}
#endif /* !NO_COLOR_ADJUSTMENTS*/
#endif /* defined (PDI1_USING_COLOR) */

#ifdef CAN_RENDER_COLOR

/* Note that monochrome print mode is encapsulated within a CAN_RENDER_COLOR
 * instead of a PDI1_USING_COLOR.  This is because mono print mode switches
 * the current rendering model from a color model to a "best" gray.  If we're
 * not doing color, why bother?
 */

/*****************************************************************************
 * Synopsis:    void monochromeprint()
 * Description: On parsing the escape sequence of the form <esc>&b#M;
 *              set up to get the setting for monochrome print mode.
 *
 * Arguments:   none.
 * Return:      (void)
 *****************************************************************************/

static void monochromeprint(void)
{
    Uint8 ch;

    if (parse.mac.macroDef || parse.relative == REL_ERROR)
	return;

    ch = TOUPPER(parse.ch);

    if (ch == 'M') {
	if (parse.wholeVal == 0 || parse.wholeVal == 1) {
	    if (!SOMETHINGONPAGE) {
	    	HPCarriageReturn();
	    	HPCursorFirstLine();
#ifdef NO_PCL_MONO_PRINT_MODE
		PCLWARNING("PCL Monochrome Print Mode is disabled.\n");
		PCLWARNING("--> To enable, add a gray scale rendering model to your build configuration.\n");
#else
	    	PDI1setmonoprint((Uint8)parse.wholeVal);
#endif
	    }
	}
    }
    else    /* just to swallow Esc&b<n>W -- undefined W command */
	UndefCommand();
}
#endif /* defined (CAN_RENDER_COLOR) */

#if defined (PDI1_USING_COLOR)
/*****************************************************************************
 * Synopsis:    void DownloadDitherMatrix()
 * Description: On parsing the escape sequence of the form <esc>*m#W;
 *              set up to get data for downloaded matrix command.
 *
 * Arguments:   none.
 * Return:      (void)
 *****************************************************************************/

static void DownloadDitherMatrix(void)
{
       /*Ignore command if in macro mode or if value error*/
    if ( parse.mac.macroDef || parse.relative==REL_ERROR )
	return;

    if ( parse.wholeVal > 0 )
    {
	if ( parse.rastXfer )
	{
	    parse.nBytesToGo = parse.wholeVal;
	    parse.state = Count;
	    parse.stateSet = 1;
	} 
	else 
	{            
	    parse.nBytesToGo = parse.nDataBytes = parse.wholeVal;
	    parse.state = UddpStart;
	    parse.stateSet = 1;
	}
    }       
}
#endif /* defined (PDI1_USING_COLOR) */

#ifdef PDI1_USING_COLOR
#if !defined(NO_COLOR_ADJUSTMENTS)
/*****************************************************************************
 * Synopsis:    void ViewingIlluminant()
 * Description: On parsing the escape sequence of the form <esc>*i#W;
 *              set up to get data for viewing illuminant command.
 *
 * Arguments:   none.
 * Return:      (void)
 *****************************************************************************/

static void ViewingIlluminant(void)
{
    Uint8 ch;

    if (parse.mac.macroDef || parse.relative == REL_ERROR)
	return;

    ch = TOUPPER(parse.ch);
 
    if (ch == 'W') {    /* <Esc>*i#W    */
	if(parse.wholeVal == 8){    /* Accept Incoming info */
	    parse.nDataBytes = parse.nBytesToGo = parse.wholeVal;
	    parse.state = ViewIllum;
	    parse.stateSet = 1;
	}
	else {          /* Absorb Incoming Data */
	    parse.nDataBytes = parse.nBytesToGo = parse.wholeVal;
	    parse.state = Count;
	    parse.stateSet = 1;
	}
    }
}
#endif  /* !NO_COLOR_ADJUSTMENTS */
#endif  /* defined (PDI1_USING_COLOR) */

# ifdef DEBUG

/*****************************************************************************
 * Synopsis:    void HPDebugLo()
 * Description: Set the low value for debugging to the command argument.
 * Arguments:   none.
 * Return:      (void)
 * Uses:
 * Sets:
 * Changes:
 * Heap Use:    none.
 *****************************************************************************/

static void HPDebugLo(void)
{
    Uint8 ch; /* ESC ! l <level> <subsystem> */

    ch = TOUPPER(parse.ch);

    if (parse.wholeVal >= 0 && parse.wholeVal <= 31) {
	/*
	 * Set the low limit debugging level.
	 * High is set to new low if it less than the new low.
	 */
	dbgsetlo(ch, parse.wholeVal);
    }
}


/*****************************************************************************
 * Synopsis:    void HPDebugHi()
 * Description: Set the high value for debugging to the command argument.
 * Arguments:   none.
 * Return:      (void)
 * Uses:
 * Sets:
 * Changes:
 * Heap Use:    none.
 *****************************************************************************/

static void HPDebugHi(void)
{
    Uint8 ch; /* ESC ! h <level> <subsystem> */

    ch = TOUPPER(parse.ch);

    if (parse.wholeVal >= 0 && parse.wholeVal <= 31) {
	/*
	 * Set the high limit debugging level.
	 * Low is set to new high if it greater than the new high.
	 */
	dbgsethi(ch, parse.wholeVal);
    }
}

/*****************************************************************************
 * Synopsis:    void HPDebugMask()
 * Description: Set the Mask for debugging.
 *              ESC ! m <mask> <subsystem>
 * Arguments:   none.
 * Return:      (void)
 * Uses:
 * Sets:
 * Changes:
 * Heap Use:    none.
 *****************************************************************************/

static void HPDebugMask(void)
{
    DbgId id;
    Sint32 mask;
    id = parse.ch;
    mask = parse.wholeVal;
    dbgsetmask(id, mask);
}

#endif /* DEBUG */

/*****************************************************************************
 * Synopsis:    Boolean Is2ByteChar()
 * Description: Determine if character read is to be processed as 1 or 2 byte
 * Arguments:   none.
 * Return:      TRUE if 2-byte character encountered.
 * Uses:
 * Sets:
 * Changes:
 * Heap Use:    none.
 *****************************************************************************/
Boolean Is2ByteChar(void)
{
    int status = FALSE;

    switch (pMode) {          /* check the text parsing method selected */
	case METHOD0:  /* This case is here to avoid a compiler warning. */
	    break;
	case METHOD21:
	    if (parse.ch > 0x20 && parse.ch <= 0xFF)
		status = TRUE;
	    break;
	case METHOD31:
	    if ((parse.ch > 0x80 && parse.ch < 0xA0) ||
		(parse.ch > 0xDF && parse.ch < 0xFD))
		status = TRUE;
	    break;
	case METHOD38:
	    if (parse.ch > 0x7F && parse.ch <= 0xFF)
		status = TRUE;
	    break;
    }
    return(status);
}

/*****************************************************************************
 * Synopsis:    void UndefCommand()
 * Description: Look for 'W' at the end of an undefined commmand.
 * Arguments:   none.
 * Return:      (void)
 * Uses:
 * Sets:
 * Changes:
 * Heap Use:    none.
 *****************************************************************************/

static void UndefCommand(void)
{
    Uint8 ch = TOUPPER(parse.ch);

    if (ch == 'W') {
	if (parse.wholeVal > 0) {
	    /*
	     *  The 'W' at the termination of a command means that some #
	     *  of data bytes follow the command.  To properly parse the
	     *  undefined commands with following databytes, skip over
	     *  the # of bytes before starting to parse the next command.
	     */
	    parse.nBytesToGo = parse.wholeVal;
	    parse.state = Count;
	    parse.stateSet = 1;
	}
    }
}

/*****************************************************************************
 * Synopsis:    void CharTxtPathDir()
 * Description: Allows vertical printing for Asian markets, which use both
 *      horizontal and vertical printing.
 * Arguments:   none.
 * Return:      (void)
 * Uses:
 * Sets:
 * Changes:
 * Heap Use:
 *
 *  Executes the escape sequence of the form <esc>&c#T, where T is decoded.
 *****************************************************************************/
static void CharTxtPathDir(void)
{
    Uint8 ch = TOUPPER(parse.ch);
 
    if (parse.mac.macroDef)
	return;
 
    if (ch == 'T') {
	if (parse.wholeVal != printer.textDir) {
	    switch(parse.wholeVal)
	    {
		case 0:                     /* Horizontal Printing */
		    printer.textDir = HORIZONTAL;
		    PDItextdir(HORIZONTAL);
		    break; 
		case 1:                     /* (-1) Rotated Vertical Printing */
		    if (parse.relative == REL_NEG) {
			printer.textDir = VERTICAL;
			PDItextdir(VERTICAL);
		    }
		    break;
	    }
	    if (options.primary)
		HPSetChooseFont(&primFont, FALSE);
	    else
		HPSetChooseFont(&secFont, FALSE);
	}
     }
     else       /* just to swallow Esc&r<n>W -- undefined W command */
	 UndefCommand();
}

/*****************************************************************************
 * Synopsis:    void TxtParsingMethod()
 * Description: Provides a method for specifying character codes to select
 *              characters in large fonts (>256 characters).
 * Arguments:   none.
 * Return:      (void)
 * Uses:
 * Sets:
 * Changes:
 * Heap Use:
 *
 *  Executes the escape sequence of the form <esc>&t#P, where P is decoded.
 *****************************************************************************/
static void TxtParsingMethod(void)
{
    Uint8 ch = TOUPPER(parse.ch);
 
    if (parse.mac.macroDef)
	return;
 
    if (ch == 'P') {
	switch(parse.wholeVal)
	{
	   case 0:
	   case 1:
	     /* all characters processed as 1-byte char */
	     pMode = METHOD0;
	     break;

	   case 21:
	     /* char code bet 0x21 - 0xFF are processed as 1st byte &
		the following byte as 2nd byte of 2-byte char */
	     pMode = METHOD21;
	     break; 

	   case 31:
	     /* char code bet 0x81 - 0x9F and 0xE0 - 0xFC are processed
		as 1st byte & the following byte as 2nd byte of 2-byte
		char */
	     pMode = METHOD31;
	     break;

	   case 38:
	     /* char code bet 0x80 - 0xFF are processed as 1st byte &
		the following byte as 2nd byte of 2-byte char */
	     pMode = METHOD38;
	     break;

	   default:
	     /* No change */
	     break;
	}
     }
     else       /* just to swallow Esc&r<n>W -- undefined W command */
	 UndefCommand();
}

/*****************************************************************************
 * Synopsis:    void AlphanumericIDCommand()
 * Description: Handle all operations of the AlphanumericID command.
 * Arguments:   none.
 * Return:      (void)
 * Uses:
 * Sets:
 * Changes:
 * Heap Use:
 *
 *  Executes the escape sequence of the form <esc>&n#W, where W is decoded.
 *****************************************************************************/
void AlphanumericIDCommand (void)
{
    Sint32 tmp;
    Uint8 operation;

    if (parse.mac.macroDef)
	return;

    if (parse.mac.macroLevel) {
	operation = *parse.mac.macroBuf[parse.mac.macroLevel];
	parse.mac.macroBuf[parse.mac.macroLevel] += 1;
    } else {
	GETCHAR(tmp);
	if (tmp < 0) {
	    UNGETCHAR();
	    return;
	}
	operation = ((Uint8) tmp & 0xff);
    }
    
    if (operation == 20) {
	/* Delete the font associatation for the current FontID */	
       HPDeleteFontAssociation(options.fontID);
    } else if (operation == 21) {
	/* Delete the macro associatation for the current MacroID */
	HPDeleteMacroAssociation(options.macroID);
    } else {
	int i;
	Sint8 *StringID = NULL;
	
	/* The StringID is of the length of parse.wholeVal-1 + 1 (for stroring
         * the null character. */
	if (parse.wholeVal>=2 && parse.wholeVal<=512) {
	    StringID = (Sint8 *)PDI1MALLOC(parse.wholeVal);
	    if (StringID == NULL) {
		SIGMOJO();
		return;
	    }
	} else {
	    /* Illegal length for the StringID */
	    /* put back the operation byte so that UndefCommand() works fine */
	    UNGETCHAR();  
	    UndefCommand();
	    return;
	}
	
	for (i=0; i<parse.wholeVal-1; i++) {
	    if (parse.mac.macroLevel) {
		StringID[i] = *parse.mac.macroBuf[parse.mac.macroLevel];
		parse.mac.macroBuf[parse.mac.macroLevel] += 1;
	    } else {
		GETCHAR(tmp);
		if (tmp < 0) { UNGETCHAR();	return; }
		StringID[i] = ((Uint8) tmp & 0xff);
	    }
	}
	StringID[i] = '\0';

	switch (operation)
	{
	case 0: /* Set the current FontID to the given StringID */
	    if (options.fontID.idType == STRINGID &&
		options.fontID.name.stringID != NULL)
	    {
		PDI1FREE(options.fontID.name.stringID);
		options.fontID.name.stringID = NULL;
	    }

	    options.fontID.idType = STRINGID;
	    options.fontID.name.stringID = StringID;
	    break;
	case 1: /* Associate current FontID to the font with the given StringID */
	    HPAssociateFontID(options.fontID, StringID);
	    if (StringID != NULL) { PDI1FREE(StringID); StringID = NULL; }
	    break;
	case 2: /* Select the font referred to by the given StringID as primary */
	    HPSelectStringFontID(StringID, TRUE);
	    if (StringID != NULL) { PDI1FREE(StringID); StringID = NULL; }
	    break;
	case 3: /* Select the font referred to by the given StringID as secondary */
	    HPSelectStringFontID(StringID, FALSE);
	    if (StringID != NULL) { PDI1FREE(StringID); StringID = NULL; }
	    break;
	case 4: /* Set the current MacroID to the given StringID */
	    if (options.macroID.idType == STRINGID &&
		options.macroID.name.stringID != NULL) {
		PDI1FREE(options.macroID.name.stringID);
		options.macroID.name.stringID = NULL;
	    }
	    
	    options.macroID.idType = STRINGID;
	    options.macroID.name.stringID = StringID;
	    break;
	case 5: /* Associate current MacroID to the macro with the given StringID */
	    HPAssociateMacroID(options.macroID, StringID);
	    if (StringID != NULL) { PDI1FREE(StringID); StringID = NULL; }
	    break;
	case 100: /* Media select: not supported yet. Just ignore the command. */
	    if (StringID != NULL) { PDI1FREE(StringID);	StringID = NULL; }
	    /* HPReset(HPRESETNOTEMPS); not when ignoring cmd */
	    break;
        default:
	    /* illegal operation */
	    if (StringID != NULL) {
		PDI1FREE(StringID);
		StringID = NULL;
	    }
	    return;
	}
    }
}

/*****************************************************************************
 * Synopsis:    void HPDoEscSequence()
 * Description: Execute an escape sequence.
 * Arguments:   none.
 * Return:      (void)
 * Uses:
 * Sets:
 * Changes:
 * Heap Use:
 *
 *  Main procedure that calls the appropriate escape sequence execution
 *  procedures to decode the terminating character and execute the function.
 *  This is called from the parser once it has collected the complete
 *  escape sequence.  All of the arguments are implicit in the parse
 *  state structure.  Every one of the cases below represents the first
 *  two bytes after the escape character (with some minor exceptions);
 *  most of the routines called contain a switch on the terminator character.
 *
 *  When adding a new handler, be sure and give it a case for 'W'
 *  even when H.P. has not explicitly defined such a command.
 *  Their parser treats those as special, but ours does not;
 *  the parameter to anything terminated with 'W' is a byte count,
 *  and that much has to be swallowed following the 'W' whether
 *  it is of any use or not.  The parser will do that but these
 *  handler routines have to get it into the mood to do so.
 *****************************************************************************/

void HPDoEscSequence(void)
{
    switch (parse.groupType) {

    case CURSOR_MARGIN:         /* ESC & a... */
	 CursorMargin();        break;
#ifdef CAN_RENDER_COLOR
    case MONOCHROME_PRINT:      /* ESC & b... */
	 if (IS_COLORFORMAT_GRAY(CurrentDevice.colorFormat)) break;
	 monochromeprint();     break;
#endif
    case CHAR_TXT_PATH_DIR:     /* ESC & c... */
	 CharTxtPathDir();  break;
    case PUSH_POP_MACRO:        /* ESC & f... */
	 PushPopMacro();        break;
    case TERM_PITCH_HMI:        /* ESC & k... */
	 TermPitchHmi();        break;
    case LPI_LEN_MARGIN:        /* ESC & l... */
	 LpiLenMargin();        break;
    case UNDERLINE_ON_OFF:      /* ESC & d... */
	 UnderlineOnOff();      break;
     case ALPHANUMERIC_ID_COMMAND: /* ESC & n... */
     AlphanumericIDCommand(); break;
    case TRANSPARENT_PRINT:     /* ESC & p... */
	 TransparentPrint();    break;
    case FLUSH_PAGES:           /* ESC & r... */
	 FlushPages();          break;
    case ON_OFF_LINE_WRAP:      /* ESC & s... */
	 OnOffLineWrap();       break;
    case TXT_PARSING_METHOD:    /* ESC & t... */
	 TxtParsingMethod();    break;
    case FONT_MANAGEMENT:       /* ESC ( s... and also ESC ) s... */
	 FontManagement();      break;
    case SYMBOL_SET_FONT_ID:    /* ESC ( <n> and also ESC ) <n> */
	 SymbolSetFontId();     break;
    case START_END_GRAPHICS:    /* ESC * r... */
	 StartEndGraphics();    break;
    case GRAPH_XFER_COMPRESS:   /* ESC * b... */
	 GraphXferCompress();   break;
    case GRAPHICS_RESOLUTION:   /* ESC * t... */
	 GraphicsResolution();  break;
    case FONT_GRAPHICS:         /* ESC * c... */
	 FontGraphics();        break;
    case PRINT_MODEL:           /* ESC * v... */
	 PrintModel();          break;
    case PIXEL_PLACEMENT:       /* ESC * l... */
	 PixelPlacement();      break;
#if defined (PDI1_USING_COLOR)
    case DOWNLOAD_DITHER_MATRIX: /* ESC * m... */
	 if (!IS_PDI1_USING_COLOR(CurrentDevice.colorFormat)) break;
	 DownloadDitherMatrix(); break;
#endif
#ifdef PDI1_USING_COLOR
#if !defined(NO_COLOR_ADJUSTMENTS)
    case VIEWING_ILLUMINANT:    /* ESC * i... */
	 if (!IS_PDI1_USING_COLOR(CurrentDevice.colorFormat)) break;
	 ViewingIlluminant();   break;
    case DRIVER_CONFIGURATION:  /* ESC * o... */
	 if (!IS_PDI1_USING_COLOR(CurrentDevice.colorFormat)) break;
	 DriverConfiguration(); break;
#endif
#endif
    case DOT_MOVEMENT:          /* ESC * p... */
	 DotMovement();         break;
    case GRP_HPGL:              /* ESC % <n>  */
	 HPDoPercent();         break;
    case STATUS_READBACK:       /* ESC * s... */
	 PCLStatusRB();         break;
    case GRP_UNITS_OF_MEASURE:  /* ESC & u... */
	 HPUnitsOfMeasure();    break;
    case SYMSET_MANAGEMENT:     /* ESC ( f... */
	 SymSetManagement();    break;

/*  Our own magic follows:  */
#ifdef HP_PORT_SUPPORT
    case GRP_PORT:              /* ESC | p<value><ch> (non-standard!) */
	 HPPort(parse.ch,parse.wholeVal); 
	 break;
#endif
	
#if defined(GENCMPLXMET) & defined(COMPLEXITY_METRICS)
    case GRP_COMPLEXITY:              /* ESC | q (non-standard!) */
	 if (parse.ch == 'X') {
	     cplxmetcalc();           /* ESC | q # X - ignore # (non-standard!) */
	 }
	 break;
#endif

#ifdef DEBUG
    case GRP_DEBUG_HI:          /* ESC ! l... (non-standard!) */
	 HPDebugHi();           break;
    case GRP_DEBUG_LO:          /* ESC ! h... (non-standard!) */
	 HPDebugLo();           break;
    case GRP_DEBUG_MASK:        /* ESC ! m... (non-standard!) */
	 HPDebugMask();         break;

#if defined(PCL_IMAGE_COMPRESS) | defined(PCL_FONT_CACHE_COMPRESS) | defined(PCL_BAND_COMPRESS) | defined(PCL_LOSSY_COMPRESS)
    case GRP_CMPR:              /* ESC | c... (non-standard!) */
	 HPCompression();       break;
#endif

#if defined(PCL_FONT_SMOOTHING) | defined(COMPLEXITY_METRICS)
    case GRP_SMOOTHING:         /* ESC | s... (non-standard!) */
	 HPmiscellaneous();     break;
#endif

#else  /* not DEBUG */

#ifdef COMPLEXITY_METRICS       /* complexity metrics enabled but no debug */
    case GRP_SMOOTHING:         /* ESC | s <n> B for controlling monsters (non-standard!) */
	 HPmiscellaneous();     break;
#endif

#endif /* not DEBUG */

    case GROUP_UNDEF:           /* unknown */
	 UndefCommand();        break;

    default:
	 break;
    }
}


/*****************************************************************************
 * Synopsis: HPDeleteFontAssociation    
 * Description: Delete the font association of the current font..
 * Arguments:   
 *              (HPFontID) fontID
 * Return:   
 * Uses:
 * Sets:
 * Changes:
 * Heap Use:    none.
 ****************************************************************************/

void HPDeleteFontAssociation(HPFontID fontID)
{
    FONT  *pFont;
    Sint32 fontNum;

    /* Find the currently selected font */
    fontNum = HPFindFontId(options.fontID, TRUE);

    if (fontNum != -1) 
    {
	pFont = PDIfonthead(fontNum);

	/* "pFont->originalStringID==NULL" indicates that this font is NOT 
	 * downloaded by using AlphanumericID commands, so we have to leave 
	 * it along. */
	if (pFont->originalStringID != NULL)
	{

	/*
         * Close the mass storage font files. Once the font has
         * been de-associated the idea is that we are done using
         * it for the time being.
         */
	  if (pFont->scalable)
	     PDI1MassFontCleanup(pFont); 


	    if (pFont->FontID.idType == STRINGID &&
		pFont->FontID.name.stringID != NULL)
	    {
		PDI1FREE(pFont->FontID.name.stringID);
		pFont->FontID.name.stringID = NULL;
	    }
	    pFont->FontID.idType = INVALID_ID;
	    pFont->FontID.name.numericID = 0;

	    /* TBA:
	     *
	     * If pFont->Origin == OrgRomDLL && pFont->scalable,
	     * we close the font file, and completely delete the entry
	     * for this font.   (font files for bitmap files are closed
	     * after loading the bitmap font data).
	     */
	}
    } 
}


/*****************************************************************************
 * Synopsis:    HPAssociateFontID(fontID, originalStringID)
 * Description: Associate the current font to originalStringID
 * Arguments:   
 *              (HPFontID) fontID
 *              (Sint8 *)originalStringID
 * Return: 
 *              None
 ****************************************************************************/
void HPAssociateFontID(HPFontID fontID, Sint8 *originalStringID)
{
    FONT  *pFont;    
    Sint32 fontNum;
    HPFontID orignalID;
    
    ASSERT(fontID.idType != INVALID_ID);

    /* Check if the fontID is used by a traditional downloaded font or 
     * associated with a font defined/downloaded using alphanumericID 
     * commands.  If so, it cannot be reassociated to the new 
     * font indicated by the originalStringID. */
    fontNum = HPFindFontId(fontID, TRUE);
    if (fontNum != -1)
    {
	return;
    }

    orignalID.idType = STRINGID;
    orignalID.name.stringID = originalStringID;

    /* AlphanumeriID command: font management commands
     *
     * The 2nd argument (a boolean) indicates that the 'fontID' should be compared
     * to the originalStringID for fonts that are downloaded (readed from disks) to
     * a StringID, but NOT the associatedID.  For fonts that are downloaded in PCL
     * to a numeric value (this is indicated by: the originalStringID of the FONT 
     * strcut being NULL), this argument is ignored. */
    fontNum = HPFindFontId(orignalID, FALSE);

    if (fontNum == -1) {
	/* Not found in the current font table.
	 *
         * Look for the font in mass storage deviceis based the originalStringID
         * as the file name. */

      /* 
       * When we find the mass storage font, tell the font renderer
       * glue code to close the file and save the offsets into the file.
       * This will save memory when dealing with large amounts of
       * mass storage fons.
       */

	fontNum = PDI1SearchMassFont(originalStringID,1);
    }

    if (fontNum != -1) 
    {
	pFont = PDIfonthead(fontNum);

	if (fontID.idType == NUMERICID)
	{
	    pFont->FontID.idType = NUMERICID;
	    pFont->FontID.name.numericID = fontID.name.numericID;
	} else {
	    pFont->FontID.name.stringID = (Sint8 *)PDI1MALLOC(1+strlen(fontID.name.stringID));
	    if (pFont->FontID.name.stringID != NULL) {
		pFont->FontID.idType = STRINGID;
		strcpy(pFont->FontID.name.stringID, fontID.name.stringID); 
	    }
	}
    } 

}


/*****************************************************************************
 * Synopsis:    HPSelectStringFontID(stringID, primary)
 * Description: select font as either primary or secondary
 *
 * Arguments: (Sint8 * ) stringID - font to select as either primary or secondary
 *            Boolean   primary  - primary Font (TRUE) || Secondary Font (FALSE )
 * Return: 
 *            None
 ****************************************************************************/
void HPSelectStringFontID(Sint8 *stringID, Boolean primary)
{
    HPFontID temp_fontID;
    
    temp_fontID.idType = STRINGID;
    temp_fontID.name.stringID = stringID;

    HPDesignateFont(temp_fontID, primary);
    HPClearJustBackspaced();  
}


/* Local Variables:
 * mode: C
 * tab-width: 8
 * c-indent-level: 4
 * c-basic-indent: 4
 * End:
 */
/* Used by some versions of vim: set tabstop=8 shiftwidth=4: */
