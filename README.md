# GameboyEmulator
Low Level Game

This is low level gameboy emulater. i.e. it emulates the gamesboys procressor at a c stlye abstraction level.
I have not worked on this project in almost two years. In its current state it uses an incredibly inefficient
structure that ignores function pointers, and memory addressing modes. Those two ideas would greatly increase
the execution efficiency (removing a 500 case switch statement replacing it with a function pointer table that
is indexed with the opcode of eac instruction) and size by generalizing each instruction type into 1 function 
rather than each different instruction being a separate C function.

Following these effiecny increases, The overal clock emulation structure needs to be changed in anticipation of
the addition of graphics emulation.

Finally, the memory emulation unit needs to have support for ROM bank addressing. This is done by checking if a write
operation is directed at one specific memory address in ROM then, based on the data valid on the "cartridge" address bus.
