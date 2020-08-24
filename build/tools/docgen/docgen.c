/******************************************************************************
* Copyright (c) 2010 Zoran Corporation.
* $Header: //depot/imgeng/sw/inferno/appsrc/build/tools/docgen/docgen.c#5 $
* $Change: 152776 $ $Date: 2010/03/02 $
* *//**
* @file
* Documentation Generator  \author Brian Dodge
*
* This program generates an html documentation tree of a build source tree
* Starts at "ROOT" and self-recurses all directories looking for readme and
* overview files and builds an index of sub systems and links to the docs in each
* and puts it all in the doc directory specified by -o
*
* @ingroup Tools Documentation
* 
******************************************************************************/
#ifdef Windows
#include <Windows.h>
#else
#include <dirent.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/* include generated css file to write it out in local dir
 */
#include "gencss.h"

/* include generated doxygen config file format to generate
 * an actual doxygen config file
 */
#include "gendoxy.h"

#define MAX_SECTS		64
#ifndef MAX_PATH
	#define MAX_PATH	1024
#endif

#define MAX_INCLS		64
#define MAX_DEFS		64
#ifndef MAX_PATH
	#define MAX_PATH	1024
#endif

char* g_incls [MAX_INCLS];
char* g_defs  [MAX_DEFS];
int   g_nIncls;
int   g_nDefs;


int   verbose;		// be loud
int   gendocs;		// make an html tree browsing system
int   gendoxy;		// make a doxygen config file in output
int   gengraphs;	// include call-graphs in doxygen config

#ifdef Windows
#define PTC '\\'
#define PTS "\\"
#else
#define stricmp strcasecmp
#define _snprintf snprintf
#define PTC '/'
#define PTS "/"
#endif

char  rootpath[MAX_PATH];
char  outpath[MAX_PATH];
char  csspath[MAX_PATH];
char  doxypath[MAX_PATH];
char  topfile[MAX_PATH];

//! List of input files for doxy, to build the config file at the end.
//!  Each structure hold a list of doxygenizable files in the current directory.
//!  At the end of recursion, all the buffer blocks are contatenated
//
typedef struct tag_doxylist
{
	char*   buf;	///< buffer to hold all paths
	char*   pb;	///< points to current insert point in buffer
	int     bz;	///< size of allocation of buf
	struct tag_doxylist* next;	///< link
}
DOXY, *PDOXY;

//! list of doxy blocks
//
PDOXY g_doxylist = NULL;

static const char* header=
	"<html>\n"
	"<head>\n"
	"<title>%s</title>\n"
	"  <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n"
	"  <meta name=\"keywords\" content=\"quatro Inferno documentation\" />\n"
	"  <meta name=\"description\" content=\"html generated by %s\" />\n"
	"  <link href=\"%s\" rel=\"stylesheet\" type=\"text/css\" />\n"
	"</head>\n"
	"<body bgcolor=#FFFFFF>\n"
	"\n";

static const char* prolog=
	"<div class=\"title\">%s</div>\n"
	"<p>This web page collection was generated by %s and should not be modified directy."
	"<hr>\n"
	"<p><p>\n";

static const char* dochdr=
	"<div class=\"subsystitle\">Further Reading</div>\n"
	"<table>\n";

static const char* docfooter=
	"</table><br>\n";

static const char* subsyshdr=
	"<a name=\"subhdr%d\"></a>\n"
	"<div class=\"subsystitle\">Sub-System Documentation</div>\n"
	"<table>\n";

static const char* subsyshdrflat=
	"<div class=\"subsystitle\">Sub-System Documentation - all components</div>\n"
	"<table>\n";

static const char* subsysentry=
	"<tr><td><div class=\"subsysdir\"><a href=\"%s\">%s</a></div></td></tr>\n";

static const char* subsysstatic=
	"<tr><td><div class=\"subsysstatic const\">%s</a></div></td></tr>\n";

static const char* subsysfooter=
	"</table>\n";
	
static const char* seclink=
	"<div class=\"docdir\"><a href=%s>%s</a></div>\n";

static const char* secanchor=
	"<a name=\"%s\"></a>\n";

static const char* sectitle=
	"<br><div class=\"doctitle\">%s</div>\n";

static const char* sechdr=
	"<div class=\"doctext\"><p>\n";

static const char* secfooter=
	"</p></div>\n";
	
static const char* sublink=
	"<div class=\"docdir\"><a href=%s>%s</a></div>\n";
static const char* backlink=
	"<div class=\"docdir\"><a href=\"javascript:history.go(-1)\">%s</a></div>\n";
static const char* toplink=
	"<div class=\"docdir\"><a href=\"%s\">%s</a></div>\n";
static const char* doxylink=
	"<div class=\"docdir\"><a href=\"%s\">%s</a></div>\n";

static const char* supdoclink=
	"<tr><td><div class=\"docdir\"><a href=file://%s>%s</a></div></td></tr>\n";

static const char* footer=
	"<p><hr>\n"
	"<div class=\"copyright\">\n"
	"Document Copyright &copy; 2010 Zoran, Inc.\n"
	"</div>\n"
	"</body>\n"
	"</html>\n";

