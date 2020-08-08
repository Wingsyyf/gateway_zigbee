cc=arm-poky-linux-gnueabi-gcc  -march=armv7ve -mfpu=neon  -mfloat-abi=hard -mcpu=cortex-a7 --sysroot=/opt/fsl-imx-x11/4.1.15-2.1.0/sysroots/cortexa7hf-neon-poky-linux-gnueabi
#cc=gcc
target=gateway
obj=gateway.o cJSON.o md5.o myuartlib.o databuffifo.o
$(target):$(obj)
	$(cc) -g -o $(target) $(obj) -lm -lcurl -lpthread
gateway.o:gateway.c cJSON.h md5.h myuart.h databuffifo.h semaphore_operation.h myport.h gateway.h
	$(cc) -c gateway.c
cJSON.o:cJSON.c cJSON.h
	$(cc) -c cJSON.c -lm -lcurl
md5.o:md5.c md5.h
	$(cc) -c md5.c -lm
myuartlib.o:myuartlib.c myuart.h gateway.h
	$(cc) -c myuartlib.c
databuffifo.o:databuffifo.c databuffifo.h gateway.h
	$(cc) -c databuffifo.c
clean:
	rm $(target) $(obj)
