# how to compile

./build.sh

# how to launch

	export IPC_RUNDIR=/tmp
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../
	./pongd

	# in another terminal
	export IPC_RUNDIR=/tmp
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../
	./pong
