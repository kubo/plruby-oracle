require 'mkmf'
require File.dirname(__FILE__) + '/oraconf'

oraconf = OraConf.get()

$CFLAGS += oraconf.cflags
$libs += oraconf.libs

if oraconf.cc_is_gcc
  $CFLAGS += " -Wall"
end

create_makefile('extproc_ruby')
