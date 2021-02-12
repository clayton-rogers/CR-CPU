

.extern get_secret
.extern __print_num
.extern clear_screen

.main:
.export main
loada .clear_screen
call .clear_screen
loada .get_secret
call .get_secret
loada .__print_num
call .__print_num
ret

