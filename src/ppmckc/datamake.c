#include	<stddef.h>
#include	<ctype.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#include	"mckc.h"

extern char	*mml_names[MML_MAX];
int		mml_idx = 0;
extern	int	mml_num;
char	songlabel[64];
extern int	debug_flag;
extern char ef_name[256];
extern char inc_name[256];
extern char out_name[256];
extern int	warning_flag;
extern int	message_flag;
extern int	include_flag;


extern int	getFileSize( char *ptr );
extern int  Asc2Int( char *ptr, int *cnt );
extern void strupper( char *ptr );
extern char *readTextFile( char *filename );
extern FILE *openDmc(char *name);
extern char *skipSpaceOld( char *ptr );
extern char *skipSpace( char *ptr );

void	putBankOrigin(FILE *fp, int bank);
int checkBankRange(int bank);
int double2int(double d);
#define arraysizeof(x) ( sizeof(x) / sizeof(x[0]) )


int		error_flag;					// ���顼��ȯ�����Ƥ����0�ʳ���
int		octave;						// �Ѵ���Υ���������
double	length;						// �Ѵ���β�Ĺ
int		octave_flag = 0;			// ���������֥����å� ("<" ">" �ν���)
int		gate_denom = 8;				//q���ޥ�ɤ�ʬ��
int		pitch_correction = 0;			//��ĥ�����Υǥ����塼�󡢥ԥå�����٥��ס�LFO����������

int		loop_flag;					// �����ͥ�롼�פ������0�ʳ���
int		putAsm_pos;					//

char		*mml_file_name;				//���ߤ�mml�ե�����̾(������֥���ϻ��˻���)
int		mml_line_pos;				//
int		mml_trk;				//

int		nest;						// ��ԡ��ȤΥͥ��ȿ�
LEN		track_count[MML_MAX][_TRACK_MAX][2];			// ��Ĺ�ȡ������ݴɾ��(��Ĺ/�ե졼��/�롼�ײ�Ĺ/�롼�ץե졼��)

int		volume_flag;				// ���̤ξ���
double		tbase = 0.625;						// �Ѵ����[frame/count]���

int		transpose;					// ���ߤΥȥ�󥹥ݡ�����

int		sndgen_flag = 0;			// ��ĥ�����ե饰
// �ȥ�å����ĥե饰
unsigned long	track_allow_flag = (TRACK(0)|TRACK(1)|TRACK(2)|NOISETRACK|DPCMTRACK);
//�ºݤ˻Ȥä��ȥ�å�
unsigned long	actual_track_flag = 0;
int		dpcm_track_num = 1;			// DPCM�ȥ�å�
int		fds_track_num = 0;			// FDS�ȥ�å�
int		vrc7_track_num = 0;			// VRC7�ȥ�å�
int		vrc6_track_num = 0;			// VRC6�ȥ�å�
int		n106_track_num = 0;			// ��ĥ����(namco106)���ѥȥ�å���
int		fme7_track_num = 0;			// FME7�ȥ�å�
int		mmc5_track_num = 0;			// MMC5�ȥ�å�


int		bank_sel[_TRACK_MAX];	// 0 �� 127 = �Х��ڤ��ؤ� , 0xFF = �ѹ�̵��
int		allow_bankswitching = 1;
int		dpcm_bankswitch = 0;
int		auto_bankswitch = 0;
int		curr_bank = 0x00;
int		bank_usage[128];		//bank_usage[0]�Ϻ��ΤȤ���̵��̣
int		bank_maximum = 0;		//8KB
int		dpcm_extra_bank_num = 0;	//8KB

int tone_tbl[          _TONE_MAX][1024];	// Tone
int envelope_tbl[  _ENVELOPE_MAX][1024];	// Envelope
int pitch_env_tbl[_PITCH_ENV_MAX][1024];	// Pitch Envelope
int pitch_mod_tbl[_PITCH_MOD_MAX][   5];	// LFO
int arpeggio_tbl[  _ARPEGGIO_MAX][1024];	// Arpeggio
int fm_tone_tbl[    _FM_TONE_MAX][2+64];	// FM Tone
int vrc7_tone_tbl[_VRC7_TONE_MAX][2+64];	// VRC7 Tone(������ϻ��Ѵؿ��δط�)
int n106_tone_tbl[_N106_TONE_MAX][2+64];	// NAMCO106 Tone
int hard_effect_tbl[_HARD_EFFECT_MAX][5];	// FDS Hardware Effect
int effect_wave_tbl[_EFFECT_WAVE_MAX][33];	// Effect Wave (4088) Data

DPCMTBL	dpcm_tbl[_DPCM_MAX];				// DPCM
unsigned char	*dpcm_data;	// DPCMŸ���ǡ���
int	dpcm_size = 0;
int	dpcm_reststop = 0;

char	song_name[1024] = "Song Name\0";
char	composer[1024] = "Artist\0";
char	maker[1024] = "Maker\0";
char	programer_buf[1024] = "";
char	*programer = NULL;

const	char	str_track[] = _TRACK_STR;

// ���顼�ֹ�
enum {
	COMMAND_NOT_DEFINED = 0,
	DATA_ENDED_BY_LOOP_DEPTH_EXCEPT_0,
	DEFINITION_IS_WRONG,
	TONE_DEFINITION_IS_WRONG,
	ENVELOPE_DEFINITION_IS_WRONG,
	PITCH_ENVELOPE_DEFINITION_IS_WRONG,
	NOTE_ENVELOPE_DEFINITION_IS_WRONG,
	LFO_DEFINITION_IS_WRONG,
	DPCM_DEFINITION_IS_WRONG,
	DPCM_PARAMETER_IS_LACKING,
	FM_TONE_DEFINITION_IS_WRONG,
	ABNORMAL_PARAMETERS_OF_FM_TONE,
	N106_TONE_DEFINITION_IS_WRONG,
	ABNORMAL_PARAMETERS_OF_N106_TONE,
	ABNORMAL_VALUE_OF_REPEAT_COUNT,
	ABNORMAL_TONE_NUMBER,
	ABNORMAL_ENVELOPE_NUMBER,
	ABNORMAL_ENVELOPE_VALUE,
	ABNORMAL_PITCH_ENVELOPE_NUMBER,
	ABNORMAL_NOTE_ENVELOPE_NUMBER,
	ABNORMAL_LFO_NUMBER,
	ABNORMAL_PITCH_VALUE,
	ABNORMAL_VOLUME_VALUE,
	ABNORMAL_TEMPO_VALUE,
	ABNORMAL_QUANTIZE_VALUE,
	ABNORMAL_SHUFFLE_QUANTIZE_VALUE,
	ABNORMAL_SWEEP_VALUE,
	ABNORMAL_DETUNE_VALUE,
	ABNORMAL_SHIFT_AMOUNT,
	RELATIVE_VOLUME_WAS_USED_WITHOUT_SPECIFYING_VOLUME,
	VOLUME_RANGE_OVER_OF_RELATIVE_VOLUME,
	VOLUME_RANGE_UNDER_OF_RELATIVE_VOLUME,
	DATA_ENDED_BY_CONTINUATION_NOTE,
	DPCM_FILE_NOT_FOUND,
	DPCM_FILE_SIZE_OVER,
	DPCM_FILE_TOTAL_SIZE_OVER,
	INVALID_TRACK_HEADER,
	HARD_EFFECT_DEFINITION_IS_WRONG,
	EFFECT_WAVE_DEFINITION_IS_WRONG,
	ABNORMAL_HARD_EFFECT_NUMBER,
	ABNORMAL_TRANSPOSE_VALUE,
	TUPLET_BRACE_EMPTY,
	BANK_IDX_OUT_OF_RANGE,
	FRAME_LENGTH_LESSTHAN_0,
	ABNORMAL_NOTE_LENGTH_VALUE,
	PARAMETER_IS_LACKING,
	ABNORMAL_SELFDELAY_VALUE,
	CANT_USE_BANK_2_OR_3_WITH_DPCMBANKSWITCH,
	CANT_USE_SHIFT_AMOUNT_WITHOUT_PITCH_CORRECTION,
	UNUSE_COMMAND_IN_THIS_TRACK,
};

// ���顼ʸ����
const	char	*ErrorlMessage[] = {
	"����Υ��ޥ�ɤϤ���ޤ���",							"Command not defined",
	"�롼�׿��٤�0�ʳ��ǥǡ�������λ���ޤ���",				"Data ended by loop depth except 0",
	"����˸�꤬����ޤ�",									"Definition is wrong",
	"PSG��������˸�꤬����ޤ�",							"PSG Tone definition is wrong",
	"����٥�������˸�꤬����ޤ�",						"Envelope definition is wrong",
	"�ԥå�����٥�������˸�꤬����ޤ�",				"Pitch envelope definition is wrong",
	"�Ρ��ȥ���٥�������˸�꤬����ޤ�",				"Note envelope definition is wrong",
	"LFO����˸�꤬����ޤ�",								"LFO definition is wrong",
	"DPCM����˸�꤬����ޤ�",								"DPCM definition is wrong",
	"DPCM����Υѥ�᡼����­��ޤ���",						"DPCM parameter is lacking",
	"FM��������˸�꤬����ޤ�",							"FM tone definition is wrong",
	"FM�Ѳ����Υѥ�᡼�����۾�Ǥ�",						"Abnormal parameters of FM tone",
	"namco106��������˸�꤬����ޤ�",						"namco106 tone definition is wrong",
	"namco106�Ѳ����Υѥ�᡼�����۾�Ǥ�",					"Abnormal parameters of namco106 tone",
	"�����֤�������ͤ��۾�Ǥ�",							"Abnormal value of repeat count",
	"�����ֹ椬�۾�Ǥ�",									"Abnormal tone number",
	"����٥����ֹ椬�۾�Ǥ�",							"Abnormal envelope number",
	"����٥��פ��ͤ��۾�Ǥ�",							"Abnormal envelope value",
	"�ԥå�����٥����ֹ���ͤ��۾�Ǥ�",					"Abnormal pitch envelope number",
	"�Ρ��ȥ���٥����ֹ���ͤ��۾�Ǥ�",					"Abnormal note envelope number",
	"LFO�ֹ���ͤ��۾�Ǥ�",								"Abnormal LFO number",
	"�������ͤ��۾�Ǥ�",									"Abnormal pitch value",
	"���̤��ͤ��۾�Ǥ�",									"Abnormal volume value",
	"�ƥ�ݤ��ͤ��۾�Ǥ�",									"Abnormal tempo value",
	"�����󥿥������ͤ��۾�Ǥ�",							"Abnormal quantize value",
	"����åե륯���󥿥������ͤ��۾�Ǥ�",							"Abnormal shuffle quantize value",
	"�������פ��ͤ��۾�Ǥ�",								"Abnormal sweep value",
	"�ǥ����塼����ͤ��۾�Ǥ�",							"Abnormal detune value",
	"�ԥå����ե��̤��ͤ��۾�Ǥ�",							"Abnormal pitch shift amount value",
	"���̤����ꤵ��Ƥ��ʤ����֤����в��̤���Ѥ��ޤ���",	"Relative volume was used without specifying volume",
	"���в���(+)�ǲ��̤��ϰϤ�Ķ���ޤ���",					"Volume range over(+) of relative volume",
	"���в���(-)�ǲ��̤��ϰϤ�Ķ���ޤ���",					"Volume range under(-) of relative volume",
	"Ϣ�����������ǥǡ�������λ���ޤ���",					"Data ended by Continuation note",
	"DPCM�ե����뤬����ޤ���",								"DPCM file not found",
	"DPCM�ǡ����Υ�������4081byte��Ķ���ޤ���",				"DPCM file size over",
	"DPCM�ǡ����Υ�����������Υ�������Ķ���ޤ���",			"DPCM file total size over",
	"����Υȥ�å��إå���̵���Ǥ�",						"Invalid track header",
	"�ϡ��ɥ��������ե���������˸�꤬����ޤ���",			"Hardware effect definition is wrong",
	"���ե������ȷ�����˸�꤬����ޤ���",					"Effect wavetable definition is wrong",
	"�ϡ��ɥ��������ե������ֹ���ͤ��۾�Ǥ���",			"Abnormal hardware effect number",
	"�ȥ�󥹥ݡ������ͤ��۾�Ǥ�",							"Abnormal transpose value",
	"Ϣ���{}����˲��䤬����ޤ���",						"Tuplet {} empty",
	"�Х󥯤��ϰϤ�Ķ���ޤ���",						"Bank index out of range",
	"��Ĺ������ͤǤ�(unexpected error)",			"Frame length is negative value (unexpected error)",
	"��Ĺ���ͤ��۾�Ǥ�",					"Abnormal note length value",
	"����Υѥ�᡼����­��ޤ���",						"Parameter is lacking",
	"����եǥ��쥤���ͤ��۾�Ǥ�",						"Abnormal self-delay value",
	"DPCM��������0x4000��Ķ������ϥХ�2��3�ϻ��ѤǤ��ޤ���",		"Cannot use bank 2 or 3 if DPCM size is greater than 0x4000",
	"#PITCH-CORRECTION����ꤷ�ʤ��¤�ԥå����ե��̥��ޥ�ɤϻ��ѤǤ��ޤ���",		"Cannot use SA<num> without #PITCH-CORRECTION",
	"���Υȥ�å��Ǥϻ��ѤǤ��ʤ����ޥ�ɤǤ�",				"Unuse command in this track",
};



enum {
	TOO_MANY_INCLUDE_FILES = 0,
	FRAME_LENGTH_IS_0,
	REPEAT2_FRAME_ERROR_OVER_3,
	IGNORE_PMCKC_BANK_CHANGE,
	THIS_NUMBER_IS_ALREADY_USED,
	DPCM_FILE_SIZE_ERROR,
};

const	char	*WarningMessage[] = {
	"���󥯥롼�ɥե�����ο���¿�����ޤ�",					"Too many include files",
	"�ե졼�಻Ĺ��0�ˤʤ�ޤ�����",						"frame length is 0",
	"��ԡ���2�Υե졼�����3�ե졼���Ķ���Ƥ��ޤ���",	"Repeat2 frame error over 3 frames",
	"#BANK-CHANGE���ѻ���#SETBANK, NB��̵�뤷�ޤ�",		"Ignoring #SETBANK and NB if #BANK-CHANGE used",
	"����ֹ椬��ʣ���Ƥ��ޤ�",				"This definition number is already used",
	"DPCM������ mod 16 ��1�ǤϤ���ޤ���",			"DPCM size mod 16 is not 1",
};



/*--------------------------------------------------------------
	���顼ɽ��
 Input:
	
 Output:
	none
--------------------------------------------------------------*/
void dispError( int no, char *file, int line )
{
	no = no*2;
	if( message_flag != 0 ) {
		no++;
	}
	if( file != NULL ) {
		printf( "Error  : %s %6d: %s\n", file, line, ErrorlMessage[no] );
	} else {
		printf( "Error  : %s\n", ErrorlMessage[no] );
	}
	error_flag = 1;
}



/*--------------------------------------------------------------
	��˥�ɽ��
 Input:
	
 Output:
	none
--------------------------------------------------------------*/
void dispWarning( int no, char *file, int line )
{
	if( warning_flag != 0 ) {
		no = no*2;
		if( message_flag != 0 ) {
			no++;
		}
		if( file != NULL ) {
			printf( "Warning: %s %6d: %s\n", file, line, WarningMessage[no] );
		} else {
			printf( "Warning: %s\n", WarningMessage[no] );
		}
	}
}



/*--------------------------------------------------------------
	C���쥿���פΥ�ޡ����κ��
 Input:
	char	*ptr		:�ǡ�����Ǽ�ݥ���
 Output:
	none
--------------------------------------------------------------*/
void deleteCRemark( char *ptr )
{
	int within_com = 0;
	while ( *ptr != '\0' ) {
		if ( *ptr == '/' && *(ptr + 1) == '*' ) {
			within_com = 1;
			*ptr++ = ' ';
			*ptr++ = ' ';
			while (*ptr) {
				if (*ptr == '*' && *(ptr + 1) == '/') {
					*ptr++ = ' ';
					*ptr++ = ' ';
					within_com = 0;
					break;
				}
				if ( *ptr != '\n' ) {
					*ptr = ' ';
				}
				ptr++;
			}
		} else {
			++ptr;
		}
	}
	if (within_com) {
		printf("Warning :");
		printf( message_flag 	? "Reached EOF in comment"
					: "�����Ȥ��Ĥ����ʤ��ޤޥե����뽪ü��ã���ޤ���");
		printf("\n");
	}
}


//����
/*--------------------------------------------------------------
	��ޡ����κ��
 Input:
	char	*ptr	:�ǡ�����Ǽ�ݥ���
 Output:
	̵��
--------------------------------------------------------------*/
void deleteRemark( char *ptr )
{
	while( *ptr != '\0' ) {
		if ( *ptr == '/' || *ptr == ';' ) {
			while( *ptr != '\0' ) {
				if( *ptr != '\n' ) {
					*ptr++ = ' ';
				} else {
					ptr++;
					break;
				}
			}
		} else {
			ptr++;
		}
	}
}



/*----------------------------------------------------------*/
/*	�ե�����Կ������								    */
/* Input:												    */
/*	char	*data		:�ǡ�����Ǽ�ݥ���				    */
/* Output:												    */
/*	none												    */
/*----------------------------------------------------------*/
int getLineCount( char *ptr )
{
	int	line;

	line = 0;

	while( *ptr != '\0' ) {
		if( *ptr == '\n' ) {
			line++;
		}
		ptr++;
	}
	if( *(ptr-1) != '\n' ) {
		line++;
	}
	return line;
}


/*--------------------------------------------------------------
--------------------------------------------------------------*/
LINE *readMmlFile(char *fname)
{
	LINE *lptr;
	int line_count;
	int i;
	char *filestr;
	filestr = readTextFile(fname);
	
	if (filestr == NULL) {
		error_flag = 1;
		return NULL;
	}
	
	deleteCRemark(filestr);
	
	//skipSpace���Ȥ߹���
	//deleteRemark(filestr);

	line_count = getLineCount(filestr);
	lptr = (LINE *)malloc( (line_count+1)*sizeof(LINE) );	/* �饤��Хåե������ */

	lptr[0].status = _HEADER;		/* LINE���ơ�����[0]��malloc���줿	*/
	lptr[0].str    = filestr;		/* �ݥ��󥿤ȥ���������Ǽ����Ƥ��� */
	lptr[0].line   = line_count;
	lptr[0].filename = fname;
	for( i = 1; i <= line_count; i++ ) {
		lptr[i].filename = fname;
		lptr[i].line = i;
	}
	return lptr;
}




/*--------------------------------------------------------------
	����/EOF��0(NULL)�ˤ���(�Хåե����ñ�̤��ڤ�ʬ��)
 Input:
	char	*ptr	:�ǡ�����Ǽ�ݥ���
 Output:
	̵��
--------------------------------------------------------------*/
char *changeNULL( char *ptr )
{
	while( *ptr != '\n' ) {
		if( *ptr == '\0' ) break;
		ptr++;
	}
	*ptr = '\0';
	ptr++;

	return ptr;
}


/*---------------------------------------------------------
  @hoge123 = { ag ae aeag g} �ν���
  
  @HOGE\s*(\d+)\s*(=|)\s*{.*?(}.*|)$
-----------------------------------------------------------*/
int setEffectSub(LINE *lptr, int line, int *ptr_status_end_flag, int min, int max, const int error_no)
{
	int param, cnt;
	char *temp;
	temp = skipSpace( lptr[line].str );
	param = Asc2Int( temp, &cnt );
	
	if (cnt == 0)
		goto on_error;
	if (param < min || max <= param)
		goto on_error;
	
	lptr[line].param = param;
	temp = skipSpace( temp+cnt );
	
	if ( *temp == '=' ) {
		temp++;
		temp = skipSpace( temp );
	}
	
	if ( *temp != '{' )
		goto on_error;
	
	lptr[line].str = temp;
	*ptr_status_end_flag = 1;
	
	while ( *temp != '\0' ) {
		if( *temp == '}' ) {
			*ptr_status_end_flag = 0;
		}
		temp++;
	}
	return 1;
on_error:
	lptr[line].status = 0;
	dispError( error_no, lptr[line].filename, line );
	return 0;
}




