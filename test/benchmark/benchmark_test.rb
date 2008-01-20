
require "#{File.dirname(__FILE__)}/../test_helper"
require 'benchmark/unit'
require 'memcache'

class BenchmarkTest < Test::Unit::TestCase

  def setup
    @value = OpenStruct.new(:a => 1, :b => 2, :c => GenericClass)
  end

  def test_original_speed
    @cache = MemCache.new(['127.0.0.1:43042', '127.0.0.1:43043'])
    assert_faster(0.01) do
      @cache.set 'test_speed', @value
      @cache.get 'test_speed'
    end
  end
  
  def test_new_speed
    @cache = Memcached.new(['127.0.0.1:43042', '127.0.0.1:43043'])
    assert_faster(0.001) do
      @cache.set 'test_speed', @value
      @cache.get 'test_speed'
    end  
  end
  
end