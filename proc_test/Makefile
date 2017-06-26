obj-m   := proc_test02.o  proc_test03.o  proc_test04.o 
KERNEL  := /lib/modules/`uname -r`/build #for mint/ubuntu  
#KERNEL := /lib/modules/`uname -r`/source #for redhat  
  
all:  
	echo "make -C $(KERNEL) M=`pwd` modules "
	make -C $(KERNEL) M=`pwd` modules  
  
install:  
	make -C $(KERNEL) M=`pwd` modules_install  
	depmod -A  
  
clean:  
	make -C $(KERNEL) M=`pwd` clean 
