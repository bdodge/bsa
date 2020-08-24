/*
 *  Start of Oak Standard Header
 *  Copyright (c) 1991 - 2003 Oak Technology, Inc.
 *
 *  All rights reserved.  Proprietary and confidential.
 *
 *  DESCRIPTION for pssnif.c
 *      PS Sniffing Module.
 * 
 *  NEW HISTORY COMMENT (description must be followed by a blank line)
 *  <Enter change description here>

 *  ===== HISTORY of changes in //depot/ips/projects/rogaine/ps/pssnif.c
 *  
 *  26/Jun/03 #6 bdodge separate entry for pdf sniffer
 *  24/Jun/03 #5 bdodge exclude PJL
 *  16/Apr/03 #4 bdodge use new interface
 *  16/Apr/03 #3 bdodge denodeify
 *  
 *  ===== HISTORY of changes in //depot/ips/releases/rel6_0/ps/pssnif.c
 *  
 *  5/Nov/02 #2 mtidd Fix link problem when no sniffer.
 *  
 *  ===== HISTORY of changes in //depot/ips/releases/rel5_0/ps/pssnif.c
 *  
 *  16/Nov/01 #3 psanclem Actively set OMSNIFFEDPDF to FALSE when sniffing PS
 *  27/Jul/01 #2 daly Add dstefanovic's fix for cq21826 sniffing problem to
 *  the rel5_0 database (cq23648)
 *  
 *  ===== HISTORY of changes in //depot/ips/projects/cleanup/ps/pssnif.c
 *  
 *  31/Mar/00 #2 rtomasso Change char to Sint8
 *  
 *  ===== HISTORY of changes in //depot/ips/projects/highwire/ps/pssnif.c
 *  
 *  11/Oct/99 #2 mtidd Add support for PDF.
 *  
 *  ===== HISTORY of changes in //depot/ips/cor/ps/pssnif.c
 *  
 *  31/Oct/97 #3 daly Fix some compiler warnings in ppc builds.
 *  
 *  End of Oak Standard Header
 * 
 *
 ** IPSNEXT MODIFICATIONS
 ** 18-Oct-96 tomd use 
 ** 18-Oct-96 tomd 
 **
 ** MODIFICATIONS
 ** 1.16 27-Jun-96 tomd include <string.h>.
 **                     do not inlude util.h, unixtarg.h
 ** 1.15 14-Jan-96 tomd use PSENDOFJOB
 ** 1.14 07-Dec-95 tomd if pssniff sees control-D, claim SUCCESS.
 ** 1.12 31-Oct-95 tomd omit obsolete includes. omit SIC support.
 ** 1.11 06-Oct-95 tomd include sttable.h
 ** 1.10 19-Jul-95 PJD  Add unixtarg.gh
 **      15-Jun-95 E K  Standardize copyright header and/or SCCS strings.
 **  1.9 12-Dec-94 JGG  Added "/statusdict" string for Ethertalk Unix build
 **                     fixes {#16876)
 **  1.8 27-Oct-93 FA   Make <return> a white space character too.
 **  1.7 12-Oct-93 JGG  Added include of em.gh
 **  1.6 09-Jul-93 JJS  Add om.h.
 **  1.5 09-Feb-93 PJD  Add #include of generated configuration files.
 **  1.4 28-Aug-92 RMH  Conditionally compile HI stuff.
 **  1.3 10-Aug-92 RMH  Fix typo in call to pswhitespace().
 **  1.2 06-Aug-92 RMH  Added SIC to stop conditions (it should have
 **                     been there all along).
 **  1.1 02-Jun-92 RMH  Moved to sn directory
 **
 ** Start of history in sn directory ****************************************
 **  1.6 01-Jun-92 RMH  Sniffer mods for PS/Not-PS
 **  1.5 22-Apr-92 RMH  Add <ESC>E to stop conditions, fix typo in ULE.
 **  1.4 12-Mar-92 RMH  Add sniffer stop conditions.
 **  1.3 04-Feb-92 RMH  Include hiport.h before es.h and sn.h
 **  1.2 03-Feb-92 PJD  Change bind to static to fix multiple defined.
 **  1.1 01-Feb-92 RMH  Created
 **/
#include <string.h>

#include "univ.gh"
#include "em.gh"
	
#include "arch.h"
#include "pile.h"
	
#include "dbg.h"
#include "propman.h"
	
#include "sn.h"
#include "psproto.h"
	
	
/* These are the PostScript strings we recognize
*/
static CONST    Sint8    dictbegin[] = "dict begin";
static CONST    Sint8    binddef[] = "bind def";
static CONST    Sint8    findfont[] = "findfont";
static CONST    Sint8    showpage[] = "showpage";
static CONST    Sint8    statusdict[] = "/statusdict";

static Boolean pswhitespace(Sint8 c)
{
	/* SPACE */    /* Tab *//* NewLine *//* return */
	return(c == 0x20 || c == 0x09 || c == 0x0a || c == 0x0d);
}

