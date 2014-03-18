# Atosl

atosl is used for converting binary addresses within a macho file to symbols for Linux platform.

## Installation

Add this line to your application's Gemfile:

    gem 'atosl'

And then execute:

    $ bundle

Or install it yourself as:

    $ gem install atosl

## Usage

    Usage: atosl --arch architecture -o executable -a [address,address,...]

## Example

    atosl --arch armv7s -o ~/res/CrashTest3Dwarf.thin -a 0x0000b1e7,0x123123

## Reference

[Original](https://github.com/renoqiu/atosl "atosl").

## Contributing

1. Fork it
2. Create your feature branch (`git checkout -b my-new-feature`)
3. Commit your changes (`git commit -am 'Add some feature'`)
4. Push to the branch (`git push origin my-new-feature`)
5. Create new Pull Request
