;Sunsoft 5B (Gimmick!)
;
;Sunsoft 5B supports noise and envelope generator.
;
;C000 :AY 3-8910 address(W)
;E000 :data(W)
;
;Reference: http://www.howell1964.freeserve.co.uk/parts/ay3891x_datasheet.htm
;           http://www.breezer.demon.co.uk/spec/tech/ay-3-8912.html etc...
;
;AY 3-8910 Registers
;00	Ch. A freq data lower bits
;	FFFFFFFF
;01	Ch. A freq data higher bits
;	----FFFF
;	freq=1.79/(F*16) Mhz
;
;	
;02	Ch. B freq data lower bits
;03	Ch. B freq data higher bits
;04	Ch. C freq data lower bits
;05	Ch. C freq data higher bits
;
;
;06	Noise pitch
;	---FFFFF
;	freq=1.79/(F*16) Mhz
;
;
;07	Mixer
;	0:Enable 1:Disable
;	IINNNTTT
;	||||||||
;	|||||||+-------- Ch A Tone
;	||||||+--------- Ch B Tone
;	|||||+---------- Ch C Tone
;	||||+----------- Ch A Noise
;	|||+------------ Ch B Noise
;	||+------------- Ch C Noise
;	|+-------------- 
;	+--------------- 
;
;
;08	Ch. A volume
;	---MVVVV
;	V: Volume
;	M: Mode (1: Use envelope, 0:Use fixed volume)
;
;
;09	Ch. B volume
;0a	Ch. C volume
;
;
;0b	Envelope duration lower bits
;	DDDDDDDD
;0c	Envelope duration higher bits
;	DDDDDDDD
;	freq = 1.79/(D*256) Mhz
;	(duration = D*256/1.79 sec)
;
;
;0d	Envelope shape
;	----CAAH
;	    ||||
;	    |||+-------- Hold
;	    ||+--------- Alternate
;	    |+---------- Atack
;	    +----------- Continue
;
;
;0e	
;0f	

FME7_ADDR	=	$C000
FME7_DATA	=	$E000

;----------------------------------------
fme7_sound_init:
	ldy	#$0A			;Volume��0��
	lda	#$00
.loop:
	sty	FME7_ADDR
	sta	FME7_DATA
	dey
	cpy	#$07
	bne	.loop
	lda	#%11111000		;Tone��Enable��
	sty	FME7_ADDR
	sta	FME7_DATA
	sta	fme7_reg7
	rts

;----------------------------------------
fme7_dst_adr_set:
	lda	<channel_sel
	sec				
	sbc	#PTRFME7		;FME7�̉��`�����l���ڂ��H
	sta	fme7_ch_sel
	asl	a
	sta	fme7_ch_selx2
	asl	a
	sta	fme7_ch_selx4
	lda	fme7_ch_sel
	clc
	adc	#$08
	sta	fme7_vol_regno
	rts

