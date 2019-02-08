# coding: utf-8
lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'atoslife/version'

Gem::Specification.new do |spec|
  spec.name          = "atoslife"
  spec.version       = Atoslife::VERSION
  spec.authors       = ["Buglife"]
  spec.email         = ["support@buglife.com"]
  spec.summary       = %q{atoslife for Linux.}
  spec.description   = %q{atoslife is used for converting binary addresses within a macho file to symbols.}
  spec.homepage      = "https://github.com/Buglife/atoslife"
  spec.license       = "MIT"

  spec.files         = `git ls-files`.split($/)
  spec.executables   = spec.files.grep(%r{^bin/}) { |f| File.basename(f) }
  spec.test_files    = spec.files.grep(%r{^(test|spec|features)/})
  spec.extensions << "ext/atoslife/extconf.rb"
  spec.require_paths = ["lib"]

  spec.add_development_dependency "bundler", "~> 1.4"
  spec.add_development_dependency "rake"
  spec.add_development_dependency "rake-compiler"
end
