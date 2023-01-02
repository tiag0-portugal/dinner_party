#!/bin/bash

rm -f error*
rm -f core

ipcs -m -s

echo "Delete semaphores and memory? [y,N]"
read opt

if [[ "$opt" == "y" ]]; then 
    ipcs -s | tail +4 | sort -r -k 2 | head -n 1 | awk '{system("ipcrm -s "$2 )}'
    ipcs -m | tail +4 | sort -r -k 2 | head -n 1 | awk '{system("ipcrm -m "$2 )}'
fi
