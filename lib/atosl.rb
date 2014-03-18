require "atosl/version"
require 'atosl/atosl'

module Atosl
  def self.convert(arch, executable, addresses)
    self.symbolicate(arch, executable, addresses);
  end
end