//*************************************************************************
//! Take a path and convert any / or \ to the char ptc,
//! convert DOS paths like C:xxxx to xxxxx
//! @retval pointer to the path handed in, maybe adjusted for DOS
//
char* NormalizeFilename(
			char* path,	///< [in] Path to normalize
			char ptc	///< [in] char which replaces and path chars
			)
{
	char* pbase = path;
	
	while(path && *path)
	{
		if(*path == '/' || *path == '\\')
			*path = ptc;
		path++;
	}
	if(ptc == '/')
	{
		if(pbase[0] && pbase[1] == ':')
		{
			pbase += 2;
		}
	}
	//fprintf(stderr, "npn=%s=\n", pbase);
	return pbase;
}

//**************************************************************************
//! calculate the relative path to get to path from curpath, in a directory
//! tree with absolute root path rootpath.
//! @retval pointer to the calculated path
//
char* RelativeHtmlPathTo(
			char* curpath,	///< [in] where the relative path starts, i.e. curdir
			char* rootpath,	///< [in] the top of the directory tree of interest
			char* path,	///< [in] the path to get to, can be absolute or relative
			char* result,	///< [out] buffer for result
			int nresult	///< [in] size of result buffer
			)
{
	int ups, downs, len;
	char* pp, *px;
	char dlc, dlr, dlp;
	
	ups = downs = 0;

	if(curpath[1] == ':')
	{
		dlc = curpath[0];
		curpath += 2;
	}
	else
	{
		dlc = 0;
	}
	if(rootpath[1] == ':')
	{
		dlr = rootpath[0];
		rootpath += 2;
	}
	else
	{
		dlr = 0;
	}
	if(path[1] == ':')
	{
		dlp = path[0];
		path += 2;
	}
	else
	{
		dlp = 0;
	}
	if((dlc != dlr && (dlc && dlr)))
	{
		fprintf(stderr, "- Root must be on same drive as tree\n");
		return "";
	}
	if((dlr != dlp && (dlp && dlr)))
	{
		fprintf(stderr, "- Root must be on same drive as path\n");
		return "";
	}
	// if path is below curpath, ignore root, short circuit here and just
	// return the path portion below path
	//
	len = strlen(curpath);
	if(strlen(path) >= len && ! strncmp(curpath, path, len))
	{
		// path == or longer than curpath and wholly inside curpath
		//
		px = path + len;
		pp = result;
	}
	else
	{
		pp = strstr(curpath, rootpath);
		if(pp)
		{
			// curpath under root, so just need to go up to root
			//
			pp += strlen(rootpath);
			ups = 1;
		}
		else
		{
			// curpath someplace else, need to go up above root and down path
			//
			pp = curpath;
			downs = 1;
		}
		while(pp && *pp)
		{
			if(*pp == '/' || *pp == '\\')
				ups++;
			pp++;
		}
		pp = result;
		while(ups-- > 0 && nresult > 0)
		{
			strcpy(pp, "../");
			pp += 3;
			nresult-= 3;
		}
		if(downs)
		{
			int len;
			
			len = strlen(rootpath);
			if(len > nresult - 1)
				len = nresult - 1;
			strncpy(pp, rootpath, len);
			pp += len;
			nresult -= len;
			strcpy(pp, "/");
			pp++;
		}
		px = strstr(path, rootpath);
		if(px)
		{
			// path under root, so just need to go up difference
			//
			px += strlen(rootpath);
		}
		else
		{
			// curpath someplace else, need to go up to root and down to path
			//
			px = path;
		}
	}
	if(px[0] == '/' || px[0] == '\\')
	{
		px++;
	}
	strncpy(pp, px, nresult - 1);
	
	//fprintf(stderr, "cur=%s root=%s res=%s\n", curpath, rootpath, result);

	return NormalizeFilename(result, '/');
}

