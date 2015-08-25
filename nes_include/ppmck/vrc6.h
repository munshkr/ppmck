;memo
;
;9000, a000 :GDDDVVVV 
;	G=0 normal, G=1 digitized
;	D duty cycle
;	V volume
;9001, a001, b001 :FFFFFFFF
;	F lower 8 bits of freq data
;9002, a002, b002 :X---FFFF
;	X=0 channel disable, X=1 channel enable
;	F higher 4 bits of freq data
;b000 :--PPPPPP
;	P Phaser accumulator input bits
;
;9003
;	Sound initialize?

	.ifndef	VRC6_BOARD_TYPE
VRC6_BOARD_TYPE	=	0		;351949A����g�p����ꍇ1��
	.endif

	.if	VRC6_BOARD_TYPE = 1
VRC6_REG_FREQ_L	=	2
VRC6_REG_FREQ_H	=	1
	.else
VRC6_REG_FREQ_L	=	1
VRC6_REG_FREQ_H	=	2
	.endif


vrc6_sound_init:
	lda	#0
	sta	$9003
	rts

;-----------------------------------------------------------
;channel_sel��ǂ��
;VRC6��1�ԖڂȂ�[VRC6_DST_REG_LOW],y��9000+y���w���悤�ɂ���
;VRC6��2�ԖڂȂ�[VRC6_DST_REG_LOW],y��a000+y���w���悤�ɂ���
;VRC6��3�ԖڂȂ�[VRC6_DST_REG_LOW],y��b000+y���w���悤�ɂ���

