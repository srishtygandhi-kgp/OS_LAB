usernames_file="usernames.txt";invalid_words_file="fruits.txt";output_file="validation_results.txt";
validate_username() {
  local username=$1
  local invalid_words=$(cat $invalid_words_file)
  if [[ ${#username} -lt 5 || ${#username} -gt 20 ]]; then
    echo "NO"
    return
  fi
  if [[ ! $username =~ ^[a-zA-Z0-9]+$ ]]; then
    echo "NO"
    return
  fi
  if [[ ! $username =~ [0-9] ]]; then
    echo "NO"
    return
  fi
  if [[ ! $username =~ ^[a-zA-Z] ]]; then
    echo "NO"
    return
  fi
  for word in $invalid_words; do
    if echo "$username" | grep -iq "$word"; then
      echo "NO"
      return
    fi
  done
  echo "YES"
}

> $output_file

while read username; do
  result=$(validate_username $username)
  echo $result >> $output_file
done < $usernames_file
