lcm=1;while read x;do reverse=$(echo $x|rev);if((reverse!=0));then a=$lcm;b=$reverse;while((b!=0));do r=$((a%b));a=$b;b=$r;done;lcm=$((lcm*reverse/a));fi;done<$1;echo $lcm