#ifndef Windows
//**************************************************************************
//! Take an absolute path, or a path relative to the current working dir
//! and return a normalized absolute path that represents the same path
//! and optionally return a pointer within that path to the filename portion
//! @retval 0 = good else fail
//
int GetFullPathName(
		char* path,	///< [in] path to get full path for
		int namelen,	///< [in] size of buffer for return
		char* fullname,	///< [out] buffer to receive full path
		char** pfp	///< [out] (optional) set to point to
				///  filename portion of path (can be NULL)
		)
{
	char  cwd[MAX_PATH];
	char  dir[MAX_PATH];
	char* file, *pfn, *pd;
	int   dirl, filel;
	int   abspath;
	int   needptc = 0;
	
	if(! path) 
	{
		return -1;
	}
	// if path starts with .\ or ./, get rid of that
	//
	if(path[0] == '.' && (path[1] == '\\' || path[1] == '/' || path[1] == '\0'))
	{
		path+= (path[1] ? 2 : 1);
	}
	if(path[0] != '/' && path[0] != '\\' && (! path[0] || (path[0] && (path[1] != ':'))))
	{
		// relative path, get current directory as starting point
		getcwd(cwd, MAX_PATH);
		strcpy(dir, cwd);
		abspath = 0;
	}
	else
	{
		// abs. path
		dir[0] = '\0';
		abspath = 1;
	}
	// for each ".." in front of path, remove one
	// dir from end of cwd
	//
	file = path;
	pd   = dir + strlen(dir) - 1;
	
	if(pd <= dir)
	{
		dir[0] = '\0';
	}
	else if(! abspath)
	{
		pfn = file;
		
		while((pfn = strstr(pfn, "..")) != NULL)
		{
			pfn += 2;
			while(*pfn == '/' || *pfn == '\\')
				pfn++;
			file = pfn;
		
			while(pd >= dir)
			{
				if(*pd == '/' || *pd == '\\')
				{
					*pd = '\0';
					break;
				}
				pd--;
			}
		}
	}
	else
	{
		file    = path;
		dir[0]  = '\0';
	}
	dirl  = strlen(dir);
	filel = strlen(file);
	if(
		(file[0] != '/' && file[0] != '\\' && (! file[0] || (file[0] && (file[1] != ':'))))
		&& 
		(! dirl || (dir[dirl - 1] != '/' && dir[dirl - 1] != '\\'))
	)
	{
		filel++;
		needptc = 1;
	}
	if(dirl + filel < MAX_PATH)
	{
		strcpy(fullname, dir);
		if(needptc)
		{
			strcat(fullname, PTS);
		}
		strcat(fullname, file);
		file = fullname;
		
		pfn = file + strlen(file) - 1;
		
		while(pfn >= file)
		{
			if(*pfn == '/' || *pfn == '\\')
			{
				pfn++;
				break;
			}
			pfn--;
		}
		if(pfp)
		{
			*pfp = pfn;
		}
	}
	return 0;
}
#endif

//**************************************************************************
//! Add an Include file path to the list of include file paths
//! Handles paths of form -I<path> or <path>
//! @retval 0==good
//
int Addincl(
		char* progname,	///< [in] name of main program, for error messages
		char* pathname,	///< [in] path to add to list
		int index	///< [in] where in list to add path to (like a count)
	)
{
	char fullpath[MAX_PATH];
	
	if(index >= MAX_INCLS || index < 0)
	{
		fprintf(stderr, "%s: Too many (%d) include paths\n", progname, index);
		return 1;
	}
	if(pathname && pathname[0] == '-' && pathname[1] == 'I')
		pathname += 2;
	if(GetFullPathName(NormalizeFilename(pathname, PTC), sizeof(fullpath), fullpath, NULL))
	{
		fprintf(stderr, "%s: Bad Path %s\n", progname, pathname);
	}
	else
	{
		g_incls[index] = malloc(strlen(fullpath) + 1);
		if(g_incls[index])
		{
			strcpy(g_incls[index], fullpath);
			if(verbose)
			{
				fprintf(stderr, "%s: Adding incl path %s\n", progname, fullpath);
			}
		}
		else
		{
			fprintf(stderr, "%s: Not enough memory\n", progname);
			return 1;
		}
	}
	return 0;
}

//**************************************************************************
//! Add a Define to the list of defines for PREDEFINED in doxygen
//! @retval 0==good
//
int Adddef(
		char* progname,	///< [in] name of main program, for error messages
		char* defname,	///< [in] define text to add to list
		int index	///< [in] where in list to add path to (like a count)
	)
{
	if(index >= MAX_DEFS || index < 0)
	{
		fprintf(stderr, "%s: Too many (%d) definitions\n", progname, index);
		return 1;
	}
	g_defs[index] = defname;
	return 0;
}

//**************************************************************************
//! Tell if the file should be linked to in the "further reading" output
//! @retval 0==no, 1==yes
//
int IsDocFile(char* path /**< [in] */)
{
	if(
			strstr(path, ".htm")
		||	strstr(path, ".html")
		||	strstr(path, ".shtml")
		||	strstr(path, ".doc")
		||	strstr(path, ".txt")
		||	strstr(path, ".odg")
		||	strstr(path, "readme")
		||	strstr(path, "README")
		||	strstr(path, "overview")
		||	strstr(path, "overview")
	)
	{
		if(strstr(path, "~")) return 0;
		if(strstr(path, ".bak")) return 0;
		
		return 1;
	}
	return 0;
}

//**************************************************************************
//! Tell if the file should be looked at by doxygen
//! @retval 0==no, 1==yes
//
int IsDoxyFile(char* path /**< [in] */)
{
	if(
			strstr(path, ".c")
		||	strstr(path, ".h")
		||	strstr(path, ".cpp")
		||	strstr(path, ".s")
	)
	{
		if(strstr(path, "~")) return 0;
		if(strstr(path, ".bak")) return 0;
		
		// special hack exclusions since files are too big
		//
		if(strstr(path, "fireregs"))
			return 0;
		if(strstr(path, "boot4") && strstr(path, ".h"))
			return 0;
		return 1;
	}
	return 0;
}

//**************************************************************************
//! Tell if the file should be excluded from recursion
//! @retval 0==no, 1==yes
//
int IsExcludedPath(char* path /**< [in] */)
{
	// exclude matching paths from further decent
	//
	if(
			strstr(path, "work")
		||	strstr(path, "compilers")
		||	(path[0] == '4')
	)
	{
		return 1;
	}
	return 0;
}

