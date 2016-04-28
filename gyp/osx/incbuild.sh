#!/bin/bash
bN=$(/usr/libexec/PlistBuddy -c "Print CFBundleVersion" "$1/Info.plist")
bN=$(($bN + 1))
bN=$(printf "%d" $bN)
/usr/libexec/PlistBuddy -c "Set :CFBundleVersion $bN" "$1/Info.plist"
