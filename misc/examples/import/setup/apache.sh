#!/bin/bash

vhpath=$(locate sites-enabled|grep apache|grep enabled$)
herepath=$(pwd)
source=import.conf

if [ -z $vhpath ]; then
    echo "Unable to find Apache sites-enabled directory."
    exit 1
fi

echo
echo "Copying virtual host file ${source} from "
echo "[33m${herepath}[39m"
echo "to [33m${vhpath}[39m"
echo

cd "${vhpath}"
cp -s "${herepath}/${source}" .

echo
echo Restarting Apache
/etc/init.d/apache2 restart

