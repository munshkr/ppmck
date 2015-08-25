#include	<stddef.h>
#include	<ctype.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#include	"mckc.h"

extern void splitPath( const char *ptr, char *path, char *name, char *ext );
extern void makePath( char *ptr, const char *path, const char *name, const char *ext );
extern char *skipSpace( char *ptr );
extern char *patchstr;
extern char *hogereleasestr;

char	*mml_names[MML_MAX];
int		debug_flag = 0;
char	ef_name[256]  = "effect.h";
char	inc_name[256] = "define.inc";
char	out_name[256];
int		warning_flag = 1;
int		include_flag = 0;
int		mml_num = 0;
extern	int data_make( void );
extern	int		message_flag;			// ɽ����å������ν�������( 0:Jp 1:En )

/*--------------------------------------------------------------
	�إ��ɽ��
 Input:

 Output:

--------------------------------------------------------------*/
void dispHelpMessage( void )
{
	if( message_flag == 0 ) {
#ifdef __MINGW32__
		puts(	"������ˡ:ppmckc [switch] InputFile.mml [OutputFile.h]\n"
			"�⤷����:ppmckc [switch] -u InputFile1.mml InputFile2.mml ... \n"
				"\t[switch]\n"
				"\t-h -?   : �إ�פ�ɽ\��\n"
				"\t-i      : ����/����٥��ץե�����˶ʥǡ������ɲä���\n"
				"\t-m<num> : ���顼/��˥�ɽ\��������(0:Jpn 1:Eng)\n"
				"\t-o<str> : ����/����٥��ץե�����Υե�����̾��<str>�ˤ���\n"
				"\t-w      : Warning��å�������ɽ\�����ޤ���\n"
				"\t-u      : ʣ������ϿNSF����\n"
	    );
#else
		puts(	"������ˡ:ppmckc [switch] InputFile.mml [OutputFile.h]\n"
			"�⤷����:ppmckc [switch] -u InputFile1.mml InputFile2.mml ... \n"
				"\t[switch]\n"
				"\t-h -?   : �إ�פ�ɽ��\n"
				"\t-i      : ����/����٥��ץե�����˶ʥǡ������ɲä���\n"
				"\t-m<num> : ���顼/��˥�ɽ��������(0:Jpn 1:Eng)\n"
				"\t-o<str> : ����/����٥��ץե�����Υե�����̾��<str>�ˤ���\n"
				"\t-w      : Warning��å�������ɽ�����ޤ���\n"
				"\t-u      : ʣ������ϿNSF����\n"
	    );
#endif

	} else {
		puts(	"Usage:ppmckc [switch] InputFile.mml [OutputFile.h]\n"
			"  or :ppmckc [switch] -u InputFile1.mml InputFile2.mml ... \n"
				"\t[switch]\n"
				"\t-h -?   : Display this help message\n"
				"\t-i      : Including song data in tone/envelope file\n"
				"\t-m<num> : Select message language(0:Jpn 1:Eng)\n"
				"\t-o<str> : Output tone/envelope file name is <str>\n"
				"\t-w      : Don't display warning message\n"
				"\t-u      : Multiple song NSF creation\n"
	    );
	}
	exit( 1 );
}



/*--------------------------------------------------------------
	�ᥤ��롼����
 Input:
	int  argc		: ���ޥ�ɥ饤������θĿ�
	char *argv[]	: ���ޥ�ɥ饤������Υݥ���
 Output:
	0:���ｪλ 0:�ʳ��ʾ彪λ
--------------------------------------------------------------*/
int main( int argc , char *argv[] )
{
	int	i,in,out;
	char	path[256],name[256],ext[256];
	int	multiple_song_nsf = 0;

	in = out = 0;

// �����ȥ�ɽ��
	printf( "MML to MCK Data Converter Ver %d.%02d by Manbow-J\npmck modification by BKC rel 0.2\n",
			(VersionNo/100), (VersionNo%100) );
	//printf("patches by [OK] and 2ch mck thread people\n");
	printf(patchstr);
	printf(hogereleasestr);
// ���ޥ�ɥ饤�����
	if( argc == 1 ) {
		dispHelpMessage();
		return -1;
	}

	for ( i = 1; i < argc; i++ ) {
		// �����å���
		if ( (argv[i][0] == '-') || (argv[i][0] == '/') ) {
			switch( toupper( argv[i][1] ) ) {
			  case 'H':
			  case '?':
				dispHelpMessage();
				return 1;
			  case 'X':
				debug_flag = 1;
				break;
			  case 'I':
				include_flag = 1;
				break;
			  case 'M':
				message_flag = atoi( &(argv[i][2]) );
				if( message_flag > 1 ) {
					dispHelpMessage();
					return 1;
				}
				break;
			  case 'N':
				//obsolete
				break;
			  case 'O':
				strcpy( ef_name, skipSpace( &(argv[i][2]) ) );
				break;
			  case 'W':
				warning_flag = 0;
				break;
			  case 'U':
				multiple_song_nsf = 1;
				break;
			  default:
				if( message_flag == 0 ) {
					puts( "�����å��λ��꤬�㤤�ޤ�\n" );
				} else {
					puts( "Invalid switch!\n" );
				}
				dispHelpMessage();
				return -1;
			}
		// ����/���ϥե�����γ�Ǽ
		} else {
			if ( in < MML_MAX ) {
				mml_names[in] = argv[i];
				in++;
			} else {
				if( message_flag == 0 ) {
					puts( "�ѥ�᡼����¿�����ޤ�\n" );
				} else {
					puts( "Too many parameters!\n" );
				}
				dispHelpMessage();
				return -1;
			}
		}
	}

	if (in == 0) {
		dispHelpMessage();
		return -1;
	}
	if (multiple_song_nsf) {
		splitPath( mml_names[0], path, name, ext );
		makePath(  out_name, path, name, ".h" );
	} else {
		if (in == 1) {
			splitPath( mml_names[0], path, name, ext );
			makePath(  out_name, path, name, ".h" );
		} else if (in == 2) {
			strcpy(out_name, mml_names[1]);
			in--;
		} else {
			if( message_flag == 0 ) {
				puts( "�ѥ�᡼����¿�����ޤ�\n" );
			} else {
				puts( "Too many parameters!\n" );
			}
			dispHelpMessage();
			return -1;
		}
	}

	mml_num = in;
	for (i = 0; i < in - 1; i++) {
		printf("%s + ", mml_names[i]);
	}
	printf( "%s -> %s\n" ,mml_names[i],  out_name );
// ����С���
	i = data_make();
// ��λ
	if (i == 0) {
		if( message_flag == 0 ) {
			puts( "\n��λ���ޤ���\n" );
		} else {
			puts( "\nCompleated!\n" );
		}
		return EXIT_SUCCESS;
	} else {
		if( message_flag == 0 ) {
			puts( "\n����ѥ���˼��Ԥ��ޤ���\n" );
		} else {
			puts( "\nCompilation failed!\n" );
		}
		return EXIT_FAILURE;
	}
}
