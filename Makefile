all: cli controller

cli:
	make -C ./src

controller:
	make -C ./Application/remote_acquire_utility/src

clean:
	make -C ./Application/remote_acquire_utility/src clean
	make -C ./src clean
