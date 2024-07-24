# PNG-to-PNM-Decoder
Convert png file to pnm. Program works with PLTE, IHDR, IEND, IDAT chunks(another will be ignored, but still working)

Invalid files will be found and program will end.

3 variant of decomposing data(macros):
1) zlib:
   ```
   gcc -o zlib.exe main.c zlib/*.c -Izlib -DZLIB
   .\zlib infile.png outfile.pnm
   ```
3) libdeflate:
   ```
    gcc -o libdeflate.exe main.c libdeflate/lib/*.c libdeflate/lib/x86/*.c -Ilibdeflate -Ilibdeflate/lib -Ilibdeflate/lib/x86 -DLIBDEFLATE
   .\libdeflate infile.png outfile.pnm
   ```
5) isa-l:
   ```
   gcc -o isal.exe main.c isa-l/isa-l_static.lib -Iisa-l/include -DISAL
   .\isal infile.png outfile.pnm
   ```
   https://github.com/intel/isa-l/blob/master/doc/build.md
