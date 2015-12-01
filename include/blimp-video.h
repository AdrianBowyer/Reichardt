/*
 *  Blimp video device software header
 *
 *  Adrian Bowyer
 *  26 October 2003
 *
 *  This works with v4l; it'll need to be upgraded for v4l2.
 */

#ifndef BLIMP_VIDEO_H
#define BLIMP_VIDEO_H

#define fftw_real double

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev.h>
#include <Magick++.h>
#include <complex.h>
#include <fftw3.h>

using namespace std;
using namespace Magick;

// *************************************************************************************

// Deal with the camera

#define CHAN 1                 // The composite video channel
#define VID_DEV "/dev/video0"  // The video device

// Class to wrap up video access

class video
{
 private:

  void *m;        // The memory area that a frame gets mapped into
  int v;          // The file descriptor for the video device
  long s;         // The size of a frame in bytes
  video_mmap vm;  // The memory map video structure

  // Return the file descriptor for the video device

  int device() const { return v; }

 public:

  video(int x, int y);        // Only one constuctor - args are the size of the image

  ~video()        // Just close the device
  {
    if(munmap(m, s))
      perror ("~video(): munmap");

    if(close(v))
    {
	cerr << "~video(): error closing video device\n";
    }
  }

  // Grab one frame and return a pointer to it

  void *grab();

  // Return the pointer to the last frame (no grab is done)

  void *frame() { return m; }

  // Grab a frame and write it to a tga image file

  int write_tga(char* name);

  // Return the size of the frame in bytes

  long size() const { return s; }

  // Return the dimensions of the frame

  int x() const { return vm.width; }
  int y() const { return vm.height; }

  // Grab a frame and return it as an ImageMagick Image

  Image image() 
  {
    grab();
    return Image(x(), y(), "RGB", CharPixel, frame()); 
  }

  // Print the device characteristics to standard output

  void print();

};

// ************************************************************************************

// Class for a frame sequence

class frames
{
 private:

  fftw_real ***pixels;
  fftw_complex ***transformed;
  int x, y, t;
  fftw_real q; 

 public:

  // Constructor creates a block of tf images, each xf x yf

  frames(int xf, int yf, int tf);

  // Copy constructor does a deep copy

  frames(const frames& a);

  // Delete returns storage

  ~frames();

  // Put the image im at frame tf; if average is true pixels
  // are averaged; otherwise they are sampled

  void add_frame(Image im, int tf, int average);

  // Get frame tf as an image magnified by mag

  Image get_frame(int tf, int mag) const;

  // Rescale all the frames so pixel values lie in [0, 1]

  void rescale();

  // Average all frames into the first

  void average();

  // Take the FFT of each pixel in time
  // If invert is true, the images are reconstructed from an FFT

  void fft(int invert);

  // Low and high-pass filter (only makes sense for FFT images)
  // cut is the fraction of t where the cutoff hapens

  void low_pass(int cut);
  void high_pass(int cut);

  // The fly-eye Reichardt algorithm - horizontal and vertical

  void reichardt(int low, int high);

  // How many frames are there?

  int count() const { return t; }

};


// ************************************************************************************

// Image ring buffer

class ring
{

 private:
  int im_count;
  int next_im;
  Image *im;

 public:

  ring(int length);

  ~ring();

  void lock();

  void unlock();

  void next(Image im);

  void get_frames(frames &f);

};

#endif
