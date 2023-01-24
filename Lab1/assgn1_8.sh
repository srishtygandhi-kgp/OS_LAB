#!/bin/bash
f="main.csv"
[ ! -f $f ] && echo "Date,Category,Amount,Name" > $f
i(){ echo "$1,$2,$3,$4" >> $f; echo "Inserted $1,$2,$3,$4 in main.csv";}
ct(){ cat $f | awk -F',' -v c="$1" '$2==c {sum+=$3} END {print "Total spent on "c": "sum}';}
nt(){ cat $f | awk -F',' -v n="$1" '$4==n {sum+=$3} END {print "Total spent by "n": "sum}' $f;}
s(){
    case $1 in
        Date) col="1,1n";;
        Category) col="2,2";;
        Amount) col="3,3n";;
        Name) col="4,4";;
        *) col="1,1n";;
    esac
    sort -t, -k $col $f -o $f;
    echo "main.csv sorted by $1 column";}
h(){ echo "Usage: sh Assgn1_8_<groupno>.sh [-c category] [-n name] [-s column] record [record...]";
echo "Description: Inserts records in main.csv, -c for category total, -n for name total, -s for sort, -h for help";}
[ $# -eq 0 ] && { h; exit 1;}
while [ $# -gt 0 ]; do
    case $1 in
        -c) ct $2; s 1; shift;;
        -n) nt $2; s 1; shift;;
        -s) s $2; shift;;
        -h) h; exit;;
        *) i $1 $2 $3 $4; s 1; shift 3;;
    esac
    shift
done
