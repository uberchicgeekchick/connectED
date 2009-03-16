api_DATA = php-function-reference.api
apidir = $(datadir)/connectED

icondir = $(datadir)/pixmaps
icon_DATA = connectED.svg

desktop_in_files = connectED.desktop.in
desktop_in_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

desktopdir = $(datadir)/applications
desktop_DATA = connectED.desktop

DATA = $(api_DATA) $(desktop_in_DATA) $(desktop_DATA) $(icon_DATA)

CONFIG_CLEAN_FILES = connectED.desktop

