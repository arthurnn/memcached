
require "#{File.dirname(__FILE__)}/../test_helper"

class RailsTest < Test::Unit::TestCase

  def setup
    @servers = ['127.0.0.1:43042', '127.0.0.1:43043']
    @namespace = 'rails_test_namespace'
    @cache = Memcached::Rails.new(@servers, :namespace => @namespace)
    @value = OpenStruct.new(:a => 1, :b => 2, :c => GenericClass)
    @marshalled_value = Marshal.dump(@value)
  end
  
end