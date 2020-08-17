all:RakNet/build/Lib/LibStatic/libRakNetLibStatic.a \
    irrlicht/lib/Linux/libIrrlicht.a
	
RakNet/build/Lib/LibStatic/libRakNetLibStatic.a:RakNet RakNet/build
	cd RakNet/build && make
RakNet/build:RakNet
	cd RakNet && mkdir build
	cd RakNet/build && make
irrlicht/lib/Linux/libIrrlicht.a:irrlicht
	cd irrlicht/source/Irrlicht && make
