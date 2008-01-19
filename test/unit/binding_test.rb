
require "#{File.dirname(__FILE__)}/../test_helper"

class BindingTest < Test::Unit::TestCase
  def test_libmemcached_loaded
    assert_nothing_raised { Libmemcached }
  end
end
