
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
  
  class SynchronizationError < RuntimeError
  end
  
  # Raised if a method depends on functionality not yet completed in libmemcached. 
  class NotImplemented < NoMethodError
  end

#:stopdoc:

  class << self
    private
    def camelize(string)
      string.downcase.gsub('/', ' or ').split(' ').map {|s| s.capitalize}.join
    end   
  end
  
  EXCEPTIONS = []
  EMPTY_STRUCT = Rlibmemcached::MemcachedSt.new
  Rlibmemcached.memcached_create(EMPTY_STRUCT)
  
  # Generate exception classes
  Rlibmemcached::MEMCACHED_MAXIMUM_RETURN.times do |index|
    description = Rlibmemcached.memcached_strerror(EMPTY_STRUCT, index)
    exception_class = eval("class #{camelize(description)} < Error; self; end")
    EXCEPTIONS << exception_class
  end
  
  # Verify library version
  # XXX Waiting on libmemcached 0.14
  
#:startdoc:
end
