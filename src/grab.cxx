//
//      grab.c
//      19-9-2001, a.martone@retepnet.it
//      grab a TV frame (via video4linux) in TGA-uncompressed-RGB24 format
//
//      compile:  cc -s -O1 grab.c -o grab
//      usage:    grab >frame.tga
//
//      info:     thanks to Gerd Knorr's xawtv sources (www.bytesex.org/xawtv)
//

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev.h>

#undef DEBUGGING

main()
{
  int fd, siz;
  short targaheader[9]={ 0,2,0,0,0,0,444,555,8216 }; /* 444 and 555 are fake */
  void *map;
  struct video_mmap v;

  v.format=VIDEO_PALETTE_RGB24;    // ... GREY, RGB555, RGB565, RGB24, RGB32
  v.frame=0;                       // 0 or 1 (buffer 0 or buffer 1)
  v.width=768;                     // from 32 to 924 (sometimes limited to 768)
  v.height=576;                    // from 32 to 576
  siz=v.width*v.height*3;          // rgb24 is: 3; rgb32:4; rgb565:2; rgb555:2

  targaheader[6]=v.width;
  targaheader[7]=v.height;         // update targa header for frame

  fd=open("/dev/video0",O_RDONLY);  // open video device
  if(fd<0) perror("/dev/video0");

  map=mmap(0, siz, PROT_READ, MAP_SHARED, fd, 0);   // select memory-map
  if(map==MAP_FAILED) perror("mmap");

#ifdef DEBUGGING
  {
    int xmin, xmax, ymin, ymax;
    unsigned long freq;
    struct video_capability cap;
    struct video_buffer b;
    struct video_picture p;

    if(ioctl(fd, VIDIOCGFREQ, &freq)<0) perror("VIDIOCGFREQ");
    fprintf(stderr,"freq: 0x%x\n", freq);

    if(ioctl(fd, VIDIOCGCAP, &cap)<0) perror("VIDIOCGCAP");
    xmin=cap.minwidth, xmax=cap.maxwidth;
    ymin=cap.minheight,ymax=cap.maxheight;
    fprintf(stderr, "capture sizes: (%dx%d)-->(%dx%d)\n",xmin,ymin,xmax,ymax);

    if(ioctl(fd, VIDIOCGFBUF, &b)<0) perror("VIDIOCGFBUF");
    fprintf(stderr,"base:%p, size:(%dx%d), depth:%d, bpl=%d\n",
      b.base, b.width, b.height, b.depth, b.bytesperline);

    if(ioctl(fd, VIDIOCGPICT, &p)<0) perror("VIDIOCGPICT");
    fprintf(stderr, "brightness:%d, hue:%d, color=%d, contrast=%d\n"
           "whiteness:%d, depth:%d, palette[%d]\n",
      p.brightness, p.hue, p.colour, p.contrast,
        p.whiteness, p.depth, p.palette);
  }
#endif

  if(ioctl(fd, VIDIOCMCAPTURE, &v)<0)        // start capturing a frame
    perror("VIDIOCMCAPTURE");

  if(ioctl(fd, VIDIOCSYNC, &v.frame)<0)      // wait for end of frame
    perror("VIDIOCSYNC");

  fwrite(targaheader, sizeof(targaheader),   // save header & data to stdout
    1, stdout);
  fwrite(map, siz, 1, stdout);
}
