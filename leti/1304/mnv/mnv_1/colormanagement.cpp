#include "colormanagement.h"

cnt_brightness::cnt_brightness(Pixel brightPixel) : brightPixel(brightPixel) {}

process_avg::process_avg(ofstream &file) : file(file) {}

void process_avg::printFile(int idImage, long avg)
{
    cout << "## write to file ##" << endl;
    file << "Id image: #" << idImage << ". Average brightness: " << avg << ".\n";
}

void highlight::toHighlight(Image* img, int pixelIndex)
{
    int w = img->getWidth();
    int h = img->getHeight();

    int l = pixelIndex - 1;
    int r = pixelIndex + 1;
    int d = pixelIndex + w;
    int dl = pixelIndex + w - 1;
    int dr = pixelIndex + w + 1;
    int u = pixelIndex - w;
    int ul = pixelIndex - w - 1;
    int ur = pixelIndex - w + 1;

    vector<int> adj_pixs = { u, d };
    //left border.
    if (pixelIndex % w != 0)
    {
        adj_pixs.push_back(ul);
        adj_pixs.push_back(l);
        adj_pixs.push_back(dl);
    }

    //right border.
    if ((pixelIndex + 1) % w != 0)
    {
        adj_pixs.push_back(ur);
        adj_pixs.push_back(r);
        adj_pixs.push_back(dr);
    }

    for (vector<int>::iterator it = adj_pixs.begin(); it != adj_pixs.end(); ++it)
    {
        int cur_pix = *it;
        if (cur_pix >= 0 && cur_pix < w * h)
        {

            img->update(cur_pix, 0);
        }
    }
}

write_avg::write_avg(ofstream &file) : file(file) {}

source_body::source_body(int n) : n(n)
{
    cur_n = 0;
}
