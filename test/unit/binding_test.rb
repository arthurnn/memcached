
require File.expand_path("#{File.dirname(__FILE__)}/../test_helper")

class BindingTest < Test::Unit::TestCase
  def test_libmemcached_loaded
    Warning.expects(:warn).with(regexp_matches(/constant ::Rlibmemcached is deprecated/), optionally(anything))
    Rlibmemcached
  end
end
