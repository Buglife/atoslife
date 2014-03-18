# coding: utf-8
lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'atosl/version'

Gem::Specification.new do |spec|
  spec.name          = "atosl"
  spec.version       = Atosl::VERSION
  spec.authors       = ["Reno Qiu"]
  spec.email         = ["dechao.qiu@gmail.com"]
  spec.summary       = %q{atos for Linux.}
  spec.description   = %q{atosl is used for converting binary addresses within a macho file to symbols.}
  spec.homepage      = "https://github.com/renoqiu/atosl"
  spec.license       = "MIT"

  spec.files         = `git ls-files`.split($/)
  spec.executables   = spec.files.grep(%r{^bin/}) { |f| File.basename(f) }
  spec.test_files    = spec.files.grep(%r{^(test|spec|features)/})
  spec.extensions << "ext/atosl/extconf.rb"
  spec.require_paths = ["lib"]

  spec.add_development_dependency "bundler", "~> 1.4"
  spec.add_development_dependency "rake"
  spec.add_development_dependency "rake-compiler"
end
