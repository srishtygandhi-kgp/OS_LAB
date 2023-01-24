declare -a smallest_prime vis
for ((i=2; i<=1000000; i+=2)); do smallest_prime[i]=2; done
for ((i=3; i<=1000000; i+=2)); do
  if ! [[ ${vis[$i]} ]]; then
        smallest_prime[$i]=$i;
        for ((j=$i; $j*$i<=1000000; j+=2));do
            if ! [[ ${vis[$j*$i]} ]]; then
                  vis[$j*$i]=1 
                  smallest_prime[$j*$i]=$i;
                fi
        done
	fi
done
while read line;do
    declare -a prime_divisors=()
    num=$(echo "$line" | tr -d '\r')
    while [[ $num -ne 1 ]] 
    do
            prime_divisors+=(${smallest_prime[$num]})
            ans=`expr $num / ${smallest_prime[$num]}`
            num=$ans
    done
    echo "${prime_divisors[@]}"
done < "$1" > "output.txt"