//**************************************************************************
//! Generate an actual css file from the build-in css file 
//! @retval 0==good
//
int MakeCSSFile(
		char* progname, 	///< [in] program name for errors
		char* outpath		///< [in] directory to put file in
		)
{
	FILE *cssf;
	
	// write out the built-in css,
	//
	strcpy(csspath, outpath);
	strcat(csspath, PTS);
	strcat(csspath, "docgen.css");

	cssf = fopen(csspath, "w");
	if(! cssf)
	{
		fprintf(stderr, "%s - can't create CSS file %s\n", progname, csspath);
		return 1;
	}
	fwrite(g_gencss, g_gencssz, 1, cssf);
	fclose(cssf);
	return 0;
}

//**************************************************************************
//! Generate a doxygen configuration file
//! @retval 0==good
//
int MakeDoxyFile(
		char* progname, 	///< [in] program name for errors
		char* outpath		///< [in] directory to put file in
		)
{
	FILE *doxf;
	PDOXY pd;
	int   totlen;
	int   i;
	char* allincls, *alldefs, *allinput;

	// write out the built-in doxygen config with the
	// project name, output path and list of input files
	//
	strcpy(doxypath, outpath);
	strcat(doxypath, PTS);
	strcat(doxypath, "doxygenconfig.txt");

	doxf = fopen(doxypath, "w");
	if(! doxf)
	{
		fprintf(stderr, "%s - can't create doxygen config file %s\n", progname, doxypath);
		return 1;
	}
	// interate through the include file paths and cat all the paths together
	//
	for(i = 0; i < g_nIncls; i++)
	{
		totlen += strlen(g_incls[i]);
	}
	allincls = NULL;
	if(totlen > 0)
	{
		allincls = (char*)malloc(totlen + MAX_INCLS * 4);
		if(! allincls)
		{
			fprintf(stderr, "%s - can't alloc doxygen includes list\n", progname);
		}
		else
		{
			allincls[0] = '\0';

			// concatenate all the include paths
			for(i = 0, totlen = 0; i < g_nIncls; i++)
			{
				strcat(allincls, g_incls[i]);
				strcat(allincls, " \\\n");
			}
			// append a blank line on the end to eat up the \
			//
			strcat(allincls, "\n\n");
		}
	}
	// interate through the defines and cat all them together
	//
	for(i = 0; i < g_nDefs; i++)
	{
		totlen += strlen(g_defs[i]);
	}
	alldefs = NULL;
	if(totlen > 0)
	{
		alldefs = (char*)malloc(totlen + MAX_INCLS * 4);
		if(! alldefs)
		{
			fprintf(stderr, "%s - can't alloc doxygen defines list\n", progname);
		}
		else
		{
			alldefs[0] = '\0';
			
			// concatenate all them in one string
			for(i = 0, totlen = 0; i < g_nDefs; i++)
			{
				strcat(alldefs, g_defs[i]);
				strcat(alldefs, " \\\n");
			}
			// append a blank line on the end to eat up the \
			//
			strcat(alldefs, "\n\n");
		}
	}
	// interate through all doxy blocks and accumulate total size of strings of files
	//
	for(pd = g_doxylist, totlen = 0; pd; pd = pd->next)
	{
		totlen += (pd->pb - pd->buf);
	}
	allinput = NULL;
	if(totlen > 0)
	{
		allinput = (char*)malloc(totlen + 64);
		if(! allinput)
		{
			fprintf(stderr, "%s - can't alloc doxygen input list\n", progname);
		}
		else
		{
			// concatenate all the input files
			for(pd = g_doxylist, totlen = 0; pd; pd = pd->next)
			{
				if(pd->pb > pd->buf)
				{
					memcpy(allinput + totlen, pd->buf, (pd->pb - pd->buf));
					totlen += (pd->pb - pd->buf);
					free(pd->buf);
				}
			}
			// append a blank line on the end to eat up the \
			//
			strcpy(allinput + totlen, "\n\n");
		}
	}
	fprintf(doxf, g_gendoxy, "Inferno", outpath,
			allinput ? allinput : "",
			allincls ? allincls : "",
			alldefs  ? alldefs : "",
			gengraphs ? "YES" : "NO"
			);
	if(allincls)
		free(allincls);
	if(allinput)
		free(allinput);
	if(alldefs)
		free(alldefs);
	fclose(doxf);
	return 0;
}

static int g_ssnum = 0;

//**************************************************************************
//! Put a few boiler-plate links into the file
//! @retval 0==good
//
int AddStandardDocLinks(
			char* progname,	///< [in] program name, for errors
			FILE* zf,	///< [in] file to write links to
			int hassubs	///< [in] true if a subsystem link should be added
			)
{
	if(hassubs)
	{
		char sslink[128];
		
		sprintf(sslink, "#subhdr%d", g_ssnum);
		fprintf(zf, sublink, sslink, "Sub-Systems");
	}
	if(gendoxy)
	{
		fprintf(zf, doxylink, "doxygen_html/index.html", "Source Code Documentation");
	}
	fprintf(zf, backlink, "Back");
	fprintf(zf, toplink, topfile, "Top");
	fprintf(zf, "%s", "<br>");
	return 0;
}

