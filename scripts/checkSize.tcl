catch { exec find . -name "*.h" } headers; set header_list [split $headers "\n"]
catch { exec find . -name "*.cpp" } sources; set source_list [split $sources "\n"]


set numFiles 0
set totalSize 0 
set totalNumLines 0
set totalNumComments 0
foreach h [lsort $header_list] {
    if {[string first CMake $h]!=-1} { continue }
    set fp [open $h r]
    set numLines 0 ; set numComments 0
    while {![eof $fp]} {
        set l [gets $fp]
        incr numLines 
	if {[string first "/*" $l]!=-1} { incr numComments }
	if {[string first "//" $l]!=-1} { incr numComments }
    }
    close $fp
    puts "${h} file_size: [file size $h] num_lines: $numLines num_cmts: $numComments"
    incr numFiles 
    incr totalSize [file size $h]
    incr totalNumLines $numLines
    incr totalNumComments $numComments
}
foreach s [lsort $source_list] {
    if {[string first CMake $s]!=-1} { continue }
    set fp [open $s r]
    set numLines 0 ; set numComments 0
    while {![eof $fp]} {
        set l [gets $fp]
        incr numLines 
	if {[string first "/*" $l]!=-1} { incr numComments }
	if {[string first "//" $l]!=-1} { incr numComments }
    }
    close $fp
    puts "${s} file_size: [file size $s] num_lines: $numLines num_cmts: $numComments"
    incr numFiles 
    incr totalSize [file size $s]
    incr totalNumLines $numLines
    incr totalNumComments $numComments
}
puts "TOTAL: num_files: $numFiles byte_size: $totalSize num_lines: $totalNumLines num_cmts: $totalNumComments"
