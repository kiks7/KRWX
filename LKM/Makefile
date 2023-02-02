obj-m += krwx.o


K=/home/parallels/hacking/kernel/working_krwx/linux-4.9.331/
K=/home/parallels/hacking/kernel/working_krwx/linux-5.10.152/
K=/home/parallels/hacking/kernel/CVE-2022-2602/linux-5.15.74/

all:
	make -C $K M=$(PWD) modules

clean:
	make -C $K M=$(PWD) clean
