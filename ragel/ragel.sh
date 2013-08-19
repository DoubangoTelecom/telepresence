# Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>.
# License: GPLv3 or proprietary (consult us)

# Ragel generator
# For more information about Ragel: http://www.complang.org/ragel/

export OPTIONS="-C -L -T0"


ragel.exe $OPTIONS -o ../source/cfg/OTCfgParser.cc cfg.rl