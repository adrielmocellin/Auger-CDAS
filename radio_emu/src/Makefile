ifdef ARM
   CC=arm-xilinx-linux-gnueabi-gcc
endif


all: su_bsu su_emu 

su_bsu: su_bsu_emu.c ub_side.c pm_side.c protocol.c
	$(CC) su_bsu_emu.c ub_side.c pm_side.c protocol.c -o su_bsu

su_emu: su_emu.c ub_side.c protocol.c;\
	$(CC) su_emu.c ub_side.c protocol.c -o su_emu

clean:;
	rm su_bsu su_emu 