/*--------------------------------------------------------------
	�إå��������
 Input:
	char	*ptr	:�ǡ�����Ǽ�ݥ���
 Output:
	̵��
--------------------------------------------------------------*/
void getLineStatus(LINE *lptr, int inc_nest )
{
	const HEAD head[] = {
		{ "#TITLE",          _TITLE          },
		{ "#COMPOSER",       _COMPOSER       },
		{ "#MAKER",          _MAKER          },
		{ "#PROGRAMER",      _PROGRAMER      },
		{ "#OCTAVE-REV",     _OCTAVE_REV     },
		{ "#GATE-DENOM",        _GATE_DENOM  },
		{ "#INCLUDE",        _INCLUDE        },
		{ "#EX-DISKFM",      _EX_DISKFM      },
		{ "#EX-NAMCO106",    _EX_NAMCO106    },
		{ "#EX-VRC7",		 _EX_VRC7		 },
		{ "#EX-VRC6",		 _EX_VRC6		 },
		{ "#EX-FME7",		 _EX_FME7		 },
		{ "#EX-MMC5",		 _EX_MMC5		 },
		{ "#NO-BANKSWITCH",    _NO_BANKSWITCH    },
		{ "#AUTO-BANKSWITCH",    _AUTO_BANKSWITCH    },
		{ "#PITCH-CORRECTION",       _PITCH_CORRECTION    },
		{ "#BANK-CHANGE",    _BANK_CHANGE    },
		{ "#SETBANK",    	 _SET_SBANK	     },
		{ "#EFFECT-INCLUDE", _EFFECT_INCLUDE },
		{ "#DPCM-RESTSTOP", _DPCM_RESTSTOP },
		{ "@DPCM",           _SET_DPCM_DATA  },
		{ "@MP",             _SET_PITCH_MOD  },
		{ "@EN",             _SET_ARPEGGIO   },
		{ "@EP",             _SET_PITCH_ENV  },
		{ "@FM",             _SET_FM_TONE    },
		{ "@MH",             _SET_HARD_EFFECT},
		{ "@MW",             _SET_EFFECT_WAVE},
		{ "@OP",             _SET_VRC7_TONE  },
		{ "@N",              _SET_N106_TONE  },			
		{ "@V",              _SET_ENVELOPE   },
		{ "@",               _SET_TONE       },
		{ "",                -1              },
	};

	int	line, i, param, cnt, track_flag, status_end_flag, bank,bank_ch;
	char	*temp, *temp2;
	char *ptr;
	ptr = lptr[0].str;

	status_end_flag = 0;

	for( line = 1; line <= lptr->line; line++ ) {
		ptr = skipSpace( ptr );
		/* ���ιԤ����ե���������������ä��� */
		if( ((lptr[line-1].status&_SET_EFFECT) != 0) && (status_end_flag != 0) ) {
			lptr[line].status = lptr[line-1].status|_SAME_LINE;
			lptr[line].param  = lptr[line-1].param;
			lptr[line].str = ptr;
			temp = ptr;
			ptr = changeNULL( ptr );
			status_end_flag = 1;
			while( *temp != '\0' ) {
				if( *temp == '}' ) {
					status_end_flag = 0;
				}
				temp++;
			}
		/* �Ԥ���Ƭ�˲���̵������̵���ʹԤȤ��� */
		} else if( *ptr == '\n' || *ptr == '\0' ) {
			lptr[line].status = 0;
			lptr[line].str = ptr;
			ptr = changeNULL( ptr );
		} else {
			/* #/@�դ��إå����λ��ϥإå�����ʸ�������ʸ���ˤ��� */
			if( *ptr == '#' || *ptr == '@' ) {
				i = 1;
				while( (*(ptr+i) != ' ') && (*(ptr+i) != '\t') && (*(ptr+i) != '\n') ) {
					*(ptr+i) = (char)toupper( *(ptr+i) );
					i++;
				}
				/* �إå���������ơ��֥�ʸ�������� */
				for( i = 0; head[i].status != -1; i++ ) {
					if( strncmp( ptr, head[i].str, strlen(head[i].str) ) == 0 ) {
						break;
					}
				}
				lptr[line].status = head[i].status;
				lptr[line].str = skipSpaceOld( ptr+strlen(head[i].str) );	/* �إå����ܶ�������Ф����Ȥ������Ƭ�� */
			} else if( strchr( str_track, *ptr ) ) {
				track_flag = 0;
				while( strchr( str_track, *ptr ) ) {
					temp = strchr( str_track, *ptr );
					if( temp == NULL ) {
						dispError( INVALID_TRACK_HEADER, lptr[line].filename, line );
					} else {
						track_flag |= (1<<(temp-str_track));
					}
					ptr++;
				}
				// �ȥ�å����ĤΥ����å�
				for (i = 0; i < _TRACK_MAX; i++) {
					if( (TRACK(i) & track_flag) && !(TRACK(i) & track_allow_flag) ) {
						dispError( INVALID_TRACK_HEADER, lptr[line].filename, line );
						track_flag &= ~TRACK(i);
					}
				}
				
				if( track_flag != 0 ) {
					lptr[line].status = _TRACK;
					lptr[line].param = track_flag;
					lptr[line].str = skipSpace( ptr );
					actual_track_flag |= track_flag;				
				} else {
					lptr[line].status = 0;
					lptr[line].param = 0;
				}
			} else {
				lptr[line].status = -1;
				lptr[line].str = skipSpace( ptr );
			}
			ptr = changeNULL( ptr );

			switch( lptr[line].status ) {
			/* Include���ޥ�ɤν��� */
			  case _INCLUDE:
				if( inc_nest > 16 ) {				/* �ͥ��Ȥ�16�ʤޤ�(�Ƶ��ǸƤФ��Ƚ�λ���ʤ��Τ�) */
					dispWarning( TOO_MANY_INCLUDE_FILES, lptr[line].filename, line );
					lptr[line].status = 0;
				} else {
					LINE *ltemp;
					temp = skipSpaceOld( lptr[line].str ); /* /��ȤФ��ʤ��褦�ˤ��Ƥߤ� */
					ltemp = readMmlFile(temp);
					if( ltemp != NULL ) {
						lptr[line].inc_ptr = ltemp;
						++inc_nest;
						getLineStatus(lptr[line].inc_ptr, inc_nest);
						--inc_nest;
					} else {
						lptr[line].status = 0;					/* �ե������ɤ߹��߼��Ԥ��դ����顼 */
						error_flag = 1;
					}
				}
				break;
			/* LFO���ޥ�� */
			  case _SET_PITCH_MOD:
				setEffectSub(lptr, line, &status_end_flag, 0, _PITCH_MOD_MAX, LFO_DEFINITION_IS_WRONG);
				break;
			/* �ԥå�����٥��ץ��ޥ�� */
			  case _SET_PITCH_ENV:
				setEffectSub(lptr, line, &status_end_flag, 0, _PITCH_ENV_MAX, PITCH_ENVELOPE_DEFINITION_IS_WRONG);
				break;
			/* ���̥���٥��ץ��ޥ�� */
			  case _SET_ENVELOPE:
				setEffectSub(lptr, line, &status_end_flag, 0, _ENVELOPE_MAX, ENVELOPE_DEFINITION_IS_WRONG);
				break;
			/* ����� */
			  case _SET_TONE:
				setEffectSub(lptr, line, &status_end_flag, 0, _TONE_MAX, TONE_DEFINITION_IS_WRONG);
				break;
			/* ����ڥ��� */
			  case _SET_ARPEGGIO:
				setEffectSub(lptr, line, &status_end_flag, 0, _ARPEGGIO_MAX, NOTE_ENVELOPE_DEFINITION_IS_WRONG);
				break;
			/* DPCM��Ͽ���ޥ�� */
			  case _SET_DPCM_DATA:
				setEffectSub(lptr, line, &status_end_flag, 0, _DPCM_MAX, DPCM_DEFINITION_IS_WRONG);
				break;
			  /* VRC7 Tone */
			  case _SET_VRC7_TONE:
				setEffectSub(lptr, line, &status_end_flag, 0, _VRC7_TONE_MAX, FM_TONE_DEFINITION_IS_WRONG);
				break;
			/* FM���� */
			  case _SET_FM_TONE:
				setEffectSub(lptr, line, &status_end_flag, 0, _FM_TONE_MAX, FM_TONE_DEFINITION_IS_WRONG);
				break;
			/* namco106�������� */
			  case _SET_N106_TONE:
				setEffectSub(lptr, line, &status_end_flag, 0, _N106_TONE_MAX, N106_TONE_DEFINITION_IS_WRONG);
				break;
			/* �ϡ��ɥ��������ե����� */
			  case _SET_HARD_EFFECT:
				setEffectSub(lptr, line, &status_end_flag, 0, _HARD_EFFECT_MAX, HARD_EFFECT_DEFINITION_IS_WRONG);
				break;
			/* ���ե������ȷ� */
			  case _SET_EFFECT_WAVE:
				setEffectSub(lptr, line, &status_end_flag, 0, _EFFECT_WAVE_MAX, EFFECT_WAVE_DEFINITION_IS_WRONG);
				break;
			/* DISKSYSTEM FM�������ѥե饰 */
			  case _EX_DISKFM:
				sndgen_flag |= BDISKFM;
				track_allow_flag |= FMTRACK;
				fds_track_num = 1;
				break;
			/* VRC7 FM�������ѥե饰 */
			  case _EX_VRC7:
				sndgen_flag |= BVRC7;
				track_allow_flag |= VRC7TRACK;
				vrc7_track_num = 6;
				break;
			/* VRC6 �������ѥե饰 */
			  case _EX_VRC6:
				sndgen_flag |= BVRC6;
				track_allow_flag |= VRC6TRACK;
				vrc6_track_num = 3;
				break;
			/* FME7 �������ѥե饰 */
			  case _EX_FME7:
				sndgen_flag |= BFME7;
				track_allow_flag |= FME7TRACK;
				fme7_track_num = 3;
				break;
			/* MMC5 �������ѥե饰 */
			  case _EX_MMC5:
				sndgen_flag |= BMMC5;
				track_allow_flag |= MMC5TRACK;
				mmc5_track_num = 2;
				break;
			/* namco106 ��ĥ�������ѥե饰 */
			  case _EX_NAMCO106:
				temp = skipSpace( lptr[line].str );
				param = Asc2Int( temp, &cnt );
				if( cnt != 0 && (0 <= param && param <= 8) ) {
					if( param == 0 ) {
						param = 1;
					}
					lptr[line].param = param;
					n106_track_num = param;
					sndgen_flag |= BNAMCO106;
					for (i = 0; i < param; i++) {
						track_allow_flag |= TRACK(BN106TRACK+i);
					}
				} else {
					dispError( DEFINITION_IS_WRONG, lptr[line].filename, line );
					lptr[line].status = 0;
				}
				break;
			/* DPCM sound stops on 'r' command */
			  case _DPCM_RESTSTOP:
				dpcm_reststop = 1;
				break;
			/* NSF mapper �� bankswitching �ػ� */
			  case _NO_BANKSWITCH:
				allow_bankswitching = 0;
				break;
			/* ��ư�Х��ڤ��ؤ� */
			  case _AUTO_BANKSWITCH:
				temp = skipSpace( lptr[line].str );
				param = Asc2Int( temp, &cnt );
				if ( cnt != 0 && (0 <= param && param <= 8192)) {
					// �ǽ�ΰ�󤷤�ͭ���ˤ��ʤ�
					if (!auto_bankswitch) {
						bank_usage[0] = 8192 - param;
					}
					auto_bankswitch = 1;
				} else {
					dispError( DEFINITION_IS_WRONG, lptr[line].filename, line );
				}
				break;
			/* �Х��ڤ��ؤ�������(��������θߴ�����) */
			  case _BANK_CHANGE:
				/*
					#BANK-CHANGE <num0>,<num1>
					�嵭�Х��ڤ��ؤ��γ�ĥ�񼰤Ǥ���<num0>�ϥХ��ֹ��0��2���ͤ�
					����ޤ���<num1>�ϥȥ�å��ֹ��1��14�ο��ͤ����ꡢ1��A�ȥ�å���
					�б����Ƥ���ʲ�2=B��3=C����P=7�ȤʤäƤ��ޤ���
					���ʤߤ˰ʲ���Ʊ�����Ȥ򤷤Ƥ��ޤ���
					#BANK-CHANGE	n
					#BANK-CHANGE	0,n
					
					#BANK-CHANGE��Ʊ���Х󥯤˥ȥ�å�����äƤ��ä���硢
					�Ǹ�˻��ꤷ����Τ�����ͭ�����Ȥ������ͤϤ��ޤ����򤵤�Ƥ��ʤ��ä���
					ppmck�Ǥ�����ͭ���Ȥ��뤿�ᡢ����������ߴ���
					
					mckc�ѤθŤ�MML�򥳥�ѥ��뤹�뤿��ˤ�
					�Ǹ�Τ�ΰʳ��ä���
				
				*/
				/*
					�����ȥȥ�å����б�����ߴ���
				
					mckc
					A B C D E | F | P Q R  S  T  U  V  W
					1 2 3 4 5 | 6 | 7 8 9 10 11 12 13 14
					pmckc
					A B C D E | F | G H I  J  K  L |  P  Q  R  S  T  U  V  W
					1 2 3 4 5 | 6 | 7 8 9 10 11 12 | 13 14 15 16 17 18 19 20
					ppmckc
					A B C D E | F | G H I  J  K  L |  M  N  O |  P  Q  R  S  T  U  V  W |  X  Y  Z |  a  b
					1 2 3 4 5 | 6 | 7 8 9 10 11 12 | 13 14 15 | 16 17 18 19 20 21 22 23 | 24 25 26 | 27 28
				
					mckc�ѤθŤ�MML�򥳥�ѥ��뤹�뤿��ˤ�
					P�ʹߤ� ��ư�� 9 ��­����OK��(��ư�ˤϤ��ʤ��ۤ����褤�Ǥ��礦)
					�Ƥ������ɽ�򸫤ʤ��㤤���ʤ����Ȥ��ְ�ä�(ry
				*/
				temp = skipSpace( lptr[line].str );
				param = Asc2Int( temp, &cnt );
				if( cnt != 0 ) {
					temp += cnt;
					temp = skipSpace( temp );
					if( *temp == ',' ) {
						/* ��ĥ�� */
						temp++;
						if( (0 <= param) && (param <= 2) ) {
							bank = param; /* 0,1,2��1,2,3���б� */
							//printf( "bank: %d\n", bank );
							temp = skipSpace( temp );
							param = Asc2Int( temp, &cnt ); /* 1,2,3 ��ABC���б� ������ 0,1,2���б� */
							if( cnt != 0 && (1 <= param && param <= _TRACK_MAX) ) {
								//bank_change[bank] = param-1;
								bank_sel[param-1] = bank+1;
							} else {
								dispError( DEFINITION_IS_WRONG, lptr[line].filename, line );
								lptr[line].status = 0;
								//bank_change[bank] = 0xff;
							}
						} else {
							dispError( DEFINITION_IS_WRONG, lptr[line].filename, line );
							lptr[line].status = 0;
						}
					} else {
						/* ���ĥ�� bank 1������� */
						if( cnt != 0 && (1 <= param && param <= _TRACK_MAX) ) {
							//bank_change[0] = param-1;
							bank_sel[param-1] = 1;
						} else {
							dispError( DEFINITION_IS_WRONG, lptr[line].filename, line );
							lptr[line].status = 0;
							//bank_change[0] = 0xff;
						}
					}
				} else {
					dispError( DEFINITION_IS_WRONG, lptr[line].filename, line );
					lptr[line].status = 0;
				}
				break;
			/* �Х��ڤ��ؤ� */
			  case _SET_SBANK:
				temp = skipSpace( lptr[line].str );
				
				if ((temp2 = strchr(str_track, *temp)) != NULL) {
					/* ABC..�ˤ��ȥ�å����� */
					param = (temp2 - str_track) + 1;
					temp++;
				} else {
					/* �����ˤ��ȥ�å����� */
					param = Asc2Int( temp, &cnt );
					if (cnt == 0) {
						dispError( DEFINITION_IS_WRONG, lptr[line].filename, line );
						lptr[line].status = 0;
						break;
					} else {
						temp += cnt;
					}
				}
				
				temp = skipSpace( temp );
				if( *temp == ',' ) {		/* �Х󥯳�ĥ */
					temp++;
					if( (1 <= param) && (param <= _TRACK_MAX) ) {
						bank_ch = param;
						// printf( "bank: %d\n", bank );
						temp = skipSpace( temp );
						param = Asc2Int( temp, &cnt );
						if( cnt != 0) {
							if (checkBankRange(param) == 0) {
								dispError( BANK_IDX_OUT_OF_RANGE, lptr[line].filename, line );
								break;
							}
							bank_sel[bank_ch-1] = param;
						} else {
							dispError( DEFINITION_IS_WRONG, lptr[line].filename, line );
							lptr[line].status = 0;
						}
					} else {
						dispError( DEFINITION_IS_WRONG, lptr[line].filename, line );
						lptr[line].status = 0;
					}
				} else {
					dispError( DEFINITION_IS_WRONG, lptr[line].filename, line );
					lptr[line].status = 0;
				}
				break;

			/*  */
			  case _EFFECT_INCLUDE:
				include_flag = 1;
				break;
			/* �����ȥ� */
			  case _TITLE:
				temp = skipSpaceOld( lptr[line].str );
				strncpy( song_name, temp, 1023 );
				break;
			/* ��ʼ� */
			  case _COMPOSER:
				temp = skipSpaceOld( lptr[line].str );
				strncpy( composer, temp, 1023 );
				break;
			/* �᡼���� */
			  case _MAKER:
				temp = skipSpaceOld( lptr[line].str );
				strncpy( maker, temp, 1023 );
				break;
			/* �Ǥ����߼� */
			  case _PROGRAMER:
				temp = skipSpaceOld( lptr[line].str );
				strncpy( programer_buf, temp, 1023 );
				programer = programer_buf;
				break;
			/* ���������ֵ����ȿž */
			  case _OCTAVE_REV:
				temp = skipSpace( lptr[line].str );
				param = Asc2Int( temp, &cnt );
				if( cnt != 0) {
					if( param == 0 ) {
						octave_flag = 0;
					} else {
						octave_flag = 1;
					}
				} else {
					octave_flag = 1;
				}
				break;
			/* q���ޥ��ʬ���ѹ� */
			  case _GATE_DENOM:
				temp = skipSpace( lptr[line].str );
				param = Asc2Int( temp, &cnt );
				if( cnt != 0 && param > 0) {
					gate_denom = param;
				} else {
					dispError( DEFINITION_IS_WRONG, lptr[line].filename, line );
					lptr[line].status = 0;
				}
				break;
			/*�ǥ����塼�󡢥ԥå�����٥��ס�LFO���������� */
			  case _PITCH_CORRECTION:
				pitch_correction = 1;
				break;
			/* �إå�̵�� */
			  case -1:
				if( (lptr[line-1].status&_SET_EFFECT) != 0 ) {
					lptr[line].status = lptr[line-1].status|_SAME_LINE;
					lptr[line].str = ptr;
				} else {
					/* ���顼�����å� */
					dispError( COMMAND_NOT_DEFINED, lptr[line].filename, line );
					lptr[line].status = 0;
					lptr[line].str = ptr;
				}
				break;
			}
		}
	}
}



/*--------------------------------------------------------------
	�����μ���
 Input:
	
 Output:
	̵��
--------------------------------------------------------------*/
void getTone( LINE *lptr )
{
	int		line, i, no, end_flag, offset, num, cnt;
	char	*ptr;

	cnt = 0;

	for( line = 1; line <= lptr->line; line++ ) {
		/* �����ǡ���ȯ���� */
		if( lptr[line].status == _SET_TONE ) {
			no = lptr[line].param;				/* �����ֹ���� */
			ptr = lptr[line].str;
			ptr++;								/* '{'��ʬ�����Ф� */
			if (tone_tbl[no][0] != 0) {
				dispWarning( THIS_NUMBER_IS_ALREADY_USED, lptr[line].filename, line );
			}
			tone_tbl[no][0] = 0;
			offset = 0;
			i = 1;
			end_flag = 0;
			while( end_flag == 0 ) {
				ptr = skipSpace( ptr );
				switch( *ptr ) {
				  case '}':
					if (tone_tbl[no][0] >= 1) {
						tone_tbl[no][i] = EFTBL_END;
						tone_tbl[no][0]++;
					} else {
						dispError( PARAMETER_IS_LACKING, lptr[line].filename, line );
						tone_tbl[no][0] = 0;
					}
					end_flag = 1;
					line += offset;
					break;
				  case '|':
					tone_tbl[no][i] = EFTBL_LOOP;
					tone_tbl[no][0]++;
					i++;
					ptr++;
					break;
				  case '\0':
					offset++;
					if( line+offset <= lptr->line ) {
						if( (lptr[line+offset].status&_SAME_LINE) == _SAME_LINE ) {
							ptr = lptr[line+offset].str;
						}
					} else {
						dispError( TONE_DEFINITION_IS_WRONG, lptr[line].filename, line );
						tone_tbl[no][0] = 0;
						end_flag = 1;
					}
					break;
				  default:
					num = Asc2Int( ptr, &cnt );
					//vrc6�Ѥ����¤򳰤�(��¢����ȡ�MMC5��3�ޤ�)
					//if( cnt != 0 && (0 <= num && num <= 3) ) {
					if( cnt != 0 && (0 <= num && num <= 7) ) {
						tone_tbl[no][i] = num;
						tone_tbl[no][0]++;
						ptr += cnt;
						i++;
					} else {
						dispError( TONE_DEFINITION_IS_WRONG, lptr[line+offset].filename, line );
						tone_tbl[no][0] = 0;
						end_flag = 1;
					}
					break;
				}
				ptr = skipSpace( ptr );
				if( *ptr == ',' ) {
					ptr++;
				}
			}
		/* �������������_SAME_LINE�λ��ϥ��顼 */
		} else if( lptr[line].status == (_SET_TONE|_SAME_LINE) ) {
			dispError( TONE_DEFINITION_IS_WRONG, lptr[line].filename, line );
		/* ���󥯥롼�ɥե�������� */
		} else if( lptr[line].status == _INCLUDE ) {
			getTone( lptr[line].inc_ptr );
		}
	}
}



/*--------------------------------------------------------------
	����٥��פμ���
 Input:
	
 Output:
	̵��
--------------------------------------------------------------*/
void getEnvelope( LINE *lptr )
{
	int		line, i, no, end_flag, offset, num, cnt;
	char	*ptr;

	cnt = 0;

	for( line = 1; line <= lptr->line; line++ ) {
		/* ����٥��ץǡ���ȯ���� */
		if( lptr[line].status == _SET_ENVELOPE ) {
			no = lptr[line].param;				/* ����٥����ֹ���� */
			ptr = lptr[line].str;
			ptr++;								/* '{'��ʬ�����Ф� */
			if (envelope_tbl[no][0] != 0) {
				dispWarning( THIS_NUMBER_IS_ALREADY_USED, lptr[line].filename, line );
			}
			envelope_tbl[no][0] = 0;
			offset = 0;
			i = 1;
			end_flag = 0;
			while( end_flag == 0 ) {
				ptr = skipSpace( ptr );
				switch( *ptr ) {
				  case '}':
					if (envelope_tbl[no][0] >= 1) {
						envelope_tbl[no][i] = EFTBL_END;
						envelope_tbl[no][0]++;
					} else {
						dispError( PARAMETER_IS_LACKING, lptr[line].filename, line );
						envelope_tbl[no][0] = 0;
					}
					end_flag = 1;
					line += offset;
					break;
				  case '|':
					envelope_tbl[no][i] = EFTBL_LOOP;
					envelope_tbl[no][0]++;
					i++;
					ptr++;
					break;
				  case '\0':
					offset++;
					if( line+offset <= lptr->line ) {
						if( (lptr[line+offset].status&_SAME_LINE) == _SAME_LINE ) {
							ptr = lptr[line+offset].str;
						}
					} else {
						dispError( ENVELOPE_DEFINITION_IS_WRONG, lptr[line].filename, line );
						envelope_tbl[no][0] = 0;
						end_flag = 1;
					}
					break;
				  default:
					num = Asc2Int( ptr, &cnt );
					if( cnt != 0 && (0 <= num && num <= 63) ) {
						envelope_tbl[no][i] = num;
						envelope_tbl[no][0]++;
						ptr += cnt;
						i++;
					} else {
						dispError( ENVELOPE_DEFINITION_IS_WRONG, lptr[line+offset].filename, line );
						envelope_tbl[no][0] = 0;
						end_flag = 1;
					}
					break;
				}
				ptr = skipSpace( ptr );
				if( *ptr == ',' ) {
					ptr++;
				}
			}
		/* ����٥������������_SAME_LINE�λ��ϥ��顼 */
		} else if( lptr[line].status == (_SET_ENVELOPE|_SAME_LINE) ) {
			dispError( ENVELOPE_DEFINITION_IS_WRONG, lptr[line].filename, line );
		/* ���󥯥롼�ɥե�������� */
		} else if( lptr[line].status == _INCLUDE ) {
			getEnvelope( lptr[line].inc_ptr );
		}
	}
}

/*--------------------------------------------------------------
	�ԥå�����٥��פμ���
 Input:
	
 Output:
	̵��
--------------------------------------------------------------*/
void getPitchEnv( LINE *lptr )
{
	int		line, i, no, end_flag, offset, num, cnt;
	char	*ptr;

	cnt = 0;

	for( line = 1; line <= lptr->line; line++ ) {
		/* �ԥå�����٥��ץǡ���ȯ���� */
		if( lptr[line].status == _SET_PITCH_ENV ) {
			no = lptr[line].param;				/* �ԥå�����٥����ֹ���� */
			ptr = lptr[line].str;
			ptr++;								/* '{'��ʬ�����Ф� */
			if (pitch_env_tbl[no][0] != 0) {
				dispWarning( THIS_NUMBER_IS_ALREADY_USED, lptr[line].filename, line );
			}
			pitch_env_tbl[no][0] = 0;
			offset = 0;
			i = 1;
			end_flag = 0;
			while( end_flag == 0 ) {
				ptr = skipSpace( ptr );
				switch( *ptr ) {
				  case '}':
					if (pitch_env_tbl[no][0] >= 1) {
						pitch_env_tbl[no][i] = EFTBL_END;
						pitch_env_tbl[no][0]++;
					} else {
						dispError( PARAMETER_IS_LACKING, lptr[line].filename, line );
						pitch_env_tbl[no][0] = 0;
					}
					end_flag = 1;
					line += offset;
					break;
				  case '|':
					pitch_env_tbl[no][i] = EFTBL_LOOP;
					pitch_env_tbl[no][0]++;
					i++;
					ptr++;
					break;
				  case '\0':
					offset++;
					if( line+offset <= lptr->line ) {
						if( (lptr[line+offset].status&_SAME_LINE) == _SAME_LINE ) {
							ptr = lptr[line+offset].str;
						}
					} else {
						dispError( PITCH_ENVELOPE_DEFINITION_IS_WRONG, lptr[line].filename, line );
						pitch_env_tbl[no][0] = 0;
						end_flag = 1;
					}
					break;
				  default:
					num = Asc2Int( ptr, &cnt );
					if( cnt != 0 && (-127 <= num && num <= 127) ) {
						pitch_env_tbl[no][i] = num;
						pitch_env_tbl[no][0]++;
						ptr += cnt;
						i++;
					} else {
						dispError( PITCH_ENVELOPE_DEFINITION_IS_WRONG, lptr[line+offset].filename, line );
						pitch_env_tbl[no][0] = 0;
						end_flag = 1;
					}
					break;
				}
				ptr = skipSpace( ptr );
				if( *ptr == ',' ) {
					ptr++;
				}
			}
		/* �ԥå�����٥������������_SAME_LINE�λ��ϥ��顼 */
		} else if( lptr[line].status == (_SET_PITCH_ENV|_SAME_LINE) ) {
			dispError( PITCH_ENVELOPE_DEFINITION_IS_WRONG, lptr[line].filename, line );
		/* ���󥯥롼�ɥե�������� */
		} else if( lptr[line].status == _INCLUDE ) {
			getPitchEnv( lptr[line].inc_ptr );
		}
	}
}

/*--------------------------------------------------------------
	�ԥå��⥸��졼�����μ���
 Input:
	
 Output:
	̵��
--------------------------------------------------------------*/
void getPitchMod( LINE *lptr )
{
	int		line, i, no, end_flag, offset, num, cnt;
	char	*ptr;

	cnt = 0;

	for( line = 1; line <= lptr->line; line++ ) {
		/* �����ǡ���ȯ���� */
		if( lptr[line].status == _SET_PITCH_MOD ) {
			no = lptr[line].param;				/* LFO�ֹ���� */
			ptr = lptr[line].str;
			ptr++;								/* '{'��ʬ�����Ф� */
			if (pitch_mod_tbl[no][0] != 0) {
				dispWarning( THIS_NUMBER_IS_ALREADY_USED, lptr[line].filename, line );
			}
			pitch_mod_tbl[no][0] = 0;
			offset = 0;
			i = 1;
			end_flag = 0;
			while( end_flag == 0 ) {
				ptr = skipSpace( ptr );
				switch( *ptr ) {
				  case '}':
					if (pitch_mod_tbl[no][0] >= 3) {
						//OK.
					} else {
						dispError( PARAMETER_IS_LACKING, lptr[line].filename, line );
						pitch_mod_tbl[no][0] = 0;
					}
					end_flag = 1;
					line += offset;
					break;
				  case '\0':
					offset++;
					if( line+offset <= lptr->line ) {
						if( (lptr[line+offset].status&_SAME_LINE) == _SAME_LINE ) {
							ptr = lptr[line+offset].str;
						}
					} else {
						dispError( LFO_DEFINITION_IS_WRONG, lptr[line].filename, line );
						pitch_mod_tbl[no][0] = 0;
						end_flag = 1;
					}
					break;
				  default:
					num = Asc2Int( ptr, &cnt );
					if( cnt != 0 ) {
						switch( i ) {
						  case 1:
						  case 2:
						  case 3:
							if( 0 <= num && num <= 255 ) {
								pitch_mod_tbl[no][i] = num;
								pitch_mod_tbl[no][0]++;
								ptr += cnt;
								i++;
							} else {
								dispError( LFO_DEFINITION_IS_WRONG, lptr[line+offset].filename, line );
								pitch_mod_tbl[no][0] = 0;
								end_flag = 1;
							}
							break;
						  case 4:
							if( 0 <= num && num <= 255 ) {
								pitch_mod_tbl[no][i] = num;
								pitch_mod_tbl[no][0]++;
								ptr += cnt;
								i++;
							} else {
								dispError( LFO_DEFINITION_IS_WRONG, lptr[line+offset].filename, line );
								pitch_mod_tbl[no][0] = 0;
								end_flag = 1;
							}
							break;
						  default:
							dispError( LFO_DEFINITION_IS_WRONG, lptr[line+offset].filename, line );
							pitch_mod_tbl[no][0] = 0;
							end_flag = 1;
							break;
						}
					} else {
						dispError( LFO_DEFINITION_IS_WRONG, lptr[line+offset].filename, line );
						pitch_mod_tbl[no][0] = 0;
						end_flag = 1;
					}
					break;
				}
				ptr = skipSpace( ptr );
				if( *ptr == ',' ) {
					ptr++;
				}
			}
		/* �������������_SAME_LINE�λ��ϥ��顼 */
		} else if( lptr[line].status == (_SET_PITCH_MOD|_SAME_LINE) ) {
			dispError( LFO_DEFINITION_IS_WRONG, lptr[line].filename, line );
		/* ���󥯥롼�ɥե�������� */
		} else if( lptr[line].status == _INCLUDE ) {
			getPitchMod( lptr[line].inc_ptr );
		}
	}
}