//**************************************************************************
//! interpret/extract/linkify a "readme.txt" file creating links to sections
//! inside the file
//! @retval 0=good
//
int ProcessTextFile(
			char* progname,		///< [in] program name, for errors
			FILE* zf,		///< [in] file to write html into
			char* path,		///< [in] filename to read and process
			char* parentpath,	///< [in] path where filename is
			int addhdr,		///< [in] true to add standard header links as well
			int hassubs		///< [in] true to add a subsystem link to std. links
			)
{
	char  fpath[MAX_PATH];
	char  buf[256];
	FILE* in;
	char* line;
	char* pi, *pe;
	int   incl;
	
	if(! path || ! *path) return -1;
	
	if(strstr(path, ".bak")) return -2;
	if(strstr(path, "strings.txt")) return -2;		
	if(strstr(path, "readme.html")) return -2;		
	if(strchr(path, '~')) return -2;

	sprintf(fpath, "%s%s%s", parentpath, PTS, path);

	if(verbose)
		fprintf(stderr, "FILE %s\n", fpath);

	// files named "readme.txt" are processed into html 
	// others are simply added to the index section
	
	if(! stricmp(path, "readme.txt"))
	{
		char* pb;
		int   state;
		
		in = fopen(fpath, "r");

		if(in)
		{
			char loclink[128];
			int len;
			
			// read in file to get section names
			while((line = fgets(buf, 255, in)) != NULL)
			{
				len = strlen(buf);
				if(len > 0)
				{
					for(pb = buf + len - 1; pb >= buf; pb--)
						if(*pb != '\r' && *pb != '\n')
							break;
					while(pb >= buf && (*pb == ' ' || *pb == '\t'))
						pb--;
					if(pb >= buf)
					{
						if(*pb == ':')
						{
							*pb = '\0';
							sprintf(loclink, "#%s", buf);
							fprintf(zf, seclink, loclink, buf);
						}
					}
				}
			}
			if(gendocs && addhdr)
			{
				// put in any boilerplate links
				AddStandardDocLinks(progname, zf, hassubs);
			}
			// read file content now
			fseek(in, 0, 0);
			state = 0;
			while((line = fgets(buf, 255, in)) != NULL)
			{
				len = strlen(buf);
				if(len > 0)
				{
					for(pb = buf + len - 1; pb >= buf; pb--)
						if(*pb != '\r' && *pb != '\n')
							break;
					while(pb >= buf && (*pb == ' ' || *pb == '\t'))
						pb--;
					if(pb >= buf)
					{
						if(*pb == ':')
						{
							if(state == 2)
							{
								fprintf(zf, "%s", secfooter);
							}
							state = 1;
							*pb = '\0';
							fprintf(zf, secanchor, buf);
							fprintf(zf, sectitle, buf);
						}
						else
						{
							if(state != 2)
							{
								fprintf(zf, "%s", sechdr);
							}
							state = 2;
							fputs(buf, zf);
						}
					}
				}
			}
			if(state == 2)
			{
				fprintf(zf, "%s", secfooter);
			}
			fclose(in);
		}
		else
		{
			fprintf(stderr, "Can't open %s\n", fpath);
		}
	}
	return 0;
}

//**************************************************************************
//! insert a "readme.html" directly into file
//! @retval 0=good
//
int ProcessHtmlFile(
			char* progname,		///< [in] program name, for errors
			FILE* zf,		///< [in] file to write html into
			char* path,		///< [in] filename to read and process
			char* parentpath,	///< [in] path where filename is
			int addhdr,		///< [in] true to add standard header links as well
			int hassubs		///< [in] true to add a subsystem link to std. links
			)
{
	char  fpath[MAX_PATH];
	char  buf[256];
	FILE* in;
	char* line;
	char* pi, *pe;
	int   incl;
	
	if(! path || ! *path) return -1;
	
	if(strstr(path, ".bak")) return -2;
	if(strchr(path, '~')) return -2;

	sprintf(fpath, "%s%s%s", parentpath, PTS, path);

	if(verbose)
		fprintf(stderr, "HTMLFILE %s\n", fpath);

	// files named "readme.html" are processed into the header
	// others are simply added to the further reading section
	//
	if(! stricmp(path, "readme.html"))
	{
		char* pb;
		int   state;
		
		in = fopen(fpath, "r");

		if(in)
		{
			if(gendocs && addhdr)
			{
				// put in any boilerplate links
				AddStandardDocLinks(progname, zf, hassubs);
			}
			// read file content now
			fseek(in, 0, 0);
			state = 0;
			while((line = fgets(buf, 255, in)) != NULL)
			{
				fputs(buf, zf);
			}
			fclose(in);
		}
		else
		{
			fprintf(stderr, "Can't open %s\n", fpath);
		}
	}
	return 0;
}

