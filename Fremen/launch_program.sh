#!/bin/bash
`whereis "$1" | awk '{print $2}'` "${@:2}"
