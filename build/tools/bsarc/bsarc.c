#include "stdio.h"
#include "stdlib.h"
#include "string.h"

int main(int argc, char** argv)
{
	FILE* in;
	FILE* out;
	char  outname[256];
	char* po;
	char *pn;
	int   ic, i, outspec;
	int   cpp = 1;

	char* name = "_zg_resources";
	char* ext = "cpp";
	
	if(argc < 2)
		return fprintf(stderr, "Usage: bsarc [-o outfilename][-n resourcevar name] filename\n", *argv);

	argv++;
	argc--;

	outname[0] = 0;
	outspec = 0;
	
	while(argc > 1 && (*argv)[0] == '-')
	{
		switch((*argv)[1])
		{
		case 'c':
			cpp = 0;
			break;
		case 'o':
			if(*(*argv+2))
				strcpy(outname, *argv+2);
			else
				strcpy(outname, *++argv);
			outspec = 1;
			break;
		case 'n':
			if(*(*argv+2))
				name=*argv+2;
			else
				name=*++argv;
			break;
		}
		argv++;
		argc--;
	}	
	if(! (in = fopen(*argv, "rb")))
		return fprintf(stderr, "BSARC: no file: %s\n", *argv);

	for(pn = *argv + strlen(*argv) - 1; pn >= *argv;)
		if(*pn == '/')
			break;
		else
			pn--;
	if(! outspec)
	{
		strcpy(outname, ++pn);
		for(po = outname; *po; po++)
			if(*po == '.')
				break;
		*po = '\0';
		strcat(po, "rc");
		po += 2;
	}
	else
	{
		for(po = outname + strlen(outname); po >= outname; po--)
			if(*po == '.')
				break;
		if(*po != '.')
			po = outname + strlen(outname);
	}
	*po = '\0';
	strcpy(po, ".h");
	if(! (out = fopen(outname, "wb")))
		return fprintf(stderr, "BSARC: can't make file: %s\n", outname);
	
	fprintf(out, "#ifndef %sRC_H_\n#define %sRC_H_ 1\n\n", outname, outname);
	fprintf(out, "//\n// BSA-WINAPI  compiled resources\n//\n// Generated by BSARC\n//\n\n");
	if(cpp) fprintf(out, "extern \"C\" {\n");
	fprintf(out, "extern int         _cb%s;\n", name);
	fprintf(out, "extern unsigned char %s[];\n", name);
	if(cpp) fprintf(out, "}\n  ");
	fprintf(out, "\n#endif\n");
	fclose(out);

	*po = '\0';
	strcpy(po, ".c");
	if(cpp) strcat(po, "pp");
	
	if(! (out = fopen(outname, "wb")))
		return fprintf(stderr, "BSARC: can't make file: %s\n", outname);

	fprintf(out, "//\n// BSA-WINAPI  compiled resources\n//\n// Generated by BSARC\n//\n\n");
	fprintf(out, "unsigned char %s[] = {\n", name);
	i  = 0;
	ic = fgetc(in);
	while(ic != EOF)
	{
		fprintf(out, "0x%02X", ic);
		i++;
		ic = fgetc(in);
		if(ic != EOF)
			fprintf(out, ",%c", ((i & 0xF) == 0) ? '\n' : ' ');
		else
			fprintf(out, "\n");
	}
	fprintf(out, "};\n\nint _cb%s = %d;\n\n", name, i);
	fclose(out);
	fclose(in);
	return 0;
}
