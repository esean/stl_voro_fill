#!/usr/bin/env python
import sys,os
import argparse
from ConfigParser import SafeConfigParser

#------------------
def die(str):
    print "\nFATAL ERROR:%s !\n" % str
    sys.exit(1)

parser = argparse.ArgumentParser(description='Read and set configuration key/values contained in provided .ini file')
parser.add_argument('-f','--ini_file', help='Input .ini file', required=True)
#parser.add_argument('-t','--section', help='Select section of .ini file', required=False)
parser.add_argument('-l','--list', help='List keys', required=False, action='store_true')
parser.add_argument('-r','--read_key', help='Read key', required=False)
#parser.add_argument('-s','--set_key', help='Set key', required=False)
args = vars(parser.parse_args())
if not os.path.isfile(args['ini_file']):
    die("file %s does not exist" % args['ini_file'])

parser = SafeConfigParser()
# don't lowercase: http://stackoverflow.com/questions/19359556/configparser-reads-capital-keys-and-make-them-lower-case
parser.optionxform = str
parser.read(args['ini_file'])

if args['list']:
    for section_name in parser.sections():
        print "[%s]" % section_name
#        print '  Options:', parser.options(section_name)
        for name, value in parser.items(section_name):
            print '  %s = %s' % (name, value)
    sys.exit(0)

#if not args['section']:
#    die("To read or set a key, you need to specify the section with '-t' param")

if args['read_key']:
    for section_name in parser.sections():
#        print "[SECT:%s]" % section_name
        for name, value in parser.items(section_name):
#            print "[SECT:%s] %s = %s" % (section_name,name,value)
            if name == args['read_key']:
                print value
		sys.exit(0)

die("Could not find read_key %s in ini file %s" % (args['read_key'],args['ini_file']))

