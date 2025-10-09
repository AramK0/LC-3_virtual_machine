# LC-3 Virtual Machine in C

a simple virtual machine with the LC-3 architecure made with c and also an assembler to turn LC-3 assembly instructions into machine code

# Running

First clone the repo
then get a simple lc-3 assembly instruction file like the one in assembly_instruction/hello.s

run 'gcc assembler.c -o assembler' and ./assembler --enter the filepath to the assembly file
run 'gcc vm.c -o virtual_machine' and ./virtual_machine --follow the cli and enter assembled_file.obj and you should see the output that it works well

