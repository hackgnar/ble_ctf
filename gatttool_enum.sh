#!/bin/bash
# run with the following syntax:
# ./ghetto_bleah.sh 11:22:33:44:55:66

mac=$1 ; 
while read i; 
do 
printf "%s " $(echo -n $i|awk '{printf "%s : ", $1'}); 
printf "%s " $(echo -n $i|awk '{printf "%s : ", $3'}); 
#printf "%s : " $(echo -n $i|awk '{printf "%s", $2'}); 
j=$(printf "%.8d" $(echo "obase=2; ibase=12; $(echo -n $i|awk '{printf "%s", toupper($2)'}|cut -b3-)"|bc)); 
#printf "%s : " $j;
if [ $(echo -n $j |cut -b7) == "1" ]; then printf "READ "; else printf "     "; fi;
if [ $(echo -n $j |cut -b5) == "1" ] || [ $(echo -n $j |cut -b6) == "1" ] ; then printf "WRITE "; else printf "      "; fi;
if [ $(echo -n $j |cut -b4) == "1" ]; then printf "NOTIFY "; else printf "       "; fi;
if [ $(echo -n $j |cut -b3) == "1" ]; then printf "INDICATE "; else printf "         "; fi;
printf ": "
gatttool --char-read -b $mac -a $(echo -n $i|awk '{printf "%s : ", $1'}) |awk -F':' '{print $2}'|tr -d ' '|xxd -r -p; printf '\n' ; 
done < <(sudo  gatttool -b $mac --characteristics |awk  '{print $12, $7, $15}' |tr -d ',')