;-----------------------------------------------------------
sound_fme7:
	lda	<channel_sel
	cmp	#PTRFME7+3
	beq	.end1
	jsr	fme7_dst_adr_set
	ldx	<channel_selx2
	dec	sound_counter,x		;�J�E���^���������炵
	beq	.sound_read_go		;�[���Ȃ�T�E���h�ǂݍ���
	jsr	fme7_do_effect		;�[���ȊO�Ȃ�G�t�F�N�g����
	rts				;�����
.sound_read_go
	jsr	sound_fme7_read
	jsr	fme7_do_effect
	lda	rest_flag,x
	and	#%00000010		;�L�[�I���t���O
	beq	.end1			
	jsr	sound_fme7_write	;�����Ă�����f�[�^�����o��
	lda	rest_flag,x
	and	#%11111101		;�L�[�I���t���O�I�t
	sta	rest_flag,x
.end1
	rts

;-------
fme7_do_effect:
	lda	rest_flag,x
	and	#%00000001
	beq	.duty_write2
	rts				;�x���Ȃ�I���

.duty_write2:

.enve_write2:
	lda	effect_flag,x
	and	#%00000001
	beq	.lfo_write2
	jsr	sound_fme7_softenve

.lfo_write2:
	lda	effect_flag,x
	and	#%00010000
	beq	.pitchenve_write2
	jsr	sound_fme7_lfo

.pitchenve_write2:
	lda	effect_flag,x
	and	#%00000010
	beq	.arpeggio_write2
	jsr	sound_fme7_pitch_enve

.arpeggio_write2:
	lda	effect_flag,x
	and	#%00001000
	beq	.return7
	lda	rest_flag,x		;�L�[�I���̂Ƃ��Ƃ����łȂ��Ƃ��ŃA���y�W�I�̋����͂�����
	and	#%00000010		;�L�[�I���t���O
	bne	.arpe_key_on
	jsr	sound_fme7_note_enve	;�L�[�I������Ȃ��Ƃ��ʏ�͂���
	jmp	.return7
.arpe_key_on				;�L�[�I���������̏ꍇ
	jsr	note_enve_sub		;���������������ŁA�����ł͏������݂͂��Ȃ�
	jsr	fme7_freq_set
	jsr	arpeggio_address
.return7:
	rts
;------------------------------------------------
fme7_freq_set:
	ldx	<channel_selx2
	lda	fme7_tone,x
	cmp	#$02			;�m�C�Y�Ȃ�
	beq	fme7_noise_freq_set	;���
	lda	sound_sel,x		;���K�f�[�^�ǂݏo��
	and	#%00001111		;����4bit�����o����
	asl	a
	tay

	lda	fme7_frequency_table,y	;Sun5B���g���e�[�u������Low��ǂݏo��
	sta	sound_freq_low,x	;��������
	lda	fme7_frequency_table+1,y	;Sun5B���g���e�[�u������High��ǂݏo��
	sta	sound_freq_high,x	;��������

	lda	sound_sel,x		;���K�f�[�^�ǂݏo��
	lsr	a			;���4bit�����o��
	lsr	a			;
	lsr	a			;
	lsr	a			;
	beq	fme7_freq_end		;�[���Ȃ炻�̂܂܏I���
	tay

fme7_oct_set2:

	lsr	sound_freq_high,x	;�E�V�t�g�@������C��
	ror	sound_freq_low,x	;C���玝���Ă���ł�@�E���[�e�C�g
	dey				;
	bne	fme7_oct_set2		;�I�N�^�[�u���J��Ԃ�

FREQ_ROUND = 0
	.if	FREQ_ROUND
	lda	sound_freq_low,x
	adc	#$00			;�Ō�ɐ؂�̂Ă�C�𑫂�(�l�̌ܓ�)
	sta	sound_freq_low,x
	bcc	fme7_freq_end
	inc	sound_freq_high,x
	.endif

fme7_freq_end:
	jsr	detune_write_sub
	rts

;----
fme7_noise_freq_set:
	lda	sound_sel,x		;���K�f�[�^�ǂݏo��
	sta	sound_freq_low,x	;���̂܂�
	rts
;------------------------------------------------
fme7_frequency_table:
	dw	$0D5C,$0C9D,$0BE7,$0B3C	; o0c  c+ d  d+
	dw	$0A9B,$0A02,$0973,$08EB	;   e  f  f+ g
	dw	$086B,$07F2,$0780,$0714	;   g+ a  a+ b
	dw	$0000,$0FE4,$0EFF,$0E28	; o-1  a  a+ b
; �Đ����g�� = 1789772.5 / (n*32) [Hz]

;------------------------------------------------
sound_fme7_read:
	ldx	<channel_selx2
	
	lda	sound_bank,x
	jsr	change_bank
	
	lda	[sound_add_low,x]
;----------
;���[�v����1
fme7_loop_program:
	cmp	#$a0
	bne	fme7_loop_program2
	jsr	loop_sub
	jmp	sound_fme7_read
;----------
;���[�v����2(����)
fme7_loop_program2:
	cmp	#$a1
	bne	fme7_bank_command
	jsr	loop_sub2
	jmp	sound_fme7_read
;----------
;�o���N�؂�ւ�
fme7_bank_command:
	cmp	#$ee
	bne	fme7_wave_set
	jsr	data_bank_addr
	jmp	sound_fme7_read
;----------
;�f�[�^�G���h�ݒ�
;fme7_data_end:
;	cmp	#$ff
;	bne	fme7_wave_set
;	jsr	data_end_sub
;	jmp	sound_fme7_read
;----------
;���F�ݒ�
fme7_wave_set:
	cmp	#$fe
	bne	fme7_volume_set

	jsr	sound_data_address
	ldy	fme7_ch_selx4
	lda	fme7_enable_bit_tbl,y	;�܂��m�C�Y�r�b�g�A�g�[���r�b�g�̗�����0�ɂ���
	eor	#$FF
	and	fme7_reg7
	sta	fme7_reg7
	lda	[sound_add_low,x]	;���F�f�[�^�ǂݏo��
	and	#%00000011
	sta	fme7_tone,x
	clc
	adc	fme7_ch_selx4
	tay
	lda	fme7_enable_bit_tbl,y	;�r�b�g�ǂݏo��
	ora	fme7_reg7		;1�̂�1�ɂ���(Disable)
	sta	fme7_reg7		;
	ldy	#$07
	sty	FME7_ADDR
	sta	FME7_DATA
	jsr	sound_data_address
	jmp	sound_fme7_read

fme7_enable_bit_tbl:
;		       @0   @1(tone)  @2(noise)   @3(both)
	db	%00001001, %00001000, %00000001, %00000000	; ch A
	db	%00010010, %00010000, %00000010, %00000000	; ch B
	db	%00100100, %00100000, %00000100, %00000000	; ch C
;----------
;���ʐݒ�
fme7_volume_set:
	cmp	#$fd
	bne	fme7_rest_set
	jsr	sound_data_address
	lda	[sound_add_low,x]
	sta	temporary
	bpl	fme7_softenve_part	;�\�t�g�G���x������

fme7_volume_part:
	lda	effect_flag,x
	and	#%11111110
	sta	effect_flag,x		;�\�t�g�G���x�����w��

	lda	temporary
	and	#%00011111		;bit 4�̓n�[�h�G���x���[�v�t���O
	sta	fme7_volume,x
	tay
	jsr	fme7_volume_write_sub

	jsr	sound_data_address
	jmp	sound_fme7_read

fme7_softenve_part:
	jsr	volume_sub
	jmp	sound_fme7_read
;----------
fme7_rest_set:
	cmp	#$fc
	bne	fme7_lfo_set

	lda	rest_flag,x
	ora	#%00000001
	sta	rest_flag,x

	jsr	sound_data_address
	lda	[sound_add_low,x]
	sta	sound_counter,x

	ldy	#$00			;�{�����[��0����������
	jsr	fme7_volume_write_sub

	jsr	sound_data_address
	rts
;----------
fme7_lfo_set:
	cmp	#$fb
	bne	fme7_detune_set
	jsr	lfo_set_sub
	jmp	sound_fme7_read
;----------
fme7_detune_set:
	cmp	#$fa
	bne	fme7_pitch_set
	jsr	detune_sub
	jmp	sound_fme7_read
;----------
;�s�b�`�G���x���[�v�ݒ�
fme7_pitch_set:
	cmp	#$f8
	bne	fme7_arpeggio_set
	jsr	pitch_set_sub
	jmp	sound_fme7_read
;----------
;�m�[�g�G���x���[�v�ݒ�
fme7_arpeggio_set:
	cmp	#$f7
	bne	fme7_freq_direct_set
	jsr	arpeggio_set_sub
	jmp	sound_fme7_read
;----------
;�Đ����g�����ڐݒ�
fme7_freq_direct_set:
	cmp	#$f6
	bne	fme7_y_command_set
	jsr	direct_freq_sub
	rts
;----------
;���R�}���h�ݒ�
fme7_y_command_set:
	cmp	#$f5
	bne	fme7_wait_set
	jsr	y_sub
	jmp	sound_fme7_read
;----------
;�E�F�C�g�ݒ�
fme7_wait_set:
	cmp	#$f4
	bne	fme7_hard_speed_set
	jsr	wait_sub
	rts
;----------
;�n�[�h�E�F�A�G���x���[�v���x�ݒ�
fme7_hard_speed_set:
	cmp	#$f2
	bne	fme7_noise_set
	jsr	sound_data_address
	ldy	#$0B
	lda	[sound_add_low,x]
	sty	FME7_ADDR
	sta	FME7_DATA
	iny
	jsr	sound_data_address
	lda	[sound_add_low,x]
	sty	FME7_ADDR
	sta	FME7_DATA
	jsr	sound_data_address
	jmp	sound_fme7_read
;----------
;�m�C�Y���g���ݒ�
fme7_noise_set:
	cmp	#$f1
	bne	fme7_oto_set
	jsr	sound_data_address
	ldy	#$06
	lda	[sound_add_low,x]
	sty	FME7_ADDR
	sta	FME7_DATA
	jsr	sound_data_address
	jmp	sound_fme7_read
;----------
fme7_oto_set:
	sta	sound_sel,x		;�����͂܂����
	jsr	sound_data_address
	lda	[sound_add_low,x]	;�����ǂݏo��
	sta	sound_counter,x		;���ۂ̃J�E���g�l�ƂȂ�܂�
	jsr	sound_data_address
	jsr	fme7_freq_set		;���g���Z�b�g��
	jsr	effect_init
	rts
;-------------------------------------------------------------------------------
sound_fme7_write:
	ldy	fme7_volume,x
	jsr	fme7_volume_write_sub

fme7_write:
	lda	fme7_tone,x
	cmp	#$02
	beq	fme7_noise_write	;�m�C�Y���̓m�C�Y���g�����o��

	ldy	fme7_ch_selx2		;���g�����W�X�^�ԍ�

	lda	sound_freq_low,x
	sty	FME7_ADDR
	sta	FME7_DATA
	iny
	lda	sound_freq_high,x
	sty	FME7_ADDR
	sta	FME7_DATA
	rts

fme7_noise_write:
	ldy	#$06
	lda	sound_freq_low,x
	and	#%00011111
	sty	FME7_ADDR
	sta	FME7_DATA
	rts
;----------------------------------------
;�{�����[���������݁A�G���x���[�v�V�F�C�v
; input: Y
fme7_volume_write_sub:
	cpy	#$10
	bcc	.write_vol
	tya				;�n�[�h�G���x���[�v�̂Ƃ���
	and	#%00001111
	ldy	#$0D			;�G���x���[�v�V�F�C�v��
	sty	FME7_ADDR
	sta	FME7_DATA
	ldy	#$10
.write_vol:				;�ʏ�{�����[���̂Ƃ��͂�������
	lda	fme7_vol_regno
	sta	FME7_ADDR
	sty	FME7_DATA
	rts
;-----------------------------------------------------
sound_fme7_softenve:
	jsr	volume_enve_sub
	sta	fme7_volume,x
	tay
	jsr	fme7_volume_write_sub
	jmp	enverope_address
;-------------------------------------------------------------------------------
sound_fme7_lfo:
	jsr	lfo_sub
	jmp	fme7_write
;-------------------------------------------------------------------------------
sound_fme7_pitch_enve:
	jsr	pitch_sub
	jsr	fme7_write
	jmp	pitch_enverope_address
;-------------------------------------------------------------------------------
sound_fme7_note_enve
	jsr	note_enve_sub
	bcs	.end4			;0�Ȃ̂ŏ����Ȃ��Ă悵
	jsr	fme7_freq_set
	jsr	fme7_write
.end4
	jmp	arpeggio_address
;-------------------------------------------------------------------------------
