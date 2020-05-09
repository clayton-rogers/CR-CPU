
.extern foo
.extern bar
# extern references are allowed to be redeclared
.extern foo


nop
nop
nop
nop
loada .foo
call .foo
