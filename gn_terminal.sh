#!/bin/bash

cd `dirname $0` >/dev/null
export cur_scriptdir=`pwd`
cd - >/dev/null
export PATH=$cur_scriptdir/gn/gn:$cur_scriptdir/gn/ninja:$PATH
