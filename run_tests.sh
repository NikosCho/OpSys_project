#!/bin/bash
# server needs to be running
ps --user `whoami` | grep "server2" > /dev/null
if [ $? -eq 1 ]; then
    echo "Server not running";
    echo "run ./server2 before the script";
    # commit suicide
 
    kill $$
fi
for i in {1..600};
do
	p1=$(($RANDOM % 3 + 0))
	p2=$(($RANDOM % 4 + 1))
	p3=$(($RANDOM % 9999 + 1000))
	echo  $p1 $p2 $p3 | ./client2 >/dev/null &
done



# for killing the server SIGINT
# if prefered as in the following command
# kill -SIGINT pid
# to allow the server to exit properly

