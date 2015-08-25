fds_sound_init:
	lda	#$00
	sta	$4080
	sta	$4082
	sta	$4083
	sta	$4084
	sta	$4085
	sta	$4086
	sta	$4087
	sta	$4088
	sta	$4089
	lda	#$e8
	sta	$408a
	rts

sound_fds:
	ldx	<channel_selx2
	dec	sound_counter,x		;�J�E���^���������炵
	beq	.sound_read_go		;�[���Ȃ�T�E���h�ǂݍ���
	jsr	fds_do_effect		;�[���ȊO�Ȃ�G�t�F�N�g����
	rts				;�����
.sound_read_go
	jsr	sound_fds_read
	jsr	fds_do_effect
	lda	rest_flag,x
	and	#%00000010		;�L�[�I���t���O
	beq	.end1			
	jsr	sound_fds_write	;�����Ă�����f�[�^�����o��
	lda	rest_flag,x
	and	#%11111101		;�L�[�I���t���O�I�t
	sta	rest_flag,x
.end1
	rts

;-------
fds_do_effect:
	lda	rest_flag,x
	and	#%00000001
	beq	.duty_write2
	rts				;�x���Ȃ�I���

.duty_write2:

.enve_write2:
	lda	effect_flag,x
	and	#%00000001
	beq	.lfo_write2
	jsr	sound_fds_softenve

.lfo_write2:
	lda	effect_flag,x
	and	#%00010000
	beq	.pitchenve_write2
	jsr	sound_fds_lfo

.pitchenve_write2:
	lda	effect_flag,x
	and	#%00000010
	beq	.arpeggio_write2
	jsr	sound_fds_pitch_enve

.arpeggio_write2:
	lda	effect_flag,x
	and	#%00001000
	beq	.hardenve_write2
	lda	rest_flag,x		;�L�[�I���̂Ƃ��Ƃ����łȂ��Ƃ��ŃA���y�W�I�̋����͂�����
	and	#%00000010		;�L�[�I���t���O
	bne	.arpe_key_on
	jsr	sound_fds_note_enve	;�L�[�I������Ȃ��Ƃ��ʏ�͂���
	jmp	.hardenve_write2
.arpe_key_on				;�L�[�I���������̏ꍇ
	jsr	note_enve_sub		;���������������ŁA�����ł͏������݂͂��Ȃ�
	jsr	fds_freq_set
	jsr	arpeggio_address
.hardenve_write2:
	lda	effect_flag,x
	and	#%00000100
	beq	.return201
	jsr	sound_fds_hard_enve
	ldx	<channel_selx2
.return201
	rts






;------------------------------------------------
fds_freq_set:
	ldx	<channel_selx2
	lda	sound_sel,x		;���K�f�[�^�ǂݏo��
	and	#%00001111		;����4bit�����o����
	asl	a
	tay

	lda	fds_frequency_table,y	;PSG���g���e�[�u������Low��ǂݏo��
	sta	sound_freq_low,x	;��������
	lda	fds_frequency_table+1,y	;PSG���g���e�[�u������High��ǂݏo��
	sta	sound_freq_high,x	;��������

fds_oct_set1:

	lda	sound_sel,x		;���K�f�[�^�ǂݏo��
	lsr	a			;���4bit�����o��
	lsr	a			;
	lsr	a			;
	lsr	a			;
	cmp	#$06
	beq	fds_freq_end		;�[���Ȃ炻�̂܂܏I���
	tay

fds_oct_set2:

	lsr	sound_freq_high,x	;�E�V�t�g�@������C��
	ror	sound_freq_low,x	;C���玝���Ă���ł�@�E���[�e�C�g
	iny				;
	cpy	#$06
	bne	fds_oct_set2		;�I�N�^�[�u���J��Ԃ�

fds_freq_end:
	jsr	detune_write_sub
	rts

;-------------------------------------------------------------------------------
fds_frequency_table:
	dw	$0995,$0a26,$0ac0,$0b64	;c ,c+,d ,d+
	dw	$0c11,$0cc9,$0d8c,$0e5a	;e ,f ,f+,g
	dw	$0f35,$101c,$1110,$1214	;g+,a ,a+,b
	dw	$0000,$0000,$0000,$0000

;���ϗ��i�ɋ߂����l�j����ɂ��Ă��܂�
;���͈ȉ��Ȋ�����
;              111860.8 Hz  
;�Đ����g�� = ------------- x n(���g���p�f�[�^)
;	  	64 x 4096
;o6a����̉��͂ł܂���i�e�[�u����o6�̃��m�j
;������������قǉ��s�ɂȂ�܂��H

;        ���j�]���F	x1.0000
;        �Z�Q�x�F	x1.0595
;        ���Q�x�F	x1.1225
;        �Z�R�x�F	x1.1892
;        ���R�x�F	x1.2599
;        ���S�S�x�F	x1.3348
;        ���S�x(���T�x):x1.4142
;        ���S�T�x�F	x1.4983
;        ���T�x(�Z�U�x):x1.5874 
;        ���U�x�F	x1.6818
;        ���V�x�F	x1.7818
;        ���V�x�F	x1.8877
;        �I�N�^�[�u �F	x2.0000 


;---------------------------------------------------------------
sound_fds_read:
	ldx	<channel_selx2

	lda	sound_bank,x
	jsr	change_bank

	lda	[sound_add_low,x]
;----------
;���[�v����1
fds_loop_program
	cmp	#$a0
	bne	fds_loop_program2
	jsr	loop_sub
	jmp	sound_fds_read
;----------
;���[�v����2(����)
fds_loop_program2
	cmp	#$a1
	bne	fds_bank_set
	jsr	loop_sub2
	jmp	sound_fds_read
;----------
;�o���N��؂�ւ��܂��`FDS��
fds_bank_set
	cmp	#$ee
	bne	fds_wave_set
	jsr	data_bank_addr
	jmp	sound_fds_read
;----------
;�f�[�^�G���h�ݒ�
;fds_data_end:
;	cmp	#$ff
;	bne	fds_wave_set
;	jsr	data_end_sub
;	jmp	sound_fds_read
;----------
;���F�ݒ�
fds_wave_set:
	cmp	#$fe
	bne	fds_volume_set
	jsr	sound_data_address
	lda	[sound_add_low,x]

	asl	a
	tax
	lda	fds_data_table,x
	sta	<temp_data_add
	inx
	lda	fds_data_table,x
	sta	<temp_data_add+1

	ldy	#$00
	ldx	#$00
	lda	#$80
	sta	$4089
wave_data_set:
	lda	[temp_data_add],y
	sta	$4040,y
	iny
	cpy	#$40
	bne	wave_data_set

	lda	#$00
	sta	$4089
	ldx	<channel_selx2
	jsr	sound_data_address
	jmp	sound_fds_read
;----------
;���ʐݒ�
fds_volume_set:
	cmp	#$fd
	bne	fds_rest_set
	jsr	sound_data_address
	lda	[sound_add_low,x]
	bpl	fds_softenve_part	;bit7��0�Ȃ�\�t�g�G���x������

fds_volume_part:
;	ora	#%10000000		;��ɒ��ڃ��[�h
	and	#%10111111
	sta	fds_volume
;	sta	$4080			;�{�����[�����n�[�h�G���x��������

	lda	effect_flag,x
	and	#%11111110
	sta	effect_flag,x		;�\�t�g�G���x�����w��
	
	jsr	sound_data_address
	jmp	sound_fds_read

fds_softenve_part:
	sta	softenve_sel,x		;0 �` 127�̔ԍ�
	asl	a
	tay
	lda	softenve_table,y	;�\�t�g�G���x�f�[�^�A�h���X�ݒ�
	sta	soft_add_low,x
	lda	softenve_table+1,y
	sta	soft_add_low+1,x
	
	lda	effect_flag,x
	ora	#%00000001
	sta	effect_flag,x		;�\�t�g�G���x�L���w��
	
	jsr	sound_data_address
	jmp	sound_fds_read
;----------
fds_rest_set:
	cmp	#$fc
	bne	fds_lfo_set

	lda	rest_flag,x
	ora	#%00000001
	sta	rest_flag,x

	jsr	sound_data_address
	lda	[sound_add_low,x]
	sta	sound_counter,x

	lda	#$00
	sta	$4080
	jsr	sound_data_address
	rts
;----------
fds_lfo_set:
	cmp	#$fb
	bne	fds_detune_set
	jsr	lfo_set_sub
	jmp	sound_fds_read
;----------
fds_detune_set:
	cmp	#$fa
	bne	fds_pitch_set
	jsr	detune_sub
	jmp	sound_fds_read
;----------
;�s�b�`�G���x���[�v�ݒ�
fds_pitch_set:
	cmp	#$f8
	bne	fds_arpeggio_set
	jsr	pitch_set_sub
	jmp	sound_fds_read
;----------
;�m�[�g�G���x���[�v�ݒ�
fds_arpeggio_set:
	cmp	#$f7
	bne	fds_freq_direct_set
	jsr	arpeggio_set_sub
	jmp	sound_fds_read
;----------
;�Đ����g�����ڐݒ�
fds_freq_direct_set:
	cmp	#$f6
	bne	fds_y_command_set
	jsr	direct_freq_sub
	rts
;----------
;���R�}���h�ݒ�
fds_y_command_set:
	cmp	#$f5
	bne	fds_wait_set
	jsr	y_sub
	jmp	sound_fds_read
;----------
;�E�F�C�g�ݒ�
fds_wait_set:
	cmp	#$f4
	bne	fds_hard_lfo_set
	jsr	wait_sub
	rts
;----------
;FDS�����n�[�h�E�F�A�G�t�F�N�g�ݒ�
fds_hard_lfo_set:
	cmp	#$f3
	bne	fds_hwenv_set

	jsr	sound_data_address
	lda	[sound_add_low,x]
	cmp	#$ff
	bne	fds_hard_lfo_data_set

	ldx	<channel_selx2
	lda	effect_flag,x
	and	#%11111011
	sta	effect_flag,x
	jsr	sound_data_address
	jmp	sound_fds_read
fds_hard_lfo_data_set:
	asl	a
	asl	a
	asl	a
	asl	a
	tay

	sta	fds_hard_select
	inc	fds_hard_select
	lda	fds_effect_select,y
	sta	fds_hard_count_1
	sta	fds_hard_count_2
	ldx	<channel_selx2
	lda	effect_flag,x
	ora	#%00000100
	sta	effect_flag,x
	jsr	sound_data_address
	jmp	sound_fds_read
;----------
;�n�[�h�E�F�A�{�����[���G���x���[�v
fds_hwenv_set:
	cmp	#$f0
	bne	fds_oto_set
	jsr	sound_data_address
	lda	[sound_add_low,x]

	and	#%01111111		;�ꉞ
	sta	fds_volume
;	sta	$4080			;�{�����[�����n�[�h�G���x��������

	lda	effect_flag,x
	and	#%11111110
	sta	effect_flag,x		;�\�t�g�G���x�����w��
	jsr	sound_data_address
	jmp	sound_fds_read
;----------
fds_oto_set:
	sta	sound_sel,x		;�����͂܂����
	jsr	sound_data_address
	lda	[sound_add_low,x]	;�����ǂݏo��
	sta	sound_counter,x		;���ۂ̃J�E���g�l�ƂȂ�܂�
	jsr	sound_data_address
	lda	#$00
	sta	$4083
;	sta	rest_flag,x		;effect_init�ł���Ă�̂ł���Ȃ�
	jsr	fds_freq_set		;���g���Z�b�g��
	lda	fds_volume
	sta	$4080
	jsr	effect_init
	ldx	<channel_selx2
;hard enveope init

	lda	fds_hard_count_2
	sta	fds_hard_count_1
	lda	#$00
	sta	$4084
	sta	$4085
	sta	$4086
	sta	$4087
	rts
;-------------------------------------------------------------------------------
sound_fds_write:
	ldx	<channel_selx2
	lda	sound_freq_low,x	;Low Write
	sta	$4082
	lda	sound_freq_high,x	;High Write
	sta	$4083
	rts
;-------------------------------------------------------------------------------
sound_fds_softenve:
	jsr	volume_enve_sub
	and	#%00111111
	ora	#$80
	sta	$4080
	jsr	enverope_address
	rts
;-------------------------------------------------------------------------------
sound_fds_lfo:
	jsr	lfo_sub
	jsr	sound_fds_write
	rts
;-------------------------------------------------------------------------------
sound_fds_pitch_enve:
	jsr	pitch_sub
	jsr	sound_fds_write
	jsr	pitch_enverope_address
	rts
;-------------------------------------------------------------------------------
sound_fds_note_enve
	jsr	note_enve_sub
	bcs	.end4			;0�Ȃ̂ŏ����Ȃ��Ă悵
	jsr	fds_freq_set
	jsr	sound_fds_write
	jsr	arpeggio_address
	rts
.end4
;	jsr	fds_freq_set
	jsr	arpeggio_address
	rts
;-------------------------------------------------------------------------------
sound_fds_hard_enve:
	lda	fds_hard_count_1
	beq	start_effect
	cmp	#$ff
	beq	return6809
	dec	fds_hard_count_1
return6809:
	rts
start_effect:
	lda	fds_hard_select
	tax
start_effect_2:
	lda	fds_effect_select,x
	tay
	inx
	cmp	#$ff
	beq	count_effect
	cmp	#$88
	beq	set_4088
	lda	fds_effect_select,x
	sta	$4000,y
	inx
	jmp	start_effect_2

count_effect:
	dec	fds_hard_count_1
	rts
set_4088:
	lda	fds_effect_select,x
	stx	temporary

	asl	a
	asl	a
	asl	a
	asl	a
	asl	a
	tay
	ldx	#$00
fds_4088_write:
	lda	fds_4088_data,y
	sta	$4088
	iny
	inx
	cpx	#$20
	beq	oshimai
	jmp	fds_4088_write
oshimai:
	ldx	temporary
	inx
	jmp	start_effect_2
;-------------------------------------------------------------------------------
