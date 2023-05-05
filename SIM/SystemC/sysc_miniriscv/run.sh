export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/systemc-2.3.3/lib-linux64/
riscv32-unknown-elf-gcc -nostdlib -c SW/reset.s -o reset.o
riscv32-unknown-elf-gcc -nostdlib -c tests/test_add.s -o app.o
riscv32-unknown-elf-gcc -nostdlib reset.o app.o -T SW/link.ld -o app
./run.exe app
