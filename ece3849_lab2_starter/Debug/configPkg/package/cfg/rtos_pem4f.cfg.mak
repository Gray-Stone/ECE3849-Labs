# invoke SourceDir generated makefile for rtos.pem4f
rtos.pem4f: .libraries,rtos.pem4f
.libraries,rtos.pem4f: package/cfg/rtos_pem4f.xdl
	$(MAKE) -f C:\Users\Edward\Documents\WPI\Junior\D_Term\ECE_3849\Labs\ECE3849-Labs\ece3849_lab2_starter/src/makefile.libs

clean::
	$(MAKE) -f C:\Users\Edward\Documents\WPI\Junior\D_Term\ECE_3849\Labs\ECE3849-Labs\ece3849_lab2_starter/src/makefile.libs clean

