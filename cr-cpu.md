# CR-CPU Instruction Set

## Introduction

The CR-CPU is a 16-bit, general purpose, FPGA soft CPU. It is intended to be minimally complex to ease code generation, while still having sufficient complexity so that hardware features directly map to the C programming language. The CR-CPU uses a Von Neumann architecture with a common 16-bit full-duplex bus for all peripherals and memory. The bus contains 16-bits each of read address, read data, write address, and write data. The full 16-bit address space is usable for mapping peripherals.

## Instructions

The CR-CPU uses 16-bit fixed width instructions. Each instruction contain 8-bits of opcode and operands, and an 8-bit immediate. Note that all instructions include the immediate, even if not required. The instructions are structured as follows:

```
	0b oooo aaaa cccc cccc
```

Where:
```
	o = Opcode
	a = Extra information (operands)
	c = Immediate
```

The opcode encodes which operation should be executed, e.g. `add`, `sub`, `jmp`, `call`. See the [Opcodes](#Opcodes) sections below for more details on each opcode.

The additional information section is used to define the sub-command and/or the operand(s) involved. Where possible, the first two bits are used to define the destination register for the result of the computation.

## Registers

The CR-CPU contains four user addressable registers. Specifically, it contains two general purpose 16-bit registers (instruction encoding indicated):

```
	00: ra
	01: rb
```

And two special purpose 16-bit registers:

```
	10: rp (pointer register)
	11: sp (stack pointer)
```

The stack pointer register is implicitly used for all `call`, `ret`, `push`, and `pop` instructions. Note that the stack is assumed to start from a (user selected) high address and grows downwards. Also note that the stack pointer points to the last item pushed onto the stack. This means a push operation will first decrement the stack pointer, then write the value to the new location of the stack pointer.

The pointer register, in addition to being usable as a general purpose register, may be used as an offset for load and store operations. It is therefore common to put pointers into this register so that they can easily be dereferenced.

The CR-CPU also contains an additional 8-bit address register which is not directly accessible. Since all instructions only include an 8-bit immediate, which is insufficient to directly address the entire memory space, the address register may be loaded with the high 8-bits of the address to be used in a store/load or jump/call operation. This gives a total address space of 64k words for program, memory, peripherals.

Finally the CR-CPU contains a 16-bit instruction pointer register, which is not directly user accessible and can only be manipulated through the use of `call`/`ret` instructions.

## Hexadecimal Reference

|HEX|DEC|BIN|
|---|---|---|
|	|0	|0000|
|	|1	|0001|
|	|2	|0010|
|	|3	|0011|
|	|4	|0100|
|	|5	|0101|
|	|6	|0110|
|	|7	|0111|
|	|8	|1000|
|	|9	|1001|
|A	|10	|1010|
|B	|11	|1011|
|C	|12	|1100|
|D	|13	|1101|
|E	|14	|1110|
|F	|15	|1111|

## Opcodes

The following is a summary of the opcodes available.

|Opcode|Description|
|---|---|
|0| Add (add)                               |
|1| Subtract (sub)                          |
|2| And (and)                               |
|3| Or (or)                                 |
|4| Exclusive or (xor)                      |
|5| Shift L/R (shftl, shftr)                |
|6| Load (load)                             |
|7| Store (store)                           |
|8| Move (mov)                              |
|9| Jump (jmp), relative jump (jmp.r)       |
|A| Load immediate (loadi, loadi.h)         |
|B| Push, pop (push, pop)                   |
|C| Call, relative call, ret (call, call.r, ret) |
|D| Load address (loada)                    |
|E| Halt (halt)                             |
|F| No operation (nop)                      |

### 0x0: Addition (add)

Calculates the sum of two arguments (registers or immediate) and places the result in the first register. Note that in the case of the immediate, it is zero extended.


Encoding:

```
	ddss

	dd = Destination and source 1 register (00: ra, 01: rb, 10: rp, 11: sp)
	ss = Source 2 register (00: ra, 01: rb, 10: rp, 11: immediate)
```

Example:

```
	add ra rb       # add two registers, result in ra
	add ra 1        # add register and immediate
	add ra .label   # labels or constants may be used in place of an immediate
	                # and will be substituted by the assembler/linker.
```

### 0x1: Subtraction (sub)

Calculates the difference between two arguments (source 1 – source 2) and places the result in source 1. Uses the same encoding as add.

### 0x2: And (and)

Performs bitwise “and” of two arguments. Uses the same encoding as add.

### 0x3: Or (or)

Performs bitwise “or” of two arguments. Uses the same encoding as add.

### 0x4: Exclusive or (xor)

Performs bitwise “xor” of two arguments. Uses the same encoding as add.

### 0x5: Shift L/R (shftl, shftr)

Performs a logical shift of a register. Note that since the registers are 16-bits wide, only the bottom four bits of rb or the immediate are used as the shift amount.

Encoding:

```
	ddsy

	dd = Source and Destination register
	s = Shift amount source (0: rb, 1: immediate)
	y = Direction (0: right, 1: left)
```

Example:

```
	shftl ra rb     # shifts by an amount in lower 4 bits of rb
	shftl ra 0x01   # shifts by an immediate
	shftr rb 0x02   # same but shift right
```

### 0x6: Load (load)

Loads a value from RAM or peripheral. The load address may be defined in one of two ways: as an absolute address, or as a relative address. In the case of a relative address, the load address is defined by the sum of the signed extended immediate plus the pointer register or stack pointer. In the case of an absolute address, the load address is defined by the address register as the high byte and the immediate as the low byte.

Encoding:

```
	ddpp

	dd = Destination register
	pp = Offset selection (00: rp, 01: sp, 10: absolute address, 11: unused)
```

Example:

```
	# absolute address
	.constant 0x8120 UART_TX
	loada .UART_TX
	load ra, .UART_TX   # loads ra with value at 0x8120

	# rp relative
	.static 1 my_var
	loadi rp .my_var
	loadi.h rp .my_var
	load.rp ra 0       # load ra with a value from my_var (through rp)

	# sp relative
	loadi ra 0
	push ra
	load.sp rb 0       # load the value from the top of the stack into rb
```

### 0x7: Store (store)

Performs the inverse operation of the [load](#0x6-Load-load) command. Specifically, stores the values of the given register into the given address. See load entry on how to compute the address.

### 0x8: Move (mov)

Moves (copies) a value from one register to another. The immediate is unused.

Encoding:

```
	ddss

	dd = Destination register (00: ra, 01: rb, 10: rp, 11: sp)
	ss = Source register (00: ra, 01: rb, 10: rp, 11: sp)
```

Example:

```
	mov ra rb   # register to register transfer
	mov rb rp
```

### 0x9: Jump (jmp)

Optionally jumps to a new location, in which case the instruction pointer is overwritten with the new value. There are three mutually exclusive, optional condition flags. The condition is evaluated based on the current value of the `ra` register. Specifically:

```
	z = Jump if ra is zero
	nz = Jump if ra is non zero
	ge = Jump if ra is greater or equal to zero (specifically, if the most significant bit is zero)
```

Note: For historic purposes, the assembler also accepts `.gz` in place of `.ge`.

The jump instruction has two address forms, relative and absolute. For relative jumps, the instruction immediate is sign extended and is added to the current instruction pointer giving a jump range of +127 .. -128. Relative jumps are selected using the `r` flag. For absolute jumps, the address is the address formed by the address register as the high byte and the instruction immediate for the low byte.

Encoding:

```
	ttrx

	tt = Type of jump (00: jmp, 01: jmp.z, 10: jmp.nz, 11: jmp.ge)
	r = Absolute vs relative jump (0: relative, 1: absolute)
	x = Unused
```

Example:

```
	jmp.r .label     # unconditionally jump to .label
	jmp.r.z .label   # jump to .label if ra is zero
```

In the above case, the relative offset between the jump instruction and the label is calculated at assembly time. Note that the assembler will issue an error if the jump is out of range.

Absolute jumps:

```
	loada .label
	jmp.nz .label   # jumps to .label if ra is not zero, label may be any address
```

### 0xA: Load Immediate (loadi)

The load immediate instruction can be used to load an immediate directly from an instruction to a register. Since the immediate is only 8 bits and the registers are 16 bits, it provides the option to load the low or high byte via the `h` flag. When the low byte is loaded the high byte is zeroed out. When the high byte is loaded, the low byte is untouched. This allows small (0 .. 255) constants to be loaded with a single instruction and larger constants to be loaded with the sequence `loadi`, `loadi.h`.

Encoding:

```
	ddtx

	dd = Destination register
	t = Type of load (0: low byte, 1: high byte)
	x = Unused
```

Example:

```
	loadi ra, 0xCD    # loads ra with the value 0x00CD
	loadi.h ra, 0xAB  # loads the high byte of ra with 0xAB, results ra = 0xABCD
```

This instruction can also be used to load the value of a constant or label:

```
	.constant 0xBEAF my_const   # no code gen, just creates the label
	loadi ra, .my_const
	loadi.h ra, .my_const       # ra is now loaded with the value 0xBEAF
```

Or can be used to load the address of a static variable. This is useful to load the address into rp where it can then be used as an offset pointer to load and store.

```
	.static 256 my_array
	loadi rp, .my_array
	loadi.h rp, .my_array   # rp now contains the address of my_array
	load.rp ra 0            # ra now contains the first value of my_array
```

### 0xB: Push, Pop (push, pop)

The behaviour of the opcode depend on which sub-command is used. Note that the stack pointer points to the last pushed item on the stack.

- push - Pushes an item onto the stack. Specifically, decrements the stack pointer and then writes to the new location of stack pointer.
- pop - Pops an item into a register from the stack. Specifically, returns the value at the location of the stack pointer, and then increments the stack pointer. Note if the value being popped is unneeded, then it is more efficient to add 1 to the stack pointer (1 clock) than it is to pop into an unused register (2 cycles).

Encoding:

```
	ddxt

	dd = Destination/Source register
	x = Unused
	t = Type of operation (0: push, 1: pop)
```

Example:

```
	push ra     # subtracts one from sp, then writes ra
	pop rb      # reads value at sp, then increments sp
	add sp, 1   # equivalent to pop, except only one cycle and the value is not read
	sub sp, 10  # allocates 10 new spots on the stack that can be used for local variables
```

### 0xC: Call, Return (call, ret)

This instruction performs various function related tasks depending on the sub-command. Note that for the call instruction, the addressing is identical to the jump instruction.

- call - Performs a function call using an absolute address. The address register is used for the high byte of the target, and the instruction immediate is used for the low byte. Specifically, this instruction will push the current instruction pointer plus one onto the stack, and then jump to the given address.
- call.r - Performs a relative function call. The destination address is the sum of the sign extended instruction immediate and the current instruction pointer. Specifically, this instruction will push the current instruction pointer plus one onto the stack, and then jump to the given address.
- ret - Performs a return from a function. Specifically, pops the top stack location into the instruction pointer, effectively jumping to that location.

Encoding:

```
	xxrc

	xx = Unused
	r = Absolute/Relative call (0: relative, 1: absolute)
	c = Type of operation (0: call, 1: ret)
```

Effectively: `00: rcall, 01: ret, 10: call, 11: ret`

Example:

```
	loada 0xEF
	call 0xAB   # will jump to the location 0xEFAB
```

Of course typically labels will be used:

```
	.... # load ra and rb with appropriate values
	loada .my_func
	call .my_func
	....  # more code
	.my_func:
	add ra, rb
	ret
```

Note that absolute calls will create relocations or external references, which will be evaluated at link time.

In the case of relative function calls, the call can be made directly without needing to set the address register first. The assembler will automatically generate the correct relative offset between the call and the function or will issue an error if the jump distance is too great.

```
	.... # load ra and rb with appropriate values
	call.r .my_func
	....  # more code
	.my_func:
	add ra, rb
	ret
```

When a call is made, the return address is pushed onto the stack. Therefore the stack must be restored prior to executing `ret` at the end of a function:

```
	.my_func:
	sub sp, 10   # allocate local storage
	....
	store.sp ra, 5   # use that storage
	....
	add sp, 10   # sp must be restored before ret is executed
	ret
```

### 0xD: Load Address (loada)

Loads the 8-bit address register with the instruction immediate in preparation for a `load`, `store`, `jmp`, or `call`. No additional information is used for this instruction.

Example:

```
	loada 0xDE   # address register now contains 0xDE
	.constant 0xABCD const
	loada .const  # address register now contains 0xAB
```

### 0xE: Halt (halt)

Halts the processor. No additional information is used for this instruction.

### 0xF: No Operation (nop)

Performs no operation. This instruction takes one clock and can therefore be used by the programmer for timing control. No additional information is used for this instruction.

## Instruction timing

All instructions take 1 clock to execute, except for those that need to read and address. This is because during typical operation the CPU must fetch a new instruction every cycle, if it needs to fetch data, then it must delay the fetch instruction by one cycle. Instructions which read data include: `load`, `pop`, and `ret`. The bus is duplex, so instructions which write data can still run in a single clock, e.g. `store`, `push`, and `call`.

## CR-CPU Assembler

An assembler is provided to convert assembly written for the CR-CPU to binary which can then be run on the CPU. The assembler has a number of features which allow the programmer to easily write assembly programs, e.g. static variable allocation, named constants, and automatic relative and absolute jump address calculation. Note commas between the instruction arguments are optional.

Assembler directives can be used by the programmer to inform the assembler of some piece of information. In general the directives do not generate any output on their own. The following directive are available:

### Comments

Comments may be placed in the assembly code. Any text which is preceded by a “#” until a new line will be ignored by the assembler. For example:

```
	# this is a line comment
	loadi ra, 0   # this comment is on the same line as an instruction
```

### Instruction Labels

A label stores the location of the next instruction generated at assembly time. This can then be used in jump and call instructions to set the offset. When labels are defined they must follow the format “.label_text:”, i.e. with a leading period and a trailing colon. They must be on a line by themselves. When they are referenced in instructions they must follow the format “.label”, i.e. leading period but not trailing colon. Note that labels are stored as 16-bit values but only either the upper or lower byte is used as the immediate for the instruction depending on the context (since all instructions include an 8-bit immediate). E.g. “loada” and “loadi.h” use the upper byte, while “loadi” and “call” use the lower byte. For example with absolute addressing:

```
	...            # arbitrary number of instructions here, in this case one
	.loop:         # creates a label on the next instruction, in this case instruction one
	sub ra, 1
	loada .loop
	jmp.nz .loop   # jumps to the label “.loop”
```

Another example with relative addressing:

```
	loadi ra, 10
	loadi rb, 20
	call.r .fn_add     # relative distance from here to the function is automatically calculated
	store.sp ra 0      # store result to top of stack for example
	....               # more code
	.fc_add:           # will jump here when call is executed
	add ra, rb
	ret
```

### Constants

Constants are similar to labels, except the programmer can directly supply their value. Note that like labels, constants are always of type unsigned 16-bit integer. Similar to labels, either the low or high byte will be used automatically depending on context. Constants are great for defining the addresses of memory mapped peripherals so that they can be changed easily in the future. For example:

```
	.constant 0xABCD my_uart
	loadi rp, .my_uart   # loads ra with 0x00CD
	loadi.h rp, .my_uart # upper byte of ra set to 0xAB while leaving the lower byte unchanged
	store.rp ra 0         # store ra to the address set from constant
```

Note that similar to static variables below, an assembly time offset can be added to a constant. This constant will roll over into the high byte where required. For example:

```
	.constant 0x20FF my_const
	loadi ra, .my_const[2]      # loads ra with 0x0001
	loadi.h ra, .my_const[2]    # loads the high byte of ra with 0x21 (forming 0x2101)
```

### Static Variables

Defining a static variable performs two operations: it places initialization data of the variable into the machine code, and it creates a label which can be used in the same way as code labels and constants. Static arrays or structs may also be defined by using a size other than 1. If no initialization data is provided, then the variable will be initialized with zero. If initialization data is provided, it must be provided for every member of a variable. The offsets of all variables are calculated during assembly and used in place of the labels. For example:

```
	.static 1 var_a           # simple var of 16-bits, initialized to zero
	.static 1 var_b 0xAFEB    # simple var with an initialization value
	.static 3 var_c 10 11 13  # static array with non-zero initialization
	loadi rp, .var_a
	loadi.h rp, .var_a        # these two instructions load the ADDRESS of var_a into rp
	load.rp ra, 0             # loads the value of .var_a into ra
	loadi rp, .var_c
	loadi.h rp, .var_c
	load.rp rb, 1             # load the second value of the array (11) into rb
```

Alternatively,

```
	loada .var_a    # loads the high byte of the address of var_a
	load ra, .var_a # loads value from variable var_a
```

As before, a constant offset may be applied at assembly time,

```
	loada .var_c[2]
	load ra, .var_c[2] # loads from address var_c + 2, i.e. value 13
```

### External and Exported References

A linker is used after the assembler to perform relocations, and resolve exported/imported references. To use a label from another translation unit you can declare it `extern`. To export a label from a translation unit, so that it may be used in other translation units, you declare it `export`. For example:

foo.s
```
	.extern add   # add is now a label that can be used, resolved at link time
	.main:
	loadi ra 1
	loadi rb 2
	loada .add    # the label can be called like normal
	call .add     # note that you cannot use external labels in relative jumps or calls
	ret
```

bar.s
```
	.add:
	.export add   # export must be after the label
	add ra rb     # label 'add' will now be visible to the linker
	ret
```

## Recommended calling conventions

Since the assembler does not handle calling conventions, no particular calling convention must be followed. However, for compatibility with the compiler, the following calling conventions are recommended.

### Function argument location

Arguments to function should be first in `ra`, then `rb`, then on the stack where the rightmost argument is pushed first. This means that inside a function body the arguments 3 through n are accessible from `sp+1` to `sp+(n-2)` (the return address is stored at sp). Registers `ra` and `rb` are only used if the value will fit in them. For example, if the first argument is a 32-bit long, then it will be stored in the `ra/rb` pair. If the first argument is a 16-bit int then the second argument is a 32-bit long, then the first argument will be stored in `ra`, `rb` will be empty, then the long will be stored on the stack. If the first argument is a 4 word (8 byte) struct, then both `ra` and `rb` will be empty and the full struct will be placed on the stack. This convention is required because multi-word values must be kept contiguous. Thus even if a function will pushes `rb` then `ra` to the stack the data would not be contiguous because the call itself pushes a word onto the stack.

### Caller vs Callee saved registers

The registers `ra` and `rb` are callee saved. Except when these registers are used to pass function argument, in which case they do not need to be restored.

The registers `rp` and `addr` are caller saved.

Register `sp` is callee saved in the sense that it must be restored before calling `ret`.

### Return value is returned in ra when required

When a return value is specified, it will be placed in ra before returning from the function. In functions which take an argument in ra but do not return anything, the value left in ra is unspecified. In functions which do not take arguments and which do not return anything, the value of ra should persist across the function call. Similar to function arguments above, return values which are 32-bits, should be placed in the register pair `ra/rb`. Return values which are larger than two words should be returned on the stack. Specifically, the caller should allocate space on the stack for the return value, then any overflow arguments.
