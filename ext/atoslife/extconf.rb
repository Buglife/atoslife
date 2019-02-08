require 'mkmf'

extension_name = 'atoslife/atoslife'

abort "missing malloc()" unless have_func "malloc"
abort "missing free()"   unless have_func "free"


LIBDWARF_TARBALL_FILENAME = 'libdwarf_feb_7_2019.tar.gz'
CWD = File.expand_path(File.dirname(__FILE__))

def sys(cmd)
  puts "  -- #{cmd}"
  unless ret = xsystem(cmd)
    raise "#{cmd} failed, please check #{CWD}/mkmf.log"
  end
  ret
end

puts "Current working directory: #{CWD}"

$LIBPATH.unshift "#{CWD}/dst/lib"
$INCFLAGS[0,0] = "-I#{CWD}/dst/include "

puts "ℹ️  Installing libdwarf..."
# sys("tar zxvf #{LIBDWARF_TARBALL_FILENAME}")

Dir.chdir("#{CWD}/libdwarf") do
	puts "• Current directory: #{`pwd`}"

	DIR_DST = "#{CWD}/dst"
	DIR_DST_LIB = "#{CWD}/dst/lib"
	DIR_DST_INCLUDE = "#{CWD}/dst/include"

	ENV['CFLAGS'] = "-fPIC -I#{CWD}/dst/include"
    ENV['LDFLAGS'] = "-L#{CWD}/dst/lib"
    puts "• Running configure..."
	sys('./configure')
	puts "• Running make..."
	sys('make')
	puts "• Running make install..."
	sys('make install')

	[DIR_DST, DIR_DST_LIB, DIR_DST_INCLUDE].each do |dir_path|
		file_path = "#{dir_path}/"
		puts "• Checking if dir exists: #{file_path}"
		unless File.directory?(file_path)
			FileUtils.mkdir_p(file_path)
			puts "  • Created directory: #{file_path}"
		end
	end
	
	FileUtils.cp 'libdwarf/.libs/libdwarf.a', "#{DIR_DST_LIB}/libdwarf_ext.a"
	FileUtils.cp 'libdwarf/dwarf.h', "#{DIR_DST_INCLUDE}/"
	FileUtils.cp 'libdwarf/libdwarf.h', "#{DIR_DST_INCLUDE}/"

	unless have_library('dwarf_ext')
	    raise 'libdwarf build failed'
	  end
end

puts "✅  libdwarf installed"

dir_config(extension_name)
create_makefile(extension_name)

require 'rake/extensiontask'
Rake::ExtensionTask.new(extension_name)

