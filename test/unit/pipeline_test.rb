require 'test_helper'

class PipelineTest < BaseTest

  def setup
    super
    @noblock_options = {
#      :prefix_key => @prefix_key,
      :no_block => true,
      :noreply => true,
      :buffer_requests => true,
      :hash => :default,
      :distribution => :modula,
    }
    @noblock_cache = Memcached::Client.new(@servers, @noblock_options)
  end

  def teardown
    super
    @noblock_cache.flush
  end

  def test_buffered_requests_return_value
    cache = Memcached::Client.new @servers, :buffer_requests => true
    # Buffered requests dont have a return
    assert_nil cache.set(key, @value)
  end

  def test_no_block_return_value
    assert @noblock_cache.set(key, @value)
  end

#  def test_no_block_prepend
#    ## TODO
#    cache.set key, "help", raw: true
#    assert_equal "help", cache.get(key, raw: true)
#
#    @noblock_cache.prepend key, "help"
#    assert_equal "help", cache.get(key, raw: true)
#    # flush in a get
#    @noblock_cache.get "no_exist", raw: true
#    assert_equal "helphelp", cache.get(key, raw: true)
#  end

  def test_no_block_get
    @noblock_cache.set key, @value
    assert_equal @value, @noblock_cache.get(key)
  end

  def test_no_block_missing_delete
    @noblock_cache.delete key
    refute @noblock_cache.delete(key)
  end

  def test_no_block_set_invalid_key
    assert_raises(Memcached::BadKeyProvided) do
      @noblock_cache.set "I'm so bad", @value
    end
  end

  def test_no_block_set_object_too_large
    noblock_cache = Memcached::Client.new(@servers, @noblock_options.merge(:noreply => false))
    noblock_cache.set key, "I'm big" * 1000000

    assert_raises(Memcached::Errno, Memcached::ConnectionFailure) do
      @noblock_cache.set key, "I'm big" * 1000000
    end
  end

  def test_no_block_existing_add
    noblock_cache = Memcached::Client.new(@servers, @noblock_options.merge(:noreply => false))
    # Dont return right away.
    assert_nil noblock_cache.set(key, @value)
    # However returns false if it tries to add
    refute noblock_cache.add(key, @value)
  end
end
