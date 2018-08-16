#!/usr/bin/env ruby
`_script_counter.sh #{$0} #{ARGV.join(' ')}`

$stdout.sync = true
$debug=0
#--------------------------------------------------------------------------------
# get_csv_column.rb
#
# Pipe in CSV dataset on STDIN and it prints the data values from those
# requested fields.
#
#--------------------------------------------------------------------------------
$delim = ','
$outdelim = ','

def usage
	puts
	puts "#{$0} [flags] [CSV field]{,[CSV field]...}"
	puts
	puts "Returns values for requested CSV field(s). Pass dataset in STDIN."
	puts
	puts "Assumes: data stream has encoded fields, like:"
	puts "    # col1#{$delim}col2#{$delim}col3"
	puts "    0.2#{$delim}21.4#{$delim}This test"
	puts
	puts "Flags:"
	puts "\t-a\taverage the returned dataset values"
	puts "\t-D [delim]\tspecify the input delimiter (default='#{$delim}')"
	puts "\t-O [delim]\tspecify the output delimiter (default='#{$outdelim}')"
    puts "\t\tand [delim] can be a list of delimiters to cycle thru"
	puts "\t-l\tjust print the available CSV fields in the dataset and exit"
	puts "\t-d\tturn on debugs"
    puts "\t-n\tDon't output CSV headers, just output data"
    puts "\t-z\tPipe input data first thru mk_proper_csv_files_in_dir.sh"
	puts
	puts "Example:"
	puts "     $ cat [csv file] | #{$0} [field1] [field2]"
	puts "     $ cat [csv file] | #{$0} -O ' :;' [field1] [field2] [field3]"
	puts
end
def dputs(msg)
        if $debug > 0
                puts "# DEBUG:#{$0}:#{msg}"
        end
end


if ARGV.empty? 
        usage
        exit(-1)
end
$fields = []
$list = 0
$do_avg = 0
$out_hdr = 1
$do_mkprop = 0
while !ARGV.empty?
    arg = ARGV.shift
    if (arg =~ /^-h$/)
        usage
        exit(0)
	elsif (arg =~ /^-z$/)
        $do_mkprop = 1
	elsif (arg =~ /^-n$/)
        $out_hdr = 0
	elsif (arg =~ /^-a$/)
		$do_avg = 1
	elsif (arg =~ /^-O$/)
		$outdelim = ARGV.shift
	elsif (arg =~ /^-D$/)
		$delim = ARGV.shift
	elsif (arg =~ /^-d$/)
		$debug = 1
	elsif (arg =~ /^-l$/)
		$list = 1
	elsif (arg =~ /^-/)
		puts "ERROR: Unknown arg:#{arg}"
		exit(-1)
	else
		$fields.push(arg)
	end
end
$outd=$outdelim.split('')
$outdn=0
# optimization so we don't compute each loop
$outdl = $outd.length
dputs "fields=#{$fields.inspect} outdelim=#{$outd}[#{$outdl}]"
if $list==0 && $fields.empty?
	puts "ERROR: No CSV fields were specified."
	exit(-1)
end

$ln = []            # current line
$hdr = []           # last-known header line
$hdr_cnt = nil      # how many fields in last-known header line
$field_index = []   # the indexes of the items in $fields
$hash_of_arrays = Hash.new { |hash,key| hash[key] = [] }
$last_line = ''

# If we are not listing header fields, print a comment field (mainly for plotit.pl) of those requested headers
if $list==0 && $out_hdr==1
	print "# "
	first=0
    $outdn=0
	$fields.each { |p|
        if first==1
		    print "#{$outd[$outdn]} "
            # optimization so we don't advance if just have 1 delim
            if $outdl > 1
                $outdn=($outdn+1) % $outd.length
            end
        end
		print "#{p}"
		first=1
	}
	print "\n"
end

# if using '-z' flag, data will come from file, otherwise it comes from stdin
$data_from = $stdin

# if requested, pipe input CSV thru mk_proper_csv_files_in_dir.sh
if $do_mkprop == 1
    File.open("/tmp/tmp.out", 'w') { |file|
        $stdin.each_line { |ln|
            file.write("#{ln}")
        }
        file.close()
    }
    # call out to make it a good CSV file, this cmd
    # overwrites file passed in
    `mk_proper_csv_files_in_dir.sh /tmp/tmp.out &> /dev/null`
    if $?.exitstatus != 0
        puts "ERROR: couldn't clean CSV with mk_proper_csv_files_in_dir.sh - is it in PATH ?"
        exit(-1)
    end
    # assign where data is coming from
    $data_from = File.open("/tmp/tmp.out", 'r')
