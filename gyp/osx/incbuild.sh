#!/bin/bash    
bN=$(/usr/libexec/PlistBuddy -c "Print CFBundleVersion" "./osx/Info.plist")
bN=$(($bN + 1)) 
bN=$(printf "%d" $bN)
/usr/libexec/PlistBuddy -c "Set :CFBundleVersion $bN" "./osx/Info.plist"