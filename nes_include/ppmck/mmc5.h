MMC5_REG_CTRL	=	$5000	; �R���g���[�����W�X�^
MMC5_REG_FREQ_L	=	$5002	; ���g��(L)���W�X�^
MMC5_REG_FREQ_H	=	$5003	; ���g��(H)���W�X�^
MMC5_START_CH	=	PTRMMC5	; �J�nch
;----------------------------------------


mmc5_sound_init:
	lda	#$03
	sta	$5015
	rts
;-----------------------------------------------------------
sound_mmc5:
	ldx	<channel_selx2
	dec	sound_counter,x		;�J�E���^���������炵
	beq	.sound_read_go		;�[���Ȃ�T�E���h�ǂݍ���
	jsr	mmc5_do_effect		;�[���ȊO�Ȃ�G�t�F�N�g����
	rts				;�����
.sound_read_go
	jsr	sound_mmc5_read
	jsr	mmc5_do_effect
	lda	rest_flag,x
	and	#%00000010		;�L�[�I���t���O
	beq	.end1			
	jsr	sound_mmc5_write	;�����Ă�����f�[�^�����o��
	lda	rest_flag,x
	and	#%11111101		;�L�[�I���t���O�I�t
	sta	rest_flag,x
.end1
	rts

;-------
mmc5_do_effect:
	lda	rest_flag,x
	and	#%00000001
	beq	.duty_write2
	rts				;�x���Ȃ�I���

.duty_write2:
	lda	effect_flag,x
	and	#%00000100
	beq	.enve_write2
	jsr	sound_mmc5_dutyenve

.enve_write2:
	lda	effect_flag,x
	and	#%00000001
	beq	.lfo_write2
	jsr	sound_mmc5_softenve

.lfo_write2:
	lda	effect_flag,x
	and	#%00010000
	beq	.pitchenve_write2
	jsr	sound_mmc5_lfo

.pitchenve_write2:
	lda	effect_flag,x
	and	#%00000010
	beq	.arpeggio_write2
	jsr	sound_mmc5_pitch_enve

.arpeggio_write2:
	lda	effect_flag,x
	and	#%00001000
	beq	.return7
	lda	rest_flag,x		;�L�[�I���̂Ƃ��Ƃ����łȂ��Ƃ��ŃA���y�W�I�̋����͂�����
	and	#%00000010		;�L�[�I���t���O
	bne	.arpe_key_on
	jsr	sound_mmc5_note_enve	;�L�[�I������Ȃ��Ƃ��ʏ�͂���
	jmp	.return7
.arpe_key_on				;�L�[�I���������̏ꍇ
	jsr	note_enve_sub		;���������������ŁA�����ł͏������݂͂��Ȃ�
	jsr	mmc5_freq_set
	jsr	arpeggio_address
.return7:
	rts

;------------------------------------------------
mmc5_freq_set:
	ldx	<channel_selx2
	lda	sound_sel,x		;���K�f�[�^�ǂݏo��
	and	#%00001111		;����4bit�����o����
	asl	a
	tay

	lda	psg_frequency_table,y	;PSG���g���e�[�u������Low��ǂݏo��
	sta	sound_freq_low,x	;��������
	lda	psg_frequency_table+1,y	;PSG���g���e�[�u������High��ǂݏo��
	sta	sound_freq_high,x	;��������

mmc5_oct_set1:

	lda	sound_sel,x		;���K�f�[�^�ǂݏo��
	lsr	a			;���4bit�����o��
	lsr	a			;
	lsr	a			;
	lsr	a			;

	sec				;�I�N�^�[�u������
	sbc	#$02

	beq	mmc5_freq_end		;�[���Ȃ炻�̂܂܏I���
	tay

mmc5_oct_set2:

	lsr	sound_freq_high,x	;�E�V�t�g�@������C��
	ror	sound_freq_low,x	;C���玝���Ă���ł�@�E���[�e�C�g
	dey				;
	bne	mmc5_oct_set2		;�I�N�^�[�u���J��Ԃ�

mmc5_freq_end:
	jsr	detune_write_sub
	rts
;---------------------------------------------------------------
sound_mmc5_read:
	ldx	<channel_selx2
	
	lda	sound_bank,x
	jsr	change_bank
	
	lda	[sound_add_low,x]
;----------
;���[�v����1
mmc5_loop_program
	cmp	#$a0
	bne	mmc5_loop_program2
	jsr	loop_sub
	jmp	sound_mmc5_read
;----------
;���[�v����2(����)
mmc5_loop_program2
	cmp	#$a1
	bne	mmc5_bank_command
	jsr	loop_sub2
	jmp	sound_mmc5_read
;----------
;�o���N�؂�ւ�
mmc5_bank_command
	cmp	#$ee
	bne	mmc5_wave_set
	jsr	data_bank_addr
	jmp	sound_mmc5_read
;----------
;�f�[�^�G���h�ݒ�
;mmc5_data_end:
;	cmp	#$ff
;	bne	mmc5_wave_set
;	jsr	data_end_sub
;	jmp	sound_mmc5_read
;----------
;���F�ݒ�
mmc5_wave_set:
	cmp	#$fe
	bne	mmc5_volume_set
	jsr	sound_data_address
	lda	[sound_add_low,x]	;���F�f�[�^�ǂݏo��
	pha
	bpl	mmc5_duty_enverope_part	;�a���[�e�B�G���x������

mmc5_duty_select_part:
	lda	effect_flag,x
	and	#%11111011
	sta	effect_flag,x		;�f���[�e�B�G���x���[�v�����w��
	pla
	asl	a
	asl	a
	asl	a
	asl	a
	asl	a
	asl	a
	ora	#%00110000		;waveform hold on & hardware envelope off
	sta	register_high,x		;��������
	ora	register_low,x
	ldy	<channel_selx4
	sta	MMC5_REG_CTRL-MMC5_START_CH*4,y
	jsr	sound_data_address
	jmp	sound_mmc5_read

mmc5_duty_enverope_part:
	lda	effect_flag,x
	ora	#%00000100
	sta	effect_flag,x		;�f���[�e�B�G���x���[�v�L���w��
	pla
	sta	duty_sel,x
	asl	a
	tay
	lda	dutyenve_table,y	;�f���[�e�B�G���x���[�v�A�h���X�ݒ�
	sta	duty_add_low,x
	lda	dutyenve_table+1,y
	sta	duty_add_high,x
	jsr	sound_data_address
	jmp	sound_mmc5_read

;----------
;���ʐݒ�
mmc5_volume_set:
	cmp	#$fd
	bne	mmc5_rest_set
	jsr	sound_data_address
	lda	[sound_add_low,x]
	sta	temporary
	bpl	mmc5_softenve_part		;�\�t�g�G���x������

mmc5_volume_part:
	lda	effect_flag,x
	and	#%11111110
	sta	effect_flag,x		;�\�t�g�G���x�����w��

	lda	temporary
	and	#%00001111
	sta	register_low,x
	ora	register_high,x
	ldy	<channel_selx4
	sta	MMC5_REG_CTRL-MMC5_START_CH*4,y			;�{�����[����������
	jsr	sound_data_address
	jmp	sound_mmc5_read

mmc5_softenve_part:
	jsr	volume_sub
	jmp	sound_mmc5_read

;----------
mmc5_rest_set:
	cmp	#$fc
	bne	mmc5_lfo_set

	lda	rest_flag,x
	ora	#%00000001
	sta	rest_flag,x

	jsr	sound_data_address
	lda	[sound_add_low,x]
	sta	sound_counter,x

	lda	register_high,x
	ldy	<channel_selx4
	sta	MMC5_REG_CTRL-MMC5_START_CH*4,y
	jsr	sound_data_address
	rts
;----------
mmc5_lfo_set:
	cmp	#$fb
	bne	mmc5_detune_set
	jsr	lfo_set_sub
	jmp	sound_mmc5_read
;----------
mmc5_detune_set:
	cmp	#$fa
	bne	mmc5_pitch_set
	jsr	detune_sub
	jmp	sound_mmc5_read
;----------
;�s�b�`�G���x���[�v�ݒ�
mmc5_pitch_set:
	cmp	#$f8
	bne	mmc5_arpeggio_set
	jsr	pitch_set_sub
	jmp	sound_mmc5_read
;----------
;�m�[�g�G���x���[�v�ݒ�
mmc5_arpeggio_set:
	cmp	#$f7
	bne	mmc5_freq_direct_set
	jsr	arpeggio_set_sub
	jmp	sound_mmc5_read
;----------
;�Đ����g�����ڐݒ�
mmc5_freq_direct_set:
	cmp	#$f6
	bne	mmc5_y_command_set
	jsr	direct_freq_sub
	rts
;----------
;���R�}���h�ݒ�
mmc5_y_command_set:
	cmp	#$f5
	bne	mmc5_wait_set
	jsr	y_sub
	jmp	sound_mmc5_read
;----------
;�E�F�C�g�ݒ�
mmc5_wait_set:
	cmp	#$f4
	bne	mmc5_oto_set
	jsr	wait_sub
	rts
;----------
mmc5_oto_set:
	sta	sound_sel,x		;�����͂܂����
	jsr	sound_data_address
	lda	[sound_add_low,x]	;�����ǂݏo��
	sta	sound_counter,x		;���ۂ̃J�E���g�l�ƂȂ�܂�
	jsr	sound_data_address
	jsr	mmc5_freq_set		;���g���Z�b�g��
	jsr	effect_init
	rts
;-------------------------------------------------------------------------------
sound_mmc5_write:
	ldx	<channel_selx2
	ldy	<channel_selx4

	lda	register_low,x		;���ʕێ�
	ora	register_high,x
	sta	MMC5_REG_CTRL-MMC5_START_CH*4,y

	lda	sound_freq_low,x	;Low Write
	sta	MMC5_REG_FREQ_L-MMC5_START_CH*4,y
	lda	sound_freq_high,x	;High Write
	sta	MMC5_REG_FREQ_H-MMC5_START_CH*4,y
	rts
;-----------------------------------------------------
sound_mmc5_softenve:
	jsr	volume_enve_sub
	sta	register_low,x
	ora	register_high,x		;���F�f�[�^�i���4bit�j�Ɖ���4bit�ő����Z
	ldy	<channel_selx4
	sta	MMC5_REG_CTRL-MMC5_START_CH*4,y			;�������݁`
	jsr	enverope_address	;�A�h���X����₵��
	rts				;�����܂�
;-------------------------------------------------------------------------------
sound_mmc5_lfo:
	lda	sound_freq_high,x
	sta	temporary
	jsr	lfo_sub
	lda	sound_freq_low,x
	ldy	<channel_selx4
	sta	MMC5_REG_FREQ_L-MMC5_START_CH*4,y
	lda	sound_freq_high,x
	cmp	temporary
	beq	mmc5_end4
	sta	MMC5_REG_FREQ_H-MMC5_START_CH*4,y
mmc5_end4:
	rts
;-------------------------------------------------------------------------------
sound_mmc5_pitch_enve:
	lda	sound_freq_high,x
	sta	temporary
	jsr	pitch_sub
mmc5_pitch_write:
	lda	sound_freq_low,x
	ldy	<channel_selx4
	sta	MMC5_REG_FREQ_L-MMC5_START_CH*4,y
	lda	sound_freq_high,x
	cmp	temporary
	beq	mmc5_end3
	sta	MMC5_REG_FREQ_H-MMC5_START_CH*4,y
mmc5_end3:
	jsr	pitch_enverope_address
	rts
;-------------------------------------------------------------------------------
sound_mmc5_note_enve
;	lda	sound_freq_high,x
;	sta	temporary2
	jsr	note_enve_sub
	bcs	.end4			;0�Ȃ̂ŏ����Ȃ��Ă悵
	jsr	mmc5_freq_set
;.mmc5_note_freq_write:
	ldx	<channel_selx2
	ldy	<channel_selx4

	lda	sound_freq_low,x
	sta	MMC5_REG_FREQ_L-MMC5_START_CH*4,y
	lda	sound_freq_high,x
;	cmp	temporary2
;	beq	.mmc5_end2
	sta	MMC5_REG_FREQ_H-MMC5_START_CH*4,y
;.mmc5_end2:
	jsr	arpeggio_address
	rts
.end4
;	jsr	mmc5_freq_set
	jsr	arpeggio_address
	rts
;-------------------------------------------------------------------------------
sound_mmc5_dutyenve:
	ldx	<channel_selx2

	indirect_lda	duty_add_low		;�G���x���[�v�f�[�^�ǂݍ���
	cmp	#$ff			;�Ōォ�ǁ[��
	beq	mmc5_return22		;�Ō�Ȃ炻�̂܂܂����܂�
	asl	a
	asl	a
	asl	a
	asl	a
	asl	a
	asl	a
	ora	#%00110000		;waveform hold on & hardware envelope off
	sta	register_high,x
	ora	register_low,x		;���F�f�[�^�i���4bit�j�Ɖ���4bit�ő����Z
	ldy	<channel_selx4
	sta	MMC5_REG_CTRL-MMC5_START_CH*4,y			;�������݁`
	jsr	duty_enverope_address	;�A�h���X����₵��
	rts				;�����܂�

mmc5_return22:
	lda	duty_sel,x
	asl	a
	tay
	lda	dutyenve_lp_table,y
	sta	duty_add_low,x
	lda	dutyenve_lp_table+1,y
	sta	duty_add_high,x
	jmp	sound_mmc5_dutyenve
;-------------------------------------------------------------------------------
