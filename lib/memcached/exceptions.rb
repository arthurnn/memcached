
class Memcached

=begin rdoc

Superclass for all Memcached runtime exceptions. 

Subclasses correspond one-to-one with server response strings or libmemcached errors. For example, raising <b>Memcached::NotFound</b> means that the server returned <tt>"NOT_FOUND\r\n"</tt>.

== Subclasses

* Memcached::ABadKeyWasProvidedOrCharactersOutOfRange
* Memcached::AKeyLengthOfZeroWasProvided
* Memcached::ATimeoutOccurred
* Memcached::ActionNotSupported
* Memcached::ActionQueued
* Memcached::ClientError
* Memcached::ConnectionBindFailure
* Memcached::ConnectionDataDoesNotExist
* Memcached::ConnectionDataExists
* Memcached::ConnectionFailure
* Memcached::ConnectionSocketCreateFailure
* Memcached::CouldNotOpenUnixSocket
* Memcached::Failure
* Memcached::FetchWasNotCompleted
* Memcached::HostnameLookupFailure
* Memcached::MemoryAllocationFailure
* Memcached::NoServersDefined
* Memcached::NotFound
* Memcached::NotStored
* Memcached::PartialRead
* Memcached::ProtocolError
* Memcached::ReadFailure
* Memcached::ServerDelete
* Memcached::ServerEnd
* Memcached::ServerError
* Memcached::ServerValue
* Memcached::SomeErrorsWereReported
* Memcached::StatValue
* Memcached::SystemError
* Memcached::UnknownReadFailure
* Memcached::WriteFailure

=end
  class Error < RuntimeError
  end 

#:stopdoc:

  class << self
    private
    def camelize(string)
      string.downcase.gsub('/', ' or ').split(' ').map {|s| s.capitalize}.join
    end           
  end
  
  ERRNO_HASH = Hash[*Errno.constants.map{ |c| [Errno.const_get(c).const_get("Errno"), Errno.const_get(c).new.message] }.flatten]
  
  EXCEPTIONS = []
  EMPTY_STRUCT = Rlibmemcached::MemcachedSt.new
  Rlibmemcached.memcached_create(EMPTY_STRUCT)
  
  # Generate exception classes
  Rlibmemcached::MEMCACHED_MAXIMUM_RETURN.times do |index|
    description = Rlibmemcached.memcached_strerror(EMPTY_STRUCT, index)
    exception_class = eval("class #{camelize(description)} < Error; self; end")
    EXCEPTIONS << exception_class
  end
  
  class NotFound
    attr_accessor :no_backtrace
    
    def set_backtrace(*args)
      @no_backtrace ? [] : super
    end
    
    def backtrace(*args)
      @no_backtrace ? [] : super
    end
  end
  
  # Verify library version
  # XXX Waiting on libmemcached 0.18
  
#:startdoc:
end
