
# Whenever the compiler creates an executable, it
# links this to the front of it to jump to main

.extern main
loada .main
call .main
ret