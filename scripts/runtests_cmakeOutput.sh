pushd ../tests
valgrind --tool=memcheck --leak-check=full --track-origins=yes ../bin/jimshppd runall.tcl
popd

