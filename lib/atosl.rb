require "atosl/version"
require 'atosl/atosl'

module Atosl
  def self.convert(arch, executable, load_address, addresses)
    self.symbolicate(arch, executable, load_address, addresses);
  end
end
