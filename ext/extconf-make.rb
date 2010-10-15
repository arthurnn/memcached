require 'rbconfig'

$CXXFLAGS = ENV['CXXFLAGS']
$PATH = ENV['SOURCE_DIR']
HERE = ENV['HERE']

BSD = RbConfig::CONFIG['build_os'] =~ /^freebsd|^openbsd/

Dir.chdir(HERE)

old_dir = Dir.pwd

Dir.chdir($PATH)

system("cd .") if BSD #Fix for a "quirk" that BSD has..
        
puts(cmd = "make CXXFLAGS='#{$CXXFLAGS}' || true 2>&1")
raise "'#{cmd}' failed" unless system(cmd)

puts(cmd = "make install || true 2>&1")
raise "'#{cmd}' failed" unless system(cmd)

Dir.chdir(old_dir)