/*--------------------------------------------------------------
	�Ρ��ȥ���٥��פμ���
 Input:
	
 Output:
	̵��
--------------------------------------------------------------*/
void getArpeggio( LINE *lptr )
{
	int		line, i, no, end_flag, offset, num, cnt;
	char	*ptr;

	cnt = 0;

	for( line = 1; line <= lptr->line; line++ ) {
		/* ����ڥ����ǡ���ȯ���� */
		if( lptr[line].status == _SET_ARPEGGIO ) {
			no = lptr[line].param;				/* ����٥����ֹ���� */
			ptr = lptr[line].str;
			ptr++;								/* '{'��ʬ�����Ф� */
			if (arpeggio_tbl[no][0] != 0) {
				dispWarning( THIS_NUMBER_IS_ALREADY_USED, lptr[line].filename, line );
			}
			arpeggio_tbl[no][0] = 0;
			offset = 0;
			i = 1;
			end_flag = 0;
			while( end_flag == 0 ) {
				ptr = skipSpace( ptr );
				switch( *ptr ) {
				  case '}':
					if (arpeggio_tbl[no][0] >= 1) {
						arpeggio_tbl[no][i] = EFTBL_END;
						arpeggio_tbl[no][0]++;
					} else {
						dispError( PARAMETER_IS_LACKING, lptr[line].filename, line );
						arpeggio_tbl[no][0] = 0;
					}
					end_flag = 1;
					line += offset;
					break;
				  case '|':
					arpeggio_tbl[no][i] = EFTBL_LOOP;
					arpeggio_tbl[no][0]++;
					i++;
					ptr++;
					break;
				  case '\0':
					offset++;
					if( line+offset <= lptr->line ) {
						if( (lptr[line+offset].status&_SAME_LINE) == _SAME_LINE ) {
							ptr = lptr[line+offset].str;
						}
					} else {
						dispError( NOTE_ENVELOPE_DEFINITION_IS_WRONG, lptr[line].filename, line );
						arpeggio_tbl[no][0] = 0;
						end_flag = 1;
					}
					break;
				  default:
					num = Asc2Int( ptr, &cnt );
					if( cnt != 0 ) {
						if( num >= 0 ) {
							arpeggio_tbl[no][i] = num;
						} else {
							arpeggio_tbl[no][i] = (-num)|0x80;
						}	
						arpeggio_tbl[no][0]++;
						ptr += cnt;
						i++;
					} else {
						dispError( NOTE_ENVELOPE_DEFINITION_IS_WRONG, lptr[line+offset].filename, line );
						arpeggio_tbl[no][0] = 0;
						end_flag = 1;
					}
					break;
				}
				ptr = skipSpace( ptr );
				if( *ptr == ',' ) {
					ptr++;
				}
			}
		/* ����ڥ������������_SAME_LINE�λ��ϥ��顼 */
		} else if( lptr[line].status == (_SET_ARPEGGIO|_SAME_LINE) ) {
			dispError( NOTE_ENVELOPE_DEFINITION_IS_WRONG, lptr[line].filename, line );
		/* ���󥯥롼�ɥե�������� */
		} else if( lptr[line].status == _INCLUDE ) {
			getArpeggio( lptr[line].inc_ptr );
		}
	}
}



/*--------------------------------------------------------------
	DPCM�μ���
 Input:
	
 Output:
	̵��
--------------------------------------------------------------*/
void getDPCM( LINE *lptr )
{
	int		line, i, no, offset, end_flag, num, cnt;
	char	*ptr;
	FILE	*fp;
	DPCMTBL	*tbl;

	cnt = 0;

	for( line = 1; line <= lptr->line; line++ ) {
		// DPCM�ǡ���ȯ����
		if( lptr[line].status == _SET_DPCM_DATA ) {
			no = lptr[line].param;				// DPCM�ֹ����
			ptr = lptr[line].str;
			ptr++;								// '{'��ʬ�����Ф�
			tbl = &dpcm_tbl[no];
			if (tbl->flag != 0) {
				dispWarning( THIS_NUMBER_IS_ALREADY_USED, lptr[line].filename, line );
			}
			tbl->flag = 1;						// �ե饰��������
			tbl->index = -1;
			tbl->fname = NULL;
			tbl->freq = 0;
			tbl->size = 0;
			tbl->delta_init = 0;
			offset = 0;
			i = 0;
			end_flag = 0;
			while( end_flag == 0 ) {
				ptr = skipSpace( ptr );
				switch( *ptr ) {
				// �ǡ�����λ
				  case '}':
					switch( i ) {
					  case 0:
					  case 1:
						dispError( DPCM_PARAMETER_IS_LACKING, lptr[line].filename, line );
						tbl->flag = 0;
						break;
					  default:
						line += offset;
						break;
					}
					end_flag = 1;
					break;
				// ����
				  case '\0':
					offset++;
					if( line+offset <= lptr->line ) {
						if( (lptr[line+offset].status&_SAME_LINE) == _SAME_LINE ) {
							ptr = lptr[line+offset].str;
						}
					} else {
						dispError( DPCM_DEFINITION_IS_WRONG, lptr[line].filename, line );
						tbl->flag = 0;
						end_flag = 1;
					}
					break;
				  default:
					switch( i ) {
					// �ե�����̾����Ͽ
					  case 0:
						// �ե�����̾��"..."�ǰϤޤ�Ƥ��롩
						if( *ptr == '\"' ) {
							ptr++;
							//ptr = skipSpace( ptr );
							//"file.dmc"��OK. " file.dmc"��NG.
							tbl->fname = ptr;
							while( *ptr != '\"' && *ptr != '\0' ) {
								ptr++;
							}
						} else {
							tbl->fname = ptr;
							//���򤬤���Ȥ���ޤǤϥե�����̾
							// '/'';'�ϤȤФ��ʤ�
							while( *ptr != ' ' && *ptr != '\t' && *ptr != '\0' ) {
								ptr++;
							}
						}
						*ptr = '\0';
						ptr++;
						// �ե�����¸�ߥ����å�/�����������å�
						if( (fp = openDmc( tbl->fname )) == NULL ) {
							dispError( DPCM_FILE_NOT_FOUND, lptr[line+offset].filename, line );
							tbl->flag = 0;
							end_flag = 1;
						} else {
							fseek( fp, 0, SEEK_END );
							tbl->size = ftell( fp );
							fseek( fp, 0, SEEK_SET );
							fclose( fp );
						}
						i++;
						break;
					// �������ȿ�����Ͽ
					  case 1:
						num = Asc2Int( ptr, &cnt );
						if( cnt != 0 && (0 <= num && num <= 15) ) {
								tbl->freq = num;
						} else {
							dispError( DPCM_DEFINITION_IS_WRONG, lptr[line+offset].filename, line );
							tbl->flag = 0;
							end_flag = 1;
						}
						ptr += cnt;
						i++;
						break;
					// ��������������Ͽ
					  case 2:
						num = Asc2Int( ptr, &cnt );
						if (cnt != 0 && num == 0) {
							//�ͤ�0�ΤȤ��Ͼ�ά��Ʊ��
							ptr += cnt;
							i++;
							break;
						}
						if( cnt != 0 && (0 < num && num < 16384) ) {
							tbl->size = num;
						} else {
							dispError( DPCM_DEFINITION_IS_WRONG, lptr[line+offset].filename, line );
							tbl->flag = 0;
							end_flag = 1;
						}
						ptr += cnt;
						i++;
						break;
					// �ǥ륿������($4011)����ͤ���Ͽ
					  case 3:
						num = Asc2Int(ptr, &cnt);
						if (cnt != 0 && ((0 <= num && num <= 0x7f) || num == 0xff)) {
							tbl->delta_init = num;
						} else {
							dispError( DPCM_DEFINITION_IS_WRONG, lptr[line+offset].filename, line );
							tbl->flag = 0;
							end_flag = 1;
						}
						ptr += cnt;
						i++;
						break;
					// �롼�׾���($4010��bit7,6)����Ͽ
					  case 4:
						num = Asc2Int(ptr, &cnt);
						if (cnt != 0 && (0 <= num && num <= 2)) {
							tbl->freq |= (num<<6);
						} else {
							dispError( DPCM_DEFINITION_IS_WRONG, lptr[line+offset].filename, line );
							tbl->flag = 0;
							end_flag = 1;
						}
						ptr += cnt;
						i++;
						break;
					  default:
						dispError( DPCM_DEFINITION_IS_WRONG, lptr[line+offset].filename, line );
						tbl->flag = 0;
						end_flag = 1;
						break;
					}
					break;
				}
				ptr = skipSpace( ptr );
				if( *ptr == ',' ) {
					ptr++;
				}
			}
			if( tbl->size > (0xff)*16+1 ) {
				dispError( DPCM_FILE_SIZE_OVER, lptr[line+offset].filename, line );
				tbl->flag = 0;
			} else if ((tbl->size % 16) != 1) {
				dispWarning( DPCM_FILE_SIZE_ERROR, lptr[line+offset].filename, line );
			}
		// DPCM���������_SAME_LINE�λ��ϥ��顼
		} else if( lptr[line].status == (_SET_DPCM_DATA|_SAME_LINE) ) {
			dispError( DPCM_DEFINITION_IS_WRONG, lptr[line].filename, line );
		// ���󥯥롼�ɥե��������
		} else if( lptr[line].status == _INCLUDE ) {
			getDPCM( lptr[line].inc_ptr );
		}
	}
}



/*--------------------------------------------------------------
	FDS FM�����μ���
 Input:
	
 Output:
	̵��
--------------------------------------------------------------*/
void getFMTone( LINE *lptr )
{
	int		line, i, no, end_flag, offset, num, cnt;
	char	*ptr;

	cnt = 0;

	for( line = 1; line <= lptr->line; line++ ) {
		/* �����ǡ���ȯ���� */
		if( lptr[line].status == _SET_FM_TONE ) {
			no = lptr[line].param;				/* �����ֹ���� */
			ptr = lptr[line].str;
			ptr++;								/* '{'��ʬ�����Ф� */
			if (fm_tone_tbl[no][0] != 0) {
				dispWarning( THIS_NUMBER_IS_ALREADY_USED, lptr[line].filename, line );
			}
			fm_tone_tbl[no][0] = 0;
			offset = 0;
			i = 1;
			end_flag = 0;
			while( end_flag == 0 ) {
				ptr = skipSpace( ptr );
				switch( *ptr ) {
				  case '}':
					if (fm_tone_tbl[no][0] == 64) {
						//OK.
					} else {
						dispError( PARAMETER_IS_LACKING, lptr[line].filename, line );
						fm_tone_tbl[no][0] = 0;
					}
					end_flag = 1;
					line += offset;
					break;
				  case '\0':
					offset++;
					if( line+offset <= lptr->line ) {
						if( (lptr[line+offset].status&_SAME_LINE) == _SAME_LINE ) {
							ptr = lptr[line+offset].str;
						}
					} else {
						dispError( FM_TONE_DEFINITION_IS_WRONG, lptr[line].filename, line+offset );
						fm_tone_tbl[no][0] = 0;
						line += offset;
						end_flag = 1;
					}
					break;
				  default:
					num = Asc2Int( ptr, &cnt );
					if( cnt != 0 && (0 <= num && num <= 0x3f) ) {
						fm_tone_tbl[no][i] = num;
						fm_tone_tbl[no][0]++;
						ptr += cnt;
						i++;
						if( i > 65 ) {
							dispError( ABNORMAL_PARAMETERS_OF_FM_TONE, lptr[line+offset].filename, line+offset );
							fm_tone_tbl[no][0] = 0;
							line += offset;
							end_flag = 1;
						}
					} else {
						dispError( FM_TONE_DEFINITION_IS_WRONG, lptr[line+offset].filename, line+offset );
						fm_tone_tbl[no][0] = 0;
						line += offset;
						end_flag = 1;
					}
					break;
				}
				ptr = skipSpace( ptr );
				if( *ptr == ',' ) {
					ptr++;
				}
			}
		/* �������������_SAME_LINE�λ��ϥ��顼 */
		} else if( lptr[line].status == (_SET_FM_TONE|_SAME_LINE) ) {
			dispError( FM_TONE_DEFINITION_IS_WRONG, lptr[line].filename, line );
		/* ���󥯥롼�ɥե�������� */
		} else if( lptr[line].status == _INCLUDE ) {
			getFMTone( lptr[line].inc_ptr );
		}
	}
}


/*--------------------------------------------------------------
	VRC7�����μ���
 Input:
	
 Output:
	̵��
--------------------------------------------------------------*/
void getVRC7Tone( LINE *lptr )
{
	int		line, i, no, end_flag, offset, num, cnt;
	char	*ptr;

	cnt = 0;

	for( line = 1; line <= lptr->line; line++ ) {
		/* �����ǡ���ȯ���� */
		if( lptr[line].status == _SET_VRC7_TONE ) {
			no = lptr[line].param;				/* �����ֹ���� */
			ptr = lptr[line].str;
			ptr++;								/* '{'��ʬ�����Ф� */
			if (vrc7_tone_tbl[no][0] != 0) {
				dispWarning( THIS_NUMBER_IS_ALREADY_USED, lptr[line].filename, line );
			}
			vrc7_tone_tbl[no][0] = 0;
			offset = 0;
			i = 1;
			end_flag = 0;
			while( end_flag == 0 ) {
				ptr = skipSpace( ptr );
				switch( *ptr ) {
				  case '}':
					if (vrc7_tone_tbl[no][0] == 8) {
						//OK.
					} else {
						dispError( PARAMETER_IS_LACKING, lptr[line].filename, line );
						vrc7_tone_tbl[no][0] = 0;
					}
					end_flag = 1;
					line += offset;
					break;
				  case '\0':
					offset++;
					if( line+offset <= lptr->line ) {
						if( (lptr[line+offset].status&_SAME_LINE) == _SAME_LINE ) {
							ptr = lptr[line+offset].str;
						}
					} else {
						dispError( FM_TONE_DEFINITION_IS_WRONG, lptr[line].filename, line+offset );
						vrc7_tone_tbl[no][0] = 0;
						line += offset;
						end_flag = 1;
					}
					break;
				  default:
					num = Asc2Int( ptr, &cnt );
					if( cnt != 0 && (0 <= num && num <= 0xff) ) {
						vrc7_tone_tbl[no][i] = num;
						vrc7_tone_tbl[no][0]++;
						ptr += cnt;
						i++;
						if( i > 9 ) {
							dispError( ABNORMAL_PARAMETERS_OF_FM_TONE, lptr[line+offset].filename, line+offset );
							vrc7_tone_tbl[no][0] = 0;
							line += offset;
							end_flag = 1;
						}
					} else {
						dispError( FM_TONE_DEFINITION_IS_WRONG, lptr[line+offset].filename, line+offset );
						vrc7_tone_tbl[no][0] = 0;
						line += offset;
						end_flag = 1;
					}
					break;
				}
				ptr = skipSpace( ptr );
				if( *ptr == ',' ) {
					ptr++;
				}
			}
			if( i != 9 ) {
				if (!error_flag) {
					dispError( ABNORMAL_PARAMETERS_OF_FM_TONE, lptr[line].filename, line);
					vrc7_tone_tbl[no][0] = 0;
				}
			}


		/* �������������_SAME_LINE�λ��ϥ��顼 */
		} else if( lptr[line].status == (_SET_VRC7_TONE|_SAME_LINE) ) {
			dispError( FM_TONE_DEFINITION_IS_WRONG, lptr[line].filename, line );
		/* ���󥯥롼�ɥե�������� */
		} else if( lptr[line].status == _INCLUDE ) {
			getVRC7Tone( lptr[line].inc_ptr );
		}
	}
}



/*--------------------------------------------------------------
	namco106�����μ���
 Input:
	
 Output:
	̵��
--------------------------------------------------------------*/
void getN106Tone( LINE *lptr )
{
	int		line, i, no, end_flag, offset, num, cnt;
	char	*ptr;
					//	   16 14 12 10  8  6  4  2 			
	int	n106_tone_max[] = { 4, 4, 5, 6, 8,10,16,32 }; 
	int	n106_tone_num;

	cnt = 0;
	for( line = 1; line <= lptr->line; line++ ) {
		/* �����ǡ���ȯ���� */
		if( lptr[line].status == _SET_N106_TONE ) {
			no = lptr[line].param;				/* �����ֹ���� */
			ptr = lptr[line].str;
			ptr++;								/* '{'��ʬ�����Ф� */
			if (n106_tone_tbl[no][0] != 0) {
				dispWarning( THIS_NUMBER_IS_ALREADY_USED, lptr[line].filename, line );
			}
			n106_tone_tbl[no][0] = 0;
			offset = 0;
			i = 1;
			end_flag = 0;
			while( end_flag == 0 ) {
				ptr = skipSpace( ptr );
				switch( *ptr ) {
				  case '}':
					//���Ǥο���while��ȴ������ǥ����å�
					end_flag = 1;
					line += offset;
					break;
				  case '\0':
					offset++;
					if( line+offset <= lptr->line ) {
						if( (lptr[line+offset].status&_SAME_LINE) == _SAME_LINE ) {
							ptr = lptr[line+offset].str;
						}
					} else {
						dispError( N106_TONE_DEFINITION_IS_WRONG, lptr[line].filename, line+offset );
						n106_tone_tbl[no][0] = 0;
						line += offset;
						end_flag = 1;
					}
					break;
				  default:
					num = Asc2Int( ptr, &cnt );
					if( i == 1 ) {						// ��Ͽ�Хåե�(0��5)			
						if( cnt != 0 && (0 <= num && num <= 32) ) {
							n106_tone_tbl[no][1] = num;
							n106_tone_tbl[no][0]++;
							ptr += cnt;
							i++;
						} else {
							dispError( N106_TONE_DEFINITION_IS_WRONG, lptr[line].filename, line+offset );
							n106_tone_tbl[no][0] = 0;
							line += offset;
							end_flag = 1;
						}
					} else {
						if( cnt != 0 && (0 <= num && num <= 15) ) {
							n106_tone_tbl[no][i] = num;
							n106_tone_tbl[no][0]++;
							ptr += cnt;
							i++;
							if( i > 2+32 ) {
								dispError( ABNORMAL_PARAMETERS_OF_N106_TONE, lptr[line+offset].filename, line+offset );
								n106_tone_tbl[no][0] = 0;
								line += offset;
								end_flag = 1;
							}
						} else {
							dispError( N106_TONE_DEFINITION_IS_WRONG, lptr[line+offset].filename, line+offset );
							n106_tone_tbl[no][0] = 0;
							line += offset;
							end_flag = 1;
						}
					}
					break;
				}
				ptr = skipSpace( ptr );
				if( *ptr == ',' ) {
					ptr++;
				}
			}
			switch( n106_tone_tbl[no][0] ) {
			  case 16*2+1: n106_tone_num =  0; break;
			  case 14*2+1: n106_tone_num =  1; break;
			  case 12*2+1: n106_tone_num =  2; break;
			  case 10*2+1: n106_tone_num =  3; break;
			  case  8*2+1: n106_tone_num =  4; break;
			  case  6*2+1: n106_tone_num =  5; break;
			  case  4*2+1: n106_tone_num =  6; break;
			  case  2*2+1: n106_tone_num =  7; break;
			  default:     n106_tone_num = -1; break;
			}
			if( n106_tone_num == -1 ) {
				dispError( ABNORMAL_PARAMETERS_OF_N106_TONE, lptr[line].filename, line );
				n106_tone_tbl[no][0] = 0;
			}
			if( n106_tone_tbl[no][1] >= n106_tone_max[n106_tone_num] ) {
				dispError( N106_TONE_DEFINITION_IS_WRONG, lptr[line].filename, line );
				n106_tone_tbl[no][0] = 0;
			}
		/* �������������_SAME_LINE�λ��ϥ��顼 */
		} else if( lptr[line].status == (_SET_N106_TONE|_SAME_LINE) ) {
			dispError( N106_TONE_DEFINITION_IS_WRONG, lptr[line].filename, line );
		/* ���󥯥롼�ɥե�������� */
		} else if( lptr[line].status == _INCLUDE ) {
			getN106Tone( lptr[line].inc_ptr );
		}
	}
}



/*--------------------------------------------------------------
	�ϡ��ɥ��������ե����Ȥμ���
 Input:
	
 Output:
	̵��
--------------------------------------------------------------*/
void getHardEffect( LINE *lptr )
{
	int		line, i, no, end_flag, offset, num, cnt;
	char	*ptr;

	cnt = 0;

	for( line = 1; line <= lptr->line; line++ ) {
		/* �����ǡ���ȯ���� */
		if( lptr[line].status == _SET_HARD_EFFECT ) {
			no = lptr[line].param;				/* ���ե������ֹ���� */
			ptr = lptr[line].str;
			ptr++;								/* '{'��ʬ�����Ф� */
			if (hard_effect_tbl[no][0] != 0) {
				dispWarning( THIS_NUMBER_IS_ALREADY_USED, lptr[line].filename, line );
			}
			hard_effect_tbl[no][0] = 0;
			offset = 0;
			i = 1;
			end_flag = 0;
			while( end_flag == 0 ) {
				ptr = skipSpace( ptr );
				switch( *ptr ) {
				  case '}':
					if (hard_effect_tbl[no][0] == 4) {
						//OK.
					} else {
						dispError( PARAMETER_IS_LACKING, lptr[line].filename, line );
						hard_effect_tbl[no][0] = 0;
					}
					end_flag = 1;
					line += offset;
					break;
				  case '\0':
					offset++;
					if( line+offset <= lptr->line ) {
						if( (lptr[line+offset].status&_SAME_LINE) == _SAME_LINE ) {
							ptr = lptr[line+offset].str;
						}
					} else {
						dispError( LFO_DEFINITION_IS_WRONG, lptr[line].filename, line );
						hard_effect_tbl[no][0] = 0;
						end_flag = 1;
					}
					break;
				  default:
					num = Asc2Int( ptr, &cnt );
					if( cnt != 0 ) {
						switch( i ) {
						  case 1:
							if( 0 <= num && num <= 255 ) {
								hard_effect_tbl[no][i] = num;
								hard_effect_tbl[no][0]++;
								ptr += cnt;
								i++;
							} else {
								dispError( LFO_DEFINITION_IS_WRONG, lptr[line+offset].filename, line );
								hard_effect_tbl[no][0] = 0;
								end_flag = 1;
							}
							break;
						  case 2:
							if( 0 <= num && num <= 4095 ) {
								hard_effect_tbl[no][i] = num;
								hard_effect_tbl[no][0]++;
								ptr += cnt;
								i++;
							} else {
								dispError( LFO_DEFINITION_IS_WRONG, lptr[line+offset].filename, line );
								hard_effect_tbl[no][0] = 0;
								end_flag = 1;
							}
							break;
						  case 3:
							if( 0 <= num && num <= 255 ) {
								hard_effect_tbl[no][i] = num;
								hard_effect_tbl[no][0]++;
								ptr += cnt;
								i++;
							} else {
								dispError( LFO_DEFINITION_IS_WRONG, lptr[line+offset].filename, line );
								hard_effect_tbl[no][0] = 0;
								end_flag = 1;
							}
							break;
						  case 4:
							if( 0 <= num && num <= 7 ) {
								hard_effect_tbl[no][i] = num;
								hard_effect_tbl[no][0]++;
								ptr += cnt;
								i++;
							} else {
								dispError( LFO_DEFINITION_IS_WRONG, lptr[line+offset].filename, line );
								hard_effect_tbl[no][0] = 0;
								end_flag = 1;
							}
							break;
						  default:
							dispError( LFO_DEFINITION_IS_WRONG, lptr[line+offset].filename, line );
							hard_effect_tbl[no][0] = 0;
							end_flag = 1;
							break;
						}
					} else {
						dispError( LFO_DEFINITION_IS_WRONG, lptr[line+offset].filename, line );
						hard_effect_tbl[no][0] = 0;
						end_flag = 1;
					}
					break;
				}
				ptr = skipSpace( ptr );
				if( *ptr == ',' ) {
					ptr++;
				}
			}
		/* �������������_SAME_LINE�λ��ϥ��顼 */
		} else if( lptr[line].status == (_SET_HARD_EFFECT|_SAME_LINE) ) {
			dispError( LFO_DEFINITION_IS_WRONG, lptr[line].filename, line );
		/* ���󥯥롼�ɥե�������� */
		} else if( lptr[line].status == _INCLUDE ) {
			getHardEffect( lptr[line].inc_ptr );
		}
	}
}



/*--------------------------------------------------------------
	���ե������ȷ��μ���
 Input:
	
 Output:
	̵��
--------------------------------------------------------------*/
void getEffectWave( LINE *lptr )
{
	int		line, i, no, end_flag, offset, num, cnt;
	char	*ptr;

	cnt = 0;

	for( line = 1; line <= lptr->line; line++ ) {
		/* �����ǡ���ȯ���� */
		if( lptr[line].status == _SET_EFFECT_WAVE ) {
			no = lptr[line].param;				/* �ȷ��ֹ���� */
			ptr = lptr[line].str;
			ptr++;								/* '{'��ʬ�����Ф� */
			if (effect_wave_tbl[no][0] != 0) {
				dispWarning( THIS_NUMBER_IS_ALREADY_USED, lptr[line].filename, line );
			}
			effect_wave_tbl[no][0] = 0;
			offset = 0;
			i = 1;
			end_flag = 0;
			while( end_flag == 0 ) {
				ptr = skipSpace( ptr );
				switch( *ptr ) {
				  case '}':
					if (effect_wave_tbl[no][0] == 32) {
						//OK.
					} else {
						dispError( PARAMETER_IS_LACKING, lptr[line].filename, line );
						effect_wave_tbl[no][0] = 0;
					}
					end_flag = 1;
					line += offset;
					break;
				  case '\0':
					offset++;
					if( line+offset <= lptr->line ) {
						if( (lptr[line+offset].status&_SAME_LINE) == _SAME_LINE ) {
							ptr = lptr[line+offset].str;
						}
					} else {
						dispError( EFFECT_WAVE_DEFINITION_IS_WRONG, lptr[line].filename, line+offset );
						effect_wave_tbl[no][0] = 0;
						line += offset;
						end_flag = 1;
					}
					break;
				  default:
					num = Asc2Int( ptr, &cnt );
					if( cnt != 0 && (0 <= num && num <= 7) ) {
						effect_wave_tbl[no][i] = num;
						effect_wave_tbl[no][0]++;
						ptr += cnt;
						i++;
						if( i > 33 ) {
							dispError( EFFECT_WAVE_DEFINITION_IS_WRONG, lptr[line+offset].filename, line+offset );
							effect_wave_tbl[no][0] = 0;
							line += offset;
							end_flag = 1;
						}
					} else {
						dispError( EFFECT_WAVE_DEFINITION_IS_WRONG, lptr[line+offset].filename, line+offset );
						effect_wave_tbl[no][0] = 0;
						line += offset;
						end_flag = 1;
					}
					break;
				}
				ptr = skipSpace( ptr );
				if( *ptr == ',' ) {
					ptr++;
				}
			}
		/* �������������_SAME_LINE�λ��ϥ��顼 */
		} else if( lptr[line].status == (_SET_EFFECT_WAVE|_SAME_LINE) ) {
			dispError( EFFECT_WAVE_DEFINITION_IS_WRONG, lptr[line].filename, line );
		/* ���󥯥롼�ɥե�������� */
		} else if( lptr[line].status == _INCLUDE ) {
			getEffectWave( lptr[line].inc_ptr );
		}
	}
}