end

$data_from.each_line { |ln|
	ln.chomp!
	#if ln =~ /^#\s*(.*)\s*$/ || ln =~ /^(timestamp\s.*)\s*$/ || ln =~ /^([a-zA-Z].*)\s*$/
	if ln =~ /^#\s*(.*)\s*$/ 
		# header field, update our local copy
		$hdr = $1.split($delim)
		cnt=0
		# strip spaces from header fields, there has to be a cleaner way...
		$hdr.each { |mm|
			if mm =~ /^\s*(.*)\s*$/
				$hdr[$hdr.index(mm)]=$1
			end
			cnt=cnt+1
		}
		dputs "HDR:#{$hdr.inspect}"
		if !$hdr_cnt.nil? && $hdr_cnt != $hdr.length
			puts "ERROR: header field has changed number of items!"
			exit(-1)
		end
		$hdr_cnt = $hdr.length

		$field_index = []
		$fields.each { |x|
			pp = $hdr.index("#{x}")
			if !pp.nil?
				#dputs "INDEX:#{pp}:field=#{x}"
				$field_index.push(pp)
			else
				#dputs "INDEX:nil"
				puts "ERROR: Could not find CSV field #{x}"
				exit(-1)
			end
		}
		dputs "FIELD_INDEX:#{$field_index.inspect}"
	elsif ln =~ /^[0-9a-zA-Z]/ || ln =~ /^\s*(\S*#{$delim}.*)\s*$/
		# skip data fields if we are just printing the dataset header fields
		next if $list==1

		# data field, print those requested fields
		if $hdr.nil? || $field_index.empty?
			puts "ERROR: No header field preceeds dataset, don't know what to do!"
			exit(-1);
		end
		$ln = ln.split($delim)
		$last_line = $ln
		dputs "DATA:#{$ln.inspect}" 
		first=0
        $outdn=0
		$field_index.map { |y|
			#dputs "FIELD_MAP:#{y}"
			# trim spaces from start and end
			if $ln[y] =~ /^\s*(.*)\s*$/
				$ln[y] = $1
			end
			if $do_avg == 0
                if first==1
				    print "#{$outd[$outdn]} "
                    # optimization so we don't advance if just have 1 delim
                    if $outdl > 1
                        $outdn=($outdn+1) % $outd.length
                    end
                end
				print "#{$ln[y]}"
				first=1
			else
				if $ln[y] =~ /^([+-]?((([0-9]+(\.)?)|([0-9]*\.[0-9]+))([eE][+-]?[0-9]+)?))$/
					# keep a list of those items we found, we can average them later
					$hash_of_arrays[y]
					$hash_of_arrays[y].push($ln[y])
					#dputs "AVG_PUSH:#{y}:#{$ln[y]}"
				else
					dputs "AVG_WARNING: It seems the value #{$ln[y]} is not Numeric... not adding to average list..."
				end
			end
		}
		print "\n" if $do_avg == 0
	end
} 
if $list==1
	$hdr.each { |y|
		puts "#{y}"
	}
end

if $do_avg==1
	# Header field is printed above...
	# Now go thru the individual datasets and avg.
	first=0
    $outdn=0
	$field_index.map { |y|
		avg_total = 0
		avg_cnt = 0
		$hash_of_arrays[y].each { |p|
			avg_total = avg_total + p.to_f
			avg_cnt = avg_cnt + 1
		}
		if avg_cnt == 0
			# we didn't find any, probably b/c the entire column is hex values (FPGA/EE field,etc) so
			# just pick the last value in that column and hope for the best...
			the_avg = $last_line[y]
		else
			the_avg = avg_total / avg_cnt
		end
        if first==1
		    print "#{$outd[$outdn]} "
            # optimization so we don't advance if just have 1 delim
            if $outdl > 1
                $outdn=($outdn+1) % $outd.length
            end
        end
		print "#{the_avg}"
		first=1
	}
	print "\n"
end

