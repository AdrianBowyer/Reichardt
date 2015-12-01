/*
 *  Blimp video device software
 *
 *  Adrian Bowyer
 *  26 October 2003
 *
 *  This works with v4l; it'll need to be upgraded for v4l2.
 */

#include "blimp-video.h"

int debug = 0;

// ************************************************************************************

// The video class - the constructor

video::video(int xs, int ys)
{
  // Open the device VID_DEV (see blimp-video.h; usually /dev/video0)

  v = open(VID_DEV, O_RDONLY); //O_RDWR);
  if(v < 0)
  {
    cerr << "video::video() - Can't open " << VID_DEV << ", returned value: " << 
      v << endl;
    return;
  }

  // Get the characteristics of channel CHAN

  video_channel vchn;
  vchn.channel = CHAN;

  if(ioctl(v, VIDIOCGCHAN , &vchn) < 0)
  {
    perror ("video::video() - VIDIOCGCHAN");
    //return;
  }

  // Set channel CHAN as the one to use

  if(ioctl(v, VIDIOCSCHAN , &vchn) < 0)
  {
    perror ("video::video() - VIDIOCSCHAN");
    //return;
  }

  // Set up the video format

  vm.format = VIDEO_PALETTE_RGB24;    // ... GREY, RGB555, RGB565, RGB24, RGB32
  vm.frame = 0;                       // 0 or 1 (buffer 0 or buffer 1)
  vm.width = xs;                      // from 32 to 924 (sometimes limited to 768)
  vm.height = ys;                     // from 32 to 576

  // Work out the frame size

  s = vm.width*vm.height*3;        // rgb24 is: 3; rgb32:4; rgb565:2; rgb555:2

  // Create the memory mapping

  m = mmap(0, s, PROT_READ, MAP_SHARED, v, 0);   // select memory-map
  if(m == MAP_FAILED) 
  {
    perror("video::video() - mmap");
    //return;
  }
}

// Grab a single frame and return the pointer to it

void *video::grab()
{
  // Start frame grab

  if(ioctl(v, VIDIOCMCAPTURE, &vm) < 0)
  {
    perror("video::grab() - VIDIOCMCAPTURE");
    //return 0;
  }

  // Wait for end of frame

  if(ioctl(v, VIDIOCSYNC, &vm.frame) < 0)
  {
    perror("video::grab() - VIDIOCSYNC");
    //return 0;
  }

  return m;
}

// Grab a frame and write it to a .tga graphics file called name

int video::write_tga(char* name)
{
  // Open the file

  FILE *ofs = fopen(name, "w");
  if(!ofs)
  {
    cerr << "Can't open " << name << " for output.\n";
    return 1;
  }

  // The initial bytes of a .tga file

  short targaheader[9]={ 0, 2, 0, 0, 0, 0, x(), y(), 8216 };

  // Get the pic

  grab();

  // Write the header...

  fwrite(targaheader, sizeof(targaheader),   // save header & data to stdout
    1, ofs);

  // ...and the frame itself

  fwrite(m, s, 1, ofs);

  // Close the file

  fclose(ofs);

  return 0;

}

// Print the device characteristics to standard output