//**************************************************************************
//! Create an html file to hold documents, links and subsystem links in
//! @retval the FILE pointer on success, NULL on failure
//
FILE* MakeLocalFile(
			char* progname,	///< [in] the program name, for errors
			char* ptitle,	///< [in] title to use for html header
			char* path,	///< [in] file location
			char* name	///< [in] file name to create
		)
{
	FILE* zf;
	char  title[256];
	char  filepath[MAX_PATH];
	int   len;
	char  relpath[MAX_PATH];

	// make a doc file named "path" to describe subsystem "name"
	//
	// fprintf(stderr, "  MLF %s+%s\n",  path, name);

	zf = fopen(path, "w");
	if(! zf)
	{
		fprintf(stderr, "%s - can't create %s\n", progname, path);
		return NULL;
	}
	if(name[1] == ':')
		name += 3;
	
	if(name && name[0])
	{		
		sprintf(title, "%s Documentation", name);
		ptitle = title;
	}
	strcpy(filepath, path);
	len = strlen(filepath);
	while(--len >= 0)
	{
		if(filepath[len] == '/' || filepath[len] == '\\')
		{
			filepath[len] = '\0';
			break;
		}
	}
	fprintf(zf, header, title, progname,
			RelativeHtmlPathTo(filepath, rootpath, csspath, relpath, MAX_PATH));
	fprintf(zf, prolog, ptitle, progname);
	return zf;
}

//**************************************************************************
//! Close a file made with MakeLocalFile, adds html footer
//
void CloseLocalFile(FILE* zf /**< [in] the file to close */)
{
	if(zf)
	{
		fprintf(zf, "%s", footer);
		fclose(zf);
	}
}

