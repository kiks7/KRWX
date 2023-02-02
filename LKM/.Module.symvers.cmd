cmd_/media/psf/KRWX/LKM/Module.symvers := sed 's/\.ko$$/\.o/' /media/psf/KRWX/LKM/modules.order | scripts/mod/modpost    -o /media/psf/KRWX/LKM/Module.symvers -e -i Module.symvers   -T -