/*--------------------------------------------------------------
	DPCM�ǡ����Υ��֤����
 Input:
	̵��
 Output:
	̵��
--------------------------------------------------------------*/
void sortDPCM( DPCMTBL dpcm_tbl[_DPCM_MAX] )
{
	int	i, j;

	for( i = 0; i < _DPCM_MAX; i++ ) {
		if( dpcm_tbl[i].flag == 0 || dpcm_tbl[i].index != -1 ) continue;
		for( j = 0; j < _DPCM_MAX; j++ ) {
			if( i == j ) continue;
			if( dpcm_tbl[j].flag == 0 ) continue;
			// �ե�����̾��Ʊ����
			if( strcmp( dpcm_tbl[i].fname, dpcm_tbl[j].fname ) == 0
			 && dpcm_tbl[i].size >= dpcm_tbl[j].size ) {
				dpcm_tbl[j].index = i;
			}
		}
	}
}



/*--------------------------------------------------------------
	DPCM����������(16byte�Х������˽���)
 Input:
	
 Output:
 	int: DPCM������
--------------------------------------------------------------*/
int checkDPCMSize( DPCMTBL dpcm_tbl[_DPCM_MAX] )
{
	int	i;
	int	adr = 0;
	int	size = 0;
	int	bank = 0; //0x4000���Ȥ�����
	for( i = 0; i < _DPCM_MAX; i++ ) {
		if( dpcm_tbl[i].flag != 0 ) {
			/*
			   $4013 * 16 + 1 = size
			   $4013 = (size - 1) / 16 
			
			   newsize % 16 == 1����Ω����褦��Ĵ��
			   size%16	(size-1)%16	diff(floor)	diff(ceil)
			   1		0		0		0
			   2		1		-1		+15
			   3		2		-2		+14
			   4		3		-3		+13
			   15		14		-14		+2
			   0		15		-15		+1
			*/
			//printf("%s size $%x\n", dpcm_tbl[i].fname, dpcm_tbl[i].size);
			if ((dpcm_tbl[i].size % 16) != 1) {
				int diff;
				diff = (16 - ((dpcm_tbl[i].size - 1) % 16)) % 16; //ceil
				//diff =    - ((dpcm_tbl[i].size - 1) % 16); //floor
				dpcm_tbl[i].size += diff;
			}
			//printf("%s fixed size $%x\n", dpcm_tbl[i].fname, dpcm_tbl[i].size);
			// �������ȥ��ɥ쥹������
			if( dpcm_tbl[i].index == -1 ) {
				if ( ((adr%0x4000 + dpcm_tbl[i].size) > 0x4000) || (adr % 0x4000 == 0 && adr != 0) ) {
					/* 16KB������ޤ������硦�ޤ�������Υ��ɥ쥹�ڤ�夲�ǿ�����16KB�ΰ�˾�ä���� */
					adr += (0x4000 - (adr % 0x4000)) % 0x4000;
					bank++;
					dpcm_bankswitch = 1;
				}
				//printf("%s bank %d a %x s %x\n", dpcm_tbl[i].fname, bank, adr, size);
				
				dpcm_tbl[i].start_adr = adr;
				dpcm_tbl[i].bank_ofs = bank;
				adr += dpcm_tbl[i].size;
				size = adr;
				// adr % 64 == 0����Ω����褦���ڤ�夲
				adr += (64 - (adr % 64)) % 64;
			}
		}
	}
	return size;
}





/*--------------------------------------------------------------
	DPCM�ǡ����ɤ߹���
 Input:
	
 Output:
--------------------------------------------------------------*/
void readDPCM( DPCMTBL dpcm_tbl[_DPCM_MAX] )
{
	int	i;
	FILE	*fp;

	for( i = 0; i < dpcm_size; i++ ) {
		dpcm_data[i] = 0xaa; 
	}

	for( i = 0; i < _DPCM_MAX; i++ ) {
		if( dpcm_tbl[i].flag != 0 && dpcm_tbl[i].index == -1 ) {
			fp = openDmc( dpcm_tbl[i].fname );
			if( fp == NULL ) {
//				disperror( DPCM_FILE_NOT_FOUND, 0 );
			} else {
				fread( &dpcm_data[dpcm_tbl[i].start_adr], 1, dpcm_tbl[i].size, fp );
				fclose( fp );
			}
		}
	}
#if DEBUG
	for( i = 0; i < _DPCM_TOTAL_SIZE; i++ ) {
		if( (i&0x0f) != 0x0f ) {
			printf( "%02x,", dpcm_data[i] );
		} else {
			printf( "%02x\n", dpcm_data[i] );
		}
	}
#endif
}



/*--------------------------------------------------------------
	����/����٥��פΥ롼�ץ����å�
 Input:
	
 Output:
	int	: �����礭�������ֹ�
--------------------------------------------------------------*/
int checkLoop( int ptr[128][1024], int max )
{
	int		i, j, lp_flag, ret;

	ret = 0;

	for( i = 0; i < max; i++ ) {
		if( ptr[i][0] != 0 ) {
			lp_flag = 0;
			for( j = 1; j <= ptr[i][0]; j++ ) {
				if( ptr[i][j] == EFTBL_LOOP ) lp_flag = 1;
			}
			if( lp_flag == 0 ) {
				j = ptr[i][0];
				ptr[i][j+1] = ptr[i][j  ]; 
				ptr[i][j  ] = ptr[i][j-1]; 
				ptr[i][j-1] = EFTBL_LOOP;
				ptr[i][0]++;
			}
			ret = i+1;
		}
	}
	return ret;
}



/*--------------------------------------------------------------
	�����λ��ѸĿ����֤�
 Input:
	
 Output:
	int	: �����礭�������ֹ�
--------------------------------------------------------------*/
int getMaxTone( int ptr[128][66], int max )
{
	int		i, ret;

	ret = 0;

	for( i = 0; i < max; i++ ) {
		if( ptr[i][0] != 0 ) {
			ret = i+1;
		}
	}
	return ret;
}



/*--------------------------------------------------------------
	LFO�λ��ѸĿ����֤�
 Input:
	
 Output:
	int	: �����礭��LFO�ֹ�
--------------------------------------------------------------*/
int getMaxLFO( int ptr[_PITCH_MOD_MAX][5], int max )
{
	int		i, ret;

	ret = 0;
	for( i = 0; i < max; i++ ) {
		if( ptr[i][0] != 0 ) {
			ret = i+1;
		}
	}
	return ret;
}



/*--------------------------------------------------------------
	DPCM�λ��ѸĿ����֤�
 Input:
	
 Output:
	int	: �����礭�������ֹ�
--------------------------------------------------------------*/
int getMaxDPCM( DPCMTBL	dpcm_tbl[_DPCM_MAX] )
{
	int		i, ret = 0;

	for( i = 0; i < _DPCM_MAX; i++ ) {
		if( dpcm_tbl[i].flag != 0 ) {
			ret = i+1;
		}
	}
	return ret;
}



/*--------------------------------------------------------------
	�ϡ��ɥ��������ե����Ȥλ��ѸĿ����֤�
 Input:
	
 Output:
	int	: �����礭�������ֹ�
--------------------------------------------------------------*/
int getMaxHardEffect( int ptr[_HARD_EFFECT_MAX][5], int max )
{
	int		i, ret;

	ret = 0;

	for( i = 0; i < max; i++ ) {
		if( ptr[i][0] != 0 ) {
			ret = i+1;
		}
	}
	return ret;
}



/*--------------------------------------------------------------
	���ե������ȷ��λ��ѸĿ����֤�
 Input:
	
 Output:
	int	: �����礭�������ֹ�
--------------------------------------------------------------*/
int getMaxEffectWave( int ptr[_EFFECT_WAVE_MAX][33], int max )
{
	int		i, ret;

	ret = 0;

	for( i = 0; i < max; i++ ) {
		if( ptr[i][0] != 0 ) {
			ret = i+1;
		}
	}
	return ret;
}



/*--------------------------------------------------------------
	����/����٥��פν񤭹���
 Input:
	
 Output:
	̵��
--------------------------------------------------------------*/
void writeTone( FILE *fp, int tbl[128][1024], char *str, int max )
{
	int		i, j, x;

	fprintf( fp, "%s_table:\n", str );
	if( max != 0 ) {
		for( i = 0; i < max; i++ ) {
			if( tbl[i][0] != 0 ) {
				fprintf( fp, "\tdw\t%s_%03d\n", str, i );
			} else {
				fprintf( fp, "\tdw\t0\n" );
			}			
		}
	}
	fprintf( fp, "%s_lp_table:\n", str );
	if( max != 0 ) {
		for( i = 0; i < max; i++ ) {
			if( tbl[i][0] != 0 ) {
				fprintf( fp, "\tdw\t%s_lp_%03d\n", str, i );
			} else {
				fprintf( fp, "\tdw\t0\n" );
			}			
		}

		for( i = 0; i < max; i++ ) {
			if( tbl[i][0] != 0 ) {
				fprintf( fp, "\n%s_%03d:\n", str, i );
				//fprintf( fp, ";# of param: %03d\n", tbl[i][0] );
				x = 0;
				for( j = 1; j <= tbl[i][0]; j++ ) {
					if( tbl[i][j] == EFTBL_LOOP ) {
						if( x != 0 ) fprintf( fp, "\n" );
						fprintf( fp, "%s_lp_%03d:\n", str, i );
						x = 0;
					} else if( x == 0 ) {
						fprintf( fp, "\tdb\t$%02x", tbl[i][j]&0xff );
						x++;
					} else if( x == 7 ) {
						fprintf( fp, ",$%02x\n", tbl[i][j]&0xff );
						x = 0;
					} else {
						fprintf( fp, ",$%02x", tbl[i][j]&0xff );
						x++;
					}
				}
			}
		}
	}
	fprintf( fp, "\n\n" );
}



/*--------------------------------------------------------------
	FM�����ν񤭹���
 Input:
	
 Output:
	̵��
--------------------------------------------------------------*/
void writeToneFM( FILE *fp, int tbl[_FM_TONE_MAX][66], char *str, int max )
{
	int		i, j, x;

	fprintf( fp, "%s_data_table:\n", str );
	if( max != 0 ) {
		for( i = 0; i < max; i++ ) {
			if( tbl[i][0] != 0 ) {
				fprintf( fp, "\tdw\t%s_%03d\n", str, i );
			} else {
				fprintf( fp, "\tdw\t0\n" );
			}
		}

		for( i = 0; i < max; i++ ) {
			if( tbl[i][0] != 0 ) {
				fprintf( fp, "\n%s_%03d:\n", str, i );
				x = 0;
				for( j = 1; j <= tbl[i][0]; j++ ) {				// tbl[i][0] = �ǡ�������(byte)
					if( x == 0 ) {
						fprintf( fp, "\tdb\t$%02x", tbl[i][j]&0xff );
						x++;
					} else if( x == 7 ) {
						fprintf( fp, ",$%02x\n", tbl[i][j]&0xff );
						x = 0;
					} else {
						fprintf( fp, ",$%02x", tbl[i][j]&0xff );
						x++;
					}
				}
			}
		}
	}
}

/*--------------------------------------------------------------
	VRC7�����ν񤭹���
 Input:
	
 Output:
	̵��
--------------------------------------------------------------*/
void writeToneVRC7( FILE *fp, int tbl[_VRC7_TONE_MAX][66], char *str, int max )
{
	int		i, j, x;

	fprintf( fp, "%s_data_table:\n", str );
	if( max != 0 ) {
		for( i = 0; i < max; i++ ) {
			if( tbl[i][0] != 0 ) {
				fprintf( fp, "\tdw\t%s_%03d\n", str, i );
			} else {
				fprintf( fp, "\tdw\t0\n" );
			}
		}

		for( i = 0; i < max; i++ ) {
			if( tbl[i][0] != 0 ) {
				fprintf( fp, "\n%s_%03d:\n", str, i );
				x = 0;
				for( j = 1; j <= tbl[i][0]; j++ ) {				// tbl[i][0] = �ǡ�������(byte)
					if( x == 0 ) {
						fprintf( fp, "\tdb\t$%02x", tbl[i][j]&0xff );
						x++;
					} else if( x == 7 ) {
						fprintf( fp, ",$%02x\n", tbl[i][j]&0xff );
						x = 0;
					} else {
						fprintf( fp, ",$%02x", tbl[i][j]&0xff );
						x++;
					}
				}
			}
		}
	}

	fprintf( fp, "\n\n" );
}




/*--------------------------------------------------------------
	�ϡ��ɥ��������ե����Ȥν񤭹���
 Input:
	
 Output:
	̵��
--------------------------------------------------------------*/
void writeHardEffect( FILE *fp, int tbl[_HARD_EFFECT_MAX][5], char *str, int max )
{
	int		i;

	fprintf( fp, "%s_effect_select:\n", str );
	for ( i = 0; i < max; i++ ) {
		fprintf( fp, "\tdb\t$%02x,$84,$%02x,$85,$00,$87,$80,$88\n",
			tbl[i][1], (tbl[i][3] | 0x80) );
		fprintf( fp, "\tdb\t$%02x,$86,$%02x,$87,$%02x,$ff,$00,$00\n",
			tbl[i][4], (tbl[i][2] & 0x00FF), ((tbl[i][2] & 0x0F00)>>8) );
	}
}

/*--------------------------------------------------------------
	���ե������ȷ��ν񤭹���
 Input:
	
 Output:
	̵��
--------------------------------------------------------------*/
void writeEffectWave( FILE *fp, int tbl[_EFFECT_WAVE_MAX][33], char *str, int max )
{
	int		i, j, x;

	fprintf( fp, "%s_4088_data:\n", str );
	for( i = 0; i < max; i++ ) {
		if( tbl[i][0] != 0 ) {
			x = 0;
			for( j = 1; j <= tbl[i][0]; j++ ) {				// tbl[i][0] = �ǡ�������(byte)
				if( x == 0 ) {
					fprintf( fp, "\tdb\t$%02x", tbl[i][j]&0xff );
					x++;
				} else if( x == 7 ) {
					fprintf( fp, ",$%02x\n", tbl[i][j]&0xff );
					x = 0;
				} else {
					fprintf( fp, ",$%02x", tbl[i][j]&0xff );
					x++;
				}
			}
		}
		else
		{
			/* ���ߡ��ǡ�������� */
			for (j = 0; j < 4; j++) {
				fprintf( fp, "\tdb\t$00,$00,$00,$00,$00,$00,$00,$00\n");
			}
		}
	}
	
	fprintf( fp, "\n\n" );
}



/*--------------------------------------------------------------
	N106�����ν񤭹���
 Input:
	
 Output:
	̵��
--------------------------------------------------------------*/
void writeToneN106( FILE *fp, int tbl[_N106_TONE_MAX][2+64], char *str, int max )
{
	int		i, j, x;

	// ���ѥ����ͥ�񤭹���
	fprintf( fp, "%s_channel:\n", str );
	fprintf( fp, "\tdb\t%d\n", n106_track_num );
	// �ѥ�᡼���񤭹���
	fprintf( fp, "%s_wave_init:\n", str );
	if( max != 0 ) {
		for( i = 0; i < max; i++ ) {
			switch( tbl[i][0] ) {
			  case  2*2+1: j = 7; x = tbl[i][1]* 2*2; break;
			  case  4*2+1: j = 6; x = tbl[i][1]* 4*2; break;
			  case  6*2+1: j = 5; x = tbl[i][1]* 6*2; break;
			  case  8*2+1: j = 4; x = tbl[i][1]* 8*2; break;
			  case 10*2+1: j = 3; x = tbl[i][1]*10*2; break;
			  case 12*2+1: j = 2; x = tbl[i][1]*12*2; break;
			  case 14*2+1: j = 1; x = tbl[i][1]*14*2; break;
			  case 16*2+1: j = 0; x = tbl[i][1]*16*2; break;
			  default:     j = 0; x = 0; break;	
			}
			fprintf( fp, "\tdb\t$%02x,$%02x\n", j, x );
		}
	}
	// �ѥ�᡼���񤭹���
	fprintf( fp, "%s_wave_table:\n", str );
	if( max != 0 ) {
		for( i = 0; i < max; i++ ) {
			if( tbl[i][0] != 0 ) {
				fprintf( fp, "\tdw\t%s_wave_%03d\n", str, i );
			} else {
				fprintf( fp, "\tdw\t0\n" );
			}
		}
	}			
	if( max != 0 ) {
		for( i = 0; i < max; i++ ) {
			if( tbl[i][0] != 0 ) {
				fprintf( fp, "%s_wave_%03d:\n", str, i );
				fprintf( fp, "\tdb\t" );
				for( j = 0; j < tbl[i][0]/2-1; j++ ) {
					fprintf( fp, "$%02x,", (tbl[i][2+(j*2)+1]<<4)+tbl[i][2+(j*2)+0] );
				}
				fprintf( fp, "$%02x\n", (tbl[i][2+(j*2)+1]<<4)+tbl[i][2+(j*2)+0] );
			}
		}
	}
	fprintf( fp, "\n\n" );
}



/*--------------------------------------------------------------
	DPCM�ơ��֥�ν񤭹���
 Input:
	
 Output:
	̵��
--------------------------------------------------------------*/
void writeDPCM( FILE *fp, DPCMTBL dpcm_tbl[_DPCM_MAX], char *str, int max )
{
	int		i;
	int		freq,adr,size,delta_init;
	char		*fname;

	fprintf( fp, "%s:\n", str );
	for( i = 0; i < max; i++ ) {
		if( dpcm_tbl[i].flag != 0 ) {
			/*
			   $4013 * 16 + 1 = size
			   $4013 = (size - 1) / 16
			   $4012 * 64 = adr
			   adr = $4012 / 64
			*/
			freq = dpcm_tbl[i].freq;
			size = (dpcm_tbl[i].size - 1)/16;
			delta_init = dpcm_tbl[i].delta_init;
			if( dpcm_tbl[i].index == -1 ) {
				adr  = dpcm_tbl[i].start_adr/64;
				fname = dpcm_tbl[i].fname;
			} else {
				adr  = dpcm_tbl[dpcm_tbl[i].index].start_adr/64;
				fname = dpcm_tbl[dpcm_tbl[i].index].fname;
			}
			fprintf( fp, "\tdb\t$%02x,$%02x,$%02x,$%02x\t;%s\n", freq, delta_init, adr % 0x100, size, fname);
		} else {
			fprintf( fp, "\tdb\t$00,$00,$00,$00\t;unused\n");
		}
	}
	
	if (dpcm_bankswitch) {
		fprintf( fp, "%s_bank:\n", str );
		for( i = 0; i < max; i++ ) {
			int bank_ofs = 0;
			if( dpcm_tbl[i].flag != 0 ) {
				if( dpcm_tbl[i].index == -1 ) {
					bank_ofs = dpcm_tbl[i].bank_ofs;
					fname = dpcm_tbl[i].fname;
				} else {
					bank_ofs = dpcm_tbl[dpcm_tbl[i].index].bank_ofs;
					fname = dpcm_tbl[dpcm_tbl[i].index].fname;
				}
				if (bank_ofs == 0) {
					fprintf( fp, "\tdb\t2*2\t;%s\n", fname);
				} else {
					bank_ofs -= 1;
					fprintf( fp, "\tdb\t(DPCM_EXTRA_BANK_START + %d*2)*2\t;%s\n", bank_ofs, fname);
				}
			} else {
				fprintf( fp, "\tdb\t0\t;unused\n");
			}
		}
	}

	fprintf( fp, "\n" );
}




/*--------------------------------------------------------------
	
--------------------------------------------------------------*/
static void writeDPCMSampleSub( FILE *fp )
{

	fprintf( fp, "\t.org\t$FFFA\n");
	fprintf( fp, "\t.dw\tDMC_NMI\n");
	fprintf( fp, "\t.dw\tDMC_RESET\n");
	fprintf( fp, "\t.dw\tDMC_IRQ\n");
}

/*--------------------------------------------------------------
	DPCM�ǡ����ν񤭹���
 Input:
	
 Output:
	̵��
--------------------------------------------------------------*/
void writeDPCMSample( FILE *fp )
{
	int		i;
	int	nes_bank = 1; //8KB
	int	bank_ofs = 0; //16KB
	
	fprintf( fp, "; begin DPCM samples\n" );
	for( i = 0; i < dpcm_size; i++ ) {
		if (i % 0x2000 == 0) {
			nes_bank++;
			if (nes_bank == 4) {
				nes_bank = 2;
				bank_ofs++;
			}
			if (bank_ofs == 0) {
				fprintf( fp, "\t.bank\t%1d\n", nes_bank );
				putBankOrigin(fp, nes_bank);
			} else {
				fprintf( fp, "\t.bank\tDPCM_EXTRA_BANK_START + %d*2 + %d - 2\n", bank_ofs-1, nes_bank );
				dpcm_extra_bank_num++;
				fprintf(fp, "\t.org\t$%04x\n", 0x8000 + 0x2000*nes_bank);
			}
		}
		if( (i&0x0f) == 0x00 ) {
			fprintf( fp, "\tdb\t$%02x", dpcm_data[i] );
		}else if( (i&0x0f) != 0x0f ) {
			fprintf( fp, ",$%02x", dpcm_data[i] );
		} else {
			fprintf( fp, ",$%02x\n", dpcm_data[i] );
		}
		if (bank_ofs == 0) {
			bank_usage[nes_bank]++;
		}
	}
	fprintf( fp, "\n" );
	fprintf( fp, "; end DPCM samples\n\n" );
	
	if (dpcm_extra_bank_num) {
		int x;
		fprintf( fp, "; begin DPCM vectors\n" );
		fprintf( fp, "\t.bank\t3\n");
		writeDPCMSampleSub(fp);
		for (x = 2; x <= dpcm_extra_bank_num; x += 2) {
			fprintf( fp, "\t.bank\tDPCM_EXTRA_BANK_START + %d\n", x - 1);
			writeDPCMSampleSub(fp);
		}
		fprintf( fp, "; end DPCM vectors\n" );
	}
	fprintf( fp, "\n" );
}



/*--------------------------------------------------------------
	�����ȥ�/��ʼ�/�᡼����/�Ǥ����߼Ԥ򥳥��ȤȤ��ƽ񤭹���
 Input:
	
 Output:
	̵��
--------------------------------------------------------------*/
void writeSongInfo( FILE *fp )
{
	fprintf( fp, "; Title: %s\n", song_name );
	fprintf( fp, "; Composer: %s\n", composer );
	fprintf( fp, "; Maker: %s\n", maker );
	if (programer != NULL) {
		fprintf( fp, "; Programer: %s\n", programer);
	}
	fprintf(fp, "\n");
}



/*--------------------------------------------------------------
	
 Input: ʸ����ǡ�����db�Ȥ���max�Х��Ƚ���(��ü�ʹߤ�0������)
	
 Output:
	
--------------------------------------------------------------*/
void printStrDb( FILE *fp, const char *str, int max)
{
	int i, end_flag = 0, newline_flag = 0;
	char c;
	for (i = 0; i < max; i++) {
		c = *(str + i);
		if (c == '\0') {
			end_flag = 1;
		}
		if (end_flag) {
			c = '\0';
		}
		switch (i % 8) {
			case 0:
				newline_flag = 0;
				fprintf(fp, "\tdb\t$%02x", c & 0xff);
				break;
			case 7:
				newline_flag = 1;
				fprintf(fp, ", $%02x",  c & 0xff);
				break;
			default:				
				fprintf(fp, ", $%02x",  c & 0xff);
				break;
		}
		if (newline_flag) {
			fprintf(fp, "\n");
		}
	}
	if (!newline_flag) {
		fprintf(fp, "\n");
	}
	
}



/*--------------------------------------------------------------
	�����ȥ�/��ʼ�/�᡼������macro�Ȥ��ƽ񤭹���
 Input:
	
 Output:
	̵��
--------------------------------------------------------------*/
void writeSongInfoMacro(FILE *fp)
{
	fprintf( fp, "TITLE\t.macro\n");
	printStrDb(fp, song_name, 32);
	fprintf(fp, "\t.endm\n");
	fprintf( fp, "COMPOSER\t.macro\n");
	printStrDb(fp, composer, 32);
	fprintf(fp, "\t.endm\n");
	fprintf( fp, "MAKER\t.macro\n");
	printStrDb(fp, maker, 32);
	fprintf(fp, "\t.endm\n");
}





