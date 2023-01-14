while read -r line;do if echo "$line"|grep -q "$2";then 
        echo "$line"|sed -r 's/(.)/\L\1/g;s/(\S)(\S)/\1\U\2/g';else 
            echo "$line";fi;done<"$1"
