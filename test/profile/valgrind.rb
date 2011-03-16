require "#{File.dirname(__FILE__)}/../setup"

COMMAND =  "--dsymutil=yes ruby -r#{File.dirname(__FILE__)}/exercise.rb -e \"Worker.new(ENV['TEST'] || 'everything', (ENV['LOOPS'] || 50).to_i, 'true').work\""

case ENV["TOOL"]
when nil, "memcheck":
  exec("valgrind --tool=memcheck --error-limit=no --undef-value-errors=no --leak-check=full --show-reachable=no --num-callers=15 --track-fds=yes --workaround-gcc296-bugs=yes --leak-resolution=med --max-stackframe=7304328 #{COMMAND}")
when "massif":
  exec("valgrind --tool=massif --time-unit=B #{COMMAND}")
end
