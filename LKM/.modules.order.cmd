cmd_/media/psf/KRWX/LKM/modules.order := {   echo /media/psf/KRWX/LKM/krwx.ko; :; } | awk '!x[$$0]++' - > /media/psf/KRWX/LKM/modules.order