static Sint32 SniffPSorPDF(char *buffer, Sint32 length, char* buf2, Sint32 len2, Boolean* pIsPDF)
{
    Sint32 retval;
    char *start, *end;
    
    char iob[SNIFF_LENGTH+1], nc;
    int  rcnt, len;
    
    *pIsPDF = FALSE;
    
    /* copy a bunch of bytes of input into a contiguous space
    */
    for(rcnt = len = 0; rcnt < (length + len2) && len < SNIFF_LENGTH; rcnt++)
    {
	if(rcnt > length)
            if(rcnt > length + len2)
		break; /* end of data */
            else
		nc = buf2[rcnt - length];
        else
            nc = buffer[rcnt];
	iob[len++] = nc;
    }
    iob[len] = '\0';
    
    if(len == 0)
    {
	return 50;	/* no data, so, yeah, like why you callin me? */
    }
    start = iob;
    end   = iob + len;
    retval = -1;
    
    /* go ahead and sniff it
    */    
    while(start < end && retval < 0) {
	
	switch(*start++) {
            
	case ESCAPE:
            retval = 0;
            break;
            
	case PSENDOFJOB:
            retval = 100;
#ifdef EMPDF
	    *pIsPDF = FALSE;
#endif
	    break;

	case '@':
	    if (end - start > 2) {
	        if(! strncmp(start, "PJL", 3))
	            retval = 0;
	    }
	    else if(! strncmp(start, "PJL", (end - start))) {
	        retval = 20; /* probably not ps */
	    }
	    break;

	case '%':
	    if (start < end) {
		if(*start == '!') {
#ifdef EMPDF
		  char*	start2;
		  char*	end2;

		  start2 = start;
		  end2   = start;
		  while ((end2 < end) && (*end2 != 0xa) && (*end2 != 0xd)) {
		    end2++;
		  }
		  while ((start2 < end2) && (retval < 0)) {
		    while ((start2 < end2) && ((*start2 == 0x20) 
			   || (*start2 == 0x9))) {
		      start2++;
		    }	
		    if ((end2-start2) >= 4) {
		      if (!strncmp(start2, "PDF-", 4)) {
			retval = 100;
			*pIsPDF = TRUE;
		      } else {
			while ((start2 < end2) && (*start2 != 0x20) 
			       && (*start2 != 0x9)) {
			  start2++;
			}
		      }
		    } else {
		      start2 = end2;
		    }
		  }
		  if (start2 >= end2) {
		    retval = 100;
		    *pIsPDF = FALSE;
		  }
#else
		  retval = 100;
#endif	/* EMPDF */
		}
		else if(*start == '%') {
		    retval = 100;
#ifdef EMPDF
		    *pIsPDF = FALSE;
#endif
#ifdef EMPDF
		} else {
		  if ((end-start) >= 4) {
		    if (!strncmp((start), "PDF-", 4)) {
		      retval = 100;
		      *pIsPDF = TRUE;
		    }
		  }
#endif
		}
	    }
	    break;

	    /* }[ \t\n]*def */
	case '}':
	    while((start < end) && pswhitespace(*start))
		start++;

	    if (start + 2 < end) {
		if(!strncmp(start,"def",3)) {
		    retval = 100;
#ifdef EMPDF
		    *pIsPDF = FALSE;
#endif
		}
	    }
	    break;

	case 'd':
	    if (start + 8 < end)
		{
		    if (!(strncmp(dictbegin, start-1, 10)))
			{
			    retval = 100;
#ifdef EMPDF
			    *pIsPDF = FALSE;
#endif
			}
		}
	    break;

	case 'f':
	    if (start + 6 < end)
	    {
	        if (!(strncmp(findfont, start-1, 8)))
	        {
	            retval = 100;
#ifdef EMPDF
	            *pIsPDF = FALSE;
#endif
	        }
	    }
	    break;
	    
	case 'b':
	    if (start + 6 < end)
	    {
	        if (!(strncmp(binddef, start-1, 8)))
	        {
	            retval = 100;
#ifdef EMPDF
	            *pIsPDF = FALSE;
#endif
	        }
	    }
	    break;
	    
	case 's':
	    if (start + 6 < end)
	    {
	        if (!strncmp(showpage, start-1, 8))
	        {
	            retval = 100;
#ifdef EMPDF
	            *pIsPDF = FALSE;
#endif
	        }
	    }
	    break;
	    
	case '/':
	    if (start + 6 < end)
	    {
	        if (!strncmp(statusdict, start-1, 11))
	        {
	            retval = 100;
#ifdef EMPDF
	            *pIsPDF = FALSE;
#endif
	        }
	    }
	    break;
	    
	default:
	    break;
}
    }

    if (start >= end && length == SNIFF_LENGTH && retval < 0)
	retval = 0;

#ifdef INSNIFFER    
    DBGPRINTF2(DBGSN,1,
	       "PSsniff (%d): %d confident\n",
	       length,
	       retval
		);
#endif
    
    return(retval);
}


Sint32 PSSniff(char *buffer, Sint32 length, char* buf2, Sint32 len2)
{
	Sint32  isPS;
	Boolean isPDF;
	
    OMsetBool(omPSSNIFFEDPDF, OMCURRENT, FALSE);
	
	isPS = SniffPSorPDF(buffer, length, buf2, len2, &isPDF);
	
	if(isPS != 0)
	{
#ifdef EMPDF
		if(isPDF)
			return 0;
#else
		return isPS;
#endif
	}
	return isPS;
}

#ifdef EMPDF
Sint32 PDFSniff(char *buffer, Sint32 length, char* buf2, Sint32 len2)
{
	Sint32  isPS;
	Boolean isPDF;
	
	isPS = SniffPSorPDF(buffer, length, buf2, len2, &isPDF);
	
	if(isPS == 100)
	{
		if(isPDF)
		{
		    OMsetBool(omPSSNIFFEDPDF, OMCURRENT, TRUE);
			return 100;
		}
	}
	return isPDF ? 70 : (isPS ? 30 : 0);
}
#endif

/*
 * Format: tabs=8, indent=4 per 30-Apr-97 standard.
 * See "Local Variables in Files", GNU Emacs Manual.
 * Local Variables:
 * mode: C
 * tab-width: 8
 * c-indent-level: 4
 * End:
 */
/* Used by vim and some versions of vi: set tabstop=8 shiftwidth=4: */
