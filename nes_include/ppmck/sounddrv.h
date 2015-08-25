;-------------------------------------------------------------------------------
;internal 4 dpcm 1 fds 1 vrc7 6 vrc6 3 n106 8 fme7 3 mmc5 3(?)
;4+1+1+6+3+8+3+3=29ch
MAX_CH		=	29
CH_COUNT	=	PTR_TRACK_END
	.if	CH_COUNT > MAX_CH
	.fail	"memory out of range"
	.endif

;-------------------------------------------------------------------------------
;memory definition
;-------------------------------------------------------------------------------
;�[���y�[�W��������`
	.zp
	.org	$00
	
t0			.ds	1		; temp 
t1			.ds	1		;
t2			.ds	1		;
t3			.ds	1		;

sound_add_low		.ds	1		;command address
sound_add_high		.ds	1		;
			.ds	CH_COUNT*2 - 2

channel_sel		.ds	1		;
channel_selx2		.ds	1		;
channel_selx4		.ds	1		;

drvtmp0			.ds	1		; �e�����̃h���C�o���Ŏg�p����
drvtmp1			.ds	1		;
drvtmp2			.ds	1		;
drvtmp3			.ds	1		;

ind_lda_add		.ds	2		; for indirect_lda

temp_data_add		.ds	2		;

VRC6_DST_REG_LOW	.ds	1		; for vrc6.h
VRC6_DST_REG_HIGH	.ds	1		;

;-----------------------------------
;��[���y�[�W�̃�������`

	.bss
BSS_BASE	=	$0200
	.org	BSS_BASE

;�e�`�����l���ɕK�v�ȃ�����
;	ldx	<channel_selx2
;	���Ă���
;	lda	memory,x
;	����̂�1�o�C�g�����Ƀf�[�^���Ȃ��

;_add_low��_add_high�͋ߐڂ��Ă���K�v������

soft_add_low		.ds	1		;software envelope(@v) address
soft_add_high		.ds	1		;
			.ds	CH_COUNT*2 - 2
pitch_add_low		.ds	1		;pitch envelope (EP) address
pitch_add_high		.ds	1		;
			.ds	CH_COUNT*2 - 2
duty_add_low		.ds	1		;duty envelope (@@) address
duty_add_high		.ds	1		;
			.ds	CH_COUNT*2 - 2
arpe_add_low		.ds	1		;note envelope (EN) address
arpe_add_high		.ds	1		;
			.ds	CH_COUNT*2 - 2
lfo_reverse_counter	.ds	1		;
lfo_adc_sbc_counter	.ds	1		;
			.ds	CH_COUNT*2 - 2
lfo_start_counter	.ds	1		;
lfo_start_time		.ds	1		;
			.ds	CH_COUNT*2 - 2
lfo_adc_sbc_time	.ds	1		;
lfo_depth		.ds	1		;
			.ds	CH_COUNT*2 - 2
lfo_reverse_time	.ds	1		;
lfo_sel			.ds	1		;vibrato (MP) no
			.ds	CH_COUNT*2 - 2
detune_dat		.ds	1		;detune value
fme7_tone
register_high		.ds	1		;
			.ds	CH_COUNT*2 - 2
fme7_volume
register_low		.ds	1		;
duty_sel		.ds	1		;duty envelope no
			.ds	CH_COUNT*2 - 2
channel_loop		.ds	1		;|: :| loop counter
rest_flag		.ds	1		;
			.ds	CH_COUNT*2 - 2
softenve_sel		.ds	1		;software envelope(@v) no
pitch_sel		.ds	1		;pitch envelope (EP) no
			.ds	CH_COUNT*2 - 2
arpeggio_sel		.ds	1		;note envelope (EN) no
effect_flag		.ds	1		;
			.ds	CH_COUNT*2 - 2
sound_sel		.ds	1		;note no.
sound_counter		.ds	1		;wait counter
			.ds	CH_COUNT*2 - 2
sound_freq_low		.ds	1		;
sound_freq_high		.ds	1		;
			.ds	CH_COUNT*2 - 2
sound_freq_n106		.ds	1		;n106����Ȃ�ch�ł��g���Ă�
sound_bank		.ds	1		;
			.ds	CH_COUNT*2 - 2
pitch_shift_amount	.ds	1		;
n106_volume
vrc7_key_stat
			.ds	1		;
			.ds	CH_COUNT*2 - 2
n106_7c
vrc7_volume
			.ds	1		;
extra_mem2		.ds	1		;
			.ds	CH_COUNT*2 - 2	;

;-------------
;���̑�
temporary		.ds	1		;
temporary2		.ds	1		;

fds_hard_select		.ds	1		;
fds_volume		.ds	1		;

n106_7f			.ds	1		;

fds_hard_count_1	.ds	1		;
fds_hard_count_2	.ds	1		;

initial_wait		.ds	1		;

fme7_ch_sel		.ds	1		;
fme7_ch_selx2		.ds	1		;
fme7_ch_selx4		.ds	1		;
fme7_reg7		.ds	1		;R7���ݒl
fme7_vol_regno		.ds	1		;

DMC_NMI:
ram_nmi			.ds	3		;
DMC_RESET:
ram_reset		.ds	3		;
DMC_IRQ:
ram_irq			.ds	3		;

;effect_flag: DLLLadpv
;+------ detune flag
;l+----- software LFO�X�s�[�h�σt���O�i�\��j
;ll+---- software LFO�����t���O0=- 1=+
;lll+--- software LFO flag
;llll+---- note enverope flag
;lllll+--- duty enverope flag / FDS hardware effect flag
;llllll+-- pitch enverope flag
;lllllll+- software enverope flag
;llllllll
;DLLLadpv


;rest_flag
;xxxxxxor
;|||||||+- rest
;||||||+-- key on (if set, must do sound_data_write)
;

	.code


;-------------------------------------------------------------------------------
;macros and misc sub routines
;-------------------------------------------------------------------------------
;indirect_lda
;statement
;	indirect_lda	hoge_add_low	;hoge_add_low is not zero page address
;is same as:
;	lda	[hoge_add_low,x]
indirect_lda	.macro
	lda	\1,x		;hoge_add_low
	sta	<ind_lda_add
	lda	\1+1,x		;hoge_add_high
	sta	<ind_lda_add+1
	ldx	#$00
	lda	[ind_lda_add,x]
	ldx	<channel_selx2
	.endm

;--------------------------------------
channel_sel_inc:
	inc	<channel_sel
	lda	<channel_sel
	asl	a
	sta	<channel_selx2
	asl	a
	sta	<channel_selx4
	rts

;-------------------------------------------------------------------------------
;initialize routine
;-------------------------------------------------------------------------------
INITIAL_WAIT_FRM = $00 ;�ŏ��ɂ��̃t���[���������E�F�C�g
;���������[�`��
sound_init:
	.if TOTAL_SONGS > 1
	pha
	.endif
	lda	#$00
	ldx	#$00
.memclear
	sta	$0000,x
	sta	$0200,x
	sta	$0300,x
	sta	$0400,x
	sta	$0500,x
	sta	$0600,x
	sta	$0700,x
	inx
	bne	.memclear

	lda	#INITIAL_WAIT_FRM
	sta	initial_wait

	lda	#$0f		;��������������
	sta	$4015		;�`�����l���g�p�t���O
	lda	#$08		
	sta	$4001		;��`�go2a�ȉ��΍�
	sta	$4005

	.if (DPCM_BANKSWITCH)
	ldx	#$4C		; X = "JMP Absolute"
	stx	ram_nmi
	lda	$FFFA		; NMI low
	sta	ram_nmi+1
	lda	$FFFB		; NMI high
	sta	ram_nmi+2

	stx	ram_reset
	lda	$FFFC		; RESET low
	sta	ram_reset+1
	lda	$FFFD		; RESET high
	sta	ram_reset+2

	stx	ram_irq
	lda	$FFFE		; IRQ/BRK low
	sta	ram_irq+1
	lda	$FFFF		; IRQ/BRK high
	sta	ram_irq+2
	.endif


	.if	SOUND_GENERATOR & __FME7
	jsr	fme7_sound_init
	.endif

	.if	SOUND_GENERATOR & __MMC5
	jsr	mmc5_sound_init
	.endif

	.if	SOUND_GENERATOR & __FDS
	jsr	fds_sound_init
	.endif
	
	.if	SOUND_GENERATOR & __N106
	jsr	n106_sound_init
	.endif

	.if	SOUND_GENERATOR & __VRC6
	jsr	vrc6_sound_init
	.endif

; use t0, t1, t2, t3
.start_add_lsb	=	t0
.start_add_lsb_hi=	t1
.start_bank	=	t2
.start_bank_hi	=	t3

	.if TOTAL_SONGS > 1
		pla
		asl	a
		tax
		
		lda	song_addr_table,x
		sta	<.start_add_lsb
		lda	song_addr_table+1,x
		sta	<.start_add_lsb+1
		
		.if (ALLOW_BANK_SWITCH)
			lda	song_bank_table,x
			sta	<.start_bank
			lda	song_bank_table+1,x
			sta	<.start_bank+1
		.endif
	
	.endif
	
	lda	#$00
	sta	<channel_sel
	sta	<channel_selx2
	sta	<channel_selx4
.sound_channel_set:
	lda	<channel_sel
	cmp	#PTR_TRACK_END		;�I���H
	beq	.sound_init_end
	
	
	.if TOTAL_SONGS > 1
		.if (ALLOW_BANK_SWITCH)
			ldy	<channel_sel		; y = ch; x = ch<<1;
			ldx	<channel_selx2
			
			lda	[.start_bank],y
			sta	sound_bank,x
			
			ldy	<channel_selx2		; x = y = ch<<1;
		.else
			ldx	<channel_selx2		; x = y = ch<<1;
			ldy	<channel_selx2
		.endif
				
		lda	[.start_add_lsb],y
		sta	<sound_add_low,x	;�f�[�^�J�n�ʒu��������
		iny
		lda	[.start_add_lsb],y
		sta	<sound_add_low+1,x	;�f�[�^�J�n�ʒu��������
	.else
		
		ldy	<channel_sel		; y = ch; x = ch<<1;
		ldx	<channel_selx2

		.if (ALLOW_BANK_SWITCH)
			lda	song_000_bank_table,y
			sta	sound_bank,x
		.endif
		
		lda	song_000_track_table,x
		sta	<sound_add_low,x	;�f�[�^�J�n�ʒu��������
		lda	song_000_track_table+1,x
		sta	<sound_add_low+1,x	;�f�[�^�J�n�ʒu��������

	.endif
	; x = ch<<1; y = ?
	
	lda	#$00
	sta	effect_flag,x
	lda	#$01
	sta	sound_counter,x
	
	jsr	channel_sel_inc
	jmp	.sound_channel_set
.sound_init_end:
	rts

;-------------------------------------------------------------------------------
;main routine
;-------------------------------------------------------------------------------
sound_driver_start:

	lda	initial_wait
	beq	.gogo
	dec	initial_wait
	rts
.gogo

	lda	#$00
	sta	<channel_sel
	sta	<channel_selx2
	sta	<channel_selx4

internal_return:
	jsr	sound_internal
	jsr	channel_sel_inc
	lda	<channel_sel
	cmp	#$04
	bne	internal_return		;�߂�

;	.if	DPCMON
sound_dpcm_part:
	jsr	sound_dpcm
;	.endif
	jsr	channel_sel_inc

	.if	SOUND_GENERATOR & __FDS
	jsr	sound_fds		;FDS�s���Ă���
	jsr	channel_sel_inc
	.endif

	.if	SOUND_GENERATOR & __VRC7
vrc7_return:
	jsr	sound_vrc7		;vrc7�s���Ă���
	jsr	channel_sel_inc
	lda	<channel_sel
	cmp	#PTRVRC7+$06		;vrc7�͏I��肩�H
	bne	vrc7_return		;�܂��Ȃ�߂�
	.endif

	.if	SOUND_GENERATOR & __VRC6
vrc6_return:
	jsr	sound_vrc6		;vrc6�s���Ă���
	jsr	channel_sel_inc
	lda	<channel_sel
	cmp	#PTRVRC6+$03		;vrc6�͏I��肩�H
	bne	vrc6_return		;�܂��Ȃ�߂�
	.endif

	.if	SOUND_GENERATOR & __N106
.rept:
	jsr	sound_n106		;n106�s���Ă���
	jsr	channel_sel_inc
	lda	<channel_sel
	sec
	sbc	#PTRN106
	cmp	n106_channel		;n106�͏I��肩�H
	bne	.rept			;�܂��Ȃ�߂�
	.endif

	.if	SOUND_GENERATOR & __FME7
fme7_return:
	jsr	sound_fme7		;fme7�s���Ă���
	jsr	channel_sel_inc
	lda	<channel_sel
	cmp	#PTRFME7+$03		;fme7�͏I��肩�H
	bne	fme7_return		;�܂��Ȃ�߂�
	.endif

	.if	SOUND_GENERATOR & __MMC5
mmc5_return:
	jsr	sound_mmc5		;mmc5�s���Ă���
	jsr	channel_sel_inc
	lda	<channel_sel
	cmp	#PTRMMC5+$02		;mmc5�͏I��肩�H
	bne	mmc5_return		;�܂��Ȃ�߂�
	.endif

	rts

;------------------------------------------------------------------------------
;command read sub routines
;------------------------------------------------------------------------------
sound_data_address:
	inc	<sound_add_low,x	;�f�[�^�A�h���X�{�P
	bne	return2			;�ʂ��オ������
sound_data_address_inc_high
	inc	<sound_add_high,x	;�f�[�^�A�h���X�S�̈ʁi��j�{�P
return2:
	rts

sound_data_address_add_a:
	clc
	adc	<sound_add_low,x
	sta	<sound_add_low,x
	bcs	sound_data_address_inc_high
	rts
;-------------------------------------------------------------------------------
change_bank:
;�o���N��Reg.A�ɕς��܂��`
;�ύX�����o���N�A�h���X�̓o���N�R���g���[���ɂ��
;���݂�NSF�̂݁B
	if (ALLOW_BANK_SWITCH)
;�o���N�؂�ւ��ł���condition: A <= BANK_MAX_IN_4KB
;i.e. A < BANK_MAX_IN_4KB + 1
;i.e. A - (BANK_MAX_IN_4KB+1) < 0
;i.e. NOT ( A - (BANK_MAX_IN_4KB+1) >= 0 )
;skip����condition: A - (BANK_MAX_IN_4KB+1) >= 0
	cmp	#BANK_MAX_IN_4KB+1
	bcs	.avoidbankswitch
	sta	$5ffa ; A000h-AFFFh
	clc
	adc	#$01
	cmp	#BANK_MAX_IN_4KB+1
	bcs	.avoidbankswitch
	sta	$5ffb ; B000h-BFFFh
.avoidbankswitch
	endif
	rts

;-------------------------------------------------------------------------------
; ���s�[�g�I���R�}���h
;
; channel_loop++;
; if (channel_loop == <num>) {
;   channel_loop = 0;
;   �c��̃p�����[�^��������adr�����ɐi�߂�;
; } else {
;   0xee�R�}���h�Ɠ�������;
; }
loop_sub:
	jsr	sound_data_address
	inc	channel_loop,x
	lda	channel_loop,x
	cmp	[sound_add_low,x]	;�J��Ԃ���
	beq	loop_end
	jsr	sound_data_address
	jmp	bank_address_change
loop_end:
	lda	#$00
	sta	channel_loop,x
loop_esc_through			;loop_sub2������ł���
	lda	#$04
	jsr	sound_data_address_add_a
	rts				;�����܂�
;-----------
; ���s�[�g�r������
;
; channel_loop++;
; if (channel_loop == <num>) {
;   channel_loop = 0;
;   0xee�R�}���h�Ɠ�������;
; } else {
;   �c��̃p�����[�^��������adr�����ɐi�߂�;
; }

loop_sub2:
	jsr	sound_data_address
	inc	channel_loop,x
	lda	channel_loop,x
	cmp	[sound_add_low,x]	;�J��Ԃ���
	bne	loop_esc_through
	lda	#$00
	sta	channel_loop,x
	jsr	sound_data_address
	jmp	bank_address_change
;-------------------------------------------------------------------------------
;�o���N�Z�b�g (goto�R�}���h�Bbank, adr_low, adr_high)
data_bank_addr:
	jsr	sound_data_address
bank_address_change:
	if (ALLOW_BANK_SWITCH)
	lda	[sound_add_low,x]
	sta	sound_bank,x
	endif

	jsr	sound_data_address
	lda	[sound_add_low,x]
	pha
	jsr	sound_data_address
	lda	[sound_add_low,x]
	sta	<sound_add_high,x
	pla
	sta	<sound_add_low,x	;�V�����A�h���X������

	rts
;-------------------------------------------------------------------------------
;data_end_sub:
;	ldy	<channel_sel
;	
;	if (ALLOW_BANK_SWITCH)
;	lda	loop_point_bank,y
;	sta	sound_bank,x
;	endif
;	
;	lda	loop_point_table,x
;	sta	<sound_add_low,x	;���[�v�J�n�ʒu�������� Low
;	inx
;	lda	loop_point_table,x
;	sta	<sound_add_low,x	;���[�v�J�n�ʒu�������� High
;	rts
;-------------------------------------------------------------------------------
volume_sub:
	lda	effect_flag,x
	ora	#%00000001
	sta	effect_flag,x		;�\�t�g�G���x�L���w��

	lda	temporary
	sta	softenve_sel,x
	asl	a
	tay
	lda	softenve_table,y	;�\�t�g�G���x�f�[�^�A�h���X�ݒ�
	sta	soft_add_low,x
	lda	softenve_table+1,y
	sta	soft_add_high,x
	jsr	sound_data_address
	rts
;-------------------------------------------------------------------------------
lfo_set_sub:
	jsr	sound_data_address
	lda	[sound_add_low,x]
	cmp	#$ff
	bne	lfo_data_set

	lda	effect_flag,x
	and	#%10001111		;LFO��������
	sta	effect_flag,x
	jsr	sound_data_address
	rts
lfo_data_set:
	asl	a
	asl	a
	sta	lfo_sel,x

	tay
	ldx	<channel_selx2
	lda	lfo_data,y
	sta	lfo_start_time,x		;�f�B���C�Z�b�g
	sta	lfo_start_counter,x
	lda	lfo_data+1,y
	sta	lfo_reverse_time,x		;�X�s�[�h�Z�b�g
	sta	lfo_reverse_counter,x
	lda	lfo_data+2,y
	sta	lfo_depth,x			;���f�v�X�Z�b�g(���warizan_start�ɂ�菑�������)
;	lda	lfo_data+3,y
;	sta	lfo_harf_time,x
;	sta	lfo_harf_count,x		;1/2�J�E���^�Z�b�g

	jsr	warizan_start

	.if PITCH_CORRECTION
		lda	effect_flag,x
		ora	#%00010000		;LFO�L���t���O�Z�b�g
		sta	effect_flag,x
		jsr	lfo_initial_vector
	.else
		lda	<channel_sel		;�Ȃ����̏��������Ă��邩�Ƃ�����
		sec				;���������Ɗg��������+-���t������ł���
		sbc	#$05
		bcc	urararara2

		lda	effect_flag,x
		ora	#%00110000
		sta	effect_flag,x
		jmp	ittoke2
urararara2:
		lda	effect_flag,x
		and	#%11011111		;�g�`�|����
		ora	#%00010000		;LFO�L���t���O�Z�b�g
		sta	effect_flag,x
ittoke2:
	.endif
	jsr	sound_data_address
	rts

	.if PITCH_CORRECTION
; �`�����l���ɂ��s�b�`�̕�����
lfo_initial_vector:
	lda	freq_vector_table,x
	bmi	.increasing_function
; 2A03�Ȃ�
.decreasing_function:
	lda	effect_flag,x
	and	#%11011111		;LFO�͍ŏ����Z
	jmp	.ittoke2
; FDS�Ȃ�
.increasing_function:
	lda	effect_flag,x
	ora	#%00100000		;LFO�͍ŏ����Z
.ittoke2:
	sta	effect_flag,x
	rts
	.endif
;-------------------------------------------------------------------------------
detune_sub:
	jsr	sound_data_address
	lda	[sound_add_low,x]
	cmp	#$ff
	bne	detune_data_set

	lda	effect_flag,x
	and	#%01111111		;detune��������
	sta	effect_flag,x
	jsr	sound_data_address
	rts
detune_data_set:
	tay
	sta	detune_dat,x
	lda	effect_flag,x
	ora	#%10000000		;detune�L������
	sta	effect_flag,x
	jsr	sound_data_address
	rts
;-------------------------------------------------------------------------------
pitch_set_sub:
	jsr	sound_data_address
	lda	[sound_add_low,x]
	cmp	#$ff
	bne	pitch_enverope_part
	lda	effect_flag,x
	and	#%11111101
	sta	effect_flag,x
	jsr	sound_data_address
	rts

pitch_enverope_part:
	sta	pitch_sel,x
	asl	a
	tay
	lda	pitchenve_table,y
	sta	pitch_add_low,x
	lda	pitchenve_table+1,y
	sta	pitch_add_high,x
	lda	effect_flag,x
	ora	#%00000010
	sta	effect_flag,x
	jsr	sound_data_address
	rts
;-------------------------------------------------------------------------------
arpeggio_set_sub:
	jsr	sound_data_address
	lda	[sound_add_low,x]
	cmp	#$ff
	bne	arpeggio_part

	lda	effect_flag,x
	and	#%11110111
	sta	effect_flag,x
	jsr	sound_data_address
	rts

arpeggio_part:
	sta	arpeggio_sel,x
	asl	a
	tay
	lda	arpeggio_table,y
	sta	arpe_add_low,x
	lda	arpeggio_table+1,y
	sta	arpe_add_high,x

	lda	effect_flag,x
	ora	#%00001000
	sta	effect_flag,x
	jsr	sound_data_address
	rts
;-------------------------------------------------------------------------------
direct_freq_sub:
	jsr	sound_data_address
	lda	[sound_add_low,x]
	sta	sound_freq_low,x		;Low
	jsr	sound_data_address
	lda	[sound_add_low,x]
	sta	sound_freq_high,x		;High
	jsr	sound_data_address
	lda	[sound_add_low,x]
	sta	sound_counter,x			;Counter
	jsr	sound_data_address
	jsr	effect_init
	rts
;-------------------------------------------------------------------------------
y_sub:
	jsr	sound_data_address
	lda	[sound_add_low,x]
	sta	<t0
	jsr	sound_data_address
	lda	[sound_add_low,x]
	sta	<t1
	jsr	sound_data_address
	lda	[sound_add_low,x]
	ldx	#$00
	sta	[t0,x]
	ldx	<channel_selx2
	jsr	sound_data_address
	rts
;-------------------------------------------------------------------------------
wait_sub:
	jsr	sound_data_address
	lda	[sound_add_low,x]
	sta	sound_counter,x
	jsr	sound_data_address

	rts
;-------------------------------------------------------------------------------


;-------------------------------------------------------------------------------
;effect sub routines
;-------------------------------------------------------------------------------
detune_write_sub:
	lda	effect_flag,x
	and	#%10000000
	bne	detune_part
	rts

detune_part:
	lda	detune_dat,x

; freq��A�������Z����
; A��$80�������Ă��Ȃ������炻�̂܂܉��Z
; A��$80�������Ă�����and $7F���Č��Z
; input: A
freq_add_mcknumber:
	.if PITCH_CORRECTION
	eor	freq_vector_table,x
	.else
	eor	#0			;set N flag
	.endif
	bmi	detune_minus

; freq��A�����Z����
; input: A
detune_plus:
	eor	#0			;set Z flag
	beq	.endo			;�v���X0�Ȃ�I��
	
	ldy	pitch_shift_amount,x
	bne	detune_plus_with_asl	;�V�t�g����
	
	clc
	adc	sound_freq_low,x
	sta	sound_freq_low,x
	bcs	.mid_plus
.endo:	rts
.mid_plus:
	inc	sound_freq_high,x
	bne	.n106_high_through
	inc	sound_freq_n106,x
.n106_high_through:
	rts

detune_minus:
	and	#%01111111
; freq����A�����Z����
; input: A
detune_minus_nomask:
	eor	#0			;set Z flag
	beq	.endo			;�v���X0�Ȃ�I��
	
	ldy	pitch_shift_amount,x
	bne	detune_minus_nomask_with_asl	;�V�t�g����
	
	sta	<t0
	lda	sound_freq_low,x
	sec
	sbc	<t0
	sta	sound_freq_low,x
	bcc	.mid_minus
.endo:	rts
.mid_minus:
	lda	sound_freq_high,x
	beq	.borrow
.no_borrow:
	dec	sound_freq_high,x
	rts
.borrow:
	dec	sound_freq_high,x
	dec	sound_freq_n106,x
	rts

;---------------------------------
; ���񂩍��V�t�g����o�[�W����
; �����͒��ڌĂяo�����A
; freq_add_mcknumber, detune_plus, detune_minus_nomask���o�R���邱��
; A = �����Z�����Z����l
; Y = �V�t�g�� (0�͋֎~)
detune_plus_with_asl:
	sta	<t0
	lda	#0
	sta	<t1
	sta	<t2
	beq	freq_add_mcknumber_with_asl		;always

detune_minus_nomask_with_asl:
	eor	#$ff
	sta	<t0
	inc	<t0
	beq	detune_through		;0
	
	lda	#$ff
	sta	<t1
	sta	<t2

freq_add_mcknumber_with_asl:
.lp
		asl	<t0
		rol	<t1
		rol	<t2
		dey
		bne	.lp
.add
	clc
	lda	<t0
	adc	sound_freq_low,x
	sta	sound_freq_low,x
	lda	<t1
	adc	sound_freq_high,x
	sta	sound_freq_high,x
	lda	<t2
	adc	sound_freq_n106,x
	sta	sound_freq_n106,x
detune_through:
	rts


;-----------------------------------------------------------------------------
sound_software_enverope:
	jsr	volume_enve_sub
	sta	register_low,x
	ora	register_high,x		;���F�f�[�^�i���4bit�j�Ɖ���4bit�ő����Z
	ldy	<channel_selx4
	sta	$4000,y			;�������݁`
	jsr	enverope_address	;�A�h���X����₵��
	rts				;�����܂�

volume_enve_sub:
	ldx	<channel_selx2

	indirect_lda	soft_add_low		;�G���x���[�v�f�[�^�ǂݍ���
	cmp	#$ff			;�Ōォ�ǁ[��
	beq	return3			;�Ō�Ȃ烋�[�v������
	rts

return3:
	lda	softenve_sel,x
	asl	a
	tay
	lda	softenve_lp_table,y
	sta	soft_add_low,x
	lda	softenve_lp_table+1,y
	sta	soft_add_high,x
	jmp	volume_enve_sub
;------------------------------------------------
enverope_address:
	inc	soft_add_low,x
	bne	return5
	inc	soft_add_high,x
return5:
	rts
;-------------------------------------------------------------------------------
sound_duty_enverope:
	ldx	<channel_selx2

	lda	<channel_sel
	cmp	#$02
	beq	return21		;�O�p�g�Ȃ��΂��`

	indirect_lda	duty_add_low		;�G���x���[�v�f�[�^�ǂݍ���
	cmp	#$ff			;�Ōォ�ǁ[��
	beq	return22		;�Ō�Ȃ炻�̂܂܂����܂�
	asl	a
	asl	a
	asl	a
	asl	a
	asl	a
	asl	a
	ora	#%00110000		;hardware envelope & ... disable
	sta	register_high,x
	ora	register_low,x		;���F�f�[�^�i���4bit�j�Ɖ���4bit�ő����Z
	ldy	<channel_selx4
	sta	$4000,y			;�������݁`
	jsr	duty_enverope_address	;�A�h���X����₵��
return21:
	rts				;�����܂�

return22:
	lda	duty_sel,x
	asl	a
	tay
	lda	dutyenve_lp_table,y
	sta	duty_add_low,x
	lda	dutyenve_lp_table+1,y
	sta	duty_add_high,x
	jmp	sound_duty_enverope

;--------------------------------------------------
duty_enverope_address:
	inc	duty_add_low,x
	bne	return23
	inc	duty_add_high,x
return23:
	rts
;--------------------------------------	
sound_lfo:
	lda	sound_freq_high,x
	sta	temporary

	jsr	lfo_sub

	lda	sound_freq_low,x
	ldy	<channel_selx4
	sta	$4002,y			;�@�@���ݒl�����W�X�^�ɃZ�b�g
	lda	sound_freq_high,x
	cmp	temporary
	beq	end4
	sta	$4003,y
end4:
	rts				;�����܂�
;-------------------------------------------------------------------------------
lfo_sub:
	ldx	<channel_selx2
	lda	lfo_start_counter,x
	beq	.lfo_start
	dec	lfo_start_counter,x
	rts

.lfo_start:
	asl	lfo_reverse_time,x	;2�{����(LFO��1/2�����ɂȂ�)
	lda	lfo_reverse_counter,x	;���]�p�J�E���^�ǂݍ���
	cmp	lfo_reverse_time,x	;LFO�̎�����1/2���Ƃɔ��]����
	bne	.lfo_depth_set		;�K�萔�ɒB���Ă��Ȃ���΃f�v�X������
.lfo_revers_set:				;�K�萔�ɒB���Ă�����������]����
		lda	#$00			;
		sta	lfo_reverse_counter,x	;���]�J�E���^������
		lda	effect_flag,x		;�����r�b�g�𔽓]
		eor	#%00100000		;
		sta	effect_flag,x		;

.lfo_depth_set:
	lsr	lfo_reverse_time,x	;1/2�ɂ���(LFO��1/4�����ɂȂ�)
	lda	lfo_adc_sbc_counter,x	;�f�v�X�p�J�E���^�ǂݍ���
	cmp	lfo_adc_sbc_time,x	;lfo_adc_sbc_time���ƂɃf�v�X��������
	bne	.lfo_count_inc		;�܂��Ȃ�J�E���^�v���X��
.lfo_depth_work:				;��v���Ă���΃f�v�X����
		lda	#$00			;
		sta	lfo_adc_sbc_counter,x	;�f�v�X�J�E���^������
		lda	effect_flag,x		;�{���|��
		and	#%00100000		;���̃r�b�g��
		bne	.lfo_depth_plus		;�����Ă�������Z
.lfo_depth_minus:
			lda	lfo_depth,x
			jsr	detune_minus_nomask
			jmp	.lfo_count_inc
.lfo_depth_plus:
			lda	lfo_depth,x
			jsr	detune_plus

.lfo_count_inc:
	inc	lfo_reverse_counter,x	;�J�E���^�����Ă��I��
	inc	lfo_adc_sbc_counter,x
	rts

;-------------------------------------------------------------------------------
warizan_start:
.quotient = t0
.divisor = t1
	lda	#$00
	sta	<.quotient
	lda	lfo_reverse_time,x	;1/4������
	cmp	lfo_depth,x		;Y���s�[�N�w��
	beq	.plus_one		;�����Ȃ�1:1
	bmi	.depth_wari		;Y���s�[�N�̂ق����傫���ꍇ

.revers_wari:				;1/4�����̂ق����傫���ꍇ
	lda	lfo_depth,x
	sta	<.divisor
	lda	lfo_reverse_time,x
	jsr	.warizan
	lda	<.quotient
	sta	lfo_adc_sbc_time,x
	sta	lfo_adc_sbc_counter,x
	lda	#$01
	sta	lfo_depth,x
	rts

.depth_wari:
	lda	lfo_reverse_time,x
	sta	<.divisor
	lda	lfo_depth,x
	jsr	.warizan
	lda	<.quotient
	sta	lfo_depth,x
	lda	#$01
	sta	lfo_adc_sbc_time,x
	sta	lfo_adc_sbc_counter,x
	rts

.plus_one:				;1�t���[�����Ƃ�1
	lda	#$01
	sta	lfo_depth,x
	sta	lfo_adc_sbc_time,x
	sta	lfo_adc_sbc_counter,x
	rts

.warizan:
	inc	<.quotient
	sec
	sbc	<.divisor
	beq	.warizan_end
	bcc	.warizan_end
	bcs	.warizan			;always
.warizan_end:
	rts

;-------------------------------------------------------------------------------
sound_pitch_enverope:
	lda	sound_freq_high,x
	sta	temporary
	jsr	pitch_sub
pitch_write:
	lda	sound_freq_low,x
	ldy	<channel_selx4
	sta	$4002,y
	lda	sound_freq_high,x
	cmp	temporary
	beq	end3
	sta	$4003,y
end3:
	jsr	pitch_enverope_address
	rts
;-------------------------------------------------------------------------------
pitch_sub:
	ldx	<channel_selx2
	indirect_lda	pitch_add_low	
	cmp	#$ff
	beq	return62

	jmp	freq_add_mcknumber

;--------------------------------------------------
return62:
	indirect_lda	pitch_add_low	
	lda	pitch_sel,x
	asl	a
	tay
	lda	pitchenve_lp_table,y
	sta	pitch_add_low,x
	lda	pitchenve_lp_table+1,y
	sta	pitch_add_high,x
	jmp	pitch_sub
;--------------------------------------------------
pitch_enverope_address:
	inc	pitch_add_low,x
	bne	return63
	inc	pitch_add_high,x
return63:
	rts
;-------------------------------------------------------------------------------
sound_high_speed_arpeggio:		;note enverope
ARPEGGIO_RETRIG = 0			; 1����sound_freq_high���ω����Ȃ��Ă���������
	.if !ARPEGGIO_RETRIG
	lda	sound_freq_high,x
	sta	temporary2
	.endif
	jsr	note_enve_sub
	bcs	.end			; 0�Ȃ̂ŏ����Ȃ��Ă悵
	jsr	frequency_set
;.note_freq_write:
	ldx	<channel_selx2
	lda	sound_freq_low,x
	ldy	<channel_selx4
	sta	$4002,y
	lda	sound_freq_high,x
	.if !ARPEGGIO_RETRIG
	cmp	temporary2
	beq	.end
	.endif
	sta	$4003,y
.end:
	jsr	arpeggio_address
	rts
;--------------------------------------------------
note_add_set:
	lda	arpeggio_sel,x
	asl	a
	tay
	lda	arpeggio_lp_table,y
	sta	arpe_add_low,x
	lda	arpeggio_lp_table+1,y
	sta	arpe_add_high,x
	jmp	note_enve_sub
;--------------------------------------------------
arpeggio_address:
	inc	arpe_add_low,x
	bne	return83
	inc	arpe_add_high,x
return83:
	rts
;-------------------------------------------------------------------------------
;Output 
;	C=0(�ǂݍ��񂾒l��0����Ȃ��̂Ŕ�����������)
;	C=1(�ǂݍ��񂾒l��0�Ȃ̂Ŕ����������Ȃ��Ă�����)
;
note_enve_sub:

	ldx	<channel_selx2
	indirect_lda	arpe_add_low		;�m�[�g�G���x�f�[�^�ǂݏo��
	cmp	#$ff			;$ff�i���I���j���H
	beq	note_add_set
	cmp	#$00			;�[�����H(Z�t���O�ăZ�b�g)
	beq	.note_enve_zero_end	;�[���Ȃ�C���ĂĂ��I��
	cmp	#$80
	beq	.note_enve_zero_end	;�[���Ȃ�C���ĂĂ��I��
	bne	.arpeggio_sign_check	;always
.note_enve_zero_end
	sec				;���������͕s�v
	rts
.arpeggio_sign_check
	eor	#0			;N flag�m�F
	bmi	arpeggio_minus		;�|������

arpeggio_plus:
	sta	<t0			;�e���|�����ɒu���i���[�v�񐔁j
arpeggio_plus2:
	lda	sound_sel,x		;���K�f�[�^�ǂݏo��
	and	#$0f			;����4bit���o
	cmp	#$0b			;����b�Ȃ�
	beq	oct_plus		;�I�N�^�[�u�{������
	inc	sound_sel,x		;�łȂ���Ή��K�{�P
	jmp	loop_1			;���[�v�����P��
oct_plus:
	lda	sound_sel,x		;���K�f�[�^�ǂݏo��
	and	#$f0			;���4bit���o��������4bit�[��
	clc
	adc	#$10			;�I�N�^�[�u�{�P
	sta	sound_sel,x		;���K�f�[�^�����o��
loop_1:
	dec	<t0			;���[�v�񐔁|�P
	lda	<t0			;��œǂݏo��
	beq	note_enve_end		;�[���Ȃ烋�[�v�����I���
	bne	arpeggio_plus2		;�łȂ���΂܂�����

arpeggio_minus:
	and	#%01111111
	sta	<t0
arpeggio_minus2:
	lda	sound_sel,x		;���K�f�[�^�ǂݏo��
	and	#$0f			;����4bit���o
	beq	oct_minus		;�[���Ȃ�|������
	dec	sound_sel,x		;�łȂ���Ή��K�|�P
	jmp	loop_2			;���[�v�����Q��
oct_minus:
	lda	sound_sel,x		;���K�f�[�^�ǂݏo��
	clc
	adc	#$0b			;+b
	sec
	sbc	#$10			;-10
	sta	sound_sel,x		;���K�f�[�^�����o��
loop_2:
	dec	<t0			;���[�v�񐔁|�P
	lda	<t0			;��œǂݏo��
	bne	arpeggio_minus2		;�[���Ȃ烋�[�v�����I���
note_enve_end:
	clc				;���������͕K�v
	rts				;
;-------------------------------------------------------------------------------
;oto_set�ŌĂ΂��
effect_init:
;�\�t�g�E�F�A�G���x���[�v�ǂݍ��݃A�h���X������
	lda	softenve_sel,x
	asl	a
	tay
	lda	softenve_table,y
	sta	soft_add_low,x
	lda	softenve_table+1,y
	sta	soft_add_high,x

;�s�b�`�G���x���[�v�ǂݍ��݃A�h���X������
	lda	pitch_sel,x
	asl	a
	tay
	lda	pitchenve_table,y
	sta	pitch_add_low,x
	lda	pitchenve_table+1,y
	sta	pitch_add_high,x

;�f���[�e�B�G���x���[�v�ǂݍ��݃A�h���X������
	lda	duty_sel,x
	asl	a
	tay
	lda	dutyenve_table,y
	sta	duty_add_low,x
	lda	dutyenve_table+1,y
	sta	duty_add_high,x

;�m�[�g�G���x���[�v�ǂݍ��݃A�h���X������
	lda	arpeggio_sel,x
	asl	a
	tay
	lda	arpeggio_table,y
	sta	arpe_add_low,x
	lda	arpeggio_table+1,y
	sta	arpe_add_high,x
;�\�t�g�E�F�ALFO������
	lda	lfo_start_time,x
	sta	lfo_start_counter,x
	lda	lfo_adc_sbc_time,x
	sta	lfo_adc_sbc_counter,x
	lda	lfo_reverse_time,x
	sta	lfo_reverse_counter,x

	.if PITCH_CORRECTION
		jsr	lfo_initial_vector
	.else
		lda	<channel_sel
		sec
		sbc	#$04
		bmi	urararara

		lda	effect_flag,x
		and	#%10111111
		ora	#%00100000
		sta	effect_flag,x
		jmp	ittoke
urararara:
		lda	effect_flag,x
		and	#%10011111
		sta	effect_flag,x
ittoke:	
	.endif
;�x���t���O�N���A&Key On�t���O��������
	lda	#%00000010
	sta	rest_flag,x
	rts

