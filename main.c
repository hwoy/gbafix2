
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "function.h"
#include "opt.h"

#define BSIZE 512

typedef struct
{
	unsigned long	start_code;		/* B instruction */
	unsigned char	logo[0xA0-0x04];	/* logo data */
	char			title[0xC];		/* game title name */
	unsigned long	game_code;		
	unsigned short	maker_code;		
	unsigned char	fixed;			/* 0x96 */
	unsigned char	unit_code;		/* 0x00 */
	unsigned char	device_type;	/* 0x80 */
	unsigned char	unused[7];		
	unsigned char	game_version;	/* 0x00 */
	unsigned char	complement;		/* 800000A0..800000BC */
	unsigned short	checksum;		/* 0x0000 */
} Header;


static const Header good_header = {
	/* start_code */
	0xEA00002E,\
	/* logo */
	{ 0x24,0xFF,0xAE,0x51,0x69,0x9A,0xA2,0x21,0x3D,0x84,0x82,0x0A,0x84,0xE4,0x09,0xAD,\
	0x11,0x24,0x8B,0x98,0xC0,0x81,0x7F,0x21,0xA3,0x52,0xBE,0x19,0x93,0x09,0xCE,0x20,\
	0x10,0x46,0x4A,0x4A,0xF8,0x27,0x31,0xEC,0x58,0xC7,0xE8,0x33,0x82,0xE3,0xCE,0xBF,\
	0x85,0xF4,0xDF,0x94,0xCE,0x4B,0x09,0xC1,0x94,0x56,0x8A,0xC0,0x13,0x72,0xA7,0xFC,\
	0x9F,0x84,0x4D,0x73,0xA3,0xCA,0x9A,0x61,0x58,0x97,0xA3,0x27,0xFC,0x03,0x98,0x76,\
	0x23,0x1D,0xC7,0x61,0x03,0x04,0xAE,0x56,0xBF,0x38,0x84,0x00,0x40,0xA7,0x0E,0xFD,\
	0xFF,0x52,0xFE,0x03,0x6F,0x95,0x30,0xF1,0x97,0xFB,0xC0,0x85,0x60,0xD6,0x80,0x25,\
	0xA9,0x63,0xBE,0x03,0x01,0x4E,0x38,0xE2,0xF9,0xA2,0x34,0xFF,0xBB,0x3E,0x03,0x44,\
	0x78,0x00,0x90,0xCB,0x88,0x11,0x3A,0x94,0x65,0xC0,0x7C,0x63,0x87,0xF0,0x3C,0xAF,\
	0xD6,0x25,0xE4,0x8B,0x38,0x0A,0xAC,0x72,0x21,0xD4,0xF8,0x07 },\
	/* title */
	{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 } ,\
	/* game code */
	0x00000000,\
	/* maker code */
	0x3130,\
	/* fixed */
	0x96,\
	/* unit_code */
	0x00,\
	/* device type */
	0x80,\
	/* unused */
	{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00 } ,\
	/* game version */
	0x00,\
	/* complement */
	0x00,\
	/* checksum */
	0x0000
};

enum Error{
	err_param,
	err_fin,
	err_fout
};

static const char *errstr[]={"Number of Param must be 2","can not access Input File","can not access Output File",NULL};


static int printerr(int id,const char **errstr)
{
	fprintf(stderr,"\nError id:%d => %s\n",id,errstr[id]);
	return 1;
}

static const char *croppath(const char *path)
{
	const char *cpath;
	for(cpath=path;*path;++path) if(*path == '/' || *path=='\\') cpath=path+1;

	return cpath;
}

static int showhelp(const char *pname)
{
	fprintf(stderr,"\n%s is GameBoy Advance Head Adder\n",pname);
	fprintf(stderr,"\nUSAGE:: %s infile outfile\n",pname);
	return 1;
}

