#!/bin/sh
prefix=`dirname "$0"`"/data"
shared_dir="/usr/share/GTK-PHP-IDE"

if [ ! -s "$prefix/php-gtk-php-ide.api" ]; then
	ln -sf "$prefix/php-function-reference.api" "$shared_dir/php-gtk-php-ide.api"
fi

if [ ! -s "$shared_dir" ]; then
	printf "I'm creating the symlink %s to support GTK-PHP-IDE's function highlighting.\nsudo will be used to do this.\n" $shared_dir
	sudo ln -sf "$prefix" "$shared_dir"
fi

php_manual="/projects/www/references/PHP/PHP"
shared_php_manual="/usr/share/doc/phpmanual"

if [ ! -L "$shared_php_manual" ]; then
       if [ ! -d "$shared_php_manual" ]; then
		printf "I'm creating a symlink to PHP's manual to support GTK-PHP-IDE's internal help.\nsudo will be used to do this.\n"
		sudo ln -s "$php_manual" "$shared_php_manual"
	fi
fi

