[ -d $2 ] || mkdir -p $2
declare -A names
for file in $1/*.txt; do
    while read -a line; do
        names[$(echo "${line:0:1}" | awk '{print tolower($1)}')]="${names[$(echo "${line:0:1}" | awk '{print tolower($1)}')]} $line";
    done < $file
done
for x in {a..z}
do  
    echo "${names[$x]}" | tr " " "\n" | sort -n > $2/${x}.txt
done
 


