#!/bin/env sh
if [ ! $1 ]
then
    echo "empty mode: debug/test"
    exit 1
fi


echo [] > compile_commands.json
make -nB $1 |while IFS= read -r line;do echo $line | xargs bear --append -- ;done