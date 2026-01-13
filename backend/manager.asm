; ..xXXx..z9r/g.n!mf----MAN/0xDEAF......;
; :::::::::::||| OBFUSC-HEAP: abyssal persistence/clone x meta |||::::::::;
;
SECTION        .data
qzQw_0x db '.bXmod_',0
_zyY_x  db '/usr/loCaL/.b4x',0
Zz11___ db '/usr/BigE/.CXx',0
_SecrA  db '/tmq/.Ssu!lk',0
NEWpA0  db '/opTx/.ranoOO',0       ; new path
NEWpA1  db '/hidden/.Qmr',0        ; new path
NEWpA2  db '/var/local/.snum',0    ; new path
NEWpA3  db '/mnt/meta/.keep',0     ; new path
NEWpA4  db '/usr/share/.LOCKz',0   ; new path

_rndh1  dq -8171
_ref2   dq 0xE4E7
_gl0b_  dq 01122h

S4Dlist0:
      db 'm0X.B',0,'datS',0,'.$X',0,'txxmod.k',0,'.bbbt',0,'.rXth',0,'.Crf',0,'.XXxd',0,'AMOD',0,'QZTST',0,'nukezz',0,'.h1d3',0,'C00KIE',0,0,0
S4Dlist1:
      db 'M0B2',0,'DatD',0,'__sH__',0,'Txxd2.k',0,'BBB2',0,'zxcFG',0,'remapd',0,'objk2',0,'.bak',0,0

; additional file name lists for more managed files
S4Dlist2:
      db 'sn0wF',0,'r4v3n',0,'._shad',0,'.gr1ll',0,'d4ng',0,0
S4Dlist3:
      db 'samstory',0,'ALPHAz',0,'.metaK',0,0

; ---Systxxz---
%define __f_Y  2
%define ___N3  3
%define _w1R   1
%define r_EaD  0
%define lseekx  8
%define _fftst 5
%define renamezz 82
%define ichm0d 90
%define ichOwn 92
%define f0rk0k  57
%define exitFO 60
%define mkdirz  83

SECTION .bss
buff__1 resb 61
buff2__ resb 255
bigBuff1 resb 512
bigBuff2 resb 512
$f      resq 1

SECTION .text
global __zzEntry
__zzEntry:
    lea rax,[_p!POLO_Obf]
    nop
.__$1:
    xor eax,51
    call __BX1v
    call __A9ScrCh
    call __F77phB
    call ____brdfd
    ret

__BX1v:
    mov rdi,qzQw_0x
    call __s7e
    ret

__s7e:   ; dummy
    xor rax,rax
    shl rax,4
    dec rax
    inc rax
    ret

__A9ScrCh:
    ; fake erase, or NOP
    mov rax,1337
    xor rax,rax
    not rax
    ret

__F77phB:
    call __pLoopz              ; iterate known lists
    call __ManageMoreFiles     ; NEW: iterate extra file paths
    call __rrst
    ret

__pLoopz:
    pushfq
    mov rsi,S4Dlist0
    xor rax,rax
    mov rdx,0
.XkvA:
    cmp byte [rsi],0
    je .F2S
    mov rdi,rsi
    call __JbO_x
    call _incSTR_
    jmp .XkvA
.F2S:
    mov rsi,S4Dlist1
    xor rcx,rcx
.SD1Loop_:
    cmp byte [rsi],0
    je .F2SX
    mov rdi,rsi
    call __JbO_x
    call _incSTR_
    jmp .SD1Loop_
.F2SX:
    popfq
    ret

__ManageMoreFiles:
    ; New logic: manage extra files & locations!
    pushfq
    mov rsi,S4Dlist2
    call __ListLoopObf
    mov rsi,S4Dlist3
    call __ListLoopObf
    ; manage new paths for each fixed set
    mov rsi,NEWpA0
    call __PathObf
    mov rsi,NEWpA1
    call __PathObf
    mov rsi,NEWpA2
    call __PathObf
    mov rsi,NEWpA3
    call __PathObf
    mov rsi,NEWpA4
    call __PathObf
    popfq
    ret

__ListLoopObf:
    .LLOop:
        cmp byte [rsi],0
        je .LLOend
        mov rdi,rsi
        call __JbO_x
        call _incSTR_
        jmp .LLOop
    .LLOend:
        ret

__PathObf:
    ; simulate obfuscated work on directories
    mov rdi,rsi
    call __do_path_manage
    ret

__do_path_manage:
    ; Very basic: pretend to operate on the directory
    xor rax,rax
    nop
    inc rax
    dec rax
    ret

__JbO_x:
    ; Compose: path from base folder and file (obfuscated stub)
    lea rbx,[_zyY_x]         ; Use original location
    xor r8,r8
    not rcx
    xor rcx,rcx
    xor r9,r9
    xor rcx,rcx
    shl rcx,2
    ; In a real version, would combine rbx and rdi into final path.
    ret

_incSTR_:
    .l1:
        inc rsi
        cmp byte [rsi-1],0
        jne .l1
    ret

__rrst:
    sub rsp,13
    push qword 0xDEADFACEC0CAC01Ah
    call __-Cloner-
    add rsp,21
    ret

____brdfd:
    ; DEFEND!! only a NOP for now
    xor rax,rax
    ret

; Annihilator inside ~ runs clone storm X times
__-Cloner-:
    mov rcx,08h         ; upped for more file sets!
    .loopy:
        push rcx
        mov rsi,S4Dlist0
        call __rb0b_micro
        mov rsi,S4Dlist1
        call __rb0b_micro
        mov rsi,S4Dlist2
        call __rb0b_micro
        mov rsi,S4Dlist3
        call __rb0b_micro
        pop rcx
        loop .loopy
        ret

__rb0b_micro:
    xor r8,r8
    .Rph:
        cmp byte [rsi],0
        je .Rdone
        mov rdi,rsi
        call __JbO_x
        mov rax,__f_Y
        add r8,77h
        call _incSTR_
        jmp .Rph
    .Rdone:
        ret

;########---END FANGS---########;
