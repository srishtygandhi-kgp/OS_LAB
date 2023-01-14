#!/bin/bash

# Store the input file name
file=$1

# Initialize variable to store reversed numbers
reversed=""

# Read each line of the input file
while read line; do
  # Reverse the number on the line
  reverse=$(echo $line | rev)
  # Append the reversed number to the reversed variable
  reversed="$reversed $reverse"
done < $file

# Output the LCM of the reversed numbers
lcm=$(echo $reversed | tr ' ' '\n' | head -n 1)
for num in $(echo $reversed | tr ' ' '\n' | tail -n +2); do
  a=$lcm
  b=$num
  while [ $b -ne 0 ]; do
    rem=$((a%b))
    a=$b
    b=$rem
  done
  gcd=$a
  lcm=$((lcm*num/gcd))
done
echo $lcm
