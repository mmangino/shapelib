class AssertFail < Exception
end

def assert(a, b)
    caller = ""
    begin
	raise "no problem"
    rescue RuntimeError => e
	caller = e.backtrace[1]
    end
    if (b === a) then
	val = b.inspect
	val = "#{val}..." if val.size > 40
	print "ok #{caller}: #{val}\n" if $VERBOSE
    else
	val = a.inspect
	val = "#{val}..." if val.size > 200
	raise AssertFail, val
    end
end

def file_delete(*args)
    for file in args
	next unless File.exist?(file)
	File.delete(file)
    end
end

def fold_line(line)
    buf = ""
    while line
	head = line[0..63]
	buf << (head + "\n")
	line = line[64..-1]
    end
    buf
end

def assert_file(filename, pat)
    f = File.read(filename)
    r = [pat.gsub(/\s+/, "")].pack('H*')

    if (r === f) then
	print "ok <#{filename}>\n" if $VERBOSE
    else
	for i in 0...(f.size)
	    next if (f[i] == r[i])
	    printf "ofs=%08x file=%02x expected=%02x\n", i, f[i], r[i]
	    break
	end
	print "#{filename} is longer than expected\n" if f.size > r.size
	h = f.unpack('H*').first
	File.open("#{filename}.hex", "w") { |fp| fp.write(fold_line(h)) }
	print "hex dump of <#{filename}> is saved to <#{filename}.hex>.\n"
	raise AssertFail
    end
end

def testblock(title)
    begin
	yield
    rescue Exception => e
	print "#{title} failed: #{e.message} (#{e.class.to_s})\n"
	print "| ", e.backtrace[1..-1].join("\n| "), "\n"
	return 1
    else
	print "#{title} ok\n"
	return 0
    end
end

def tainttest(msg)
    begin
	yield
    rescue SecurityError => e
	assert(e.message, msg)
    else
	raise SecurityError, "<#{msg}> doesn't come" if $SAFE > 0
    end
end
