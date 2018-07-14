char *patchstr = "patches FDS enable patch([OK]), 4-46, 4-356, 5-17, 5-95, 5-313, 5-658\n";
char *hogereleasestr = (
"ppmck release 9a by h7\n"
"ppmck release 9a ex11.3 by BouKiCHi\n"
"ppmck release 9a ex11.3 am1 by AoiMoe\n");

#ifdef ENGLISH
#define	LANGUAGE		1			// 0だとデフォルトで日本語 1だと英語
#else
#define LANGUAGE		0
#endif

int message_flag = LANGUAGE;			// 表示メッセージの出力設定( 0:Jp 1:En )