vrc6_dst_adr_set:
	lda	<channel_sel
	clc				;VRC6�̉��`�����l���ڂ��H
	adc	#($09 - PTRVRC6)	;ch1�Ȃ�9
	asl	a
	asl	a
	asl	a
	asl	a			;ch1�Ȃ�90
	sta	VRC6_DST_REG_LOW+1
	lda	#0
	sta	VRC6_DST_REG_LOW	;����0��������Ȃ��̂Ŗ��ʂ���
	rts
;-----------------------------------------------------------

;���W�X�^��������

vrc6_ctrl_reg_write:
	ldy	#$00
	ldx	<channel_selx2
	cpx	#(PTRVRC6+2)*2
	beq	.saw
	lda	register_low,x
	ora	register_high,x
	and	#%01111111
	sta	[VRC6_DST_REG_LOW],y
	rts
.saw
	lda	register_low,x
	and	#%00111111
	sta	[VRC6_DST_REG_LOW],y
	rts

vrc6_frq_reg_write:
	ldx	<channel_selx2
	lda	sound_freq_low,x
	ldy	#VRC6_REG_FREQ_L
	sta	[VRC6_DST_REG_LOW],y
	lda	sound_freq_high,x
	ora	#%10000000
	ldy	#VRC6_REG_FREQ_H
	sta	[VRC6_DST_REG_LOW],y
	rts

vrc6_frq_low_reg_write:
	ldx	<channel_selx2
	lda	sound_freq_low,x
	ldy	#VRC6_REG_FREQ_L
	sta	[VRC6_DST_REG_LOW],y
	rts

vrc6_frq_high_reg_write:
	ldx	<channel_selx2
	lda	sound_freq_high,x
	ora	#%10000000
	ldy	#VRC6_REG_FREQ_H
	sta	[VRC6_DST_REG_LOW],y
	rts

vrc6_mute_write:
	ldy	#$00
	ldx	<channel_selx2
	cpx	#(PTRVRC6+2)*2
	beq	.saw
	lda	register_low,x
	ora	register_high,x
	and	#%01110000
	sta	[VRC6_DST_REG_LOW],y
	rts
.saw
	lda	#$00
	sta	[VRC6_DST_REG_LOW],y
	rts
	
	;lda	sound_freq_high,x
	;and	#%01111111		;channel disable
	;ldy	#VRC6_REG_FREQ_H
	;sta	[VRC6_DST_REG_LOW],y
	;rts
	

;----------------------------------------
sound_vrc6:
	lda	<channel_sel
	cmp	#PTRVRC6+3
	beq	.end1
	jsr	vrc6_dst_adr_set
	ldx	<channel_selx2
	dec	sound_counter,x		;�J�E���^���������炵
	beq	.sound_read_go		;�[���Ȃ�T�E���h�ǂݍ���
	jsr	vrc6_do_effect		;�[���ȊO�Ȃ�G�t�F�N�g����
	rts				;�����
.sound_read_go
	jsr	sound_vrc6_read
	jsr	vrc6_do_effect
	lda	rest_flag,x
	and	#%00000010		;�L�[�I���t���O
	beq	.end1			
	jsr	sound_vrc6_write	;�����Ă�����f�[�^�����o��
	lda	rest_flag,x
	and	#%11111101		;�L�[�I���t���O�I�t
	sta	rest_flag,x
.end1
	rts

;-------
vrc6_do_effect:
	lda	rest_flag,x
	and	#%00000001
	beq	.duty_write2
	rts				;�x���Ȃ�I���

.duty_write2:
	lda	effect_flag,x
	and	#%00000100
	beq	.enve_write2
	jsr	sound_vrc6_dutyenve

.enve_write2:
	lda	effect_flag,x
	and	#%00000001
	beq	.lfo_write2
	jsr	sound_vrc6_softenve

.lfo_write2:
	lda	effect_flag,x
	and	#%00010000
	beq	.pitchenve_write2
	jsr	sound_vrc6_lfo

.pitchenve_write2:
	lda	effect_flag,x
	and	#%00000010
	beq	.arpeggio_write2
	jsr	sound_vrc6_pitch_enve

.arpeggio_write2:
	lda	effect_flag,x
	and	#%00001000
	beq	.return7
	lda	rest_flag,x		;�L�[�I���̂Ƃ��Ƃ����łȂ��Ƃ��ŃA���y�W�I�̋����͂�����
	and	#%00000010		;�L�[�I���t���O
	bne	.arpe_key_on
	jsr	sound_vrc6_note_enve	;�L�[�I������Ȃ��Ƃ��ʏ�͂���
	jmp	.return7
.arpe_key_on				;�L�[�I���������̏ꍇ
	jsr	note_enve_sub		;���������������ŁA�����ł͏������݂͂��Ȃ�
	jsr	vrc6_freq_set
	jsr	arpeggio_address
.return7:
	rts

;------------------------------------------------
vrc6_freq_set:
	ldx	<channel_selx2
	lda	sound_sel,x		;���K�f�[�^�ǂݏo��
	and	#%00001111		;����4bit�����o����
	asl	a
	tay
	lda	<channel_sel
	cmp	#PTRVRC6+2
	beq	.vrc6_saw_frequency_get

	lda	vrc6_pls_frequency_table,y	;PSG���g���e�[�u������Low��ǂݏo��
	sta	sound_freq_low,x	;��������
	lda	vrc6_pls_frequency_table+1,y	;PSG���g���e�[�u������High��ǂݏo��
	sta	sound_freq_high,x	;��������
	jmp	.vrc6_oct_set1
	
.vrc6_saw_frequency_get:
	lda	vrc6_saw_frequency_table,y	;���g���e�[�u������Low��ǂݏo��
	sta	sound_freq_low,x	;��������
	lda	vrc6_saw_frequency_table+1,y	;���g���e�[�u������High��ǂݏo��
	sta	sound_freq_high,x	;��������
	
.vrc6_oct_set1:

	lda	sound_sel,x		;���K�f�[�^�ǂݏo��
	lsr	a			;���4bit�����o��
	lsr	a			;
	lsr	a			;
	lsr	a			;
;	pha				;��U���
;	lda	<channel_sel
;	cmp	#PTRVRC6+2
;	beq	.saw_skip
;.squ_oct_adjust				;��`�g �I�N�^�[�u����
;	pla				;�R���p�C�����ł�����ق��������̂��H
	sec
	sbc	#$01
;	jmp	.branch_end
;.saw_skip				;�m�R�M���g �I�N�^�[�u����
;	pla				;
;	sec
;	sbc	#$
;.branch_end
	beq	vrc6_freq_end		;�[���Ȃ炻�̂܂܏I���
	tay

vrc6_oct_set2:

	lsr	sound_freq_high,x	;�E�V�t�g�@������C��
	ror	sound_freq_low,x	;C���玝���Ă���ł�@�E���[�e�C�g
	dey				;
	bne	vrc6_oct_set2		;�I�N�^�[�u���J��Ԃ�

vrc6_freq_end:
	jsr	detune_write_sub
	rts
;---------------------------------------------------------------
sound_vrc6_read:
	ldx	<channel_selx2
	
	lda	sound_bank,x
	jsr	change_bank
	
	lda	[sound_add_low,x]
;----------
;���[�v����1
vrc6_loop_program
	cmp	#$a0
	bne	vrc6_loop_program2
	jsr	loop_sub
	jmp	sound_vrc6_read
;----------
;���[�v����2(����)
vrc6_loop_program2
	cmp	#$a1
	bne	vrc6_bank_command
	jsr	loop_sub2
	jmp	sound_vrc6_read
;----------
;�o���N�؂�ւ�
vrc6_bank_command
	cmp	#$ee
	bne	vrc6_wave_set
	jsr	data_bank_addr
	jmp	sound_vrc6_read
;----------
;�f�[�^�G���h�ݒ�
;vrc6_data_end:
;	cmp	#$ff
;	bne	vrc6_wave_set
;	jsr	data_end_sub
;	jmp	sound_vrc6_read
;----------
;���F�ݒ�
vrc6_wave_set:
	cmp	#$fe
	bne	vrc6_volume_set
	jsr	sound_data_address
	lda	[sound_add_low,x]	;���F�f�[�^�ǂݏo��
	pha
	bpl	vrc6_duty_enverope_part	;�a���[�e�B�G���x������

vrc6_duty_select_part:
	lda	effect_flag,x
	and	#%11111011
	sta	effect_flag,x		;�f���[�e�B�G���x���[�v�����w��
	ldx	<channel_selx2
	pla
	asl	a
	asl	a
	asl	a
	asl	a
	sta	register_high,x		;��������
	jsr	vrc6_ctrl_reg_write
	jsr	sound_data_address
	jmp	sound_vrc6_read

vrc6_duty_enverope_part:
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
	jmp	sound_vrc6_read

;----------
;���ʐݒ�
vrc6_volume_set:
	cmp	#$fd
	bne	vrc6_rest_set
	jsr	sound_data_address
	lda	[sound_add_low,x]
	sta	temporary
	bpl	vrc6_softenve_part		;�\�t�g�G���x������

vrc6_volume_part:
	lda	effect_flag,x
	and	#%11111110
	sta	effect_flag,x		;�\�t�g�G���x�����w��

	lda	<channel_sel
	cmp	#PTRVRC6+2
	beq	.saw
	lda	temporary
	and	#%00001111
	jmp	.kakikomi
.saw
	lda	temporary
	and	#%00111111
.kakikomi
	sta	register_low,x
	jsr	vrc6_ctrl_reg_write
	jsr	sound_data_address
	jmp	sound_vrc6_read

vrc6_softenve_part:
	jsr	volume_sub
	jmp	sound_vrc6_read

;----------
vrc6_rest_set:
	cmp	#$fc
	bne	vrc6_lfo_set

	lda	rest_flag,x
	ora	#%00000001
	sta	rest_flag,x

	jsr	sound_data_address
	lda	[sound_add_low,x]
	sta	sound_counter,x

	jsr	vrc6_mute_write

	jsr	sound_data_address
	rts
;----------
vrc6_lfo_set:
	cmp	#$fb
	bne	vrc6_detune_set
	jsr	lfo_set_sub
	jmp	sound_vrc6_read
;----------
vrc6_detune_set:
	cmp	#$fa
	bne	vrc6_pitch_set
	jsr	detune_sub
	jmp	sound_vrc6_read
;----------
;�s�b�`�G���x���[�v�ݒ�
vrc6_pitch_set:
	cmp	#$f8
	bne	vrc6_arpeggio_set
	jsr	pitch_set_sub
	jmp	sound_vrc6_read
;----------
;�m�[�g�G���x���[�v�ݒ�
vrc6_arpeggio_set:
	cmp	#$f7
	bne	vrc6_freq_direct_set
	jsr	arpeggio_set_sub
	jmp	sound_vrc6_read
;----------
;�Đ����g�����ڐݒ�
vrc6_freq_direct_set:
	cmp	#$f6
	bne	vrc6_y_command_set
	jsr	direct_freq_sub
	rts
;----------
;���R�}���h�ݒ�
vrc6_y_command_set:
	cmp	#$f5
	bne	vrc6_wait_set
	jsr	y_sub
	jmp	sound_vrc6_read
;----------
;�E�F�C�g�ݒ�
vrc6_wait_set:
	cmp	#$f4
	bne	vrc6_oto_set
	jsr	wait_sub
	rts
;----------
vrc6_oto_set:
	sta	sound_sel,x		;�����͂܂����
	jsr	sound_data_address
	lda	[sound_add_low,x]	;�����ǂݏo��
	sta	sound_counter,x		;���ۂ̃J�E���g�l�ƂȂ�܂�
	jsr	sound_data_address
	jsr	vrc6_freq_set		;���g���Z�b�g��
	jsr	effect_init
	rts
;-------------------------------------------------------------------------------
sound_vrc6_write:
	jsr	vrc6_ctrl_reg_write
	jsr	vrc6_frq_reg_write
	rts
;-----------------------------------------------------
sound_vrc6_softenve:
	jsr	volume_enve_sub
	sta	register_low,x
	jsr	vrc6_ctrl_reg_write
	jsr	enverope_address	;�A�h���X����₵��
	rts				;�����܂�
;-------------------------------------------------------------------------------
sound_vrc6_lfo:
	lda	sound_freq_high,x
	sta	temporary
	jsr	lfo_sub
	jsr	vrc6_frq_low_reg_write
	lda	sound_freq_high,x
	cmp	temporary
	beq	vrc6_end4
	jsr	vrc6_frq_high_reg_write
vrc6_end4:
	rts
;-------------------------------------------------------------------------------
sound_vrc6_pitch_enve:
	lda	sound_freq_high,x
	sta	temporary
	jsr	pitch_sub
vrc6_pitch_write:
	jsr	vrc6_frq_low_reg_write
	lda	sound_freq_high,x
	cmp	temporary
	beq	vrc6_end3
	jsr	vrc6_frq_high_reg_write
vrc6_end3:
	jsr	pitch_enverope_address
	rts
;-------------------------------------------------------------------------------
sound_vrc6_note_enve
;	lda	sound_freq_high,x
;	sta	temporary2
	jsr	note_enve_sub
	bcs	.end4			;0�Ȃ̂ŏ����Ȃ��Ă悵
	jsr	vrc6_freq_set
;.vrc6_note_freq_write:
	ldx	<channel_selx2
	jsr	vrc6_frq_low_reg_write
	lda	sound_freq_high,x
;	cmp	temporary2
;	beq	.vrc6_end2
	jsr	vrc6_frq_high_reg_write
;.vrc6_end2:
	jsr	arpeggio_address
	rts
.end4
;	jsr	vrc6_freq_set
	jsr	arpeggio_address
	rts
;-------------------------------------------------------------------------------
sound_vrc6_dutyenve:
	ldx	<channel_selx2

	indirect_lda	duty_add_low		;�G���x���[�v�f�[�^�ǂݍ���
	cmp	#$ff			;�Ōォ�ǁ[��
	beq	vrc6_return22		;�Ō�Ȃ炻�̂܂܂����܂�
	asl	a
	asl	a
	asl	a
	asl	a
	sta	register_high,x
	jsr	vrc6_ctrl_reg_write
	jsr	duty_enverope_address	;�A�h���X����₵��
	rts				;�����܂�

vrc6_return22:
	lda	duty_sel,x
	asl	a
	tay
	lda	dutyenve_lp_table,y
	sta	duty_add_low,x
	lda	dutyenve_lp_table+1,y
	sta	duty_add_high,x
	jmp	sound_vrc6_dutyenve
;-------------------------------------------------------------------------------
vrc6_pls_frequency_table
;psg_frequency_table�̊e�l��2�{�Ɠ����͂��Ȃ̂�����
	dw	$0D5C, $0C9D, $0BE7, $0B3C
	dw	$0A9B, $0A02, $0973, $08EB
	dw	$086B, $07F2, $0780, $0714
	dw	$0000, $0FE4, $0EFF, $0E28
	
vrc6_saw_frequency_table:
	dw	$0F45, $0E6A, $0D9B, $0CD7
	dw	$0C1F, $0B71, $0ACC, $0A31
	dw	$099F, $0914, $0892, $0817
	dw	$0000, $0000, $0000, $0000

