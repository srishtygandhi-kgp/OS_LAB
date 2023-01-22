dir=$1
find $dir -name "*.py"|while read file
do echo"File:$file"
awk '{if($0~/.*#.*/){if($0!~/^[[:blank:]]*#/){gsub(/[^#]+#/,"",$0)
    print"",NR,$0}else{print"",NR,$0}}if($0~/.*\"\"\"/){if(multi_line==0){if($0~/\"\"\".*\"\"\"/){print"Start_and_End_of_multiline_comment_at_line_",NR}else{multi_line=1
    multi_line_start=NR
    print"Start_of_multiline_comment_at_line_",NR}}else{multi_line=0
    print"",NR,$0
    print"End_of_multiline_comment_at_line_",NR}}if($0~/#/){if($0!~/^[[:blank:]]*#/){gsub(/[^#]+#/,"",$0)
    print"",NR,$0}}if(multi_line==1){print"",NR,$0}}' $file
done
