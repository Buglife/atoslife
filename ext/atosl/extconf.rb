require "mkmf"

extension_name = 'atosl'

abort "missing malloc()" unless have_func "malloc"
abort "missing free()"   unless have_func "free"

dir_config(extension_name)
create_makefile(extension_name)
