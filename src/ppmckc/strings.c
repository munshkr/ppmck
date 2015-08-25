/************************************************************/
/*															*/
/************************************************************/
#include	<stddef.h>
#include	<ctype.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>


/*--------------------------------------------------------------
	���ڡ��������֤Υ����å�
 Input:
	char	*ptr		:�ǡ�����Ǽ�ݥ���
 Output:
	char	*ptr		;�����å׸�Υݥ���
--------------------------------------------------------------*/
char *skipSpaceOld( char *ptr )
{
	while( *ptr != '\0' ) {
		if( *ptr != ' ' && *ptr != '\t' ) {
			break;
		}
		ptr++;
	}
	return ptr;
}


/*--------------------------------------------------------------
	���ڡ��������֤Υ����å�(�ԥ����Ȥ����Ф�)
--------------------------------------------------------------*/
char *skipSpace( char *ptr )
{
	while (1) {
		if (*ptr == '\0') break; //EOL or EOF
		if (*ptr == ' ' || *ptr == '\t') {
			//Skip Space
			ptr++;
			continue;
		} else if (*ptr == '/' || *ptr == ';') {
			//Skip Comment(return EOL)
			while (1) {
				if (*ptr == '\0') break; //EOL or EOF;
				if (*ptr == '\n') break; //EOL;
				ptr++;
			}
			break;
		} else {
			//Normal Chars
			break;
		}
	}
	return ptr;
}




/*----------------------------------------------------------
	ʸ�����������ɤ����Υ����å�
 Input:
	char	c	: ʸ��
 Return:
	0:�����ʳ� 1: ����������
----------------------------------------------------------*/
int checkKanji( unsigned char c )
{
	if( 0x81 <= c && c <= 0x9f ) return 1;
	if( 0xe0 <= c && c <= 0xef ) return 1;
	return 0;
}



/*----------------------------------------------------------
	ʸ�������ʸ���ˤ���(�����б���)
 Input:
	char *ptr	: ʸ����ؤΥݥ���
 Output:
	none
----------------------------------------------------------*/
void strupper( char *ptr )
{
	while( *ptr != '\0' ) {
		if( checkKanji( (unsigned char)*ptr ) == 0 ) {
			*ptr = toupper( (int)*ptr );
			ptr++;
		} else {
			/* �����λ��ν��� */
			ptr += 2;
		}
	}
}




/*--------------------------------------------------------------
	ʸ�������ͤ��Ѵ�
 Input:

 Output:

--------------------------------------------------------------*/
int Asc2Int( char *ptr, int *cnt )
{
	int		num;
	char	c;
	int		minus_flag = 0;

	num = 0;
	*cnt = 0;

	if( *ptr == '-' ) {
		minus_flag = 1;
		ptr++;
		(*cnt)++;
	}
	switch( *ptr ) {
	/* 16�ʿ� */
	  case 'x':
	  case '$':
		ptr++;
		(*cnt)++;
		while( 1 ) {
			c = toupper( *ptr );
			if( '0' <= c && c <= '9' ) {
				num = num * 16 + (c-'0');
			} else if( 'A' <= c && c <= 'F' ) {
				num = num * 16 + (c-'A'+10);
			} else {
				break;
			}
			(*cnt)++;
			ptr++;
		}
		break;
	/* 2�ʿ� */
	  case '%':
		ptr++;
		(*cnt)++;
		while( 1 ) {
			if( '0' <= *ptr && *ptr <= '1' ) {
				num = num * 2 + (*ptr-'0');
			} else {
				break;
			}
			(*cnt)++;
			ptr++;
		}
	/* 10�ʿ� */
	  default:
		while( 1 ) {
			if( '0' <= *ptr && *ptr <= '9' ) {
				num = num * 10 + (*ptr-'0');
			} else {
				break;
			}
			(*cnt)++;
			ptr++;
		}
	}
	if( minus_flag != 0 ) {
		num = -num;
	}
	return num;
}

