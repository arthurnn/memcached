require "#{File.dirname(__FILE__)}/../setup"

exec("valgrind  --tool=memcheck --leak-check=full --show-reachable=no --num-callers=15 --track-fds=yes --workaround-gcc296-bugs=yes --max-stackframe=7304328 --dsymutil=yes --track-origins=yes ruby -r#{File.dirname(__FILE__)}/exercise.rb -e \"Worker.new('mixed', 10000, 'true').work\"")