//**************************************************************************
//! Extract Documents/Links/Subsystems recursively.  *recursive*
//! @retval 0==good
//
int ExtractDocs(
		char* progname,		///< [in] program name, for errors
		char* ptitle,		///< [in] title to use in document header
		FILE* out,		///< [in] main index file to write links to
		char* path,		///< [in] current subsystem to process (NULL or blank == top)
		char* outpath,		///< [in] place to put sub system html files
		char* parentpath,	///< [in] directory where "path" is in
		int*  haskids		///< [out] set to non-0 if there is valid docs in or under
	)
{
	struct stat ds, fs;
	char   dpath[MAX_PATH];
	char   xpath[MAX_PATH];
	char   fpath[MAX_PATH];
	char   dirfile[MAX_PATH];
	char   relpath[MAX_PATH];
	char*  fname;
	FILE*  zf = NULL;
	
	int pass, ndocs, ntxt, nhtm, nsubs;
	int needhdr, nkids;

	PDOXY pdoxy;
	int   doxylen;
	int   filesize;
	
	if(! strcmp(path, ".")) return 0;
	if(! strcmp(path, "..")) return 0;
	
	// fprintf(stderr, "EXDOC %s->%s\n", parentpath, path);

	if(haskids)
	{
		*haskids = 0;
	}
	nkids = 0;

	dpath[0] = '\0';
	if(parentpath && *parentpath)
	{
		strcpy(dpath, parentpath);
		strcat(dpath, PTS); 	//should already be there
	}
	strcat(dpath, path);
	
	if(stat(dpath, &ds))
	{
		fprintf(stderr, "%s - directory %s does not exist\n", progname, dpath);
		return -1;
	}
	if(! (ds.st_mode & S_IFDIR))
	{
		fprintf(stderr, "%s - %s is not a directory\n", progname, dpath);
		return -1;
	}
	strcpy(dirfile, outpath);	// generated files go in "outpath/"
	strcat(dirfile, PTS);		// and are called "subdir_subdir_subdir.html"

	GetFullPathName(dpath, sizeof(xpath), xpath, &fname);
	if(! fname || ! *fname) 
	{
		// no dir or file name, must be top level
		// this one goes directly in out directory
		//
		fname = "index.html";
		strcat(dirfile, fname);	// dir name == filename
	}
	else
	{
		// this path is a sub-path of root, so get the part of the path under
		// root and convert that to the actual name of the file to use to
		// avoid duplicate filenames
		//
		RelativeHtmlPathTo(rootpath, rootpath, xpath, relpath, MAX_PATH);
		NormalizeFilename(relpath, '_');
		
		// a sub-system explanatory file, put it in the subfiles dir
		//
		strcat(dirfile, relpath);
		strcat(dirfile, "_");
		strcat(dirfile, fname);	// dir name == filename
		strcat(dirfile, ".html");
	}
	if(! topfile[0])
	{
		RelativeHtmlPathTo(outpath, rootpath, dirfile, relpath, MAX_PATH);		
		strcpy(topfile, relpath);
		NormalizeFilename(topfile, '/');
	}
	if(gendocs)
		zf = MakeLocalFile(progname, ptitle, dirfile, path);
	else
		zf = NULL;

	g_ssnum++;
	
	pdoxy = NULL;
	doxylen = 0;

	/** enumerate files in the directory.  There are 3 passes (0, 1, 2)
	//
	// Pass 0 - Collect counts of files that will be expanded at the top (readme.txt) (ntxt)
	//          files that will included in the top (readme.html) (nhtm) and files that
	//          will be linked (ndocs) at the top for further reading.
	//
	//          keep running total of lengths of paths of files that will be handed to
	//          doxygen config file for pass 1
	//
	// Pass 1  - Insert expanded file into header (ntxt) and files (nhtm) 
	//          
	//           allocate and append files for doxygen into a block for this dir
	//
	// Pass 2  - Write out subsystem header and links to each subsystem (dir) found (nsubs)
	//
	*/
	for(pass = ndocs = ntxt = nsubs = nhtm = 0, needhdr = 1; pass < 3; pass++)
	{
#ifdef Windows

		Windows_FIND_DATA fdata;
		HANDLE			hFind;

		strcpy(xpath, dpath);
		strcat(xpath, PTS);
		strcat(xpath, "*");
		
		if((hFind = FindFirstFile(xpath, &fdata)) != INVALID_HANDLE_VALUE)

#endif
#ifdef Linux
		
		DIR*  	       pDir;
		struct dirent* dp;
		
		strcpy(xpath, dpath);
		strcat(xpath, PTS);

		if((pDir = opendir(xpath)) != NULL)

#endif
		{			
			if(pass == 2)
			{
				if(nsubs > 0)
				{
					// add subsystem header
					if(gendocs)
						fprintf(zf, subsyshdr, g_ssnum);
				}
			}
			else if(pass == 1)
			{
				if(ntxt > 0)
				{
					// process the readme.txt file
					if(gendocs)
					{
						ProcessTextFile(progname, zf, "readme.txt", dpath, needhdr, nsubs > 0);
					}
					needhdr = 0;
				}
				if(nhtm > 0)
				{
					// process the readme.html file
					if(gendocs)
					{
						ProcessHtmlFile(progname, zf, "readme.html", dpath, needhdr, nsubs > 0);
					}
					needhdr = 0;
				}
				if(gendoxy && (! ntxt && ! nhtm) && needhdr)
				{
					AddStandardDocLinks(progname, zf, nsubs);
				}
				if(ndocs > 0)
				{
					// add supplemental docs header
					if(gendocs)
					{
						fprintf(zf, "%s", dochdr);
					}
				}
			}
#ifdef Windows
			do
			{
				strcpy(fpath, fdata.cFileName);
				strcpy(xpath, dpath);
				strcat(xpath, PTS);
				strcat(xpath, fpath);
				filesize = fdata.dwFileSize;
				
				if(! (fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
#endif
#ifdef Linux
			while((dp = readdir(pDir)) != NULL)
			{
				strcpy(xpath, dpath);
				strcat(xpath, PTS);
				strcat(xpath, dp->d_name);
				
				if(! stat(xpath, &fs))
				{
				filesize = fs.st_size;
				strcpy(fpath, dp->d_name);
				if(! (fs.st_mode & S_IFDIR))
#endif
				{
					//fprintf(stderr, "file %08X in dir %s is %s\n", fs.st_mode, xpath, fpath);
					
					// only look at documentation files
					//
					if(IsDocFile(fpath))
					{
						if(pass == 1)
						{
							if(stricmp(fpath, "readme.txt") && stricmp(fpath, "readme.html"))
							{
								char suppath[MAX_PATH];
								
								_snprintf(suppath, MAX_PATH, "%s/%s/%s", parentpath, path, fpath);
								NormalizeFilename(suppath, '/');
								// add link to this doc
								if(gendocs)
									fprintf(zf, supdoclink, suppath, fpath);
							}
						}
						else if(pass == 0)
						{
							if(!stricmp(fpath, "readme.txt"))
							{
								ntxt++;
							}
							else if(! stricmp(fpath, "readme.html"))
							{
								nhtm++;
							}
							else
							{
								ndocs++;
							}
						}
					}
					else if(IsDoxyFile(fpath)/* && filesize < 0x100000*/)
					{
						// make the relative path from where the doxygen config
						// file is (outpath) to where this file is (xpath)
						//
						RelativeHtmlPathTo(outpath, rootpath, xpath, relpath, MAX_PATH);

						// a file that doxygen would be interested in
						//
						if(pass == 1 && doxylen)
						{
							if(! pdoxy)
							{
								char* doxybuf = (char*)malloc(doxylen);
								if(doxybuf)
								{
									if(! pdoxy)
									{
										pdoxy = (PDOXY)malloc(sizeof(DOXY));
										pdoxy->buf = doxybuf;
										pdoxy->pb  = pdoxy->buf;
										pdoxy->bz  = doxylen;
										
										pdoxy->next = g_doxylist;
										g_doxylist = pdoxy;
									}
								}
							}
							if(pdoxy)
							{
								int room = pdoxy->bz - (pdoxy->pb - pdoxy->buf);
								int len = strlen(relpath) + 3;
								
								if(len < room)
								{
									snprintf(pdoxy->pb, room, "%s \\\n", relpath);
									pdoxy->pb += len;
								}
								else 
								{
									fprintf(stderr, "%s - overflow of doxygen buffer\n", progname);
								}
							}
						}
						else if(pass == 0)
						{
							doxylen += 4 + strlen(relpath);
						}
					}
				}
				else if(strcmp(fpath, ".") && strcmp(fpath, ".."))
				{
					if(IsExcludedPath(fpath))
					{
						if(verbose)
							fprintf(stderr, "%s - skipping dir %s in %s\n", progname, fpath, dpath);
					}
					else
					{
						if(pass == 2)
						{
							int kidcnt = 0;

						//	fprintf(stderr, " ---- subdir =%s\n", fpath);
							ExtractDocs(
								progname, 
								ptitle,
								zf,
								fpath,
								outpath,
								dpath,
								&kidcnt
							);
							nkids += kidcnt;
						}
						else if(pass == 0)
						{
							nsubs++;
						}
					}
				}
			}
#ifdef Windows
			while(FindNextFile(hFind, &fdata));
			FindClose(hFind);
#endif
#ifdef Linux
			}
			closedir(pDir);
#endif
			if(pass == 1)
			{
				if(ndocs > 0)
				{
					// add supplemental docs footer
					if(gendocs)
					{
						fprintf(zf, "%s", docfooter);
					}
				}
			}
			else if(pass == 2)
			{
				if(nsubs > 0)
				{
					// add subsystem list footer
					if(gendocs)
					{
						fprintf(zf, "%s", subsysfooter);
					}
				}
			}
		}
	}
	nkids += ntxt;
	nkids += ndocs;
	nkids += nhtm;
	if(haskids)
	{
		*haskids = nkids;
	}
	if(zf && gendocs)
	{		
		if(out)
		{
			// add a link in the main index file to the file generated for this subdir
			//
			RelativeHtmlPathTo(outpath, rootpath, dirfile, relpath, MAX_PATH);
			
			if(nkids > 0)
				fprintf(out, subsysentry, relpath, path);
			else
				fprintf(out, subsysstatic, path);
		}
		CloseLocalFile(zf);
	}
	return 0;
}

//**************************************************************************
static int usage(char* progname)
{
	return fprintf(stderr,
			"\nUsage: %s [options] <rootpath>  ---- build a documentation file set\n"
			"  rootpath is path to top of code source tree.\n"
			"  options:\n"
			"    -a           - generate doc tree and doxygen sub tree.\n"
			"    -d           - generate a configuration for doxygen instead.\n"
			"    -g           - add call-graphs to doxygen (takes a long time).\n"
			"    -o <path>    - use <path> to place output files in.\n"
			"    -t <title>   - use title in html header.\n"
			"    -v           - verbose: be loud.\n\n",
			progname
	);
}

//**************************************************************************
int main(int argc, char** argv)
{
	char* ptitle;
	char* progname;
	char* prootpath;
	char* poutpath;
	int   argo;
	int   kids;

	progname = *argv;
	while(progname && (*progname == '.' || *progname == '/' || *progname == '\\'))
	{
		progname++;
	}
	if(argc < 2)
	{
		return usage(progname);
	}
	argv++;
	argc--;

	g_nIncls = 0;
	g_nDefs  = 0;

	verbose  = 0;
	
	topfile[0] = '\0';
	rootpath[0] = '\0';
	outpath[0] = '\0';
	prootpath = NULL;
	poutpath = NULL;
	
	ptitle   = "Generated Documentation";
	
	gendocs = 1;
	gendoxy = 0;
	gengraphs = 0;

	// add local dir
	Addincl(progname, "", g_nIncls++);

	while(argc > 0)
	{
		if((*argv)[0] == '-')
		{
			char opt = (*argv)[1];
			
			if((*argv)[2])
			{
				argo = 2;
			}
			else
			{
				argo = 0;
				argv++;
				argc--;
				if(argc < 0 || ! *argv)
				{
					return usage(progname);
				}
			}
			switch(opt)
			{
			case 'o':		// outpath
				poutpath = *argv + argo;
				break;
			case 't':		// title
				ptitle = *argv + argo;
				break;
			case 'd':		// just doxygen
				gendoxy = 1;
				gendocs = 0;
				argc++;
				argv--;
				break;
			case 'a':		// all
				gendoxy = 1;
				gendocs = 1;
				argc++;
				argv--;
				break;
			case 'g':		// call graphs
				gengraphs = 1;
				argc++;
				argv--;
				break;
			case 'I':		// include path
				if(Addincl(progname, *argv, g_nIncls))
					return -1;
				else
					g_nIncls++;
				break;
			case 'D':		// define
				if(Adddef(progname, *argv, g_nDefs))
					return -1;
				else
					g_nDefs++;
				break;
			case 'v':		// verbose
				verbose = 1;
				argc++;
				argv--;
				break;
			default:
				if(verbose)
				{
					fprintf(
						stderr,
						"%s - ignoring switch %c\n",
						progname,
						opt
						);
				}
				break;
			}
			argv++;
			argc--;
		}
		else
		{
			argc--;
			if(argc == 0)
			{
				prootpath = *argv++;
				break;
			}
			else
			{
				if(verbose)
				{
					fprintf(
						stderr,
						"%s - ignoring loose parm %s\n",
						progname, *argv
						);
				}
				argv++;
			}
		}
	}
	if(! prootpath)
	{
		return usage(progname);
	}
	if(! poutpath)
	{
		poutpath = ".";
	}
	// get full name of root path to source tree
	//
	GetFullPathName(NormalizeFilename(prootpath, PTC), sizeof(rootpath), rootpath, NULL);

	// get full name of output path
	//
	GetFullPathName(NormalizeFilename(poutpath, PTC), sizeof(outpath), outpath, NULL);

	if(gendocs)
	{
		// create a css file in output path
		//
		MakeCSSFile(progname, outpath);
	}
	// make documentation from root tree
	//
	ExtractDocs(progname, ptitle, NULL, rootpath, outpath, "", &kids);

	if(gendoxy)
	{
		// create a doxygen config
		//
		MakeDoxyFile(progname, outpath);
	}
	return 0;
}

