#include <iostream>
#include <X11/Xlib.h>
#include <Magick++.h>
#include <cstring>
#include <limits>
static std::string renamestr = "bak_";
static int minargs = 2;
static int aspectadjusttries = 5; // hack

bool nearlyEqual(float a, float b, float epsilon) {
		const float absA = abs(a);
		const float absB = abs(b);
		const float diff = abs(a - b);

		if (a == b) { // shortcut, handles infinities
			return true;
		} else if (a == 0 || b == 0 || diff < std::numeric_limits<float>::min()) {
			// a or b is zero or both are extremely close to it
			// relative error is less meaningful here
			return diff < (epsilon * std::numeric_limits<float>::min());
		} else { // use relative error
			return diff / std::min((absA + absB), std::numeric_limits<float>::max()) < epsilon;
		}
	}

void getAspectRatio(int width, int height, int &w, int &h)
{
    // get biggest divisor
    int divisor = 1;
    for (int i = 1; i < width/2; i++)
        divisor = (width % i ==0 && height % i == 0)? i:divisor;
    // get dividends
    w = width/divisor;
    h = height/divisor;

    return;
}

void adjustImageResolution(int screenw, int screenh, int &w, int &h)
{
    /// calc height using image width and screen aspect ratio
    int imagew = w/screenw;
    int supimageh = imagew*screenh;
    if(h > supimageh)
    {
        /// height bigger than aspect ratio
        // get reduced height
        h = supimageh;
    }
    else if (h < supimageh)
    {
        /// height lower than aspect ratio
        // use height as default, reduce width
        int imageh = h/screenh;
        w = screenw*imageh;
    }
    else
    {
    }

        std::cout << h << '_' << w << std::endl;
    return;
}

void getScreenResolution(int &width, int &height)
{
    Display* d = XOpenDisplay(NULL);
    Screen* s  = DefaultScreenOfDisplay(d);

    width = s->width;
    height = s->height;

    return;
}

int main(int argc, char* argv[])
{
    //// check parameters
    if (argc < minargs)
    {
        std::cout << "not enough parameters" << std::endl;
        return 1;
    }
    /// check parameter options
    int mode = 0;
    int i = 1;
    while(mode == 0 && i < argc)
    {
        if(strcmp(argv[i], "-l") == 0)
            mode = 1;
        else if(strcmp(argv[i], "-r") == 0)
            mode = 2;
        else
            mode = 0;
        i++;
    }
    //// get screen resolution
    int width, height = 0;
    getScreenResolution(width, height);
    // aspect ratio
    int screenratiow, screenratioh = 0;
    getAspectRatio(width, height, screenratiow, screenratioh);
    //// get image resolution
    // get input image
    std::string imgfile = argv[1];
    Magick::Image img;
    try
    {
        img.read(imgfile);

        //// change aspect ratio
        int iwidth = img.columns();
        int iheight = img.rows();
        
        // try circumvent rounding errors
        int limit = 0; // hack
        do
        {    adjustImageResolution(screenratiow, screenratioh, iwidth, iheight); limit++;}
        while(nearlyEqual((screenratiow/screenratioh), (iwidth/iheight), 0.00001f) && (iwidth>img.columns()/2) && limit < aspectadjusttries);

        // set position of image frame
        int offsetx, offsety = 0;
        if(iwidth != img.columns() || iheight != img.rows())
        {
            if(iwidth != img.columns())
            {
                offsety = 0;
                // set image viewport according to mode
                switch(mode)
                {
                    case 0:// center
                        offsetx = (img.columns()-iwidth)/2;
                        break;
                    case 1:// left
                        offsetx = 0;
                        break;
                    case 2:// right
                        offsetx = img.columns()-iwidth;
                        break;
                    default:// defaults to center
                        offsetx = (img.columns()-iwidth)/2;
                        break;
                }
                
            }
            else
            {
                offsetx = 0;
                // offsety = (img.rows()-iheight)/2; // disabled until I decide that I need it
            }
        }

        //// crop image
        img.crop(Magick::Geometry(iwidth, iheight, offsetx, offsety));

        img.write(renamestr+imgfile);
    }
    catch(int e){}

    return 0;
}