void video::print()
{
  video_capability vc;
  if(ioctl(v, VIDIOCGCAP , &vc) < 0)
  {
    perror ("VIDIOCGCAP");
    //return;
  }
  cout << "\nVideo device characteristics:\n";
  cout << " name: " << vc.name << '\n';
  cout << " type: " << vc.type << '\n';
  if(vc.type & VID_TYPE_CAPTURE)
    cout << " Can capture to memory.\n";
  if(vc.type & VID_TYPE_TUNER)
    cout << " Has a tuner of some form.\n";
  if(vc.type & VID_TYPE_TELETEXT)
    cout << " Has teletext capability.\n";
  if(vc.type & VID_TYPE_OVERLAY)
    cout << " Can overlay its image onto the frame buffer.\n";
  if(vc.type & VID_TYPE_CHROMAKEY)
    cout << " Overlay is Chromakeyed.\n";
  if(vc.type & VID_TYPE_CLIPPING)
    cout << " Overlay clipping is supported.\n";
  if(vc.type & VID_TYPE_FRAMERAM)
    cout << " Overlay overwrites frame buffer memory.\n";
  if(vc.type & VID_TYPE_SCALES)
    cout << " The hardware supports image scaling.\n";
  if(vc.type & VID_TYPE_MONOCHROME)
    cout << " Image capture is grey scale only.\n";
  if(vc.type & VID_TYPE_SUBCAPTURE)
    cout << " Capture can be of only part of the image.\n";
  cout << " channels: " << vc.channels << '\n';
  cout << " audios: " << vc.audios << '\n';
  cout << " maxwidth: " << vc.maxwidth << '\n';
  cout << " maxheight: " << vc.maxheight << '\n';
  cout << " minwidth: " << vc.minwidth << '\n';
  cout << " minheight: " << vc.minheight << '\n';

  video_picture vp;
  if(ioctl(v, VIDIOCGPICT, &vp) < 0)
  {
    perror ("VIDIOCGPICT");
    //return;
  }
  cout << "\nVideo picture characteristics:\n";
  cout << " brightness: " << vp.brightness << '\n';
  cout << " hue: " << vp.hue << '\n';
  cout << " colour: " << vp.colour << '\n';
  cout << " contrast: " << vp.contrast << '\n';
  cout << " whiteness: " << vp.whiteness << '\n';
  cout << " depth: " << vp.depth << '\n';
  cout << " palette: " << vp.palette << '\n';

  video_window vw;
  if(ioctl(v, VIDIOCGWIN, &vw) < 0)
  {
    perror ("VIDIOCGWIN");
    //return;
  }
  cout << "\nVideo window characteristics:\n";
  cout << " x: " << vw.x << '\n';
  cout << " y: " << vw.y << '\n';
  cout << " width: " << vw.width << '\n';
  cout << " height: " << vw.height << '\n';
  cout << " chromakey: " << vw.chromakey << '\n';
  cout << " flags: " << vw.flags << '\n';

  video_mbuf vb;
  if(ioctl(v, VIDIOCGMBUF, &vb) < 0)
  {
    perror ("VIDIOCGMBUF");
    //return;
  }
  cout << "\nVideo buffer characteristics:\n";
  cout << " size: " << vb.size << '\n';
  cout << " frames: " << vb.frames << '\n';
  cout << " offsets: " << vb.offsets << endl;

  cout << "\nVideo memory-map characteristics:\n";
  cout << " format: " << vm.format << '\n';
  cout << " frame: " << vm.frame << '\n';
  cout << " width: " << vm.width << '\n';
  cout << " height: " << vm.height << '\n';

  cout << "----------------------------\n" << endl;
}

// *************************************************************************************

// The frames class - a collection of images as frames in time, and in the frequency
// domain (transformed).

frames::frames(int xf, int yf, int tf)
{
    int i, j;
    pixels = new fftw_real**[xf];
    transformed = new fftw_complex**[xf];
    for(i = 0; i < xf; i++)
    {
      pixels[i] = new fftw_real*[yf];
      transformed[i] = new fftw_complex*[yf];
      for(j = 0; j < yf; j++)
      {
	pixels[i][j] = new fftw_real[tf];
	transformed[i] = new fftw_complex*[tf/2 + 1];
      }
    }
    x = xf;
    y = yf;
    t = tf;
    q = 1.0/(fftw_real)MaxRGB;
}

// Copy constructor does a deep copy

frames::frames(const frames& a)
{
    int i, j, k;

    x = a.x;
    y = a.y;
    t = a.t;
    q = a.q;

    pixels = new fftw_real**[x];
    transformed = new fftw_complex**[x];
    for(i = 0; i < x; i++)
    {
      pixels[i] = new fftw_real*[y];
      transformed[i] = new fftw_complex*[y];
      for(j = 0; j < y; j++)
      {
	pixels[i][j] = new fftw_real[t];
        transformed[i] = new fftw_complex*[t/2 + 1];
	for(k = 0; k < t; k++)
        {
	  pixels[i][j][k] = a.pixels[i][j][k];
	  transformed[i][j][k] = a.transformed[i][j][k];
	}
      }

    }
}

// Delete returns storage

frames::~frames()
{
    int i, j;

    for(i = 0; i < x; i++)
    {
      for(j = 0; j < y; j++)
      {
	delete [] pixels[i][j];
	delete [] transformed[i][j];
      }
      delete [] pixels[i];
      delete [] transformed[i];
    }
    delete [] pixels;
    delete [] transformed;
}

// Put the image im at frame tf; if average is true pixels
// are averaged; otherwise they are sampled

void frames::add_frame(Image im, int tf, int average)
{
    int xstep = im.columns()/x;
    if(xstep < 1)
    {
      cerr << "frames::add_frame() - image x too small\n";
      xstep = 1;
    }
    int ystep = im.rows()/y;
    if(ystep < 1)
    {
      cerr << "frames::add_frame() - image y too small\n";
      ystep = 1;
    }

    im.modulate(100, 0, 100); // Go grey

    fftw_real a;
    for(int i = 0; i < x; i++)
      for(int j = 0; j < y; j++)
      {
	if(average)
	{
	  a = 0;
	  for(int k = 0; k < xstep; k++)
	    for(int l = 0; l < ystep; l++)
	      a = a + (fftw_real)im.pixelColor(i*xstep + k, j*ystep + l).redQuantum()*q;
	  pixels[i][j][tf] = a/(xstep*ystep);
	} else
	{
	  pixels[i][j][tf] = (fftw_real)im.pixelColor(i*xstep, j*ystep).redQuantum()*q;
	}
      }
}

