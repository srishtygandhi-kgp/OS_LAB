#awk -F ' ' '{major[$2]++} END{for (i in major) print i, major[i]}' "$1" | sort -nk1 | sort -rsnk2
#echo
#awk -F ' ' '{student[$1]++} END{for (i in student) {if (student[i]>1) {print i;} else {cnt++}} print cnt; }' "$1"
awk '{major[$2]++}END{for(i in major){print i, major[i]}}' "$1"|sort -nk1|sort -rsnk2
echo
awk '{student[$1]++}END{for(i in student){if(student[i]>1){print i;}else{cnt++}}print cnt;}' "$1"