/*--------------------------------------------------------------
	�ѥ�᡼����n�ĤΥ��ޥ�ɤν���
 Input:
	
 Output:
	̵��
--------------------------------------------------------------*/
char *setCommandBuf( int n, CMD *cmd, int com_no, char *ptr, int line, int enable )
{
	int		cnt, i;
	int		param[PARAM_MAX];

	for( i = 0; i < PARAM_MAX; i++ ) {
		param[i] = 0;
	}

	if( n != 0 ) {
		i = 0;
		if( n > 1 ) {					// n��2�İʾ�ΤȤ���","�ν���������
			for( i = 0; i < n-1; i++ ) {
				cnt = 0;
				param[i] = Asc2Int( ptr, &cnt );
				if( cnt == 0 ) {		/* �ѥ�᡼����̵�����ϥ��顼�νФ���ͤ˽񤭴����� */
					param[i] = PARAM_OMITTED;
				}
				ptr += cnt;
				ptr = skipSpace( ptr );
				if( *ptr == ',' ) {
					ptr++;
					ptr = skipSpace( ptr );
				}
			}
		}
		cnt = 0;
		param[i] = Asc2Int( ptr, &cnt );
		if( cnt == 0 ) {		/* �ѥ�᡼����̵�����ϥ��顼�νФ���ͤ˽񤭴����� */
			param[i] = PARAM_OMITTED;
		}
		ptr += cnt;
	}

	if( enable != 0 ) {
		cmd->cnt = 0;
		cmd->line = line;
		cmd->cmd  = com_no;
		cmd->len = 0;
		if( n != 0 ) {
			for( i = 0; i < n; i++ ) {
				cmd->param[i] = param[i];
			}
		}
	}

	return ptr;
}



/*--------------------------------------------------------------
	��Ĺ�ѥ�᡼���μ���
 Input:
	
 Output:
	̵��
--------------------------------------------------------------*/
char *getLengthSub( char *ptr, double *len, double def )
{
	int		cnt;
	double 	temp;

	/* �ե졼����� */
	if( *ptr == '#' ) {
		ptr++;
		*len = Asc2Int( ptr, &cnt );
		if( cnt != 0 ) {
			ptr += cnt;
			*len = *len / tbase;
		} else {
			*len = -1;
		}
	/* ������Ȼ��� */
	} else if( *ptr == '%' ) {
		ptr++;
		*len = Asc2Int( ptr, &cnt );
		if( cnt != 0 ) {
			ptr += cnt;
		} else {
			*len = -1;
		}
	/* ����Ū��Ĺ���� */
	} else {
		*len = Asc2Int( ptr, &cnt );
		if( cnt != 0 ) {
			ptr += cnt;
			*len = _BASE/(*len);
		} else {
			/* �ѥ�᡼����̵�����ϥ��顼�νФ���ͤ˽񤭴����� */
			*len = def;
		}
		/* ���顼/l���ޥ�ɤλ��Ͻ��������ʤ� */
		if( *len != -1 ) {
			/* �����ν���(ʣ����ǽ��) */
			temp = *len;
			while( *ptr == '.' ) {
				temp /= 2;
				*len += temp;
				ptr++;
			}
		}
	}
	return ptr;
}


/*--------------------------------------------------------------
	��Ĺ����
 Output:
	*len: 
--------------------------------------------------------------*/
char *getLength( char *ptr, double *len, double def )
{
	ptr = getLengthSub(ptr, len, def);
	/* ��Ĺ����(��������ǽ) */
	if (*ptr == '-' || *ptr == '~') {
		double len_adjust;
		ptr++;
		ptr = getLengthSub(ptr, &len_adjust, def);
		if (*len - len_adjust > 0) {
			*len = *len - len_adjust;
		} else {
			//dispError();�ƤӽФ����ǥ��顼��ª
			*len = *len - len_adjust;
		}
	}
	return ptr;
}



/*--------------------------------------------------------------
	�ѥ�᡼����1��(��Ĺ)�Υ��ޥ�ɤν���
 Input:
	
 Output:
	̵��
--------------------------------------------------------------*/
char *setCommandBufL( CMD *cmd, int com_no, char *ptr, int line, int enable )
{
	cmd->cnt = 0;
	cmd->line = line;
	cmd->cmd  = com_no;

	ptr = getLength( ptr, &(cmd->len), -1 );
	if( cmd->len > 0 ) {
		if( enable != 0 ) {
			length = cmd->len;
		}
	} else {
		dispError( ABNORMAL_NOTE_LENGTH_VALUE, cmd->filename, line );
	}

	return ptr;
}



/*--------------------------------------------------------------
	�ѥ�᡼����1��(����/��Ĺ)�Υ��ޥ�ɤν���
 Input:
	
 Output:
	̵��
--------------------------------------------------------------*/
char *setCommandBufN( CMD *cmd, int com_no, char *ptr, int line, int enable )
{
	int		oct_ofs, note;
	double	len;

	com_no += transpose;

	/* c+-++-++--�Ȥ�������褦���к�(���̤��ʤ�����) */
	while( 1 ) {
		if( *ptr == '+' ) {
			com_no++;
			ptr++;
		} else if( *ptr == '-' ) {
			com_no--;
			ptr++;
		} else {
			break;
		}
	}
	/* ���������֤�ޤ��������������� */
	oct_ofs = 0;
	while( com_no < _NOTE_C ) {
		com_no += 12;
		oct_ofs--;
	}
	while( com_no > _NOTE_B ) {
		com_no -= 12;
		oct_ofs++;
	}

	note = ((octave+oct_ofs)<<4)+com_no;
	/* �������ϰϥ����å� */
	if( note < 0 ) {
		switch( note ) {
		  case -5: note = 15; break;
		  case -6: note = 14; break;
		  case -7: note = 13; break;
		  default: note =  0; break;
		}
	} else if( note > MAX_NOTE ) {
		note = MAX_NOTE;
	}

	ptr = getLength( ptr, &len, length );
	if (len <= 0) {
		dispError( ABNORMAL_NOTE_LENGTH_VALUE, cmd->filename, line );
		len = 0.0;
	}
	if( enable != 0 ) {
		cmd->cnt  = 0;
		cmd->line = line;
		cmd->cmd  = note;
		cmd->len  = len;
	}

	return ptr;
}



/*--------------------------------------------------------------
	�ѥ�᡼����1��(����(ľ�ܻ���)/��Ĺ)�Υ��ޥ�ɤν���
 Input:
	
 Output:
	̵��
--------------------------------------------------------------*/
char *setCommandBufN0( CMD *cmd, char *ptr, int line, int enable )
{
	int		cnt, note;
	double	len;

	cnt  = 0;
	note = Asc2Int( ptr, &cnt );
	if( cnt == 0 ) {
		dispError( ABNORMAL_PITCH_VALUE, cmd->filename, line );
		return ptr+1;
	}
	ptr += cnt;

	// �������ϰϥ����å�
	if( note < 0 ) {
		note = 0;
	} else if( note > MAX_NOTE ) {
		note = MAX_NOTE;
	}

	ptr = skipSpace( ptr );				// ;ʬ�ʥ��ڡ����򥹥��å�
	// ","������Ȥ��ϲ�Ĺ��¸�ߤ���
	if( *ptr == ',' ) {
		ptr++;
		ptr = skipSpace( ptr );			// ;ʬ�ʥ��ڡ����򥹥��å�

		ptr = getLength( ptr, &len, length );
		if (len <= 0) {
			dispError( ABNORMAL_NOTE_LENGTH_VALUE, cmd->filename, line );
			len = 0.0;
		}
	// ","���ʤ��Ȥ��ϥǥե���Ȥβ�Ĺ����Ѥ���
	} else {
		len = length;
	}

	if( enable != 0 ) {
		cmd->cnt  = 0;
		cmd->line = line;
		cmd->cmd  = note;
		cmd->len  = len;
	}

	return ptr;
}



/*--------------------------------------------------------------
	�ѥ�᡼����1��(���ȿ�(ľ�ܻ���)/��Ĺ)�Υ��ޥ�ɤν���
 Input:
	
 Output:
	̵��
--------------------------------------------------------------*/
char *setCommandBufN1( CMD *cmd, int com_no, char *ptr, int line, int enable )
{
	int		cnt, freq;
	double	len;

	cnt = 0;
	freq = Asc2Int( ptr, &cnt );
	// ʸ���������å�
	if( cnt == 0 ) {
		dispError( ABNORMAL_PITCH_VALUE, cmd->filename, line );
		return ptr+1;
	}
	ptr += cnt;
	// �ѥ�᡼���ϰϥ����å�
	if( 0x0008 > freq && freq >= 0x07f2 ) {
		dispError( ABNORMAL_PITCH_VALUE, cmd->filename, line );
		return ptr+1;
	}
	// "," ������Ȥ��ϲ�Ĺ����
	ptr = skipSpace( ptr );
	if( *ptr == ',' ) {
		ptr++;
		ptr = skipSpace( ptr );
		ptr = getLength( ptr, &len, length );
		if (len <= 0) {
			dispError( ABNORMAL_NOTE_LENGTH_VALUE, cmd->filename, line );
			len = 0.0;
		}
	// "," ���ʤ��Ȥ��ϥǥե���Ȳ�Ĺ��
	} else {
		len = length;
	}

	if( enable != 0 ) {
		cmd->cnt      = 0;
		cmd->line     = line;
		cmd->cmd      = com_no;
		cmd->len      = len;
		cmd->param[0] = freq;
	}

	return ptr;
}



/*--------------------------------------------------------------
	�ѥ�᡼����1��(����/��Ĺ)�Υ��ޥ�ɤν���
 Input:
	
 Output:
	̵��
--------------------------------------------------------------*/
char *setCommandBufR( CMD *cmd, int com_no, char *ptr, int line, int enable )
{
	double	len;

	ptr = getLength( ptr, &len, length );
	if (len <= 0) {
		dispError( ABNORMAL_NOTE_LENGTH_VALUE, cmd->filename, line );
		len = 0.0;
	}
	
	if( enable != 0 ) {
		cmd->cnt = 0;
		cmd->line = line;
		cmd->cmd = com_no;
		cmd->len = len;
	}

	return ptr;
}


/*--------------------------------------------------------------
	�ѥ�᡼����1��(��������/��Ĺ)�Υ��ޥ�ɤν���
 Input:
	
 Output:
	̵��
--------------------------------------------------------------*/
char *setCommandBufK( CMD *cmd, int com_no, char *ptr, int line, int enable )
{
	double	len;

	ptr = getLength( ptr, &len, length );
	if (len < 0) { /* ��Ĺ0���� */
		dispError( ABNORMAL_NOTE_LENGTH_VALUE, cmd->filename, line );
		len = 0.0;
	}
	
	if( enable != 0 ) {
		cmd->cnt = 0;
		cmd->line = line;
		cmd->cmd = com_no;
		cmd->len = len;
	}

	return ptr;
}




/*--------------------------------------------------------------
	
 Input:
	
 Output:
	̵��
--------------------------------------------------------------*/
CMD * analyzeData( int trk, CMD *cmd, LINE *lptr )
{
	int		i, line, com, cnt;
	char	*ptr;

	typedef struct {
		char	*cmd;
		int		num;
		unsigned long		enable;
	} MML;
	const MML mml[] = {
		{ "c", _NOTE_C,			(ALLTRACK) },
		{ "d", _NOTE_D,			(ALLTRACK) },
		{ "e", _NOTE_E,			(ALLTRACK) },
		{ "f", _NOTE_F,			(ALLTRACK) },
		{ "g", _NOTE_G,			(ALLTRACK) },
		{ "a", _NOTE_A,			(ALLTRACK) },
		{ "b", _NOTE_B,			(ALLTRACK) },
		{ "@n", _KEY,			(ALLTRACK&~DPCMTRACK&~VRC7TRACK&~N106TRACK) },
		{ "n", _NOTE,			(ALLTRACK) },
		{ "w", _WAIT,			(ALLTRACK) },
		{ "@t", _TEMPO2,		(ALLTRACK) },
		{ "t", _TEMPO,			(ALLTRACK) },
		{ "o", _OCTAVE,			(ALLTRACK) },
		{ ">", _OCT_UP,			(ALLTRACK) },
		{ "<", _OCT_DW,			(ALLTRACK) },
		{ "l", _LENGTH,			(ALLTRACK) },
		{ "v+", _VOL_PLUS,		(ALLTRACK&~DPCMTRACK) },
		{ "v-", _VOL_MINUS,		(ALLTRACK&~DPCMTRACK) },
		{ "v", _VOLUME,			(ALLTRACK&~DPCMTRACK) },
		{ "NB", _NEW_BANK,		(ALLTRACK) },
		{ "EPOF", _EP_OFF,		(ALLTRACK&~DPCMTRACK) },
		{ "EP",   _EP_ON,		(ALLTRACK&~DPCMTRACK) },
		{ "ENOF", _EN_OFF,		(ALLTRACK&~DPCMTRACK) },
		{ "EN",   _EN_ON,		(ALLTRACK&~DPCMTRACK) },
		{ "MPOF", _LFO_OFF,		(ALLTRACK&~DPCMTRACK) },
		{ "MP",   _LFO_ON,		(ALLTRACK&~DPCMTRACK) },
		{ "EH",	_HARD_ENVELOPE,		(FMTRACK) },
		{ "MHOF", _MH_OFF,		(FMTRACK) },
		{ "MH",	  _MH_ON,		(FMTRACK) },
		{ "OP",   _VRC7_TONE,		(VRC7TRACK) },
		{ "SDQR", _SELF_DELAY_QUEUE_RESET,	(ALLTRACK&~TRACK(2)&~DPCMTRACK) },
		{ "SDOF", _SELF_DELAY_OFF,	(ALLTRACK&~TRACK(2)&~DPCMTRACK) },
		{ "SD", _SELF_DELAY_ON,		(ALLTRACK&~TRACK(2)&~DPCMTRACK) },
		{ "SA", _SHIFT_AMOUNT,		(N106TRACK) },
		{ "D", _DETUNE,			(ALLTRACK&~DPCMTRACK) },
		{ "K", _TRANSPOSE,		(ALLTRACK&~NOISETRACK&~DPCMTRACK) },
		{ "M", _SUN5B_HARD_SPEED,	(FME7TRACK) },
		{ "S", _SUN5B_HARD_ENV,	(FME7TRACK) },
		{ "N", _SUN5B_NOISE_FREQ,	(FME7TRACK) },
		{ "@q", _QUONTIZE2,		(ALLTRACK) },
		{ "@vr", _REL_ENV,		(ALLTRACK&~TRACK(2)&~DPCMTRACK) }, 
		{ "@v", _ENVELOPE,		(ALLTRACK&~TRACK(2)&~DPCMTRACK) },
		{ "@@r", _REL_ORG_TONE,		(TRACK(0)|TRACK(1)|FMTRACK|VRC7TRACK|VRC6PLSTRACK|N106TRACK|MMC5PLSTRACK) },
		{ "@@", _ORG_TONE,		(TRACK(0)|TRACK(1)|FMTRACK|VRC7TRACK|VRC6PLSTRACK|N106TRACK|MMC5PLSTRACK) },
		{ "@", _TONE,			(TRACK(0)|TRACK(1)|VRC6PLSTRACK|MMC5PLSTRACK|FME7TRACK) },
		{ "s", _SWEEP,			(TRACK(0)|TRACK(1)|FMTRACK) },
		{ "&", _SLAR,			(ALLTRACK) },
		{ "y", _DATA_WRITE,		(ALLTRACK) },
		{ "x", _DATA_THRUE,		(ALLTRACK) },

		{ "|:", _REPEAT_ST2,		(ALLTRACK) },
		{ ":|", _REPEAT_END2,		(ALLTRACK) },
		{ "\\", _REPEAT_ESC2,		(ALLTRACK) },

#if 0
		{ "SQOF", _SHUFFLE_QUONTIZE_OFF,	(ALLTRACK) },
		{ "SQR", _SHUFFLE_QUONTIZE_RESET,(ALLTRACK) },
		{ "SQ", _SHUFFLE_QUONTIZE,	(ALLTRACK) },
		{ "XX", _XX_COMMAND,		(ALLTRACK) },
#endif
//		{ "'", _ARTICULATION_ADJUST,	(ALLTRACK) },
		{ "k", _KEY_OFF,		(ALLTRACK) },
		
		{ "L", _SONG_LOOP,		(ALLTRACK) },
		{ "[", _REPEAT_ST,		(ALLTRACK) },
		{ "]", _REPEAT_END,		(ALLTRACK) },
		{ "|", _REPEAT_ESC,		(ALLTRACK) },
		{ "{", _CONT_NOTE,		(ALLTRACK) },
		{ "}", _CONT_END,		(ALLTRACK) },
		{ "q", _QUONTIZE,		(ALLTRACK) },
		{ "r", _REST,			(ALLTRACK) },
		{ "^", _TIE,			(ALLTRACK) },
		{ "!", _DATA_BREAK,		(ALLTRACK) },
		{ "",  _TRACK_END,		(ALLTRACK) },
	};

	cnt = 0;

	transpose = 0;

	for( line = 1; line <= lptr->line; line++ ) {
		if( (lptr[line].status == _TRACK) && ((lptr[line].param&(1<<trk)) != 0) ) {
			ptr = lptr[line].str;
			while( *ptr != '\0' ) {
				ptr = skipSpace( ptr );			// ;ʬ�ʥ��ڡ����򥹥��å�
				if( *ptr == '\0' ) break;		// ���Υ饤��Ͻ���ꡩ
				// ���ޥ�ɤ򸡺�����
				for( i = 0; mml[i].num != _TRACK_END; i++ ) {
					if( strncmp( mml[i].cmd, ptr, strlen(mml[i].cmd) ) == 0 ) break;
				}
				ptr += strlen(mml[i].cmd);		// ���ޥ�ɤ�ʸ��������ʸ���򥹥��å�
				cmd->filename = lptr[line].filename;	// ���顼���ϻ��Υե�����̾����
				switch( mml[i].num ) {
				/* ���������� */
				  case _OCTAVE:
					com = Asc2Int( ptr, &cnt );
					if( cnt != 0 ) {
						// ���ޥ�ɤ�ͭ���λ��Ͻ�������Ͽ
						if( (mml[i].enable&(1<<trk)) != 0 ) {
							if( trk == BTRACK(0) || trk == BTRACK(1) || trk == BTRACK(2) ) {
								octave = com-2;
							} else {
								octave = com;
							}
						}
						ptr += cnt;
					}
					break;
				/* ���������֥��å� */
				  case _OCT_UP:
					if( (mml[i].enable&(1<<trk)) != 0 ) {
						if( octave_flag == 0 ) { octave++; } else { octave--; }
					} else {
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
					}
					break;
				/* ���������֥����� */
				  case _OCT_DW:
					if( (mml[i].enable&(1<<trk)) != 0 ) {
						if( octave_flag == 0 ) { octave--; } else { octave++; }
					} else {
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
					}
					break;
				/* ��Ĺ���� */
				  case _LENGTH:
					ptr = setCommandBufL( cmd, _LENGTH, ptr, line, mml[i].enable&(1<<trk) );
					if( (mml[i].enable&(1<<trk)) == 0 ) {
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
					}
					break;
				/* ����(n���ޥ��) */
				  case _NOTE:
					ptr = setCommandBufN0( cmd, ptr, line, mml[i].enable&(1<<trk) );
					if( (mml[i].enable&(1<<trk)) == 0 ) {
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
					}
					break;
				/* ����(@n���ޥ��) */
				  case _KEY:
					ptr = setCommandBufN1( cmd, _KEY, ptr, line, mml[i].enable&(1<<trk) );
					if( (mml[i].enable&(1<<trk)) == 0 ) {
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
					}
					break;
				/* ���� */
				  case _NOTE_C:
				  case _NOTE_D:
				  case _NOTE_E:
				  case _NOTE_F:
				  case _NOTE_G:
				  case _NOTE_A:
				  case _NOTE_B:
					ptr = setCommandBufN( cmd, mml[i].num, ptr, line, mml[i].enable&(1<<trk) );
					if( (mml[i].enable&(1<<trk)) == 0 ) {
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
					}
					break;
				/* ����/Ϣ�� */
				  case _REST:
				  case _CONT_END:
				  case _TIE:
				  case _WAIT:
					ptr = setCommandBufR( cmd, mml[i].num, ptr, line, mml[i].enable&(1<<trk) );
					if( (mml[i].enable&(1<<trk)) == 0 ) {
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
					}
					break;
				 /* �������� */
				  case _KEY_OFF:
					ptr = setCommandBufK( cmd, mml[i].num, ptr, line, mml[i].enable&(1<<trk) );
					if( (mml[i].enable&(1<<trk)) == 0 ) {
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
					}
					break;
				/* ���ޥ�ɥѥ�᡼����0�Ĥ�ʪ */
				  case _SLAR:			/* ���顼 */
				  case _SONG_LOOP:			/* �ʥ롼�� */
				  case _REPEAT_ST:		/* ��ԡ���(�����Ǥ�Ÿ������) */
				  case _REPEAT_ESC:		/* ��ԡ�������ȴ�� */
				  case _CONT_NOTE:		/* Ϣ�䳫�� */
				  case _LFO_OFF:
				  case _EP_OFF:
				  case _EN_OFF:
				  case _MH_OFF:
				  case _REPEAT_ST2:		/* ��ԡ���2 */
				  case _REPEAT_ESC2:	/* ��ԡ�������ȴ��2 */
//				  case _SHUFFLE_QUONTIZE_RESET:
//				  case _SHUFFLE_QUONTIZE_OFF:
				  case _SELF_DELAY_OFF:
				  case _SELF_DELAY_QUEUE_RESET:
					setCommandBuf( 0, cmd, mml[i].num, ptr, line, mml[i].enable&(1<<trk) );
					if( (mml[i].enable&(1<<trk)) == 0 ) {
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
					}
					break;					
				/* ���ޥ�ɥѥ�᡼����1�Ĥ�ʪ */
				  case _TEMPO:			/* �ƥ�� */
					ptr = setCommandBuf( 1, cmd, mml[i].num, ptr, line, mml[i].enable&(1<<trk) );
					if( (mml[i].enable&(1<<trk)) != 0 ) {
						if( cmd->param[0] <= 0 ) {
							dispError( ABNORMAL_TEMPO_VALUE, lptr[line].filename, line );
							cmd->cmd = _NOP;
						} else {
							tbase = (double)_BASETEMPO/(double)cmd->param[0];
						}
					} else {
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
					}
					break;
				  case _TONE:			/* �����ڤ��ؤ� */
					ptr = setCommandBuf( 1, cmd, mml[i].num, ptr, line, mml[i].enable&(1<<trk) );
					if( (mml[i].enable&(1<<trk)) != 0 ) {
						//vrc6�Ѥ����¤򳰤�(��¢����ȡ�MMC5��@3�ޤ�)
						//if( cmd->param[0] < 0 || cmd->param[0] > 3 ) {
						if( cmd->param[0] < 0 || cmd->param[0] > 7 ) {
							dispError( ABNORMAL_TONE_NUMBER, lptr[line].filename, line );
							cmd->cmd = 0;
							cmd->line = 0;
						}
					} else {
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
					}
					break;
				  case _REL_ORG_TONE:		/* ��꡼������ */
				  case _ORG_TONE:		/* �����ڤ��ؤ� */
					ptr = setCommandBuf( 1, cmd, mml[i].num, ptr, line, mml[i].enable&(1<<trk) );
					if( (mml[i].enable&(1<<trk)) != 0 ) {
						if ((mml[i].num == _REL_ORG_TONE) && (cmd->param[0] == 255)) {
							//ok
						} else if ( cmd->param[0] < 0 || cmd->param[0] > 127 ) {
							dispError( ABNORMAL_TONE_NUMBER, lptr[line].filename, line );
							cmd->cmd = 0;
							cmd->line = 0;
						}
					} else {
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
					}
					break;
				  case _ENVELOPE:		/* ����٥��׻��� */
					cmd->filename = lptr[line].filename;
					ptr = setCommandBuf( 1, cmd, mml[i].num, ptr, line, mml[i].enable&(1<<trk) );
					if( (mml[i].enable&(1<<trk)) != 0 ) {
						if( cmd->param[0] == 255) { 
							volume_flag = 0x0000; 
						} else if( 0 <= cmd->param[0] && cmd->param[0] <= 127) {
							volume_flag = 0x8000; 
						} else {
							dispError( ABNORMAL_ENVELOPE_NUMBER, lptr[line].filename, line );
							cmd->cmd = 0;
							cmd->line = 0;
						} 
					} else {
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line ); 
					} 
					break; 
				  case _REL_ENV:		/* ��꡼������٥��׻��� */ 
					cmd->filename = lptr[line].filename; 
					ptr = setCommandBuf( 1, cmd, mml[i].num, ptr, line, mml[i].enable&(1<<trk) ); 
					if( (mml[i].enable&(1<<trk)) != 0 ) { 
						if( cmd->param[0] == 255 ) { 
							volume_flag = 0x0000; 
						} else if( 0 <= cmd->param[0] && cmd->param[0] <= 127) {
							volume_flag = 0x8000;
						} else {
							dispError( ABNORMAL_ENVELOPE_NUMBER, lptr[line].filename, line ); 
							cmd->cmd = 0; 
							cmd->line = 0; 
						}
					} else {
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
					}
					break;
				  case _VOL_PLUS:		/* ���̻��� */
					ptr = setCommandBuf( 1, cmd, mml[i].num, ptr, line, mml[i].enable&(1<<trk) );
					if( (mml[i].enable&(1<<trk)) != 0 ) {
						if( cmd->param[0] == PARAM_OMITTED ) {
							cmd->param[0] = 1;
						}
						if( ((1<<trk) & ~(FMTRACK|VRC6SAWTRACK) && 0 <= volume_flag && volume_flag <= 15)
						 || ((1<<trk) & (FMTRACK|VRC6SAWTRACK) && 0 <= volume_flag && volume_flag <= 63) ) {
							cmd->cmd = _VOLUME;
							cmd->param[0] = volume_flag+cmd->param[0];
							if( ((1<<trk) & ~(FMTRACK|VRC6SAWTRACK) && (cmd->param[0] < 0 || cmd->param[0] > 15))
							 || ((1<<trk) & (FMTRACK|VRC6SAWTRACK) && (cmd->param[0] < 0 || cmd->param[0] > 63)) ) {
								dispError( VOLUME_RANGE_OVER_OF_RELATIVE_VOLUME, lptr[line].filename, line );
								cmd->cmd = 0;
								cmd->line = 0;
							} else {
								volume_flag = cmd->param[0];
							}
						} else {
							dispError( RELATIVE_VOLUME_WAS_USED_WITHOUT_SPECIFYING_VOLUME, lptr[line].filename, line );
							cmd->cmd = 0;
							cmd->line = 0;
						}
					} else {
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
					}
					break;
				  case _VOL_MINUS:		/* ���̻��� */
					ptr = setCommandBuf( 1, cmd, mml[i].num, ptr, line, mml[i].enable&(1<<trk) );
					if( (mml[i].enable&(1<<trk)) != 0 ) {
						if( cmd->param[0] == PARAM_OMITTED ) {
							cmd->param[0] = 1;
						}
						if( ((1<<trk) & ~(FMTRACK|VRC6SAWTRACK) && 0 <= volume_flag && volume_flag <= 15)
						 || ((1<<trk) & (FMTRACK|VRC6SAWTRACK) && 0 <= volume_flag && volume_flag <= 63) ) {
							cmd->cmd = _VOLUME;
							cmd->param[0] = volume_flag-cmd->param[0];
							if( cmd->param[0] < 0 ) {
								dispError( VOLUME_RANGE_UNDER_OF_RELATIVE_VOLUME, lptr[line].filename, line );
								cmd->cmd = 0;
								cmd->line = 0;
							} else {
								volume_flag = cmd->param[0];
							}
						} else {
							dispError( RELATIVE_VOLUME_WAS_USED_WITHOUT_SPECIFYING_VOLUME, lptr[line].filename, line );
							cmd->cmd = 0;
							cmd->line = 0;
						}
					} else {
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
					}
					break;
				  case _VOLUME:			/* ���̻��� */
					ptr = setCommandBuf( 1, cmd, mml[i].num, ptr, line, mml[i].enable&(1<<trk) );
					if( (mml[i].enable&(1<<trk)) != 0 ) {
						if( ((1<<trk) & ~(FMTRACK|VRC6SAWTRACK) && (cmd->param[0] < 0 || cmd->param[0] > 15))
						 || ((1<<trk) & (FMTRACK|VRC6SAWTRACK) && (cmd->param[0] < 0 || cmd->param[0] > 63)) ) {
							dispError( ABNORMAL_VOLUME_VALUE, lptr[line].filename, line );
							cmd->cmd = 0;
							cmd->line = 0;
						} else {
							volume_flag = cmd->param[0];
						}
					} else {
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
					}
					break;
				  case _HARD_ENVELOPE:
					ptr = setCommandBuf( 2, cmd, mml[i].num, ptr, line, mml[i].enable&(1<<trk) );
					if( (mml[i].enable&(1<<trk)) != 0 ) {
						if( (cmd->param[0] < 0 || cmd->param[0] >  1)
						 && (cmd->param[1] < 0 || cmd->param[1] > 63) ) {
							dispError( ABNORMAL_ENVELOPE_VALUE, lptr[line].filename, line );
							cmd->cmd = 0;
							cmd->line = 0;
						} else {
							volume_flag = 0x8000;
						}
					} else {
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
					}
					break;
				  case _QUONTIZE:		/* �����󥿥���(length*n/gate_denom) */
					ptr = setCommandBuf( 2, cmd, mml[i].num, ptr, line, mml[i].enable&(1<<trk) );
					if( (mml[i].enable&(1<<trk)) != 0 ) {
						if ( cmd->param[1] == PARAM_OMITTED ) {
							cmd->param[1] = 0;
						}
						if (   cmd->param[0] < 0
						     ||cmd->param[0] > gate_denom
						     ||(cmd->param[0] == 0 && cmd->param[1] <= 0)
						     ||(cmd->param[0] == gate_denom && cmd->param[1] > 0) ) {
							dispError( ABNORMAL_QUANTIZE_VALUE,  lptr[line].filename, line );
							cmd->cmd = 0;
							cmd->line = 0;
						}
					} else {
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
					}
					break;
				  case _QUONTIZE2:		/* �����󥿥���(length-n) */
					ptr = setCommandBuf( 1, cmd, mml[i].num, ptr, line, mml[i].enable&(1<<trk) );
					if( (mml[i].enable&(1<<trk)) == 0 ) {
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
					}
					break;
#if 0
				  case _SHUFFLE_QUONTIZE:	/* ����åե륯���󥿥������� */
					ptr = setCommandBuf( 3, cmd, mml[i].num, ptr, line, mml[i].enable&(1<<trk) );
					if( (mml[i].enable&(1<<trk)) != 0 ) {
						if (   cmd->param[0] <= 0
						     ||cmd->param[1] <= 0
						     ||cmd->param[2] <= 0
						     ||cmd->param[0] == PARAM_OMITTED
						     ||cmd->param[1] == PARAM_OMITTED
						     ||cmd->param[2] == PARAM_OMITTED  ) {
							dispError( ABNORMAL_SHUFFLE_QUANTIZE_VALUE,  lptr[line].filename, line );
							cmd->cmd = _NOP;
							cmd->line = 0;
						}
					} else {
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
					}
					break;
#endif
				  case _LFO_ON:			/* ���եȣ̣ƣ� */
					ptr = setCommandBuf( 1, cmd, mml[i].num, ptr, line, mml[i].enable&(1<<trk) );
					if( (mml[i].enable&(1<<trk)) != 0 ) {
						if( (cmd->param[0] != 255) 
						 && (cmd->param[0] < 0 || cmd->param[0] > 63) ) { 
							dispError( ABNORMAL_LFO_NUMBER, lptr[line].filename, line );
							cmd->cmd = 0;
							cmd->line = 0;
						}
					} else {
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
					}
					break;
				  case _EP_ON:			/* �ԥå�����٥��� */
					ptr = setCommandBuf( 1, cmd, mml[i].num, ptr, line, mml[i].enable&(1<<trk) );
					if( (mml[i].enable&(1<<trk)) != 0 ) {
						if( (cmd->param[0] != 255) 
						 && (cmd->param[0] < 0 || cmd->param[0] > 127) ) { 
							dispError( ABNORMAL_PITCH_ENVELOPE_NUMBER, lptr[line].filename, line );
							cmd->cmd = 0;
							cmd->line = 0;
						}
					} else {
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
					}
					break;
				  case _EN_ON:			/* �Ρ��ȥ���٥��� */
					ptr = setCommandBuf( 1, cmd, mml[i].num, ptr, line, mml[i].enable&(1<<trk) );
					if( (mml[i].enable&(1<<trk)) != 0 ) {
						if( (cmd->param[0] != 255)
						 && (cmd->param[0] < 0 || cmd->param[0] > 127) ) { 
							dispError( ABNORMAL_NOTE_ENVELOPE_NUMBER, lptr[line].filename, line );
							cmd->cmd = 0;
							cmd->line = 0;
						}
					} else {
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
					}
					break;
				  case _MH_ON:			/* �ϡ��ɥ��������ե����� */
					ptr = setCommandBuf( 1, cmd, mml[i].num, ptr, line, mml[i].enable&(1<<trk) );
					if( (mml[i].enable&(1<<trk)) != 0 ) {
						if( (cmd->param[0] != 255) 
						 && (cmd->param[0] < 0 || cmd->param[0] > 15) ) { 
							dispError( ABNORMAL_HARD_EFFECT_NUMBER, lptr[line].filename, line );
							cmd->cmd = 0;
							cmd->line = 0;
						}
					} else {
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
					}
					break;
				  case _DETUNE:			/* �ǥ����塼�� */
					ptr = setCommandBuf( 1, cmd, mml[i].num, ptr, line, mml[i].enable&(1<<trk) );
					if( (mml[i].enable&(1<<trk)) != 0 ) {
						if( (cmd->param[0] != 255)
						 && (cmd->param[0] <-127 || cmd->param[0] > 126) ) {
							dispError( ABNORMAL_DETUNE_VALUE, lptr[line].filename, line );
							cmd->cmd = 0;
							cmd->line = 0;
						}
					} else {
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
					}
					break;
				  case _TRANSPOSE:			/* �ȥ�󥹥ݡ��� */
					ptr = setCommandBuf( 1, cmd, mml[i].num, ptr, line, mml[i].enable&(1<<trk) );
					if( (mml[i].enable&(1<<trk)) != 0 ) {
						if( (cmd->param[0] != 255)
						 && (cmd->param[0] <-127 || cmd->param[0] > 126) ) {
							dispError( ABNORMAL_TRANSPOSE_VALUE, lptr[line].filename, line );
							cmd->cmd = 0;
							cmd->line = 0;
						}
						transpose = cmd->param[0];
					} else {
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
					}
					break;
				  case _REPEAT_END:		/* ��ԡ��Ƚ�λ */
				  case _REPEAT_END2:	/* ��ԡ��Ƚ�λ */
					ptr = setCommandBuf( 1, cmd, mml[i].num, ptr, line, mml[i].enable&(1<<trk) );
					if( (mml[i].enable&(1<<trk)) != 0 ) {
						if( cmd->param[0] < 2 ) {
							dispError( ABNORMAL_VALUE_OF_REPEAT_COUNT, lptr[line].filename, line );
							cmd->param[0] = 2;
						}
					} else {
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
					}
					break;
				  case _VRC7_TONE:			/* VRC7�桼���������ڤ��ؤ� */
					ptr = setCommandBuf( 1, cmd, mml[i].num, ptr, line, mml[i].enable&(1<<trk) );
					if( (mml[i].enable&(1<<trk)) != 0 ) {
						if( cmd->param[0] < 0 || cmd->param[0] > 63 ) {
							dispError( ABNORMAL_TONE_NUMBER, lptr[line].filename, line );
							cmd->cmd = 0;
							cmd->line = 0;
						}
					} else {
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
					}
					break;
				  case _SUN5B_HARD_SPEED:		/* PSG�ϡ��ɥ���������٥���®�� */
					ptr = setCommandBuf( 1, cmd, mml[i].num, ptr, line, mml[i].enable&(1<<trk) );
					if( (mml[i].enable&(1<<trk)) != 0 ) {
						if( cmd->param[0] < 0 || cmd->param[0] > 65535 ) {
							dispError( ABNORMAL_ENVELOPE_VALUE, lptr[line].filename, line );
							cmd->cmd = 0;
							cmd->line = 0;
						}
					} else {
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
					}
					break;
				  case _SUN5B_HARD_ENV:		/* PSG�ϡ��ɥ���������٥������� */
					ptr = setCommandBuf( 1, cmd, mml[i].num, ptr, line, mml[i].enable&(1<<trk) );
					if( (mml[i].enable&(1<<trk)) != 0 ) {
						if( cmd->param[0] < 0 || cmd->param[0] > 15 ) {
							dispError( ABNORMAL_ENVELOPE_VALUE, lptr[line].filename, line );
							cmd->cmd = 0;
							cmd->line = 0;
						} else {
							volume_flag = 0x8000;
						}
					} else {
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
					}
					break;
				  case _SUN5B_NOISE_FREQ:	/* PSG�Υ������ȿ� */
					ptr = setCommandBuf( 1, cmd, mml[i].num, ptr, line, mml[i].enable&(1<<trk) );
					if( (mml[i].enable&(1<<trk)) != 0 ) {
						if( cmd->param[0] < 0 || cmd->param[0] > 31 ) {
							dispError( ABNORMAL_PITCH_VALUE, lptr[line].filename, line );
							cmd->cmd = 0;
							cmd->line = 0;
						} else {
							volume_flag = 0x8000;
						}
					} else {
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
					}
					break;
				  case _TEMPO2:			/* �ե졼����ƥ�� */
					ptr = setCommandBuf( 2, cmd, mml[i].num, ptr, line, mml[i].enable&(1<<trk) );
					if( (mml[i].enable&(1<<trk)) != 0 ) {
						if( (cmd->param[0] <= 0) || (cmd->param[1] <= 0) ) {
							dispError( ABNORMAL_TEMPO_VALUE, lptr[line].filename, line );
							cmd->cmd = _NOP;
						} else {
							tbase = (double)cmd->param[0] * (double)cmd->param[1] / _BASE ;
						}
					} else {
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
					}
					break;
				  case _SWEEP:			/* ���������� */
					ptr = setCommandBuf( 2, cmd, _SWEEP, ptr, line, mml[i].enable&(1<<trk) );
					if( (mml[i].enable&(1<<trk)) != 0 ) {
						if( (cmd->param[0] < 0 || cmd->param[0] > 15)
						 || (cmd->param[1] < 0 || cmd->param[1] > 15) ) {
							dispError( ABNORMAL_SWEEP_VALUE, lptr[line].filename, line );
							cmd->cmd = 0;
							cmd->line = 0;
						}
					} else {
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
					}
					break;
				  case _DATA_WRITE:		/* �ǡ���(�쥸����)�񤭹��� */
					ptr = setCommandBuf( 2, cmd, _DATA_WRITE, ptr, line, mml[i].enable&(1<<trk) );
					if( (mml[i].enable&(1<<trk)) == 0 ) {
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
					}
					break;
				  case _DATA_THRUE:		/* �ǡ���ľ�ܽ񤭹��� */
					ptr = setCommandBuf( 2, cmd, _DATA_THRUE, ptr, line, mml[i].enable&(1<<trk) );
					if( (mml[i].enable&(1<<trk)) == 0 ) {
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
					}
					break;
#if 0
				  case _XX_COMMAND:		/* �ǥХå��� */
					ptr = setCommandBuf( 2, cmd, _XX_COMMAND, ptr, line, mml[i].enable&(1<<trk) );
					if( (mml[i].enable&(1<<trk)) == 0 ) {
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
					}
					break;
#endif
				  case _SELF_DELAY_ON:		/* ����եǥ��쥤 */
					ptr = setCommandBuf( 1, cmd, mml[i].num, ptr, line, mml[i].enable&(1<<trk) );
					if( (mml[i].enable&(1<<trk)) != 0 ) {
						if( (cmd->param[0] != 255)
						 && (cmd->param[0] < 0 || cmd->param[0] > SELF_DELAY_MAX) ) {
							dispError( ABNORMAL_SELFDELAY_VALUE, lptr[line].filename, line );
							cmd->cmd = 0;
							cmd->line = 0;
						}
					} else {
						cmd->cmd = _NOP;
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
					}
					break;
				  case _DATA_BREAK:		/* �ǡ����Ѵ���� */
					setCommandBuf( 0, cmd, _TRACK_END, ptr, line, mml[i].enable&(1<<trk) );
					if( (mml[i].enable&(1<<trk)) == 0 ) {
						dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
					}
					break;

				  case _NEW_BANK:
					// ̵�뤹����Ǥ�ptr���ɤ߿ʤ��
					ptr = setCommandBuf( 1, cmd, mml[i].num, ptr, line, mml[i].enable&(1<<trk) );
					if (!auto_bankswitch) {
						if( (mml[i].enable&(1<<trk)) != 0 ) {
							if( cmd->param[0] == PARAM_OMITTED ) {
								/* ����������礬����ޤ� */
							}
						} else {
							dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
						}
					} else {
						cmd->cmd = _NOP;
					}
					break;

				  case _SHIFT_AMOUNT:			/* �ԥå����ե��� (0��8) */
					ptr = setCommandBuf( 1, cmd, mml[i].num, ptr, line, mml[i].enable&(1<<trk) );
					if (pitch_correction) {
						if( (mml[i].enable&(1<<trk)) != 0 ) {
							if( (cmd->param[0] < 0 || cmd->param[0] > 8) ) { 
								dispError( ABNORMAL_SHIFT_AMOUNT, lptr[line].filename, line );
								cmd->cmd = 0;
								cmd->line = 0;
							}
						} else {
							dispError( UNUSE_COMMAND_IN_THIS_TRACK, lptr[line].filename, line );
						}
					} else {
						dispError( CANT_USE_SHIFT_AMOUNT_WITHOUT_PITCH_CORRECTION, lptr[line].filename, line );
					}
					break;
						
				  default:				/* ����¾(���顼) */
					dispError( COMMAND_NOT_DEFINED, lptr[line].filename, line );
					ptr++;
					break;
				}
				if( cmd->line != 0 ) {
					cmd++;
				}
			}
		} else if( lptr[line].status == _INCLUDE ) {
			cmd = analyzeData( trk, cmd, lptr[line].inc_ptr );
		}
	}
	return cmd;
}




