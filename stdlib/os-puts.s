

.extern __write_uart
# void puts(char * string)

.__uart_puts:
.export __uart_puts
mov rp ra
.read_next:
load.rp ra 0
jmp.r.z .exit
loada .__write_uart
call .__write_uart
add rp 1
jmp.r .read_next

.exit:
ret
