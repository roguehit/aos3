#! /bin/bash 

killall -KILL server
if [ $# -ne 1 ] 
then 
	echo "Argument (Number of Server Missing)"
	exit -1;
fi 

for ((c = 0; c < $1 ; c++))
do
	port=$((8080+$c))
	./server $port &
done
