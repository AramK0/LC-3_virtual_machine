# LC-3 Virtual Machine

A simple virtual machine implementing the LC-3 architecture in C, with an assembler for converting LC-3 assembly to machine code.
Helps in understanding low level programming and how instructions are decoded

## Building

Compile the assembler:
```bash
gcc assembler.c -o assembler
```

Compile the virtual machine:
```bash
gcc vm.c -o virtual_machine
```

## Usage

Assemble your program:
```bash
./assembler
```
Enter the path to your assembly file when prompted (e.g., `assembly_instruction/hello.s`).

Run the assembled program:
```bash
./virtual_machine
```
Enter `assembled_file.obj` when prompted.

## Example

```bash
./assembler
# Enter: assembly_instruction/hello.s

./virtual_machine
# Enter: assembled_file.obj
```
