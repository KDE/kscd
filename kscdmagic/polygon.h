#include <string.h>

template<class Pixel,int extra=0>
struct Bitmap {
  Pixel *data;
  int width, height;

  Bitmap() : data(0) { };
  ~Bitmap() { delete[] data; };

  void size(int w,int h) {
    delete[] data;
    width = w;
    height = h;
    data = new Pixel[w*h+extra];
    clear();
  }
  
  void clear() {
    memset(data,0,sizeof(Pixel)*(width*height+extra));
  }
};

template<class Pixel, Pixel combine(Pixel a,Pixel b), int superSampleShift>
struct PolygonEngine : public Bitmap<Pixel,1> {
#define super (1<<superSampleShift)
  void apply(Pixel *dest) {
    /*    Pixel sum=0; */
    int count = width*height;
    Pixel *src = data;
    /*
     * I don't really want this.
     *
    while(count--) {
      sum += *(src++);
      if (sum)
        *dest = combine(sum,*dest);
      dest++;
    }
    */
    while(count--)
      {
	*dest = combine(*src, *dest);
	src++;
	dest++;
      }
  }

  void add(Pixel color,int x,int y) {
    if (y < 0) return;
    if (y >= height) return;
    if (x < 0) x = 0;
    if (x > width) x = width;
    data[x+y*width] += color;
  }

  /* Color is char[layers] */

  //  zwoosh, yknow, it goes... zwoosh an all these bars and lines and
  //  crap intersect.
  Pixel colorTable[2][super+1];
  void pen(Pixel color) {
    for(int i=0;i<super+1;i++) {
      colorTable[0][i] = color*i;
      colorTable[1][i] = -(color*i);
    }
  }
  
  void line(int x1,int y1,int x2,int y2) {
    Pixel *colors;
    if (y2 < y1) {
      int temp;
      temp = x2; x2 = x1; x1 = temp;
      temp = y2; y2 = y1; y1 = temp;
      colors = colorTable[1];
    } else {
      if (y1 == y2) return;
    
      colors= colorTable[0];
    }

    int slope = (x1-x2 << 16)/(y1-y2);
    int x = x1<<16, y = y1;
    while(y < y2) {
      add(colors[super-((x>>16)&(super-1))],
	  x>>(16+superSampleShift),y>>superSampleShift); 
      add(colors[(x>>16)&(super-1)],
	  1+(x>>(16+superSampleShift)),y>>superSampleShift); 
      x += slope;
      y++;
    }
  }

  void icon(double icon[][4],Pixel color,double x,double y,
            double scaleX, double scaleY) {
    pen(color);
    x *= super;
    y *= super;
    scaleX *= super;
    scaleY *= super;
    for(int i=0;icon[i][1] != icon[i][3];i++)
      line(int(icon[i][0]*scaleX+x),int(icon[i][1]*scaleY+y),
	   int(icon[i][2]*scaleX+x),int(icon[i][3]*scaleY+y));
  }
#undef super
};