static const char* const opt[] = { "-a", "-p", "-t:","-c:","-m:","-r:","-o:", NULL };
static const char* const optstr[] = { "Add header to an output file", "Pad to next exact power of 2. No minimum size",\
"Patch title. Stripped filename if none given",\
"Patch game code (four characters)",\
"Patch maker code (two characters)",\
"Patch game version (number)",\
"Output file (must be assigned)", NULL };

enum {
    opt_a,
    opt_p,
    opt_t,
	opt_c,
	opt_m,
	opt_r,
	opt_o
};

static unsigned char HeaderComplement(const Header *header)
{
	unsigned char c = 0;
	const unsigned char *p = (const unsigned char *)header + 0xA0;
	unsigned int n;
	for (n=0xA0; n<=0xBC; n++)
		c += *p++;

	return -(0x19+c);
}

int main(int argc, const char *argv[])
{
	static Header header;
	static Header addheader=good_header;
	static char buff[BSIZE];
	const char *msg;

	FILE *fin=NULL,*fout=NULL;

	int isadd=0,ispadding=0;

	if(argc==1)
	{
		printerr(err_param,errstr);
		return showhelp(croppath(argv[0]));
	}


  {
        int i;
        unsigned int ui_cindex;

        for (ui_cindex = DSTART; (i = opt_action(argc, argv, opt, buff,
                                      BSIZE, DSTART))
             != e_optend;
             ui_cindex++) {

            switch (i) {
            case opt_a:

					isadd = 1;

                	break;

            case opt_p:

					ispadding =1;

                	break;

			case opt_t:
					if(*buff)
						memcpy(header.title, buff, sizeof(header.title));
					else
						memset(header.title, 0, sizeof(header.title));
					
					memcpy(addheader.title, header.title, sizeof(header.title));

					break;

			case opt_c:
					header.game_code = buff[0] | buff[1]<<8 | buff[2]<<16 | buff[3]<<24;

					memcpy(&addheader.game_code, &header.game_code, sizeof(header.game_code));

					break;

			case opt_m:
					header.maker_code = buff[0] | buff[1]<<8;

					memcpy(&addheader.maker_code, &header.maker_code, sizeof(header.maker_code));

			case opt_r:
					if (!*buff) printf("Need value \n") ; 

					else header.game_version = s2ui(buff);

					memcpy(&addheader.game_version, &header.game_version, sizeof(header.game_version));

					break;

			case opt_o:

					if(!(fout=fopen(buff, "wb"))) 
					{ 
						if(fin) free(fin);
						return printerr(err_fout,errstr); 
					}

					break;

            default:
					if(!(fin=fopen(buff, "r+b"))) 
					{
						if(fout) free(fout);
						return printerr(err_fin,errstr);
					}

					fread(&header, sizeof(header), 1, fin);
					rewind(fin);


					break;
            }
        }
    }

	if(isadd)
	{
		addheader.complement = HeaderComplement(&addheader);
		
		fwrite(&addheader, sizeof(addheader), 1, fout);

		msg="Header ROM Added!";
	}
	else
	{
		memcpy(header.logo, good_header.logo, sizeof(header.logo));
		memcpy(&header.fixed, &good_header.fixed, sizeof(header.fixed));
		memcpy(&header.device_type, &good_header.device_type, sizeof(header.device_type));
			
		header.complement = 0;
		header.checksum = 0;
		header.complement = HeaderComplement(&header);

		fwrite(&header, sizeof(header), 1, fout);

		msg="Header fixed!";
	}

	{
		int ch;
		while((ch=fgetc(fin))!=EOF)
		fputc(ch,fout);
	}

	puts(msg);

	if(ispadding)
	{
		size_t size ;
		size_t bit;

		size = ftell(fout);

		for (bit=31; bit>=0; bit--) if (size & (1<<bit)) break;
		if (size != (1<<bit))
			{
				size_t todo = (1<<(bit+1)) - size;
					while (todo--) fputc(0xFF, fout);
			}

		puts("Header ROM padded!");		
	}

	fclose(fin);
	fclose(fout);

	return 0;
}