#if 0
typedef struct {
	int flag;
	double diff;
	double base; //����åե뤵����Nʬ����Υ������Ĺ
} SHFL_Q;


/*--------------------------------------------------------------
	����åե륯���󥿥���
--------------------------------------------------------------*/
void shuffleQuontizeSub(CMD *ptr, SHFL_Q *shf, double count)
{
	if (shf->flag != 0) {
		double noteoff_time;
		if (double2int(count / shf->base) % 2 == 1 ) {
			//�Ρ��ȥ���λ��郎΢��
			ptr->len = ptr->len - shf->diff;
		}
		noteoff_time = count + ptr->len;
		if (double2int(noteoff_time / shf->base) % 2 == 1) {
			//�Ρ��ȥ��դλ��郎΢��
			ptr->len = ptr->len + shf->diff;
		}
	}
}





void shuffleQuontize(CMD *ptr)
{
	double count = 0.0; //��Ĺ�����ѡ����ʤ�����٥��ȯ������(�������ñ��)
	SHFL_Q shuffle = {0, 0.0, 192.0};
	while (1) {
		if (ptr->cmd == _SHUFFLE_QUONTIZE) {
			shuffle.flag = 1;
			shuffle.base = _BASE / ptr->param[0];
			printf("shfl %e\n", shuffle.base);
			shuffle.diff = shuffle.base * 2 * ptr->param[1]/(ptr->param[2] + ptr->param[1]) - shuffle.base;
			/*
			���Ȥ���16ʬ�����2:1�ˤ櫓��ʤ�
			shuffle.base = 192/16 = 12; �Ĥޤ�l16=l%12
			shuffle.diff = 24 * 2/3 - 12 
			             = 16 - 12 = 4
			�Ȥ����櫓��8ʬ����(%24)��%12+4��%12-4�����ʤ��%16��%8�ˤ櫓��
			*/
			ptr->cmd = _NOP;
			ptr++;
		} else if (ptr->cmd == _SHUFFLE_QUONTIZE_RESET) {
			count = 0.0;
			ptr->cmd = _NOP;
			ptr++;
		} else if (ptr->cmd == _SHUFFLE_QUONTIZE_OFF) {
			shuffle.flag = 0;
			ptr->cmd = _NOP;
			ptr++;
		} else if (ptr->cmd == _CONT_NOTE) {
			//Ϣ�����Ȥˤϴ�Ϳ���ʤ�����Ϣ��򥫥��ޥ�Ȥ���ª����
			while( 1 ) {
				if( ptr->cmd == _TRACK_END ) {
					//Ϣ������ǽ�λ
					//�����Ǥϥ��顼��Ф��ʤ�
					return;
				} else if( ptr->cmd == _CONT_END ) {
					//���Υ��ޥ�ɤ����äƤ��벻Ĺ���Ф��ƥ����󥿥�������
					shuffleQuontizeSub(ptr, &shuffle, count);
					count += ptr->len;
					ptr++;
					break;
				} else if (ptr->cmd <= MAX_NOTE || ptr->cmd == _REST || ptr->cmd == _KEY
						|| ptr->cmd == _NOTE || ptr->cmd == _WAIT || ptr->cmd == _TIE || temp->cmd == _KEY_OFF) {
					//��Ȥϥ��롼
					ptr++;
				} else {
					ptr++;
				}
			}
		} else if (ptr->cmd <= MAX_NOTE || ptr->cmd == _REST || ptr->cmd == _KEY
				|| ptr->cmd == _NOTE || ptr->cmd == _WAIT || ptr->cmd == _TIE || temp->cmd == _KEY_OFF) {
			shuffleQuontizeSub(ptr, &shuffle, count);
			count += ptr->len;
			ptr++;
		} else if (ptr->cmd == _TRACK_END) {
			break;
		} else {
			//¾�Τϥ��롼
			ptr++;
		}
	}
}
#endif



/*--------------------------------------------------------------
	�롼��/Ϣ���Ÿ��
 Input:
	*ptr
 Output:
	**cmd
--------------------------------------------------------------*/
CMD *translateData( CMD **cmd, CMD *ptr )
{
	CMD		*top,*end,*temp;
	int		cnt ,i, loop;
	double	len, gate;

	loop = 0;
	gate = 0;
	top = ptr;
	end = NULL;

	while( 1 ) {
		switch( ptr->cmd ) {
		  case _REPEAT_ST:
			ptr++;
			nest++;
			ptr = translateData( cmd, ptr );
			if (ptr == NULL) {
				/* [���Ĥ����Ƥ��ʤ� */
				return NULL;
			}
			nest--;
			break;
		  case _REPEAT_END:
			if( nest <= 0 ) {
				dispError( DATA_ENDED_BY_LOOP_DEPTH_EXCEPT_0, ptr->filename, ptr->line );
				ptr->cmd = _NOP;
				ptr++;
				break;
			}
			if( loop == 0 ) {
				loop = ptr->param[0];
				end = ptr+1;
			}
			if( loop == 1 ) {
				return end;
			}
			ptr = top;
			loop--;
			break;
		  case _REPEAT_ESC:
			if( nest <= 0 ) {
				dispError( DATA_ENDED_BY_LOOP_DEPTH_EXCEPT_0, ptr->filename, ptr->line );
				ptr->cmd = _NOP;
				ptr++;
				break;
			}
			if( loop == 1 ) {
				if( end != NULL ) {
					return end;
				}
			}
			ptr++;
			break;
		  case _CONT_NOTE:
			ptr++;
			temp = ptr;
			/* {} �����[cdefgab]|n|@n|r|w�����Ĥ��뤫? */
			cnt = 0;
			len = 0;
			while( 1 ) {
				if( temp->cmd == _TRACK_END ) {
					dispError( DATA_ENDED_BY_CONTINUATION_NOTE, mml_names[mml_idx], (ptr-1)->line );
					setCommandBuf( 0, *cmd, _TRACK_END, NULL, ptr->line, 1 );
					break;
				} else if( temp->cmd == _CONT_END ) {
					if (cnt == 0) {
						dispError( TUPLET_BRACE_EMPTY, mml_names[mml_idx], (ptr-1)->line );
						len = 0;
					} else {
						/* {}����Ȥ���������Ĺ���ˤʤ� */
						len = temp->len/(double)cnt;
					}
					break;
				} else if( temp->cmd <= MAX_NOTE || temp->cmd == _REST || temp->cmd == _KEY
					|| temp->cmd == _NOTE || temp->cmd == _WAIT || temp->cmd == _KEY_OFF ) {
					cnt++;
				}
				temp++;
			}
			if( temp->cmd != _TRACK_END ) {
				while( ptr->cmd != _TRACK_END ) {
					if( ptr->cmd == _CONT_END ) {
						ptr++;
						break;
					} else if( ptr->cmd <= MAX_NOTE || ptr->cmd == _REST || ptr->cmd == _KEY
						|| ptr->cmd == _NOTE || ptr->cmd == _WAIT || temp->cmd == _KEY_OFF ) {
						gate += len;
						(*cmd)->filename = ptr->filename;
						(*cmd)->cnt      = ptr->cnt;
						(*cmd)->frm      = ptr->frm;
						(*cmd)->line     = ptr->line;
						(*cmd)->cmd      = ptr->cmd;
						(*cmd)->len      = len;
						for( i = 0; i < 8; i++ ) { 
							(*cmd)->param[i] = ptr->param[i];
						}
						gate -= (*cmd)->len;
					} else if (ptr->cmd == _TIE) {
						/* Ϣ����Υ����Ϻ�� */
						(*cmd)->filename = ptr->filename;
						(*cmd)->cnt      = 0;
						(*cmd)->frm      = 0;
						(*cmd)->line     = ptr->line;
						(*cmd)->cmd      = _NOP;
						(*cmd)->len      = 0;
					} else {
						(*cmd)->filename = ptr->filename;
						(*cmd)->cnt      = ptr->cnt;
						(*cmd)->frm      = ptr->frm;
						(*cmd)->line     = ptr->line;
						(*cmd)->cmd      = ptr->cmd;
						(*cmd)->len      = ptr->len;
						for( i = 0; i < 8; i++ ) { 
							(*cmd)->param[i] = ptr->param[i];
						}
					}
					(*cmd)++;
					ptr++;
				}

			}
			break;
		  case _TRACK_END:
			(*cmd)->filename = ptr->filename;
			(*cmd)->cnt      = ptr->cnt;
			(*cmd)->frm      = ptr->frm;
			(*cmd)->line     = ptr->line;
			(*cmd)->cmd      = ptr->cmd;
			(*cmd)->len      = ptr->len;
			for( i = 0; i < 8; i++ ) { 
				(*cmd)->param[i] = ptr->param[i];
			}
			(*cmd)++;
			ptr++;
			if( nest != 0 ) {
				dispError( DATA_ENDED_BY_LOOP_DEPTH_EXCEPT_0, mml_names[mml_idx], (ptr-1)->line );
			}
			return NULL;
		  default:
			(*cmd)->filename = ptr->filename;
			(*cmd)->cnt      = ptr->cnt;
			(*cmd)->frm      = ptr->frm;
			(*cmd)->line     = ptr->line;
			(*cmd)->cmd      = ptr->cmd;
			(*cmd)->len      = ptr->len;
			for( i = 0; i < 8; i++ ) { 
				(*cmd)->param[i] = ptr->param[i];
			}
			(*cmd)++;
			ptr++;
			break;
		}
	}
}


/*--------------------------------------------------------------
	
 Input:
	
 Output:
	̵��
--------------------------------------------------------------*/
void putAsm( FILE *fp, int data )
{
	static char *fn = "";
	static int ln = 0;
	if( putAsm_pos == 0 ) {
		fn = mml_file_name;
		ln = mml_line_pos;
		fprintf( fp, "\tdb\t$%02x", data&0xff );
	} else if( putAsm_pos == 7 ) {
		fprintf( fp, ",$%02x",  data&0xff );
		fprintf( fp, "\t;Trk %c; %s: %d", str_track[mml_trk], fn, ln);
		fprintf( fp, "\n");
	} else {
		fprintf( fp, ",$%02x",  data&0xff );
	}
	if( ++putAsm_pos > 7 ) {
		putAsm_pos = 0;
	}
	bank_usage[curr_bank]++;
}


