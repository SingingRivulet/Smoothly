VERSION_MAJOR = 0
VERSION_MINOR = 1
VERSION_RELEASE = 9

IRRBULLET =  src/irrBullet.o  src/irrBulletBoxShape.o src/irrBulletBvhTriangleMeshShape.o src/irrBulletCapsuleShape.o src/irrBulletCollisionCallBackInformation.o  src/irrBulletCollisionObject.o  src/irrBulletCollisionObjectAffector.o src/irrBulletCollisionObjectAffectorAttract.o src/irrBulletCollisionObjectAffectorDelete.o  src/irrBulletCollisionShape.o  src/irrBulletcommon.o  src/irrBulletConvexHullShape.o  src/irrBulletCylinderShape.o src/irrBulletGImpactMeshShape.o src/irrBulletKinematicCharacterController.o  src/irrBulletLiquidBody.o   src/irrBulletMotionState.o   src/irrBulletPhysicsDebug.o src/irrBulletRayCastVehicle.o src/irrBulletRigidBody.o src/irrBulletSoftBody.o src/irrBulletSphereShape.o src/irrBulletTriangleMeshShape.o src/irrBulletWorld.o
EXTRAOBJ =
LINKOBJ = $(IRRBULLET)

IrrlichtHome := $(HOME)/irrlicht
BulletHome := $(HOME)/bullet3
CXXINCS = -I $(IrrlichtHome)/include -I $(BulletHome)/src -Iinclude
CPPFLAGS += $(CXXINCS)
CXXFLAGS += -std=c++11 -Wall -pipe -fno-exceptions -fno-rtti -fstrict-aliasing
ifdef DEBUG
CXXFLAGS += -g -D_DEBUG
else
CXXFLAGS += -fexpensive-optimizations -O3
endif
ifdef PROFILE
CXXFLAGS += -pg
endif

# multilib handling
ifeq ($(HOSTTYPE), x86_64)
LIBSELECT=64
endif

# Linux specific options
staticlib sharedlib install:
STATIC_LIB = libirrBullet.a
LIB_PATH = lib/
INSTALL_DIR = /usr/local/lib
sharedlib install: SHARED_LIB = libirrBullet.so
sharedlib: LDFLAGS += -L/usr/X11R6/lib$(LIBSELECT) -lGL -lXxf86vm
sharedlib: LDFLAGS += -L$(IrrlichtHome)/lib/Linux -lIrrlicht
staticlib sharedlib: CXXINCS += -I/usr/X11R6/include

ifeq ($(MACHINE), x86_64)
sharedlib: CPPFLAGS += -fPIC
endif

VERSION = $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_RELEASE)
SHARED_FULLNAME = $(SHARED_LIB).$(VERSION)
SONAME = $(SHARED_LIB).$(VERSION_MAJOR).$(VERSION_MINOR)

all: sharedlib

sharedlib: $(LINKOBJ)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -shared -Wl,-soname,$(SONAME) -o $(SHARED_FULLNAME) $^ $(LDFLAGS)
	mkdir -p $(LIB_PATH)
	mv $(SHARED_FULLNAME) $(LIB_PATH)

$(STATIC_LIB): $(LINKOBJ)
	$(AR) rs $@ $^

install:
	cp $(LIB_PATH)/$(SHARED_FULLNAME) $(INSTALL_DIR)
	cd $(INSTALL_DIR) && ln -s -f $(SHARED_FULLNAME) $(SONAME)
	cd $(INSTALL_DIR) && ln -s -f $(SONAME) $(SHARED_LIB)
	ldconfig -n $(INSTALL_DIR)
	
staticlib staticlib_osx: $(STATIC_LIB)
	mkdir -p $(LIB_PATH)
	mv $^ $(LIB_PATH)

TAGS:
	ctags *.cpp ../../include/*.h *.h

# Create dependency files for automatic recompilation
%.d:%.cpp
	$(CXX) $(CPPFLAGS) -MM -MF $@ $<

# Create dependency files for automatic recompilation
%.d:%.c
	$(CC) $(CPPFLAGS) -MM -MF $@ $<

ifneq ($(MAKECMDGOALS),clean)
-include $(LINKOBJ:.o=.d)
endif

help:
	@echo "Available targets for irrBullet"
	@echo " sharedlib: Build shared library irrBullet.so for Linux"
	@echo " staticlib: Build static library irrBullet.a for Linux"
	@echo " install: Copy shared library to /usr/local/lib"
	@echo ""
	@echo " sharedlib_win32: Build shared library irrBullet.dll for Windows"
	@echo " staticlib_win32: Build static library irrBullet.a for Windows"
	@echo ""
	@echo " clean: Clean up directory"

# Cleans all temporary files and compilation results.
clean:
	$(RM) $(LINKOBJ) $(SHARED_FULLNAME) $(STATIC_LIB) $(LINKOBJ:.o=.d)

.PHONY: all sharedlib staticlib help install clean

