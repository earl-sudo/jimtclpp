jimtclpp - Port of jimtcl to C++.  This exists because of COVID-19.

The original Jimtcl is a wonderful piece of (C language) coding which you can find at.  
http://jim.tcl.tk/index.html/doc/www/www/download/
or
https://github.com/msteveb/jimtcl

I was curious about how Jimtcl worked and just started taking it apart to 
see it's inter workings and this happened.

* Known bugs
	* exec event.test:88   event-10.1
	* tailcall-1.9
	* memory leaks when all test (i.e. runall.tcl).
	* sockets not working
	* SSL not working
	* zlib module not working.
	* linenoise removed.

* General Features
	* Most of Jimtcl features.
	* Builds and runs on Windows, Linux.
	* Coded in C++.

* Features added
	* All coded in C++, but not converted to object orientated.
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
	in Java, C#, Python, or even Tcl.
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
	* Why not use C++ standard library.  Well for one it wasn't required the code
	works and the goal was to make it easier for me to understand and work 
	with, not to change just to change.  I do plan to use some of the C++ 
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