jimsh ?= ../jimsh
tclsh ?= tclsh

DEF_LD_PATH := LD_LIBRARY_PATH="/home/smarty/project/jimtcl-a5a84b1:$(LD_LIBRARY_PATH)"

test:
	@$(DEF_LD_PATH) $(jimsh) runall.tcl

tcl:
	@rc=0; for i in *.test; do $(tclsh) -encoding utf-8 $$i || rc=$?; done; exit $$rc

clean:
	rm -f gorp.file2 gorp.file sleepx test1 exec.tmp1
