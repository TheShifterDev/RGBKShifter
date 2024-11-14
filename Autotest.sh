#!/bin/bash
# run all '.test' files in a row and exit if any tests result in non 0 exits
TESTFAIL=0
for CURRENTTEST in $(find ./Tests/*.test); do
	./Built/rgbkshifter.bin $(cat $CURRENTTEST)
	# NOTE: $? is the result of the previously ran command 
	# NOTE: -ne is !=
	RESULT=$?
	if [ $RESULT -ne 0 ]; then
		echo $CURRENTTEST" returned result "$RESULT
		TESTFAIL=$TESTFAIL+1
		break;
	fi
done

if [ $TESTFAIL -eq 0 ];then
	echo "no tests failed"
else
	echo "a test failed"
fi