#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(Linux) || defined(OSX)
#include <unistd.h>
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

int   verbose;

#ifdef Windows
#define PTC '\\'
#define PTS "\\"
#else
#define PTC '/'
#define PTS "/"
#endif

//**************************************************************************
char* NormalizeFilename(char* path, char ptc)
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
int GetFullPathName(char* path, char* fullname, char** pfp)
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
			if(pfn != file)
			{
				// don't look at .. inside a path, just at beginning
				break;
			}
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

//**************************************************************************
int addincl(char* progname, char* pathname, int index)
{
	char fullpath[MAX_PATH];
	
	if(index >= MAX_INCLS || index < 0)
	{
		fprintf(stderr, "%s: Too many (%d) include paths\n", progname, index);
		return 1;
	}
	if(GetFullPathName(NormalizeFilename(pathname, PTC), fullpath, NULL))
	{
		fprintf(stderr, "%s: Bad Path %s\n", progname, pathname);
	}
	else
	{
		// just uses relative paths, why not?
		strcpy(fullpath, NormalizeFilename(pathname, PTC));
		
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
int adddef(char* progname, char* def, int index)
{
	if(index >= MAX_DEFS || index < 0)
	{
		fprintf(stderr, "%s: Too many (%d) definitions\n", progname, index);
		return 1;
	}
	g_defs[index] = def;
	return 0;
}

//**************************************************************************
int ExtractIncludes(char* progname, FILE* in, FILE* out, char* ptag)
{
	FILE* subfile;
	char  buf[256];
	char  newpath[MAX_PATH];
	char* line;
	char* pi;
	int   incl;
	
	// read in file
	while((line = fgets(buf, 255, in)) != NULL)
	{
		// parse line for directive
		//
		pi = strstr(line, ptag);
		if(pi)
		{
			// get filename from line
			pi += strlen(ptag);
			while(*pi == ' ' || *pi == '\t' || *pi == '\"')
			{
				pi++;
			}
			if(*pi == '<')
			{
				if(verbose)
				{
					fprintf(stderr, "%s: ignoring compiler include %s", progname, line);
				}
			}
			else
			{
				char* pe;
				
				for(pe = pi; *pe && *pe != ' ' && *pe != '\t' && *pe != '\"' && *pe != '\r' && *pe != '\n';)
				{
					pe++;
				}
				// try each include path while not able to open file
				//
				if(pe > pi)
				{
					*pe = '\0';
					// convert filename to host system format
					pi = NormalizeFilename(pi, PTC);
					if(pi[0] != '/' && pi[0] != '\\' && (! pi[0] || (pi[0] && pi[1] != ':')))
					{
						for(subfile = NULL, incl = 0; subfile == NULL && incl < g_nIncls; incl++)
						{
							strcpy(newpath, g_incls[incl]);
							if(newpath[0])
							{
								char ec;
									
								ec = newpath[strlen(newpath) - 1];
								if(ec != '/' && ec != '\\')
								{
									strcat(newpath, PTS);
								}
							}
							strcat(newpath, pi);
							subfile = fopen(newpath, "r");
							if(verbose)
							{
									fprintf(stderr, "%s: open \"%s\" => %s\n",
									progname, newpath, subfile ? "yup" : "nope");
							}
						}
					}
					else
					{
						strcpy(newpath, pi);
						subfile = fopen(newpath, "r");
						if(verbose)
						{
								fprintf(stderr, "%s: open \"%s\" => %s\n",
								progname, newpath, subfile ? "yup" : "nope");
						}
					}
					if(subfile)
					{
						// add abs, path to depends file in unix format
						fprintf(out, " \\\n   %s", NormalizeFilename(newpath, '/'));
						ExtractIncludes(progname, subfile, out, ptag);
					}
					else if(verbose)
					{
						fprintf(stderr, "%s: \"%s\" not found\n", progname, pi);
					}
				}
			}
		}
	}
	fclose(in);
	return 0;
}

//**************************************************************************
int usage(char* progname)
{
	return fprintf(stderr,
			"\nUsage: %s [options] <filename>\n"
			"  options, which are cleverly similar to typical \"C\" compiler\n"
			"  command line options, are\n"
			"    -I <inclpath>     - adds inclpath to list of directories searched\n"
			"                        for files included in source file.\n"
			"    -D <define>       - adds <define> to list of manifests defined.\n"
			"    -f <outfile>      - write to outfile, not stdout.\n"
			"    -o <objpath>      - directory where object are placed when compiled\n"
			"                        used to prefix source name in output.\n"
			"    -x <objext>       - extension of object file, e.g \"-d .obj\",\n"
			"                      - defaults to .o.\n"
			"    -d <directive>    - format of \"include\" prefix, e.g. \"-d #include\"\n"
			"                        or \"-d INCLUDE\".  Defaults to #include.\n"
			"    -v                - verbose: be loud\n"
			"    all other option switches are ingored, so that the same flags can be\n"
			"    be passed to this program and a compiler\n\n",
			progname
	);
}

//**************************************************************************
int main(int argc, char** argv)
{
	FILE* in;
	FILE* out;
	char* progname;
	char* poutpath;
	char* poutfile;
	char* pobjext;
	char* pincldir;
	char* pinfile;
	char* psrcname;
	int   argo;
	
	char  objpath[MAX_PATH];
	char  srcpath[MAX_PATH];
	
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
	
	poutpath = NULL;
	poutfile = NULL;
	
	pincldir = NULL; // set after banner to "#include";
	pobjext  = NULL; // set after banner to ".o";

	// add local dir
	addincl(progname, "", g_nIncls++);
	
	while(argc > 0)
	{
		if((*argv)[0] == '-')
		{
			char opt = (*argv)[1];
			
			if((*argv)[2])	{
				argo = 2;
			} else {
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
			case 'o':		// out path
				poutpath = *argv + argo;
				break;
			case 'f':		// out file
				poutfile = *argv + argo;
				break;
			case 'd':		// include directive
				if(argc < 0 || ! *argv)
					return usage(progname);
				pincldir = *argv + argo;
				break;
			case 'I':		// incls
				if(addincl(progname, *argv, g_nIncls))
					return -1;
				else
					g_nIncls++;
				break;
			case 'D':		// define
				if(adddef(progname, *argv, g_nDefs))
					return -1;
				else
					g_nDefs++;
				break;
			case 'v':
				verbose = 1;
				argc++;
				argv--;
				break;
			default:
				if(verbose)
				{
					fprintf(stderr, "%s - ignoring switch %c\n", progname, opt);
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
				pinfile = *argv++;
				break;
			}
			else
			{
				if(verbose)
				{
					fprintf(stderr, "%s - ignoring loose parm %s\n", progname, *argv);
				}
				argv++;
			}
		}
	}
	if(! pinfile)
	{
		return usage(progname);
	}
	if(verbose)
	{
		fprintf(
				stderr,
				"depgen objdir=%s outfile=%s incldir=%s objext=%s\n"
				"       inclpaths:%d defs:%d src=%s\n",
				poutpath ? poutpath : "Not Specified",
				poutfile ? poutfile : "Not Specified",
				pincldir ? pincldir : "Not Specified",
				pobjext ? pobjext : "Not Specified",
				g_nIncls, g_nDefs, pinfile);
	}
	GetFullPathName(NormalizeFilename(pinfile, PTC), srcpath, NULL);
	in = fopen(srcpath, "r");
	if(! in)
	{
		return fprintf(stderr, "%s: can't open %s\n", progname, srcpath);
	}
	if(poutfile)
	{
		out = fopen(poutfile, "w");
	}
	else
	{
		poutfile = "stdout";
		out = stdout;
	}
	if(! out)
	{
		fclose(in);
		return fprintf(stderr, "%s: can't write %s\n", progname, poutfile);
	}
	if(! poutpath)
	{
		poutpath = "";
	}
	if(! pincldir)
	{
		pincldir = "#include";
	}
	if(! pobjext)
	{
		pobjext = ".o";
	}
	
	if(! strstr(poutpath, "."))
	{
		// outpath is a directory only
		// srcname = notdir pinfile
		for(psrcname = srcpath + strlen(srcpath); psrcname >= srcpath; psrcname--)
		{
			if(*psrcname == '\\' || *psrcname == '/' || *psrcname == ':')
			{
				break;
			}
		}
		psrcname++;
		strcpy(objpath, psrcname);
		
		// get rid of extension
		for(psrcname = objpath; *psrcname && (*psrcname != '.');)
			psrcname++;
		
		strcpy(psrcname, pobjext);
	}
	else
	{
		// file part included in outpath
		objpath[0] = '\0';
	}
	// header
	fprintf(out, "%s%s: %s", poutpath, objpath, 
			NormalizeFilename(pinfile, '/') /*srcpath*/);
	
	ExtractIncludes(progname, in, out, pincldir); 

	// footer
	fprintf(out, "\n");
	
	if(out != stdout)
	{
		fclose(out);
	}
	return 0;
}


