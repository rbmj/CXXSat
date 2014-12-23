#!/bin/sh
cryptominisat "$@"
[ $? -eq 10 ]
exit $?
