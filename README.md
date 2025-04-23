# AppManager
An application manager software for Linux and QNX

The Application takes a configuration file that lists the applications to be started in sequence.
Each application binary is spawned out as a separate process with its own process id, with its configured user and group.

To build, invoke "make all"

#usage 
For starting apps ==> ./AppManager.bin -c conf.txt
For help menu ======> ./AppManager.bin -h

#Help menu:
./AppManager.bin -h
******************************************************************************
*********************************** APP Manager ******************************
******************************************************************************
	 -c <configuration file> : configuartion file containing application
	 -h                      : help 
******************************************************************************


#Conf file format:

$

# app.bin -d"/usr/local/bin" -u1000 -g1000 -a"-help"
# app2.bin -d"/usr/local/bin" -u2000 -g2000 -a"-c test.info -p 4000"

$

As, seen above, the file should have "$" sign at the start and the end.
Each, application entry should begin with a '#' sign.
The first entry should be the application binary name with extension. The rest of the arguments does not mandate a sequence.

