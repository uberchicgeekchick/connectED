#!/bin/tcsh -f
foreach src ( `find src/ -maxdepth 1 -regex '.*\.[c|h]$'` )
	switch ( "${src}" )
	case "src/comments.c":
	case "src/comments.h":
	case "src/grel2abs.c":
		continue
		breaksw
	endsw
	ex '+1,25d' '+0r src/comments.c' '+wq' "${src}"
end
