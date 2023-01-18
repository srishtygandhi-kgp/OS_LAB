#!/bin/bash
f="main.csv"
[ ! -f $f ] && echo "Date,Category,Amount,Name" > $f
i(){ echo "$1,$2,$3,$4" >> $f; echo "Inserted $1,$2,$3,$4 in main.csv";}
ct(){ awk -F','-v c="$1"'$2==c {sum+=$3} END {print "Total spent on "c": "sum}' $f;}
nt(){ awk -F','-v n="$1"'$4==n {sum+=$3} END {print "Total spent by "n": "sum}' $f;}
s(){ sort -t',' -k$1 $f -o $f; echo "main.csv sorted by $1 column";}
h(){ echo "Usage: sh Assgn1_8_<groupno>.sh [-c category] [-n name] [-s column] record [record...]";
echo "Description: Inserts records in main.csv, -c for category total, -n for name total, -s for sort, -h for help";}
[ $# -eq 0 ] && { h; exit 1;}
while [ $# -gt 0 ]; do
    case $1 in
        -c) ct $2; shift;;
        -n) nt $2; shift;;
        -s) s $2; shift;;
        -h) h; exit;;
        *) i $1 $2 $3 $4; shift 3;;
    esac
    shift
done
s 1