// Get frame tf as an image

Image frames::get_frame(int tf, int mag) const
{
    fftw_real q1 = 1.0/q;
    Quantum qq;
    Image result = Image(Geometry(x*mag, y*mag), Color("Black"));
    for(int i = 0; i < x; i++)
      for(int j = 0; j < y; j++)
      {
	qq = (Quantum)(pixels[i][j][tf]*q1);
	for(int k = 0; k < mag; k++)
	  for(int l = 0; l < mag; l++)
	    result.pixelColor(i*mag + k, j*mag + l, Color(qq, qq, qq));
      }

    return result;
}

// Rescale all the frames so pixel values lie in [0, 1]

void frames::rescale()
{
  int i, j, k;
  fftw_real mx, mn, s;

  mx = pixels[0][0][0];
  mn = mx;
  for(i = 0; i < x; i++)
    for(j = 0; j < y; j++)
      for(k = 0; k < t; k++)
      {
	if(pixels[i][j][k] > mx) mx = pixels[i][j][k];
	if(pixels[i][j][k] < mn) mn = pixels[i][j][k];
      }

  s = mx - mn;
  if(s <= 0) s = 1.0;
  s = 1.0/s;
  for(i = 0; i < x; i++)
    for(j = 0; j < y; j++)
      for(k = 0; k < t; k++)
	pixels[i][j][k] = (pixels[i][j][k] - mn)*s;
}

// Average all frames into the first

void frames::average()
{
  int i, j, k;
  fftw_real s;

  for(i = 0; i < x; i++)
    for(j = 0; j < y; j++)
    {
      s = 0;
      for(k = 0; k < t; k++)
	s = s + pixels[i][j][k];
      pixels[i][j][0] = s/(fftw_real)t;
    }
}
  
// Take the FFT of each pixel in time into transformed
// If invert is true, the images are reconstructed from transformed back into pixels

void frames::fft(int invert)
{
  int i, j, k;

  fftw_plan f_plan;

  for(i = 0; i < x; i++)
    for(j = 0; j < y; j++)
    {
	if(invert)
    		f_plan = fftw_plan_dft_c2r_1d(t, transformed[i][j], pixels[i][j], FFTW_BACKWARD);
  	else
    		f_plan = fftw_plan_dft_r2c_1d(t, pixels[i][j], transformed[i][j], FFTW_FORWARD);
      	fftw_execute(f_plan);
  	fftw_destroy_plan(f_plan);
    }

  if(invert) rescale();

}

// Low and high-pass filter (only makes sense for FFT images)
// cut is the fraction of t where the cutoff hapens

void frames::high_pass(int cut)
{
  int i, j, k;

  for(i = 0; i < x; i++)
    for(j = 0; j < y; j++)
      for(k = 0; k < cut; k++)
	transformed[i][j][k] = 0.0 + I*0.0;
}


void frames::low_pass(int cut)
{
  int i, j, k;

  for(i = 0; i < x; i++)
    for(j = 0; j < y; j++)
      for(k = cut; k < t; k++)
	transformed[i][j][k] = 0.0 + I*0.0;;
}

// The fly-eye Reichardt filter; l - low-pass, h - high-pass
// TODO add Y filter as well as x, plus maybe diagonals?

void frames::reichardt(int l, int h)
{
  int i, j, k;
  fftw_real p, q;

  fft(0);
  frames b = *this;
  high_pass(h);
  b.low_pass(l);
  fft(1);
  b.fft(1);

  for(i = 0; i < x-1; i++)
    for(j = 0; j < y; j++)
      for(k = 0; k < t; k++)
      {
	p = pixels[i+1][j][k]*b.pixels[i][j][k];
	q = b.pixels[i+1][j][k]*pixels[i][j][k];
	pixels[i][j][k] = p - q;
      }

  for(j = 0; j < y; j++)
    for(k = 0; k < t; k++)
      pixels[x-1][j][k] = pixels[x-2][j][k];

  rescale();

}

// *************************************************************************************

// Image ring buffer

ring::ring(int length)
{
  lock();
  im = new Image[length];
  im_count = length;
  next_im = 0;
  unlock();
}

ring::~ring()
{
  lock();
  delete [] im;
}

void ring::lock()
{

}

void ring::unlock()
{

}

void ring::next(Image img)
{
  lock();
  im[next_im] = img;
  next_im++;
  if(next_im >= im_count) next_im = 0;
  unlock();
}

void ring::get_frames(frames &f)
{
  lock();
  int fm = next_im - f.count();
  if(fm < 0) fm = im_count + fm;
  for(int i = 0; i < f.count(); i++)
  {
    f.add_frame(im[fm], i, 1);
    fm++;
    if(fm >= im_count) fm = 0;
  }
  unlock();
}
