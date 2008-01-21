
require "#{File.dirname(__FILE__)}/../test_helper"

require 'rubygems'
require 'benchmark/unit'
require 'memcache'

class BenchmarkTest < Test::Unit::TestCase

  def setup
    @value = OpenStruct.new(:a => 1, :b => 2, :c => GenericClass)
    @opts = [
      ['127.0.0.1:43042', '127.0.0.1:43043'], 
      {:namespace => "benchmark_namespace"}
    ]
  end

  #  def test_original_speed
  #    @cache = MemCache.new(*@opts)
  #    assert_faster(0.02) do
  #      @cache.set 'key1', @value
  #      @cache.get 'key1'
  #      @cache.set 'key2', @value
  #      @cache.set 'key3', @value
  #      @cache.get 'key2'
  #      @cache.get 'key3'
  #    end
  #  end
  
  def test_basic_speed
    @cache = Memcached.new(*@opts)
    assert_faster(0.005) do
      @cache.set 'key1', @value
      @cache.get 'key1'
      @cache.set 'key2', @value
      @cache.set 'key3', @value
      @cache.get 'key2'
      @cache.get 'key3'
    end  
  end
  
end