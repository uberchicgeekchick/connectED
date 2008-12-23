#!/bin/tcsh -f
php_manual="/projects/reference/www/PHP/PHP"
shared_php_manual="/usr/share/doc/phpmanual"

if ( ! -l "$shared_php_manual" ) then
       if ( ! -d "$shared_php_manual" ) then
		printf "I'm creating a symlink to PHP's manual to support GTK-PHP-IDE's internal help.\nsudo will be used to do this.\n"
		sudo ln -s "$php_manual" "$shared_php_manual"
	endif
endif

