# NEW STUFF

Development notes:

1. `rvm current` to find out which rvm
2. Make sure gems are being installed at `/Users/USERNAME/.rvm/gems/ruby-2.X.Y/gems`
3. Make sure you have libelf `brew install libelf`
3. Make whatever code changes
4. To test, run this (but **change ruby-2.5.1 to your ruby version!!!**)
```
rm atoslife-0.0.40.gem ; rm -rf /Users/schukin/.rvm/gems/ruby-2.5.1/lib/ruby/gems/2.5.0/gems/atoslife-0.0.40 ; gem build atoslife.gemspec ; gem install atoslife-0.0.40.gem ; ruby ./test.rb
```


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
