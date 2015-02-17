# My_char
Linked list char driver 

just go through my_char.c and load files
Use command line "sh load" to load the driver

use echo "data">/dev/my_char to write data to driver
dmesg
or
cat filename>/dev/my_char to wrtite an entire file
dmesg.

dmesg -c to clear printks
to read use cat /dev/my_char
dmesg
