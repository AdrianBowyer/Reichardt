 
FFT = .
 
# The name of the C++ compiler
CC = g++
 
# Flags to send the compiler (change the last two with care...)
CFLAGS = -g
 
# Object, Runnable and Include directories
 
ODIR = $(FFT)/obj
IDIR = $(FFT)/include
RDIR = $(FFT)/bin
SDIR = $(FFT)/src
 
 
 
FL = -g -I$(IDIR) -Wno-deprecated -O2 -fopenmp -I/usr/include/freetype2 -D_REENTRANT -D_FILE_OFFSET_BITS=64 -I/usr/local/include -I/usr/include/X11 -I/usr/include/ImageMagick
GB = -L/usr/lib -L/usr/local/lib -L/usr/lib/X11 -lfftw3 -lMagick++ -lMagickCore -ltiff -ljpeg -lpng -lXext -lXt -lSM -lICE -lX11 -lbz2 -lz -lpthread -lm 
 

video-test:  $(ODIR)/video-test.o $(ODIR)/blimp-video.o
			 $(CC) -o $(RDIR)/video-test $(ODIR)/video-test.o $(ODIR)/blimp-video.o $(GB)

fft-test:  $(ODIR)/fft-test.o
			 $(CC) -o $(RDIR)/fft-test $(ODIR)/fft-test.o $(GB)

clean:
	rm -f bin/* obj/*
 
# Make the objects
 
$(ODIR)/fft-test.o:  $(SDIR)/fft-test.cpp
			 $(CC) -c $(FL) -o $(ODIR)/fft-test.o $(SDIR)/fft-test.cpp

$(ODIR)/video-test.o:  $(SDIR)/video-test.cpp $(IDIR)/blimp-video.h
			 $(CC) -c $(FL) -o $(ODIR)/video-test.o $(SDIR)/video-test.cpp

$(ODIR)/blimp-video.o:  $(SDIR)/blimp-video.cpp $(IDIR)/blimp-video.h
			 $(CC) -c $(FL) -o $(ODIR)/blimp-video.o $(SDIR)/blimp-video.cpp
