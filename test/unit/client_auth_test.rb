require 'test_helper'

class ClientAuthTest < BaseTest

  def setup
    skip unless ENV['TEST_CREDENTIAL_SERVER']
    super
    @cache = Memcached::Client.new(ENV['TEST_CREDENTIAL_SERVER'], hash: :default, distribution: :modula, prefix_key: @prefix_key, credentials: [ENV['TEST_USERNAME'], ENV['TEST_PASSWORD']])
  end

  def test_empty_credentials
    assert_raises(ArgumentError) do
      cache = Memcached::Client.new(ENV['TEST_CREDENTIAL_SERVER'], credentials: [])
      cache.connection
    end
  end

  def test_nil_credentials
    assert_raises(TypeError) do
      cache = Memcached::Client.new(ENV['TEST_CREDENTIAL_SERVER'], credentials: [nil, nil])
      cache.connection
    end
  end

  def test_wrong_credentials
    cache = Memcached::Client.new(ENV['TEST_CREDENTIAL_SERVER'], credentials: ['foo', 'bar'])
    cache.connection

    assert_raises(Memcached::AuthFailure) do
      cache.set key, 'hi'
    end
  end

  def test_set
    @cache.set key, 'arthurnn'
  end

  def test_set_and_get
    @cache.set key, 'arthurnn'
    assert_equal 'arthurnn', @cache.get(key)
  end
end
