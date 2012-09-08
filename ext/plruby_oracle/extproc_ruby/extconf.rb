require 'mkmf'

oracle_home = ENV['ORACLE_HOME']
if oracle_home.nil? || oracle_home.length == 0
  raise "The environment variable ORACLE_HOME is not set"
end

is_windows = RUBY_PLATFORM =~ /mswin32|cygwin|mingw32/

if is_windows
  $CFLAGS += " -I#{oracle_home}/oci/include"
  $libs += " #{oracle_home}/oci/lib/msvc/oci.lib"
else
  $CFLAGS += " -I#{oracle_home}/rdbms/public"
end

create_makefile('extproc_ruby')
