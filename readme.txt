Network Management and Network Applications and Network Administration Project

Author: Lukas Chmelo
Login: xchmel33

Installation (linux only):
make all

Usage:
./mytftpclient
>-R/-W -d [path file to be transfered] -a [ipv4/v6 address] -c [mode] -t [timeout] -s [size]
>exit

Example usage:
./mytftpclient
>-R -d test.txt 
>-W -d menu.jpg -a virtual  
>exit
Notes:
-a virtual => 192.168.56.1,69
-a localhost => 127.0.0.1,69
-t => only rejects strings, not fully implemented
-s => only rejects strings, not fully implemented
-m => not implemented 
