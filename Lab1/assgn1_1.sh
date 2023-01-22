lcm=1&&while IFS= read -r line
do test "$line"&&reversed_num=$(echo "$line" | rev)&&a=$lcm&&b=$reversed_num
while test $b -ne 0
do temp=$b&&b=`expr $a % $b`
a=$temp
done&&lcm=$(echo "$lcm*$reversed_num/$a"|bc)
done<"$1"&&echo "LCM:$lcm"