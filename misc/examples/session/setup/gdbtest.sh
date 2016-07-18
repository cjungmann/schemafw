#!/bin/bash

result=$(mysql -N -s -u root -pprunes2u SessionDemo -e "SELECT * FROM SSYS_SESSION WHERE available=0")

echo $result

if [[ $result =~ ([0-9]+)[[:space:]]([^[:space:]]+).* ]]; then
    sess_id="${BASH_REMATCH[1]}"
    sess_hash="${BASH_REMATCH[2]}"
    export HTTP_COOKIE="SessionId=${sess_id}; SessionHash=${sess_hash}"
    cd /home/chuck/work/cgi/schema
    gdbtui schema
#    ./schema
    cd -
else
    echo No match!
fi




