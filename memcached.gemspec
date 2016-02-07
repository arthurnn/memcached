# -*- encoding: utf-8 -*-

Gem::Specification.new do |s|
  s.name = "memcached"
  s.version = "1.8.0"

  s.authors = ["Evan Weaver"]
  s.email = ""
  s.description = "An interface to the libmemcached C client."
  s.summary = "An interface to the libmemcached C client."
  s.homepage = "http://evan.github.com/evan/memcached/"

  s.files         = `git ls-files`.split("\n")
  s.test_files    = `git ls-files -- {test,spec,features}/*`.split("\n")
  s.require_paths = ["lib", "ext"]
  s.extensions = ["ext/extconf.rb"]

  s.licenses = ["Academic Free License 3.0 (AFL-3.0)"]
  s.rdoc_options = ["--line-numbers", "--inline-source", "--title", "Memcached", "--main", "README.rdoc", "--exclude=ext/bin", "--exclude=ext/libmemcached-.*/(clients|tests)"]

  s.add_development_dependency 'minitest'
  s.add_development_dependency 'rake'
  s.add_development_dependency 'mocha'
  s.add_development_dependency 'activesupport'
  s.add_development_dependency "rake-compiler"
end
