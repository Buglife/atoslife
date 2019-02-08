require 'atosl'
puts "Testing..."
test = Atosl.convert("arm64", "/Users/schukin/Desktop/CrashDummy-iPhoneX", "0x100a34000", ["0x100a38f0c"])
#test = Atosl.convert("arm64", "/Users/schukin/Desktop/CrashDummy-iPhoneXSMax", ["0x100c58f0c"])
puts "Result: #{test}"