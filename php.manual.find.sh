#!/bin/sh
PHP_MANUAL_DIR=""
echo "checking PHP manual directory..."
if test "x$enable_php_manual" != "xNONE"; then
  PHP_MANUAL_DIR=$enable_php_manual
fi
prefix=/usr

if ! test -d "$PHP_MANUAL_DIR" && ! test -L "$PHP_MANUAL_DIR"; then
	PHP_MANUAL_DIR=""
	for php_manual_dir in \
		"$prefix/share/doc/php"* "$prefix/share/doc/php"*	\
		"/usr/share/doc/phpmanual" "/usr/share/doc/phpdoc"
	do
		printf "\n\tchecking $php_manual_dir";
		if test -e "$php_manual_dir/html/funcref.html"; then
			PHP_MANUAL_DIR="$php_manual_dir/html"
			break
		elif test "$php_manual_dir/funcref.html"; then
			PHP_MANUAL_DIR="$php_manual_dir"
			break
		else
			unset php_manual_dir
		fi
	done
fi

if test -z "$PHP_MANUAL_DIR"; then
	printf "\n\t**error:** PHP's manual could not be found.\n";
else
	printf "\n\t$GETTEXT_PACKAGE will use the copy of PHP's manual found in $PHP_MANUAL_DIR\n";
fi


