
# apt backend
do_refresh_apt () {
    $GETROOT apt-get refresh
    $GETROOT apt-get -q -s upgrade
}
do_upgrade_apt () {
    $GETROOT apt-get upgrade
}
do_install_apt () {
    $GETROOT apt-get install $@
}

# aptitude backend
do_refresh_aptitude () {
    $GETROOT aptitude update
    $GETROOT aptitude -s -y -v upgrade
}
do_upgrade_aptitude () {
    $GETROOT aptitude upgrade
}
do_install_aptitude () {
    $GETROOT aptitude install $@
}

# generic functions called by apt-dater-host
do_refresh () {
    do_refresh_$DPKGTOOL $@
    $LIBPATH/kernelinfo.sh
}
do_upgrade () {
    do_upgrade_$DPKGTOOL $@
}
do_install () {
    do_install_$DPKGTOOL $@
}

# sanitfy check
case "$DPKGTOOL" in
    apt|aptitude)
	;;
    *)
	echo "Unsupported dpkg front-end '$DPKGTOOL'!" 1>&2
	exit 1
	;;
esac
