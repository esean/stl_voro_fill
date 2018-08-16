#!/bin/bash
_script_counter.sh $0 $@
msg="$@"
#xmessage "$msg"; df=$?
#exit $df


popup() {
		/usr/bin/osascript <<-EOF

			tell application "System Events"
				activate
				display dialog "$msg"
			end tell

		EOF
}

s=`(popup 2>&1)`
if echo "$s" | grep -q 'returned:OK'; then
	#echo "OK"
	exit 0
else
	#echo "Cancel"
	exit 1
fi
