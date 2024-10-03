#!/bin/bash
if [ $# -lt 2 ]
then
    echo "Less arugments"
    exit 1
fi
writefile=$1
writestr=$2

install -D /dev/null $writefile
echo $writefile
echo $writestr>$writefile
#echo "The number of files are $filescount and the number of matching lines are $searchcnt"
