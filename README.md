PL/Ruby for Oracle
==================

PL/Ruby for Oracle is a loadable procedural language that enables you
to write Oracle stored procedures in the Ruby programming language. 

This is inspired by [PL/Ruby][1] and [extproc_perl][2].

Current Status
==============

Extremely early stage.
Oracle packages names and function names will be changed later.

Prerequisite
============

* Oracle 10g or later
* Ruby 1.9.1 or later

Installation
============

1.  Set the library search path described in [the ruby-oci8 installation page][3].

2.  Compile plruby_oracle.

    ruby setup.rb

3.  Copy the compiled shared object file.

    On Unix:

        cp ext/plruby_oracle/extproc_ruby/extproc_ruby.so $ORACLE_HOME/lib

    On Windows:

        copy ext\plruby_oracle\extproc_ruby\extproc_ruby.so $ORACLE_HOME\bin

4.  Connect as system and create the library in a user.

    Replace the 'password' and 'username' in the following.

        SQL> connect system/'password'
        SQL> CREATE OR REPLACE LIBRARY username.extproc_ruby_lib AS '${ORACLE_HOME}/lib/extproc_ruby.so';
           2 /

5.  Connect as a user and create the ruby package.

        $ cd lib/plruby_oracle
        $ sqlplus username/password @plruby_oracle_setup.sql

Examples
========

Only primitive functions are defined now. I'll add a wrapper
package generator to use it easily.

ruby.call_n(varchar2) returns an evaluated value as varchar2.

    SQL> select ruby.call_s('["foo", "bar"].join("-")') from dual;
    
    RUBY.CALL_S('["FOO","BAR"].JOIN("-")')
    --------------------------------------------------------------------------------
    foo-bar

ruby.call_n(varchar2) returns an evaluated value as number.

    SQL> select ruby.call_n('1 + 2') from dual;
    
    RUBY.CALL_N('1+2')
    ------------------
                     3

If the second argument is passed to ruby.call_n or ruby_call.s, it is
used as a method name and applied to the result of the first argument.

    SQL> select ruby.call_s('["foo", "bar"].join("-")', 'length') from dual;
    
    RUBY.CALL_S('["FOO","BAR"].JOIN("-")','LENGTH')
    --------------------------------------------------------------------------------
    7

This is equivalent to `eval('["foo", "bar"].join("-")').length` in
ruby.

The third argument is the number of arguments which passed to the ruby
method and rests are ruby method arguments.

    SQL> select ruby.call_n('Math', 'cos', 1, ANYData.ConvertNumber(10)) from dual;
    
    RUBY.CALL_N('MATH','COS',1,ANYDATA.CONVERTNUMBER(10))
    -----------------------------------------------------
                                           -.83907153

This is equivalent to `eval('Math').cos(10)` -> `Math.cos(10)`.

Ruby method arguments must be passed as [ANYData][4] values, whose
type must be binary_double, number or varchar2.

[1]: http://rubyforge.org/projects/plruby/
[2]: http://www.smashing.org/extproc_perl/
[3]: http://ruby-oci8.rubyforge.org/en/file.install-full-client.html
[4]: http://docs.oracle.com/cd/E14072_01/appdev.112/e10577/t_anydat.htm
