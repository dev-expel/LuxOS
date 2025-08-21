; mnemonic operand1, operand2
; magic number, first sector
org 0x0 ; BIOS loads OS at this addr
bits 16 ; emit 16 bit code

%define ENDL 0x0D, 0x0A

; BIOS = Basic Input Output System
; interrupts = signals to stop the processer from what it's doing to handle the signal
; int 10 - video services

; GOAL: load rest of OS and switch to 64 bit mode

start:
    ; print msg
    mov si, onload_msg
    call prints

.halt:
    cli
    hlt

;
; prints a string to the screen.
; params:
;   - ds:si points to a string
prints:
    push si
    push ax
    jmp .loop

.loop:
    lodsb           ; loads next char in al
    or al, al       ; verify if next char is null
    jz .done

    mov ah, 0x0e    ; call BIOS interrupt
    mov bh, 0
    int 0x10

    jmp .loop

.done:
    pop ax
    pop si
    ret

onload_msg: db 'LOADED: kernel.bin', ENDL, 0