/*--------------------------------------------------------------
	
--------------------------------------------------------------*/
void putBankOrigin(FILE *fp, int bank)
{
	static int bank_org_written_flag[128] = {1};
	int org;
	if (bank > 127) {
		//assert(0);
		return;
	}
	if (bank_org_written_flag[bank] == 0) {
		switch (bank) {
		case 0:
			org = 0x8000;
			//assert(0);
			break;
		case 1:
			org = 0xa000;
			break;
		case 2:
			org = 0xc000;
			break;
		case 3:
			org = 0xe000;
			break;
		default:
			org = 0xa000;
			break;
		}
		fprintf(fp, "\t.org\t$%04x\n", org);
		bank_org_written_flag[bank] = 1;
		if (bank > bank_maximum) {
			bank_maximum = bank;
		}
	}

}

/*--------------------------------------------------------------
	!=0: OK,  ==0: out of range
--------------------------------------------------------------*/
int checkBankRange(int bank)
{
	if (allow_bankswitching) {
		if (bank < 0 || bank > 127) {
			return 0;
		}
	} else {
		if (bank < 0 || bank > 3) {
			return 0;
		}
	}
	return 1;
}



/*--------------------------------------------------------------
	
 Input:
	
 Output:
	̵��
--------------------------------------------------------------*/
int double2int(double d)
{
	return (int)(d + 0.5);
}


/*******************************************************
 *
 *��ȯ��           ����������
 *    _                  _
 *   | ��               | �� ���β�(�Ȥ����٥��)
 *  |    ��_________   |    ��_________
 * |                ��|                ��
 * |                  |                  ��
 * <------------------> delta_time ȯ�����鼡�Υ��٥�Ȥޤ�
 * <--------------->    gate_time  ȯ�����饭�����դޤ�
 *                  <-> left_time  �������դ��鼡�Υ��٥�ȤޤǤλĤ����
 *
 *******************************************************/

/*--------------------------------------------------------------
	���顼���������θ�����ǥ륿�����������
 Input:
	CMD *cmd; �ǥ륿��������ɤ߻Ϥ�륳�ޥ�ɤΰ���
	int allow_slur = 1; ���顼����(����ξ��)
	               = 0; ���顼�ʤ�(����Ȥ�)
 Output:
	int *delta; �ǥ륿������
 Return:
	CMD *cmd; ���δؿ����cmd���ɤ߿ʤ᤿�Τǡ�������cmd���֤��֤�
--------------------------------------------------------------*/
CMD *getDeltaTime(CMD *cmd, int *delta, int allow_slur) {
	*delta = 0;
	while( 1 ) {
		if( loop_flag == 0 ) {
			*delta += ((cmd+1)->frm - cmd->frm);
		} else {
			*delta += ((cmd+1)->lfrm - cmd->lfrm);
		}
		cmd++;
		if( cmd->cmd == _SLAR && allow_slur) {
			cmd++;
		} else if( cmd->cmd != _TIE ) {
			break;
		}
	}
	return cmd;
}

/*--------------------------------------------------------------
	q�Ȳ�Ĺ���饲���ȥ�����׻�
 Input:
 
 Output:
	
 Return:
	int gate;
--------------------------------------------------------------*/
int calcGateTime(int delta_time, const GATE_Q *gate_q) {
	int gate;
	gate = (delta_time * gate_q->rate) / gate_denom + gate_q->adjust;
	if (gate > delta_time) {
		gate = delta_time;
	} else if (gate < 0) {
		gate = 0;
	}
	if ( delta_time != 0 && gate <= 0 ) {
		gate = 1;
	}
	return gate;
}

/*--------------------------------------------------------------
	��Ĺ�Τ��륳�ޥ�ɤΡ���Ĺ��ʬ�ν���(256�ե졼��ʾ�ΤȤ��ν���)
 Input:
	int wait_com_no; 256�ե졼��ʾ�ΤȤ��˷Ҥ����ޥ��(w��r)
	int len; �ե졼�಻Ĺ
 Output:
	
--------------------------------------------------------------*/
void putLengthAndWait(FILE *fp, int wait_com_no, const int len, const CMD *cmd) {
	int len_nokori = len; /* ���Ϥ��٤��Ĥ겻Ĺ(�ե졼���) */

	if (len == 0) {
		dispWarning( FRAME_LENGTH_IS_0, cmd->filename, cmd->line );
		return;
	} else if (len < 0) {
		dispError(FRAME_LENGTH_LESSTHAN_0, cmd->filename, cmd->line);
		return;
	}

	if( len_nokori > 0xff ) {
		putAsm( fp, 0xff );
		len_nokori -= 0xff;
	} else {
		putAsm( fp, len_nokori );
		len_nokori = 0;
	}
	while (len_nokori != 0) { /* ���Ϥ��٤��Ĥ�Υե졼�����0�ˤʤ�ޤǥ�ԡ��� */
		if( len_nokori > 0xff ) {
			/* �Ĥ�256�ե졼��ʾ�ΤȤ� */
			putAsm( fp, wait_com_no ); 
			putAsm( fp, 0xff ); /* 255�ե졼����� */
			len_nokori -= 0xff;
		} else {
			/* 255�ե졼��ʲ��ΤȤ� */
			putAsm( fp, wait_com_no );
			putAsm( fp, len_nokori ); /* �Ĥ��������� */
			len_nokori = 0;
		}
	}
}


typedef struct {
	GATE_Q	gate_q;
	int	env;			// ���ߤ��̾��(��������ΤȤ���)����٥����ֹ�or����
	int	rel_env;		// ���ߤΥ�꡼������٥����ֹ�(-1:̤����)
	int	last_written_env;	// �Ǹ�˽񤭹��������٥����ֹ�or����
	int	tone;			// 
	int	rel_tone;		// 
	int	last_written_tone;	//
	int	key_pressed;		// �������󥪥դξ���
	int	last_note[SELF_DELAY_MAX+1];		// �Ǹ�˽񤤤��Ρ���(@n��̵���)
	int	last_note_keep[SELF_DELAY_MAX+1];	// \���ޥ�ɻ��ѻ���last_note����
	int	self_delay;		// ���������ΥΡ��Ȥ���Ѥ��뤫������ʤ饻��եǥ��쥤���ʤ���
} PLAYSTATE;

void defaultPlayState(PLAYSTATE *ps)
{
	int i;
	ps->gate_q.rate = gate_denom;
	ps->gate_q.adjust = 0;
	ps->env = -1;
	ps->rel_env = -1;
	ps->last_written_env = -1;
	ps->tone = -1;
	ps->rel_tone = -1;
	ps->last_written_tone = -1;
	ps->key_pressed = 0;
	for (i = 0; i < arraysizeof(ps->last_note); i++) {
		ps->last_note[i] = -1;
		ps->last_note_keep[i] = -1;
	}
	ps->self_delay = -1;
}

/*--------------------------------------------------------------
	��꡼������٥��ס��������ϡ��Ĥ���֤�r��w������
 Input:
	*cmd putLengthWait�˥��顼ɽ�������뤿�������¸�ߤ���
--------------------------------------------------------------*/
void putReleaseEffect(FILE *fp, const int left_time, const CMD *cmd, PLAYSTATE *ps)
{
	int note = MCK_REST;		//�ǥե���ȤϻĤ���֤ϵ���ǤĤʤ�
	
	//��ť������ե����å�
	if (ps->key_pressed == 0) {
		putAsm(fp, note);
		putLengthAndWait(fp, MCK_WAIT, left_time, cmd);
		return;
	}
	
	if( (ps->rel_env != -1 )		// ��꡼������٥���ư����
	 && (ps->last_written_env != ps->rel_env) ) {	// ���ߤΥ���٥��פ��Ѵ���Υ���٥��פ��㤦
		putAsm( fp, MCK_SET_VOL );	// ��꡼������٥��׽���
		putAsm( fp, ps->rel_env );
	 	ps->last_written_env = ps->rel_env;
		note = MCK_WAIT;		//�Ĥ���֤ϥ�������
	}
	if( (ps->rel_tone != -1 )		// ��꡼������ư����
	 && (ps->last_written_tone != ps->rel_tone) ) {	// ���ߤΥ���٥��פ��Ѵ���β������㤦
		putAsm( fp, MCK_SET_TONE );	// ��꡼����������
		putAsm( fp, ps->rel_tone );
	 	ps->last_written_tone = ps->rel_tone;
		note = MCK_WAIT;		//�Ĥ���֤ϥ�������
	}
	if (note == MCK_WAIT && ps->self_delay >= 0 && ps->last_note[ps->self_delay] >= 0) {
		/* ����եǥ��쥤 */
		note = ps->last_note[ps->self_delay];
	}
	if (left_time != 0) {
		putAsm(fp, note);
		putLengthAndWait(fp, note, left_time, cmd);
	}
}



void doNewBank(FILE *fp, int trk, const CMD *cmd)
{
	int banktemp = curr_bank;
	if (cmd->param[0] == PARAM_OMITTED) {
		/* �ǥե���� */
		banktemp++;
	} else {
		banktemp = cmd->param[0];
	}
	if (checkBankRange(banktemp) == 0) {
		dispError( BANK_IDX_OUT_OF_RANGE,  cmd->filename, cmd->line );
		return;
	}
	if ((banktemp == 2 || banktemp == 3) && dpcm_bankswitch) {
		dispError( CANT_USE_BANK_2_OR_3_WITH_DPCMBANKSWITCH,  cmd->filename, cmd->line );
		return;
	}
	putAsm( fp, MCK_GOTO );
	fprintf( fp,"\n\tdb\tbank(%s_%02d_bnk%03d)*2\n",songlabel,trk,banktemp);
	bank_usage[curr_bank]++;
	fprintf( fp,"\tdw\t%s_%02d_bnk%03d\n",songlabel,trk,banktemp);
	bank_usage[curr_bank]+=2;
	fprintf( fp,"\n\t.bank\t%d\n",banktemp);
	curr_bank = banktemp;
	putBankOrigin(fp, curr_bank);
	fprintf( fp,"%s_%02d_bnk%03d:\n",songlabel,trk,curr_bank);
	putAsm_pos = 0; // ���ϰ��֥��ꥢ
	return;
}







/*--------------------------------------------------------------
	
 Input:
	
 Output:
	̵��
--------------------------------------------------------------*/
void developeData( FILE *fp, const int trk, CMD *const cmdtop, LINE *lptr )
{
	tbase = 0.625;
	length = 48;
	volume_flag = -1;

	{
		/* �ƥ�ݥ��������� */
		CMD *cmd = cmdtop;
		CMD *temp = malloc( sizeof(CMD)*32*1024 );
		CMD *const tempback = temp;
		int i, j;
		for( i = 0; i < 32*1024; i++ ) {
			temp->cmd = 0;
			temp->cnt = 0;
			temp->frm = 0;
			temp->line = 0;
			for( j = 0; j < 8; j++ ) {
				temp->param[0] = 0;
			}
			temp++;
		}
		temp = tempback;
		/* �����ͥ�ǡ�����Ƭ���饳�ޥ�ɤ���ϡ��Хåե��ˤ���� */
		temp = analyzeData( trk, temp, lptr );
		setCommandBuf( 0, temp, _TRACK_END, NULL, 0, 1 );
		temp = tempback;
		//shuffleQuontize(temp);
		nest = 0;
		translateData( &cmd, temp );
		cmd = cmdtop;
		free( tempback );
	}

	tbase = 0.625;
	
	{
		CMD *cmd = cmdtop;
		double	count, lcount, count_t;
		int		frame, lframe, frame_p, frame_d;
		double	tbase_p;

		/* ������Ȥ���ե졼����Ѵ� */
		/* �ʤ�٤�����Τ������������ˤ��� */
		loop_flag = 0;
		
		count = 0; //�ȥ�å����ϻ�������ηвᥫ����ȿ�
		frame = 0; //�ȥ�å����ϻ�������ηв�ե졼���
		lcount = 0; //�롼�׳��ϻ�������ηвᥫ����ȿ�
		lframe = 0; //�롼�׳��ϻ�������ηв�ե졼���
		/*
			������Ȥϥƥ�ݴط��ʤ��û����Ƥ���
			�ե졼���
			      A t120 l4 c  d   e   f  t240   g   a   b   c   !
			count:          0 48  96 144  192  192 240 288 336 384
			frame:          0 30  60  90  120  120 135 150 165 180
			tbase:      0.625           0.3125
			count_t:        0 48  96 144  192  384 432 480 528 576
			      B t240 l4 cc dd ee ff          g   a   b   c   !
		*/
		count_t = 0; //�ǽ餫�麣�ޤǸ��ߤΥƥ�ݤ��ä��Ȳ��ꤷ���������ߤξ��֤�Ʊ�����֤�вᤵ���뤿��Υ�����ȿ�
		do {
			cmd->cnt = count;
			cmd->frm = frame;
			cmd->lcnt = lcount;
			cmd->lfrm = lframe;

	//		printf("%s:%d:%4x %f %d %f\n", cmd->filename, cmd->line, cmd->cmd, cmd->cnt, cmd->frm, cmd->len);

			if( cmd->cmd == _REPEAT_ST2 ) {
				double	rcount = 0;
				double	rcount_esc = 0;		// \�μ����ޤ�
				double	rcount_t = 0;
				double	rcount_esc_t = 0;
				int	rframe = 0;
				int	rframe_esc = 0;
				int	rframe_err;
				int	repeat_esc_flag = 0;
				CMD	*repeat_esc2_cmd_ptr = NULL;
				
				cmd++;
				while( 1 ) {
					cmd->cnt = count;
					cmd->frm = frame;
					cmd->lcnt = lcount;
					cmd->lfrm = lframe;
					if( cmd->cmd == _REPEAT_END2 ) {
						count_t += rcount_t*(cmd->param[0]-2)+rcount_esc_t;
						count += rcount*(cmd->param[0]-2)+rcount_esc;
						frame += rframe*(cmd->param[0]-2)+rframe_esc;
						if( loop_flag != 0 ) {
							lcount += rcount*(cmd->param[0]-2)+rcount_esc;
							lframe += rframe*(cmd->param[0]-2)+rframe_esc;
						}
						/* �ե졼������ */
						rframe_err = double2int(count_t * tbase) - frame;
						//printf( "frame-error: %d frame\n", rframe_err );
						if (rframe_err > 0) {
							//printf( "frame-correct: %d frame\n", rframe_err );
							if (rframe_err >= 3)
							{
								dispWarning(REPEAT2_FRAME_ERROR_OVER_3, cmd->filename, cmd->line);
							}
							/* 2004.09.02 ��äѤ����
							cmd->param[1] = rframe_err;
							frame += rframe_err;
							if( loop_flag != 0 ) {
								lframe += rframe_err;
							}
							*/
						} else {
							cmd->param[1] = 0;
						}
						if (repeat_esc_flag) {
							// �����֤�������б�����\\���ޥ�ɤˤ�
							repeat_esc2_cmd_ptr->param[0] = cmd->param[0];
						}
						break;
						
					} else if( cmd->cmd == _REPEAT_ESC2 ) {
						repeat_esc_flag = 1;
						repeat_esc2_cmd_ptr = cmd;
					} else if( cmd->cmd <= MAX_NOTE || cmd->cmd == _REST || cmd->cmd == _TIE
							|| cmd->cmd == _KEY || cmd->cmd == _NOTE || cmd->cmd == _WAIT || cmd->cmd == _KEY_OFF ) {
						count_t += cmd->len;
						rcount_t += cmd->len;
						frame_p = rframe;
						rframe = double2int(rcount_t * tbase);
						frame_d = rframe - frame_p;
						count += cmd->len;
						frame += frame_d;
/* �Х롼�פ����к� */
						if( loop_flag != 0 ) {
							lcount += cmd->len;
							lframe += frame_d;
						}
						rcount += cmd->len;
						if( repeat_esc_flag == 0 ) {
							rcount_esc_t += cmd->len;
							rcount_esc += cmd->len;
							rframe_esc += frame_d;
						} 
					} else if( cmd->cmd == _TEMPO ) {
						tbase_p = tbase;
						tbase = (double)_BASETEMPO / (double)cmd->param[0];
						count_t = count_t * tbase / tbase_p;
						rcount_t = rcount_t * tbase / tbase_p;
						rcount_esc_t = rcount_esc_t * tbase / tbase_p;
					} else if( cmd->cmd == _TEMPO2 ) {
						tbase_p = tbase;
						tbase = (double)cmd->param[0] * (double)cmd->param[1] / _BASE;
						count_t = count_t * tbase / tbase_p;
						rcount_t = rcount_t * tbase / tbase_p;
						rcount_esc_t = rcount_esc_t * tbase / tbase_p;
					} else if( cmd->cmd == _SONG_LOOP ) {
						loop_flag = 1;
					}
					cmd++;
				}
			} else if( cmd->cmd <= MAX_NOTE || cmd->cmd == _REST || cmd->cmd == _TIE
					|| cmd->cmd == _KEY || cmd->cmd == _NOTE || cmd->cmd == _WAIT || cmd->cmd == _KEY_OFF ) {
				count_t += cmd->len;
				frame_p = frame;
				frame = double2int(count_t * tbase);
				frame_d = frame - frame_p;
				count += cmd->len;
	/* �Х롼�פ����к� */
				if( loop_flag != 0 ) {
					lcount += cmd->len;
					lframe += frame_d;
				}
			} else if( cmd->cmd == _TEMPO ) {
				tbase_p = tbase;
				tbase = (double)_BASETEMPO / (double)cmd->param[0];
				count_t = count_t * tbase_p / tbase;
			} else if( cmd->cmd == _TEMPO2 ) {
				tbase_p = tbase;
				tbase = (double)cmd->param[0] * (double)cmd->param[1] / _BASE;
				count_t = count_t * tbase_p / tbase;
			} else if( cmd->cmd == _SONG_LOOP ) {
				loop_flag = 1;
			}
		} while( cmd++->cmd != _TRACK_END );
	}
	
	{
		CMD *cmd = cmdtop;
		PLAYSTATE ps;
		int repeat_depth = 0;
		int repeat_index = 0;
		int repeat_esc_flag = 0;
		int i;
		char loop_point_label[256];
		
		defaultPlayState(&ps);
		
		cmd = cmdtop;
		putAsm_pos = 0;
		loop_flag = 0;
		
		sprintf(loop_point_label, "%s_%02d_lp", songlabel, trk );
		
		mml_trk = trk;
		fprintf( fp, "\n%s_%02d:\t;Trk %c\n", songlabel, trk, str_track[trk] );
		
		mml_file_name = cmd->filename;
		mml_line_pos = cmd->line;
		
		// ������/�Υ����ȥ�å��к�
		if( (trk == BTRACK(2)) || (trk == BTRACK(3)) ) {
			putAsm( fp, MCK_SET_TONE );
			putAsm( fp, 0x8f );
		}		
		
		
		
		do {
			const CMD cmdtemp = *cmd; //��switch���cmd�ݥ��󥿤��ʤ��ǽ��������Τǰ�ö��¸
			mml_file_name = cmd->filename;
			mml_line_pos = cmd->line;
			
			// ��ư�Х��ڤ��ؤ�
			if (auto_bankswitch) {
				const int bank_limit = 8192 - 20; // Ŭ����;͵���������
				if (bank_usage[curr_bank] > bank_limit) {
					CMD nbcmd;
					nbcmd.param[0] = curr_bank;
					while (bank_usage[ nbcmd.param[0] ] > bank_limit) {
						nbcmd.param[0]++;
					}
					nbcmd.filename = cmd->filename;
					nbcmd.line = cmd->line;
					doNewBank( fp, trk, &nbcmd );
				}
			}
			
			switch (cmdtemp.cmd) {
			  case _NOP:
			  case _TEMPO:
			  case _TEMPO2:
			  case _OCTAVE:
			  case _OCT_UP:
			  case _OCT_DW:
			  case _LENGTH:
			  case _TRANSPOSE:
				cmd++;
				break;
			  case _ENVELOPE:
				putAsm( fp, MCK_SET_VOL );
				ps.env = cmd->param[0]&0x7f; 
				ps.last_written_env = ps.env;
				putAsm( fp, ps.env ); 
				ps.last_written_env = ps.env;
				cmd++; 
				break; 
			  case _REL_ENV: 
				if( cmd->param[0] == 255 ) { 
					ps.rel_env = -1;
				} else { 
					ps.rel_env = cmd->param[0]&0x7f; 
				}
				cmd++;
				break;
			  case _VOLUME:
				putAsm( fp, MCK_SET_VOL );
				if( trk == BFMTRACK || trk == BVRC6SAWTRACK) {
					ps.env = (cmd->param[0]&0x3f)|0x80;
				} else {
					ps.env = (cmd->param[0]&0x0f)|0x80;
				}
				putAsm( fp, ps.env );
				ps.last_written_env = ps.env;
				cmd++;
				break;
			  case _HARD_ENVELOPE:
				putAsm( fp, MCK_SET_FDS_HWENV );
				ps.env = ((cmd->param[0]&1)<<6)|(cmd->param[1]&0x3f|0x100);
				putAsm( fp, (ps.env & 0xff) ); 
				ps.last_written_env = ps.env;
				cmd++;
				break;
			  case _TONE:
				ps.tone = cmd->param[0]|0x80;
				putAsm( fp, MCK_SET_TONE );
				putAsm( fp, ps.tone );
				ps.last_written_tone = ps.tone;
				cmd++;
				break;
			  case _ORG_TONE:
				ps.tone = cmd->param[0]&0x7f;
				putAsm( fp, MCK_SET_TONE );
				putAsm( fp, ps.tone );
				ps.last_written_tone = ps.tone;
				cmd++;
				break;
			  case _REL_ORG_TONE:
				if( cmd->param[0] == 255 ) { 
					ps.rel_tone = -1;
				} else {
					ps.rel_tone = cmd->param[0]&0x7f;
				}
				cmd++;
				break;
			  case _SONG_LOOP:
				//loop_count.cnt = cmd->cnt; //LEN
				//loop_count.frm = cmd->frm;
				fprintf( fp, "\n%s:\n", loop_point_label);
				loop_flag = 1;
				putAsm_pos = 0;
				cmd++;
				break;
			  case _QUONTIZE:
				ps.gate_q.rate = cmd->param[0];
				ps.gate_q.adjust = cmd->param[1];
				cmd++;
				break;
			  case _QUONTIZE2:
				ps.gate_q.rate = gate_denom;
				ps.gate_q.adjust = - cmd->param[0];
				cmd++;
				break;
			  case _REST:
				{
					int delta_time = 0;
					cmd = getDeltaTime(cmd, &delta_time, 0);
					if( delta_time == 0 ) {
						dispWarning( FRAME_LENGTH_IS_0, cmdtemp.filename, cmdtemp.line );
						break;
					}
					putAsm(fp, MCK_REST);
					putLengthAndWait(fp, MCK_REST, delta_time, &cmdtemp);
					ps.key_pressed = 0;
				}
				break;
			  case _WAIT:
				{
					int delta_time = 0;
					cmd = getDeltaTime(cmd, &delta_time, 0);
					if( delta_time == 0 ) {
						dispWarning( FRAME_LENGTH_IS_0, cmdtemp.filename, cmdtemp.line );
						break;
					}
					putAsm(fp, MCK_WAIT);
					putLengthAndWait(fp, MCK_WAIT, delta_time, &cmdtemp);
				}
				break;
			  case _KEY_OFF: /* Ĺ���Ĥ��������� */ 
				{
					int delta_time = 0;
					cmd = getDeltaTime(cmd, &delta_time, 0);
					if( delta_time == 0 ) {
						/* ��Ĺ0����� */
					}
					putReleaseEffect(fp, delta_time, &cmdtemp, &ps);
					ps.key_pressed = 0;
				}
				break;
			  case _LFO_ON:
				putAsm( fp, MCK_SET_LFO );
				if( (cmd->param[0]&0xff) == 0xff ) {
					putAsm( fp, 0xff );
				} else {
					putAsm( fp, cmd->param[0]&0x7f );
				}
				cmd++;
				break;
			  case _LFO_OFF:
				putAsm( fp, MCK_SET_LFO );
				putAsm( fp, 0xff );
				cmd++;
				break;
			  case _DETUNE:
				putAsm( fp, MCK_SET_DETUNE );
				if( cmd->param[0] >= 0 ) {
					putAsm( fp, ( cmd->param[0]&0x7f)|0x80 );
				} else {
					putAsm( fp, (-cmd->param[0])&0x7f );
				}
				cmd++;
				break;
			  case _SWEEP:
				putAsm( fp, MCK_SET_HWSWEEP );
				putAsm( fp, ((cmd->param[0]&0xf)<<4)+(cmd->param[1]&0xf) );
				cmd++;
				break;
			  case _EP_ON:
				putAsm( fp, MCK_SET_PITCHENV );
				putAsm( fp, cmd->param[0]&0xff );
				cmd++;
				break;
			  case _EP_OFF:
				putAsm( fp, MCK_SET_PITCHENV );
				putAsm( fp, 0xff );
				cmd++;
				break;
			  case _EN_ON:
				putAsm( fp, MCK_SET_NOTEENV );
				putAsm( fp, cmd->param[0]&0xff );
				cmd++;
				break;
			  case _EN_OFF:
				putAsm( fp, MCK_SET_NOTEENV );
				putAsm( fp, 0xff );
				cmd++;
				break;
			  case _MH_ON:
				putAsm( fp, MCK_SET_FDS_HWEFFECT );
				putAsm( fp, cmd->param[0]&0xff );
				cmd++;
				break;
			  case _MH_OFF:
				putAsm( fp, MCK_SET_FDS_HWEFFECT );
				putAsm( fp, 0xff );
				cmd++;
				break;
			  case _VRC7_TONE:
				putAsm( fp, MCK_SET_TONE );
				putAsm( fp, cmd->param[0]|0x40 );
				cmd++;
				break;
			  case _SUN5B_HARD_SPEED:
				putAsm( fp, MCK_SET_SUN5B_HARD_SPEED );
				putAsm( fp, cmd->param[0]&0xff );
				putAsm( fp, (cmd->param[0]>>8)&0xff );
				cmd++;
				break;
			  case _SUN5B_HARD_ENV:
				putAsm( fp, MCK_SUN5B_HARD_ENV );
				ps.env = (cmd->param[0]&0x0f)|0x10|0x80;
				putAsm( fp,  ps.env );
				cmd++;
				break;
			  case _SUN5B_NOISE_FREQ:
				putAsm( fp, MCK_SET_SUN5B_NOISE_FREQ );
				putAsm( fp, cmd->param[0]&0x1f );
				cmd++;
				break;
			  case _NEW_BANK:
				doNewBank( fp, trk, cmd );
				cmd++;
				break;
			  case _DATA_WRITE:
				putAsm( fp, MCK_DATA_WRITE );
				putAsm( fp,  cmd->param[0]    &0xff );
				putAsm( fp, (cmd->param[0]>>8)&0xff );
				putAsm( fp,  cmd->param[1]    &0xff );
				cmd++;
				break;
			  case _DATA_THRUE:
				putAsm( fp,  cmd->param[0]    &0xff );
				putAsm( fp,  cmd->param[1]    &0xff );
				cmd++;
				break;
			  case _REPEAT_ST2:
				fprintf( fp, "\n%s_%02d_lp_%04d:\n", songlabel, trk, repeat_index );
				repeat_depth++;
				putAsm_pos = 0;
				cmd++;
				break;
			  case _REPEAT_END2:
				if( --repeat_depth < 0 ) {
					dispError( DATA_ENDED_BY_LOOP_DEPTH_EXCEPT_0, cmd->filename, cmd->line );
				} else {
					if (repeat_esc_flag != 0) {
						// ������
						putAsm( fp, MCK_GOTO );
					} else {
						putAsm( fp, MCK_REPEAT_END );
						putAsm( fp, cmd->param[0]&0x7f );
					}
					fprintf( fp,"\n\tdb\tbank(%s_%02d_lp_%04d)*2\n", songlabel, trk, repeat_index );
					bank_usage[curr_bank]++;
					fprintf( fp,"\tdw\t%s_%02d_lp_%04d\n", songlabel, trk, repeat_index );
					bank_usage[curr_bank]+=2;
					
					fprintf( fp, "%s_%02d_lp_exit_%04d:\n", songlabel, trk, repeat_index );
					repeat_index++;
					putAsm_pos = 0;
					/* 2004.09.02 ��äѤ����
					if ( cmd->param[1] > 0 ) {
						putAsm( fp, MCK_WAIT );
						putAsm( fp, cmd->param[1]&0xFF);
					} */
					if (repeat_esc_flag != 0) {
						for (i = 0; i < arraysizeof(ps.last_note); i++) {
							ps.last_note[i] = ps.last_note_keep[i];
						}
						repeat_esc_flag = 0;
					}
				}
				cmd++;
				break;
			  case _REPEAT_ESC2:
				if( (repeat_depth-1) < 0 ) {
					dispError( DATA_ENDED_BY_LOOP_DEPTH_EXCEPT_0, cmd->filename, cmd->line );
				} else {
					putAsm( fp, MCK_REPEAT_ESC );
					putAsm( fp, cmd->param[0]&0x7f );
					fprintf( fp,"\n\tdb\tbank(%s_%02d_lp_exit_%04d)*2\n", songlabel, trk, repeat_index );
					bank_usage[curr_bank]++;
					fprintf( fp, "\tdw\t%s_%02d_lp_exit_%04d\n", songlabel, trk, repeat_index );
					bank_usage[curr_bank]+=2;
					putAsm_pos = 0;
					repeat_esc_flag = 1;
					for (i = 0; i < arraysizeof(ps.last_note); i++) {
						ps.last_note_keep[i] = ps.last_note[i];
					}
				}
				cmd++;
				break;
			  case _SELF_DELAY_ON:
				if( cmd->param[0] == 255 ) { 
					ps.self_delay = -1;
				} else {
					ps.self_delay = cmd->param[0];
				}
				cmd++;
				break;
			  case _SELF_DELAY_OFF:
				ps.self_delay = -1;
				cmd++;
				break;
			  case _SELF_DELAY_QUEUE_RESET:
				for (i = 0; i < arraysizeof(ps.last_note); i++) {
					ps.last_note[i] = -1;
					ps.last_note_keep[i] = -1;
				}
				cmd++;
				break;
			  case _SHIFT_AMOUNT:
				putAsm( fp, MCK_SET_SHIFT_AMOUNT );
				putAsm( fp, cmd->param[0] & 0xff );
				cmd++;
				break;
			  case _TRACK_END:
				break;
			  case _KEY:
			  default:
				{
					int note;
					int delta_time; /* ȯ�����鼡�Υ��٥�ȤޤǤΥե졼��� */
					int gate_time; /* ȯ�����饭�����դޤǤΥե졼��� */
					int left_time; /* �������դ��鼡�Υ��٥�ȤޤǤλĤ�ե졼��� */
					
					if (cmdtemp.cmd == _KEY) {
						note = cmd->param[0]&0xffff;
					} else {
						note = cmdtemp.cmd;
						if (note < MIN_NOTE || MAX_NOTE < note) {
							dispError( COMMAND_NOT_DEFINED, cmd->filename, cmd->line );
							cmd++;
							break;
						}
					}
					
					
					delta_time = 0;
					cmd = getDeltaTime(cmd, &delta_time, 1);
					
					gate_time = calcGateTime(delta_time, &(ps.gate_q));
					left_time = delta_time - gate_time;
					
					if( delta_time == 0 ) {
						dispWarning( FRAME_LENGTH_IS_0, cmdtemp.filename, cmdtemp.line );
						break;
					}
					
					
					if( ps.last_written_env != ps.env ) {		// �Ǹ�˽񤭹��������٥���or���̤ȡ����ߤ��̾�Υ���٥���or���̤��㤦
						if ( (trk == BFMTRACK) && (ps.env > 0xFF) ) {
							putAsm( fp, MCK_SET_FDS_HWENV );	// �ϡ��ɥ���ٽ���
							putAsm( fp, (ps.env & 0xff) );
						} else {
							putAsm( fp, MCK_SET_VOL );	// ����٥��׽���
							putAsm( fp, ps.env );
						}
						ps.last_written_env = ps.env;
					}
					
					if( ps.last_written_tone != ps.tone ) {	// �Ǹ�˽񤭹���������ȡ����ߤ��̾�β������㤦
						putAsm( fp, MCK_SET_TONE );	// ��������
						putAsm( fp, ps.tone );
						ps.last_written_tone = ps.tone;
					}

					if( (ps.tone == -1) &&
					    ((trk == BTRACK(0))  || (trk == BTRACK(1)) ||
					     (trk == BMMC5TRACK) || (trk == BMMC5TRACK+1)) ) {
						// ��¢����ȡ�MMC5�ϲ���̤�����@0��
						putAsm( fp, MCK_SET_TONE );
						ps.tone = 0x80;
						putAsm( fp, ps.tone );
						ps.last_written_tone = ps.tone;
					}
					
					if (cmdtemp.cmd == _KEY) {
						putAsm( fp, MCK_DIRECT_FREQ );
						putAsm( fp,  note    &0xff );
						if ( ((trk >= BVRC6TRACK) && (trk <= BVRC6SAWTRACK)) ||
						     ((trk >= BFME7TRACK) && (trk <= BFME7TRACK+2 )) ) {
							// VRC6��SUN5B��12bit
							putAsm( fp, (note>>8)&0x0f );
						} else {
							// 2A03��MMC5��11bit
							putAsm( fp, (note>>8)&0x07 );
						}
					} else {
						if( note < 0 ) {				/* ���㲻���к� */
							note += 16;
						}
						putAsm( fp, note );
						
						
						for (i = arraysizeof(ps.last_note) - 1 ; i > 0; i--) {
							ps.last_note[i] = ps.last_note[i-1];
						}
						ps.last_note[0] = note;
					}
					
					
					putLengthAndWait(fp, MCK_WAIT, gate_time, &cmdtemp);
					ps.key_pressed = 1;
					
					// �����󥿥�������
					if ( left_time != 0 ) {
						putReleaseEffect(fp, left_time, &cmdtemp, &ps);
						ps.key_pressed = 0;
					}
				}
				break;
			} // switch (cmdtemp.cmd)
			
			
		} while( cmd->cmd != _TRACK_END );
		
		
		track_count[mml_idx][trk][0].cnt = cmd->cnt;
		track_count[mml_idx][trk][0].frm = cmd->frm;

		if( loop_flag == 0 ) {
			track_count[mml_idx][trk][1].cnt = 0;
			track_count[mml_idx][trk][1].frm = 0;

			fprintf( fp, "\n%s:\n", loop_point_label );
			putAsm_pos = 0;
			putAsm( fp, MCK_REST );
			putAsm( fp, 0xff );
		} else {
			track_count[mml_idx][trk][1].cnt = cmd->lcnt;
			track_count[mml_idx][trk][1].frm = cmd->lfrm;
		}
		// putAsm( fp, MCK_DATA_END );
		putAsm( fp, MCK_GOTO );
		fprintf( fp,"\n\tdb\tbank(%s)*2\n", loop_point_label );
		bank_usage[curr_bank]++;
		fprintf( fp,"\tdw\t%s\n", loop_point_label );
		bank_usage[curr_bank]+=2;
		fputc( '\n', fp );
	}
}


