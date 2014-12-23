#!/bin/sh
minisat "$@"
[ $? -eq 10 ]
exit $?
