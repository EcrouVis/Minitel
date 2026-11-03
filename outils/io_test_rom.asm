;io_test_rom.asm
;
;rom to test the io of the minitel M12
;
;compilation tested with naken_asm (https://www.mikekohn.net/micro/naken_asm.php)

;limitations (to reduce program complexity):
;as small and simple as possible (<2048 bytes ? - exclusively use ajmp/acall)
;external memory not used
;address bits 16 and 17 not used

;program the interrupts
;set the serial at 9600bauds 8bits+parity
;kick the watchdog timer (100Hz)
;power up serial connection

;commands (address: 0bABCDEFGH / data: 0bIJKLMNOP / R=read / W=write / U=bit undefined / reading is performed right after writing)
;command: 0b0101WRAI 0b1BCDEFGH 0b1JKLMNOP (0bIJKLMNOP -> data to write / data is omitted if unused - ex: 0b010101AU 0b1BCDEFGH)
;ack: 0b0101WRUI 0b1JKLMNOP (0bIJKLMNOP -> data that has been read / data is omitted if unused - ex: 0b010110UU)
;nack: 0b010100UU
;when the watchdog timer kick in -> level low

;memory organization:
;0x00 - 0x07: R0 - R7 (bank 0)
;	temp variables
;	variables used by all the interrupts because the watchdog timer interrupt cannot run while the serial interrupt executes and vice versa
;0x08 - 0x0F: R0 - R7 (bank 1)
;0x10 - 0x17: R0 - R7 (bank 2)
;0x18 - 0x1F: R0 - R7 (bank 3)
;0x20 - 0x2F: byte and bit addressable memory
;	0x20: IO 0x70 current state
;		0x20.3: watchdog timer state
;	0x21.0: Tx free
;	0x22 - 0x24: command input
;0x30 - 0x7F: memory with standard access
;	0x34: command input counter
;	0x35: command output counter write
;	0x36: command output counter serial
;	0x40 - 0x4F: command output queue
;0x80 - 0xFF: indirect addressing memory
;	0xA0 - 0xFF: stack

;hardware:
;timer 1 used as baudrate generator
;	(RCAP2H,RCAP2L)=65536-fosc/(32*baudrate)
;nINT0 used for watchdog warning
;timer 2 used for kicking the watchdog timer (200Hz interrupt -> 100Hz kick)
;hardware serial used for the commands inputs/outputs

;interrupt priority (general case - higher to lower):
;	watchdog timer warn
;	serial
;	watchdog timer kick
;	main program
;interrupt priority (inside watchdog timer kick - higher to lower):
;	watchdog timer warn
;	watchdog timer kick
;	serial
;	main program

.8051

;registers
SP equ 0x81
ACC equ 0xE0;A when used as a direct address
B equ 0xF0
TMOD equ 0x89
TH1 equ 0x8D
SCON equ 0x98
RCAP2H equ 0xCB
RCAP2L equ 0xCA
T2CON equ 0xC8
IP equ 0xB8
IE equ 0xA8
SBUF equ 0x99
PSW equ 0xD0

TR1 equ 0x88.6
TF2 equ T2CON.7
RI equ SCON.0
TI equ SCON.1
P equ PSW.0
PT2 equ IP.5
RB8 equ SCON.2
TB8 equ SCON.3
TxD equ 0xB0.1

nIO equ 0x90.5

;memory
IO70 equ 0x20
TxFree equ 0x21.0
input_counter equ 0x34
output_counter_write equ 0x35
output_counter_serial equ 0x36
p_command_input equ #0x22
command_type equ 0x22
command_address equ 0x23
command_data equ 0x24
command_data_msb equ command_type.0
command_address_msb equ command_type.1
command_read equ command_type.2
command_write equ command_type.3
p_output_buffer equ #0x40

;data
nak equ #0b01010000

;code
.org 0x0000
start:
	clr A
	ajmp main

.org 0x0003
external0_isr:;watchdog timer warn
	clr TxD;force Tx to 0
	sjmp $;while(true) loop

w_io:;@R0=A / C used
	cjne R0,#0x70,w_io_ism
	mov C,IO70.3
	mov ACC.3,C
	mov IO70,A
	w_io_ism:
	movx @R0,A
	ret

wr_io:;@R0=A;A=@R0 / C used
	cjne R0,#0x70,wr_io_ism
	mov C,IO70.3
	mov ACC.3,C
	mov IO70,A
	wr_io_ism:
	movx @R0,A
	movx A,@R0
	ret

r_io:;A=@R0
	movx A,@R0
	ret

.org 0x0023
serial_isr:
	jnb RI,serial_isr_next
	acall serial_rx
	serial_isr_next:
	acall serial_tx
	reti

.org 0x002B
timer2_isr:
	setb PT2;set the watchdog timer priority to be higher than the serial priority while inside the watchdog timer interrupt (avoid accessing IO70 simultaneously)
	clr TF2
	
	mov R0,#0x70
	cpl IO70.3
	mov A,IO70
	movx @R0,A
	
	clr PT2
	reti

main:
	mov SP,#0xA0;set SP to point to a location in memory that is not reachable by a big part of the instructions (stack: 0xA0 - 0xFF)
	
	;init variables
	mov R0,#0xFF
	init_mem_0:
	mov @R0,#0;init all memory (including input_counter, output_counter_write and output_counter_serial) to 0
	djnz R0,init_mem_0
	
	clr nIO;select IO
	
	;mov R0,#0x51
	;mov A,#0x80
	;acall w_io;power up IC?
	;mov R0,#0xFF
	;wait_init:
	;djnz R0,wait_init
	;mov R0,#0x51
	;mov A,#0x00
	;acall w_io
	
	mov R0,#0x70
	mov A,#0x00
	acall w_io;init IO70 and activate DIN power pin

	mov TMOD,#0b00100000;set timer 1 to mode 2 for serial clock generation
	;SMOD already 0
	mov TH1,#0xFC;9600bauds
	
	mov RCAP2H,#0xE8
	;mov RCAP2L,#0x00;200Hz / RCAP2L initialized at 0 when reset

	mov IP,#0b00000001;priority to nINT0 then serial then watchdog timer kick
	mov IE,#0b10110001
	
	setb TxFree
	
	setb TR1;enable serial clock
	mov SCON,#0b11010000;set serial to mode 3 and enable serial Rx
	
	mov T2CON,#0b00000100;set timer 2 in auto reload mode and enable timer 2
	
	sjmp $;while(true) loop
	
serial_rx:
	clr RI;clear interrupt
	
	mov A,SBUF
	mov R1,A
	;mov @R0,A
	
	mov B,PSW;copy parity to B.0
	mov C,RB8
	rlc A;set RB8 to A.0 (with rotate left with carry)
	xrl A,B;A.0=P XOR RB8
	
	jb ACC.0,no_parity_error
	cmd_error:
		mov input_counter,#0
		
		mov A,p_output_buffer
		add A,output_counter_write
		mov R0,A
		mov @R0,nak
		inc output_counter_write
		anl output_counter_write,#0x0F;nak command and increment output counter write
		ret
	no_parity_error:
	mov A,R1
	anl A,#0xF0
	cjne A,#0x50,cmd_args
	;command type -> resync if needed
		mov A,R1
		mov R0,p_command_input
		mov @R0,A
		
		anl A,#0x0C
		jz cmd_error;command type cannot have R==0 and W==0
		
		mov input_counter,#1
		ret
	cmd_args:
	;command args
		mov A,R1
		jnb ACC.7,cmd_error;command argument cannot have msb==0
		
		mov A,p_command_input
		add A,input_counter
		xch A,R1
		mov @R1,A;construct command
		
		;execute command
		
		;command_address may be uninitialized
		mov C,command_address_msb
		mov command_address.7,C;reconstruct command address
		mov R0,command_address
		
		jnb command_write,execute_command_r
		
			;command_data may be uninitialized
			mov C,command_data_msb
			mov command_data.7,C;reconstruct command data
		
			mov A,input_counter
			cjne A,#1,execute_command_w
				ret
			execute_command_w:
				jb command_read,execute_command_wr
					mov A,command_data
					acall w_io
					ajmp output_result
				execute_command_wr:
					mov A,command_data
					acall wr_io
					ajmp output_result
		execute_command_r:
			acall r_io
		output_result:
		mov input_counter,#0
			
		;A (command result) may be uninitialized
		mov C,ACC.7
		mov command_data_msb,C
		setb ACC.7;mov data msb to command_type and set i to 1
		mov R1,A;backup result if read command
		
		mov A,p_output_buffer
		add A,output_counter_write
		mov R0,A
		mov @R0,command_type
		inc output_counter_write
		anl output_counter_write,#0x0F;
		
		jnb command_read,end_cmd
			mov A,p_output_buffer
			add A,output_counter_write
			xch A,R1
			mov @R1,A
			inc output_counter_write
			anl output_counter_write,#0x0F;
		end_cmd:
			
	ret

serial_tx:
	;could send data if TI or TxFree
	mov C,TI
	orl C,TxFree
	jnc tx_end
		clr TI;clear flag
		
		;check if there is data in the buffer
		mov A,output_counter_serial
		cjne A,output_counter_write,tx_send
			setb TxFree
			ret
		tx_send:
		clr TxFree
		
		add A,p_output_buffer
		mov R0,A
		mov A,@R0;read output buffer
		
		mov C,P
		mov TB8,C
		mov SBUF,A;send data
		
		inc output_counter_serial
		anl output_counter_serial,#0x0F;increment read pointer
	tx_end:
	ret