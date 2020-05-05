jimtclpp - Port of jimtcl to C++.  This exists because of COVID-19 gave me too much time on my hands.

The original Jimtcl is a wonderful piece of (C language) coding which you can find at.  
http://jim.tcl.tk/index.html/doc/www/www/download/
or
https://github.com/msteveb/jimtcl

I was curious about how Jimtcl worked and just started taking it apart to 
see it's inter workings and this happened.

* How to build with cmake
On most platforms you can use cmake. (tested in Linux/MacOS)
Steps:
(1) Extract to a directory (i.e. <top>), and cd to that directory.
	So <top>/core and <top>/jimsh exists.
(2) Make a subdirectory called "build", and cd into it.
(3) Call cmake. "> cmake .."
(4) Call make. "> make"
The output will be in the "bin" directory (i.e. <top>/bin)

* How to build with Visual Studio 2019.
Steps:
(1) Open project file <top>/vc/jimtclpp.sln.
(2) Do a build all.

* Known bugs
	* exec event.test:88   event-10.1
	* tailcall-1.9
	* memory leaks when all test (i.e. runall.tcl).
	* sockets not working
	* SSL not working
	* zlib module not working.
	* signals module not working
	* linenoise removed.
	* lock.test:37	lock-1.5
	* Something is wrong with the github releases are empty.
	* On github's Windows MSVC build the test won't run. An issue with cmake I think.

* Mistakes made
	* This started as a small thing to keep me busy for a couple of days while I was out of work due 
		to COVID-19, so I didn't prepare for some things.
		* So instead of forking the original this starts from a old copy of jimtcl I had laying around.

* General Features
	* Most of Jimtcl features.
	* Builds and runs on Windows (MinGW/MSVC), Linux(gnu/clang), MacOS(clang)
	* Coded in C++.
	* Doesn't use (so far) any C++ stdlib.

* Features added
	* All coded in C++, but not converted to object orientated.
	* Re-organized source code to something I think easier to understand.
	* Got rid of most all uses of preprocessor.  I personally find code 
	with a lot of preprocessor use hard to read.
	* Added a bunch of small accessors to structures.  Mostly I encasulate
	reading of values, not setting of values.  The idea is to be able to
	quickly determine where and how any field in a structure is used, not
	really to hide it's type.
	* Added explicit initializers when possible, instead of depending on 
	good coding to set values.
	* Made all cast explicit for readability.
	* Builds via cmake.
	* Everything put into it's a namespace Jim::.
	* Has per object specialized allocators so easier to add memory caches.
	* Removed most #if(s) from code. 
		* This improves readability and doesn't allow bugs to hide inside 
		#if out sections.
	* Replace most pointers with types so they can be replaced with smart
	pointers if required.
	* Added a compatiblity layer named prj_compat.
		* Allows you to test in code if a function exists on current 
		systems and if it does guarantees it's behavior it what you want.
	* Seperated out files directories by what they are.
	* Moved all simulated functions into prj_compat code to simplify 
		code.
	* Added a C++ interface which you can write extensions against that
		actually never exposes data structures.  i.e jim-api.h
	* Added a C interface based on C++ interface. i.e. jim-capi.h
	* Added a experimental swig interface so you could embedded jimtclpp 
	in Java, C#, Python, or even Tcl.  Not really tested yet.
	* Added Visual Studio 2019 project files.
	* Did some coverage analysis.
	* Did some leak analysis.
	* Marked the code with a series of "hash marks" to make it easier to 
	find things
		* Incomplete list of marks
			* #JimCmd - All functions implementing a command.
			* #JimType - All data structures defining a "type".
			* #MissInCoverage - Something never touched after running "runall.tcl".
			* #Alloc,#AllocF,#Free,#FreeF - Places where heap is changed.
			* #NonPortFunc, #NonPortHeader - For things which may not work on different 
				systems.
			* #stdoutput 

* Why not.
	* Why not use C++ standard library.  It wasn't required the code already 
	works, and the goal was to make it easier for me to understand and work 
	with, not change for change sake.  I do plan to use some of the C++ 
	standard library in some extensions I have planned.

* Features removed
	* tty support 
	* linenoise code.
	* socket code was disabled.
	* SSL support was disabled.
	* A bug in exec was introduced, and so exec is now disabled.
	* zlib support was removed.
	* Only supports UTF8 right now.
	* scripts to support embedding jimtcl scripts in code not working.
	* scripts to add/remove modules from jimtcl not working.
	* automatic configuration not working.

* Plans
	* Fix exec
	* Fix tailcall
	* Add an advance C++ interface which simplifies embedding code.
	* Allow it to build and work with USE_UTF8 not defined.
	* Add a jimtclpp profile to swig so it can build modules for jimtclpp.
	* Add speed test.
	* Add an jimtclpp specific heap so it can't leak memory.
	* Build a DLL from jimshpp.

Some questions I anticipate
* What's with "#pragma once"?
	All the compilers I could find support this feature, so even though it is not covered by the 
	standards I consider it part of the language.  And it does speed up header use.
* What's with getting rid of most of the macros?
	I find use of macros confusing.
* What's with getting rid of most "#if"(s)?
	I find them confusing and they also mean you build and test system must be complicated enought
	to build and run against all the possible combinations of conditions on "#if"(s).
* What's with adding a bunch of small accessors?
	I ussually wrap all reads so that writes to those fields become obvious.
* Why C++?
	My favorate langauge and has been as portable as C for years.  
* Why not use C++ stdlib and BOOST?
	Not needed and I didn't want to force more dependieces into the code base.
* Why Jim?
	I love Tcl.  I while ago I ported all the Tcl8.5 to C++, and while it worked it was just too ugly to
	release to the world.  I thought something smallar like Jim I might be able to better handle.
* About Jim
