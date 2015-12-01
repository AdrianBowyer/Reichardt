/*
 *  Blimp video device software
 *
 *  Adrian Bowyer
 *  26 October 2003
 *
 *  This works with v4l; it'll need to be upgraded for v4l2.
 */

#include "blimp-video.h"

int main()
{

  int xv, yv, tv, lp, hp;

  cout << "Video resolution (x, y): ";
  cin >> xv >> yv;

  video vid(xv, yv);

  vid.print();

  cout << "Filter resolution (x, y, t): ";
  cin >> xv >> yv >> tv;

  cout << "Low and high pass (< " << 1+tv/2 << "): ";
  cin >> lp >> hp;  

  frames f(xv, yv, tv);

  for(int i = 0; i < tv; i++)
    f.add_frame(vid.image(), i, 1);

  f.reichardt(lp, hp);

  cout << "Magnification: ";
  cin >> xv;

  for(int i = 0; i < tv; i++)
    f.get_frame(i, xv).display();

  return 0;
}
