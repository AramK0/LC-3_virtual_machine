.ORIG x3000

LEA R0, HELLO ; loading address of hello string into R0

PUTS
HALT

HELLO .STRINGZ "Hello from LC-3 vm."

.END