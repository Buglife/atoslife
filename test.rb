require 'atoslife'

sample_path = File.expand_path('samples/CrashDummy-iPhoneX')
puts "Testing with dSYM at #{sample_path} ..."

result = Atoslife.convert("arm64", "samples/CrashDummy-iPhoneX", "0x100a34000", ["0x100a38f0c"])
#test = Atosl.convert("arm64", "/Users/schukin/Desktop/CrashDummy-iPhoneXSMax", ["0x100c58f0c"])

puts "Result: #{result}"