; Autor reseni: Filip Novak xnovakf00

; Projekt 2 - INP 2024
; Vigenerova sifra na architekture MIPS64

; DATA SEGMENT
                .data
msg:            .asciiz "filipnovak" ; sem doplnte vase "jmenoprijmeni"
cipher:         .space  31 ; misto pro zapis zasifrovaneho textu
; zde si muzete nadefinovat vlastni promenne ci konstanty,
; napr. hodnoty posuvu pro jednotlive znaky sifrovacho klice

; shifts              n    o    v
key:            .byte 14, 15, 22
ascii_a:        .byte 97
ascii_z:        .byte 122                
params_sys5:    .space  8 ; misto pro ulozeni adresy pocatku
                          ; retezce pro vypis pomoci syscall 5
                          ; (viz nize "funkce" print_string)

; CODE SEGMENT
                .text
; t0 is index to msg
; t1 stores current character
; t2 will signalize the shift direction -> 0 increment, else decrement
; t3 will store index to key array -> which value we use to shift
; t4 will store the value of shifting from key (t4 = key[t3])
; t5 will store value 3 -> used for looping in key array
; t6 will be flag for comparison 
; t7 stores ascii value of 'a'
; t8 stores ascii value of 'z'
main:   
        ; init to 0           
        xor $t1, $t1, $t1 
        xor $t0, $t0, $t0 
        xor $t2, $t2, $t2 
        ; init to 3
        daddi $t5, r0, 3
        ; init to values
        lb $t7, ascii_a($zero)
        lb $t8, ascii_z($zero)

loop:
        xor $t3, $t3, $t3 ; zero, just to be sure
        xor $t1, $t1, $t1 ; zero, just to be sure
        lb $t1, msg($t0)  ; load character from msg on index t0 to t1
        beqz $t1, end_    ; null char found, end of string
        
        div $t0, $t5      ; index of key will be reminder of division, (t0 mod 3)
        nop
        nop
        mfhi $t3          ; store the reminder
        lb $t4, key($t3)  ; load the shift value

        beqz $t2, shift_r ; looking at flag whether shifting right or left

shift_l:
    xor $t2, $t2, $t2  ; set next shift to be shift_r
    sub $t1, $t1, $t4  ; shift left
    j to_alphabet  

shift_r:
    daddi $t2, $zero, 1 ; set next shift to be shift_l
    add $t1, $t1, $t4   ; shift right
    j to_alphabet

; puts the character back into range of alphabet, if it is out of it
to_alphabet:
    slt $t6, $t1, $t7   ; t6 = char < a
    bnez $t6, before_a  ; if char < a jump before_a
    slt $t6, $t8, $t1   ; t6 = z < char
    bnez $t6, after_z   ; else if z < char jump after_z
    j store             ; else jump store

before_a:
    daddi $t1, $t1, 26 ; it will never go out of bound for more than 26
    j store

after_z:
    daddi $t1, $t1, -26 ; it will never go out of bound for more than 26

store:
        sb $t1, cipher($t0) ; store the character to cipher
        daddi $t0, $t0, 1   ; increment index
        j loop 

end_:
    sb $zero, cipher($t0)      ; add \0 to the end
    daddi r4, r0, cipher    ; set the address for printing
    jal print_string
; NASLEDUJICI KOD NEMODIFIKUJTE!

                syscall 0   ; halt

print_string:   ; adresa retezce se ocekava v r4
                sw      r4, params_sys5(r0)
                daddi   r14, r0, params_sys5    ; adr pro syscall 5 musi do r14
                syscall 5   ; systemova procedura - vypis retezce na terminal
                jr      r31 ; return - r31 je urcen na return address
