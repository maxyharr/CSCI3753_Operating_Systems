fname = "/dev/max_driv"

while true
	puts ""
	puts "===================================================" 
	puts "Type 'clear' to empty its contents"
	puts "Press r to read from device"
	puts "Press w to write to device"
	puts "Press wa to write append to device"
	puts "Press e to exit"
	puts "Press anything else to keep reading or writing"
	print "Enter Command: "
	STDOUT.flush  
	str = gets.chomp

	if str == "e"
		puts "Goodbye"
		break
	end

	if str == "r"
		File.open(fname, "r"){|somefile| puts "Device Contents: " + somefile.read} 
	end	

	if str == "wa"
		print "Enter write data: "
		STDOUT.flush
		write_string = gets.chomp
		
		read_string = File.open(fname, "r"){|somefile| somefile.read.chomp}
		if read_string != ""
			write_string = "#{read_string}#{write_string}"
		end
		File.open(fname, "w"){|somefile| somefile.print write_string}
	end

	if str == "w"
		print "Enter write data: "
		STDOUT.flush
		write_string = gets.chomp

		File.open(fname, "w"){|somefile| somefile.print write_string}
	end

	if str == "clear"
		print "are you sure you want to clear the device? (y/n): "
		STDOUT.flush
		confirm = gets.chomp
		if (confirm == "y")
			File.open(fname, "w"){|f| f.print "\0"}
			puts "Success: Cleared the device"
		else
			puts "Did NOT clear the device contents"
		end
	end
end
