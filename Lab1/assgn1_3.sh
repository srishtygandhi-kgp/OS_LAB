# Check if the required number of arguments are provided
if [ $# -lt 2 ]; then
    echo "Usage: $0 <directory> <destination> <attribute1> [attribute2...]"
    exit 1
fi

# Assign the arguments to variables
INPUT_DIR=$1
OUTPUT_DIR=$2
attributes=( "${@:3}" )

# Create the destination folder if it does not exist
mkdir -p $OUTPUT_DIR

# Loop through all the JSONL files in the directory
for file in $INPUT_DIR/*.jsonl; do
    # Create the corresponding CSV file in the destination folder
    csv_file=$OUTPUT_DIR/$(basename $file .jsonl).csv
    # Write the header row with the attribute names
    printf "%s\n" "$(IFS=,; echo "${attributes[*]}")" > $csv_file
    # Loop through each line in the JSONL file
    while IFS= read -r line; do
        # Extract the values of the specified attributes
        row=""
        for attribute in "${attributes[@]}"; do
            value=$(echo $line | jq -r ".$attribute")
            # Check if the value contains a comma and add double quotes if necessary
            if [[ $value == *,* ]]; then
                value="\"$value\""
            fi
            row="$row,$value"
        done
        # Write the row to the CSV file
        printf "%s\n" "${row:1}" >> $csv_file
    done < $file
done