/*--------------------------------------------------------------
	
--------------------------------------------------------------*/
void setSongLabel(void)
{
	sprintf(songlabel, "song_%03d", mml_idx);
}


/*--------------------------------------------------------------
	�ꥶ���ɽ���롼����
	i:trk number
	trk: track symbol
--------------------------------------------------------------*/

void display_counts_sub(int i, char trk)
{
	printf( "   %c   |", trk);
	if( track_count[mml_idx][i][0].cnt != 0 ) {
		printf (" %6d   %5d|", double2int(track_count[mml_idx][i][0].cnt), track_count[mml_idx][i][0].frm );
	} else {
		printf( "               |" );
	}
	if( track_count[mml_idx][i][1].cnt != 0 ) {
		printf (" %6d   %5d|\n", double2int(track_count[mml_idx][i][1].cnt),track_count[mml_idx][i][1].frm );
	} else {
		printf( "               |\n" );
	}
}



/*--------------------------------------------------------------
	�ǡ��������롼����
 Input:
	̵��
 Return:
	==0:���� !=0:�۾�
--------------------------------------------------------------*/
int data_make( void )
{
	FILE	*fp;
	int		i, j, track_ptr;
	int		tone_max, envelope_max, pitch_env_max, pitch_mod_max;
	int		arpeggio_max, fm_tone_max, dpcm_max, n106_tone_max,vrc7_tone_max;
	int		hard_effect_max, effect_wave_max;
	LINE	*line_ptr[MML_MAX];
	CMD		*cmd_buf;
	int	trk_flag[_TRACK_MAX];

	for(i=0; i < _TRACK_MAX; i++) {
		bank_sel[i] = -1; // ������֤��ڤ��ؤ�̵��
	}
	for( i = 0; i < _DPCM_MAX; i++ ) {
		dpcm_tbl[i].flag = 0;
		dpcm_tbl[i].index = -1;
	}

	/* ���Ƥ�MML���饨�ե����Ȥ��ɤ߹��� */
	for (mml_idx = 0; mml_idx < mml_num; mml_idx++) {
		line_ptr[mml_idx] = readMmlFile(mml_names[mml_idx]);
		if( line_ptr[mml_idx] == NULL ) return -1;
		getLineStatus(line_ptr[mml_idx], 0 );
#if DEBUG
		for( i = 1; i < line_max; i++ ) {
			printf( "%4d : %04x\n", i, line_ptr[mml_idx][i].status );
		}
#endif


		getTone(     line_ptr[mml_idx] );
		getEnvelope( line_ptr[mml_idx] );
		getPitchEnv( line_ptr[mml_idx] );
		getPitchMod( line_ptr[mml_idx] );
		getArpeggio( line_ptr[mml_idx] );
		getDPCM(     line_ptr[mml_idx] );
		getFMTone(   line_ptr[mml_idx] );
		getVRC7Tone( line_ptr[mml_idx] );
		getN106Tone( line_ptr[mml_idx] );
		getHardEffect(line_ptr[mml_idx]);
		getEffectWave(line_ptr[mml_idx]);
	}

	tone_max      = checkLoop(       tone_tbl,      _TONE_MAX );
	envelope_max  = checkLoop(   envelope_tbl,  _ENVELOPE_MAX );
	pitch_env_max = checkLoop(  pitch_env_tbl, _PITCH_ENV_MAX );
	pitch_mod_max = getMaxLFO(  pitch_mod_tbl, _PITCH_MOD_MAX );
	arpeggio_max  = checkLoop(   arpeggio_tbl,  _ARPEGGIO_MAX );
	dpcm_max      = getMaxDPCM( dpcm_tbl );
	fm_tone_max   = getMaxTone(   fm_tone_tbl,   _FM_TONE_MAX );
	n106_tone_max = getMaxTone( n106_tone_tbl, _N106_TONE_MAX );
	vrc7_tone_max = getMaxTone( vrc7_tone_tbl, _VRC7_TONE_MAX );
	hard_effect_max = getMaxHardEffect( hard_effect_tbl, _HARD_EFFECT_MAX );
	effect_wave_max = getMaxEffectWave( effect_wave_tbl, _EFFECT_WAVE_MAX );

	sortDPCM( dpcm_tbl );					// �����Υ��֤����
	dpcm_size = checkDPCMSize( dpcm_tbl );
	//printf("dpcmsize $%x\n",dpcm_size);
	if ( !allow_bankswitching && (dpcm_size > _DPCM_TOTAL_SIZE)) {	// ������������å�
		dispError( DPCM_FILE_TOTAL_SIZE_OVER, NULL, 0 );
		dpcm_size = 0;
	} else {
		dpcm_data = malloc( dpcm_size );
		readDPCM( dpcm_tbl );
	}
	/* �ԥå�����٥��פΥѥ�᡼������ */
	for( i = 0; i < pitch_env_max; i++ ) {
		if( pitch_env_tbl[i][0] != 0 ) {
			for( j = 1; j <= pitch_env_tbl[i][0]; j++ ) {
				if( 0 < pitch_env_tbl[i][j] && pitch_env_tbl[i][j] < 127 ) {
					pitch_env_tbl[i][j] = pitch_env_tbl[i][j]|0x80;
				} else if( 0 >= pitch_env_tbl[i][j] && pitch_env_tbl[i][j] >= -127 ) {
					pitch_env_tbl[i][j] = (0-pitch_env_tbl[i][j]);
				}
			}
		}
	}

	{
		fp = fopen( ef_name, "wt" );
		if( fp == NULL ) {
			if( message_flag == 0 ) {
				printf( "%s : �ե����뤬�����Ǥ��ޤ���Ǥ�������ߤ��ޤ���\n", ef_name );
			} else {
				printf( "%s : Don't create file. Stops.\n", ef_name );
			}
			return -1;
		}
		


		/* �����񤭹��� */
		writeTone( fp, tone_tbl, "dutyenve", tone_max );
		/* ����٥��׽񤭹��� */
		writeTone( fp, envelope_tbl, "softenve", envelope_max );
		/* �ԥå�����٥��׽񤭹��� */
		writeTone( fp, pitch_env_tbl, "pitchenve", pitch_env_max );
		/* �Ρ��ȥ���٥��׽񤭹��� */
		writeTone( fp, arpeggio_tbl, "arpeggio", arpeggio_max );
		/* LFO�񤭹��� */
		fprintf( fp,"lfo_data:\n" );
		if( pitch_mod_max != 0 ) {
			for( i = 0; i < pitch_mod_max; i++ ) {
				if( pitch_mod_tbl[i][0] != 0 ) {
					fprintf( fp, "\tdb\t$%02x,$%02x,$%02x,$%02x\n",
						pitch_mod_tbl[i][1], pitch_mod_tbl[i][2],
						pitch_mod_tbl[i][3], pitch_mod_tbl[i][4] );
				} else {
					fprintf( fp, "\tdb\t$00,$00,$00,$00\n" );
				}
			}
			fprintf( fp, "\n" );
		}
		/* FM�����񤭹��� */
		writeToneFM( fp, fm_tone_tbl, "fds", fm_tone_max );
		writeHardEffect( fp, hard_effect_tbl, "fds", hard_effect_max );
		writeEffectWave( fp, effect_wave_tbl, "fds", effect_wave_max );
		/* namco106�����񤭹��� */
		writeToneN106( fp, n106_tone_tbl, "n106", n106_tone_max );
		/* VRC7�����񤭹��� */
		writeToneVRC7( fp, vrc7_tone_tbl, "vrc7", vrc7_tone_max );
		/* DPCM�񤭹��� */
		writeDPCM( fp, dpcm_tbl, "dpcm_data", dpcm_max );
		writeDPCMSample( fp );
		
		// MML�ե�����񤭹���
		if( include_flag != 0 ) {
			fprintf( fp, "\t.include\t\"%s\"\n", out_name );
		}
		
		fclose( fp );
	}

	/* MML->ASM�ǡ����Ѵ� */
	fp = fopen( out_name, "wt" );
	if( fp == NULL ) {
		if( message_flag == 0 ) {
			printf( "%s : �ե����뤬�����Ǥ��ޤ���Ǥ�������ߤ��ޤ���\n", out_name );
		} else {
			printf( "%s : Don't create file. Stops.\n", out_name );
		}
		return -1;
	}

	/* ���ϥե�����˥����ȥ�/��ʼ�/�Ǥ����߼Ԥξ���򥳥��ȤȤ��ƽ񤭹��� */
	writeSongInfo(fp);

	//printf(" test info:vrc7:%d vrc6:%d n106:%d\n",vrc7_track_num,vrc6_track_num, n106_track_num);

	for(i=0; i < _TRACK_MAX; i++) trk_flag[i]=0;

	for(i=0; i <= BNOISETRACK; i++) trk_flag[i]=1;
	
	trk_flag[BDPCMTRACK]=1;
		
	if (fds_track_num)
		trk_flag[BFMTRACK]=1;
		
	if (vrc7_track_num)
		for(i=BVRC7TRACK; i < BVRC7TRACK+vrc7_track_num; i++) trk_flag[i]=1;
	if (vrc6_track_num)
		for(i=BVRC6TRACK; i < BVRC6TRACK+vrc6_track_num; i++) trk_flag[i]=1;
	if (n106_track_num)
		for(i=BN106TRACK; i < BN106TRACK+n106_track_num; i++) trk_flag[i]=1;
	if (fme7_track_num)
		for(i=BFME7TRACK; i < BFME7TRACK+fme7_track_num; i++) trk_flag[i]=1;
	if (mmc5_track_num)
		for(i=BMMC5TRACK; i < BMMC5TRACK+mmc5_track_num; i++) trk_flag[i]=1;


	
	fprintf( fp, "\t.bank\t0\n");
	fprintf( fp, "\t.if TOTAL_SONGS > 1\n");
	fprintf( fp, "song_addr_table:\n" );
	for (mml_idx = 0; mml_idx < mml_num; mml_idx++) {
		setSongLabel();
		fprintf( fp, "\tdw\t%s_track_table\n", songlabel);
	}
	
	fprintf( fp, "\t.if (ALLOW_BANK_SWITCH)\n" );
	fprintf( fp, "song_bank_table:\n" );
	for (mml_idx = 0; mml_idx < mml_num; mml_idx++) {
		setSongLabel();
		fprintf( fp, "\tdw\t%s_bank_table\n", songlabel);
	}
	fprintf( fp, "\t.endif ; ALLOW_BANK_SWITCH\n" );
	fprintf( fp, "\t.endif ; TOTAL_SONGS > 1\n" );
	
	for (mml_idx = 0; mml_idx < mml_num; mml_idx++) {
		setSongLabel();
		fprintf( fp, "%s_track_table:\n", songlabel );
		for( i = 0; i < _TRACK_MAX; i++ ){
			if (trk_flag[i]) fprintf( fp, "\tdw\t%s_%02d\n", songlabel, i );
		}
		
		fprintf( fp, "\t.if (ALLOW_BANK_SWITCH)\n" );
		fprintf( fp, "%s_bank_table:\n", songlabel );
		for( i = 0; i < _TRACK_MAX; i++ ){
			if (trk_flag[i]) fprintf( fp, "\tdb\tbank(%s_%02d)*2\n", songlabel, i );
		}
		fprintf( fp, "\t.endif\n" );
	}

	


	curr_bank = 0x00;

	/* ���Ƥ�MML�ˤĤ��� */
	for (mml_idx = 0; mml_idx < mml_num; mml_idx++) {
		setSongLabel();
		/* �ȥ�å�ñ�̤ǥǡ����Ѵ� */
		for( i = 0; i < _TRACK_MAX; i++ ) {
			if ( bank_sel[i] != -1 && !auto_bankswitch) {
				if (trk_flag[i] == 0) {
					if( message_flag == 0 ) {
						printf( "Warning: ̤���ѥȥ�å�(%c)���Ф��Ƥ�#SETBANK��̵�뤷�ޤ�\n", str_track[i]);
					} else {
						printf( "Warning: Ignored #SETBANK on unused track(%c)\n", str_track[i]);
					}
				} else if ((bank_sel[i] == 2 || bank_sel[i] == 3) && dpcm_bankswitch) {
					dispError( CANT_USE_BANK_2_OR_3_WITH_DPCMBANKSWITCH, NULL, 0);
				} else {
					curr_bank = bank_sel[i];
					fprintf( fp, "\n\n");
					fprintf( fp, "\t.bank\t%d\n", bank_sel[i] );
					putBankOrigin(fp, bank_sel[i]);
				}
			}
			
			if (trk_flag[i]) {
				cmd_buf = malloc( sizeof(CMD)*32*1024 );
				developeData( fp, i, cmd_buf, line_ptr[mml_idx] );
				free( cmd_buf );
			}
		}
	}
	fclose( fp );
	
	{

		fp = fopen( inc_name, "wt" );
		if( fp == NULL ) {
			if( message_flag == 0 ) {
				printf( "%s : �ե����뤬�����Ǥ��ޤ���Ǥ�������ߤ��ޤ���\n", inc_name );
			} else {
				printf( "%s : Don't create file. Stops.\n", inc_name );
			}
			return -1;
		}

		fprintf( fp, "TOTAL_SONGS\tequ\t$%02x\n", mml_num );
		fprintf( fp, "SOUND_GENERATOR\tequ\t$%02x\n", sndgen_flag );
		track_ptr = 0;
		track_ptr += 4;
		fprintf( fp, "PTRDPCM\t\tequ\t%2d\n", track_ptr);
		track_ptr += dpcm_track_num;
		fprintf( fp, "PTRFDS\t\tequ\t%2d\n", track_ptr);
		track_ptr += fds_track_num;
		fprintf( fp, "PTRVRC7\t\tequ\t%2d\n", track_ptr);
		track_ptr += vrc7_track_num;
		fprintf( fp, "PTRVRC6\t\tequ\t%2d\n", track_ptr);
		track_ptr += vrc6_track_num;
		fprintf( fp, "PTRN106\t\tequ\t%2d\n", track_ptr);
		track_ptr += n106_track_num;
		fprintf( fp, "PTRFME7\t\tequ\t%2d\n", track_ptr);
		track_ptr += fme7_track_num;
		fprintf( fp, "PTRMMC5\t\tequ\t%2d\n", track_ptr);
		track_ptr += mmc5_track_num;
		fprintf( fp, "PTR_TRACK_END\t\tequ\t%2d\n", track_ptr);
		
		//fprintf( fp, "INITIAL_WAIT_FRM\t\tequ\t%2d\n", 0x26);
		fprintf( fp, "PITCH_CORRECTION\t\tequ\t%2d\n", pitch_correction);
		fprintf( fp, "DPCM_RESTSTOP\t\tequ\t%2d\n", dpcm_reststop);
		fprintf( fp, "DPCM_BANKSWITCH\t\tequ\t%2d\n", dpcm_bankswitch);
		fprintf( fp, "DPCM_EXTRA_BANK_START\t\tequ\t%2d\n", bank_maximum+1);
		fprintf( fp, "BANK_MAX_IN_4KB\t\tequ\t(%d + %d)*2+1\n", bank_maximum, dpcm_extra_bank_num);

		if (!allow_bankswitching || (!dpcm_bankswitch && (bank_maximum + dpcm_extra_bank_num <= 3))) {
			fprintf( fp, "ALLOW_BANK_SWITCH\t\tequ\t0\n");
		} else {
			fprintf( fp, "ALLOW_BANK_SWITCH\t\tequ\t1\n");
			fprintf( fp, "BANKSWITCH_INIT_MACRO\t.macro\n");
			switch (bank_maximum) {
			case 0:
				fprintf(fp, "\tdb\t0,1,0,0,0,0,0,0\n");
				break;
			case 1:
				fprintf(fp, "\tdb\t0,1,2,3,0,0,0,0\n");
				break;
			case 2:
				fprintf(fp, "\tdb\t0,1,2,3,4,5,0,0\n");
				break;
			case 3:
			default:
				fprintf(fp, "\tdb\t0,1,2,3,4,5,6,7\n");
				break;
			}
			fprintf(fp, "\t.endm\n");
		}
		
		/* ���ϥե�����˥����ȥ�/��ʼ�/�Ǥ����߼Ԥξ����ޥ���Ȥ��ƽ񤭹��� */
		writeSongInfoMacro(fp);

		fprintf(fp,"\n\n");
		fclose( fp );
	}

	if( error_flag == 0 ) {
		
		/* ���Ƥ�MML�ˤĤ��� */
		for (mml_idx = 0; mml_idx < mml_num; mml_idx++) {
			printf("\n");
			if (mml_num > 1) {
				printf(	"Song %d: %s\n", mml_idx+1, mml_names[mml_idx]);
			}
			printf(	"-------+---------------+---------------+\n"
				"Track  |     Total     |     Loop      |\n"
				" Symbol|(count)|(frame)|(count)|(frame)|\n"
				"-------+-------+-------+-------+-------+\n");
			for (i = 0; i < _TRACK_MAX; i++) {
				if (trk_flag[i])
					display_counts_sub(i, str_track[i]);
			}
			printf(	"-------+-------+-------+-------+-------+\n");
		}
		return 0;
	} else {
		remove( out_name );				/* ���顼�����ä��Ȥ��Ͻ��ϥե�������� */
		remove( ef_name );
		return -1;
	}
}
