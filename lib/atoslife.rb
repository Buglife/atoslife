require "atoslife/version"
require 'atoslife/atoslife'

module Atoslife
  def self.convert(arch, executable, load_address, addresses)
    self.symbolicate(arch, executable, load_address, addresses);
  end
end
