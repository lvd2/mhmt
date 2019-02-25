\ forth MegaLZ depacker
\
\
\

include prhelp.fs
include argparse.fs
include input.fs
include obuf.fs
include ofile.fs
include depack.fs



: fdemlz

	decimal ( !!!!!!!! )


	argparse

	2dup input-load

	ofile-create

	obuf-init


	
	depack



	obuf-release

	ofile-close

	input-close
;


fdemlz  bye

