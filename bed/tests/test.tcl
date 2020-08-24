	
proc rsock {sock} {
	global eventLoop
	set l [ gets $sock ]
#	puts stdout "Reply=$l"
	if {[eof $sock]} {
		close $sock
		set eventLoop "done"
	} else {
		Insert "\n"
		Insert "$l"
	}
}

proc crapola {a, b, c}
{
}


set SYM XION
set site "quote.yahoo.com"
set fname "/download/quotes.csv?symbols=$SYM&format=sl1d1t1c1&ext=.xls"
set port 80

set s [ socket $site $port ]
fconfigure $s -buffering line
fileevent $s readable [list rsock $s]
puts $s "GET $fname\n"

vwait eventLoop


