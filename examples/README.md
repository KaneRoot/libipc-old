# how to compile

make

# how to launch

If libipc is already installed and you have the rights to create unix sockets in the default directory (/run/ipc), just run the code:

	./pongd


In case you want to test the library and example programs:

	# This is a directory anybody can write in
	export IPC_RUNDIR=/tmp

	# to test the library without installing it
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../

	# finally, run the program, here a pong daemon
	./pongd

	# same thing in another terminal, to test the client
	export IPC_RUNDIR=/tmp
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../
	./pong
