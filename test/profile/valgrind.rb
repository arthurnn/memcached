require "#{File.dirname(__FILE__)}/../setup"

exec("valgrind --tool=memcheck --error-limit=no --undef-value-errors=no --leak-check=full --show-reachable=no --num-callers=15 --track-fds=yes --workaround-gcc296-bugs=yes --max-stackframe=7304328 --dsymutil=yes ruby -r#{File.dirname(__FILE__)}/exercise.rb -e \"Worker.new(ENV['TEST'] || 'mixed', 50, 'true').work\"")
