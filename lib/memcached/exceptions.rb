
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
* Memcached::EncounteredAnUnknownStatKey
* Memcached::Failure
* Memcached::FetchWasNotCompleted
* Memcached::HostnameLookupFailure
* Memcached::ItemValue
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
* Memcached::ServerIsMarkedDead
* Memcached::ServerValue
* Memcached::SomeErrorsWereReported
* Memcached::StatValue
* Memcached::SystemError
* Memcached::TheHostTransportProtocolDoesNotMatchThatOfTheClient
* Memcached::UnknownReadFailure
* Memcached::WriteFailure

=end
  class Error < RuntimeError
    attr_accessor :no_backtrace

    def set_backtrace(*args)
      @no_backtrace ? [] : super
    end

    def backtrace(*args)
      @no_backtrace ? [] : super
    end
  end

#:stopdoc:

  class << self
    private
    def camelize(string)
      string.downcase.gsub('/', ' or ').split(' ').map {|s| s.capitalize}.join
    end
  end

  ERRNO_HASH = Hash[*Errno.constants.grep(/^E/).map{ |c| [Errno.const_get(c)::Errno, Errno.const_get(c).new.message] }.flatten]

  EXCEPTIONS = []
  empty_struct = Lib.memcached_create(nil)
  Lib.memcached_create(empty_struct)

  # Generate exception classes
  Lib::MEMCACHED_MAXIMUM_RETURN.times do |index|
    description = Lib.memcached_strerror(empty_struct, index).gsub("!", "")
    exception_class = eval("class #{camelize(description)} < Error; self; end")
    EXCEPTIONS << exception_class
  end

#:startdoc:
end
