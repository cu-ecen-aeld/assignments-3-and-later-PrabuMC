#!/bin/bash
if [ $# -lt 2 ]
then
    echo "Less arugments"
    exit 1
fi
filesdir=$1
searchstr=$2
echo 
if [ ! -d "$filesdir" ]
then
    echo "Directory does not exist"
    exit 1
fi
filescount=$(ls -1 $filesdir | wc -l)
echo "num files in $filesdir are $filescount"

searchcnt=$(cd $filesdir && grep "$searchstr" * | wc -l)
echo "search count for $searchstr is $searchcnt"

echo "The number of files are $filescount and the number of matching lines are $searchcnt"
