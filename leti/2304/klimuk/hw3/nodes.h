#ifndef NODES
#define NODES

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

using std::vector;
using std::pair;
using std::ofstream;

struct img {
	img(std::string name_, vector<vector<uint>> pixels_): name(name_), pixels(pixels_) {}

	std::string name;
	vector<vector<uint>> pixels;
};

typedef img image;

//consts
const uint MAX_BRIGHT = 255;
const int MARGIN = 2;
const uint MAX_COUNT_IMAGES = 100;
const uint IMAGE_WIDTH = 100;
const uint IMAGE_HEIGHT = 100;

//Source node
class img_creator {
	uint counter;
public:
    img_creator(): counter(0) {}

	bool operator()(std::shared_ptr<image>& result) {
		if (counter < MAX_COUNT_IMAGES)
		{
			result.reset(new image(std::to_string(counter), vector<vector<uint>> (IMAGE_WIDTH, vector<uint>(IMAGE_HEIGHT))));
			image* result_ptr = result.get();

			for (int i = 0; i < IMAGE_WIDTH; ++i) {
				for (int j = 0; j < IMAGE_HEIGHT; ++j) {
					(*result_ptr).pixels[i][j] = rand() % MAX_BRIGHT;
				}
			}

			++counter;
			return true;
		}
		return false;
    }
};

//functions_nodes
struct find_max {
    vector<pair<int, int>> operator()(std::shared_ptr<image> img) {
        //std::cout << "find_max" << std::endl;

        uint max = 0;
        vector<pair<int, int>> result;
        image* img_ptr = img.get();

        for(int i = 0; i < IMAGE_WIDTH; ++i) {
            for(int j = 0; j < IMAGE_HEIGHT; ++j) {
                uint br = (*img_ptr).pixels[i][j];
                if(br > max) {
                    max = br;
                }
            }
        }

        for(int i = 0; i < IMAGE_WIDTH; ++i) {
            for(int j = 0; j < IMAGE_HEIGHT; ++j) {
                uint br = (*img_ptr).pixels[i][j];
                if(br == max) {
                    result.push_back(std::make_pair(i, j));
                }
            }
        }

        /*for(int i = 0; i < result.size(); ++i) {
            uint br = (*img_ptr).pixels[result[i].first][result[i].second];
            std::cout << (*img_ptr).name << ": max at (" << result[i].first << ", " << result[i].second << "): (" << br  << ")" << std::endl;
        }*/

        return result;
    }
};

struct find_min {
    vector<pair<int, int>> operator()(std::shared_ptr<image> img) {
       // std::cout << "find_min" << std::endl;

        uint min = 256;
        vector<pair<int, int>> result;
        image* img_ptr = img.get();

        for(int i = 0; i < IMAGE_WIDTH; ++i) {
            for(int j = 0; j < IMAGE_HEIGHT; ++j) {
                uint br = (*img_ptr).pixels[i][j];
                if(br < min) {
                    min = br;
                }
            }
        }

        for(int i = 0; i < IMAGE_WIDTH; ++i) {
            for(int j = 0; j < IMAGE_HEIGHT; ++j) {
                uint br = (*img_ptr).pixels[i][j];
                if(min == br) {
                    result.push_back(std::make_pair(i, j));
                }
            }
        }

        /*for(int i = 0; i < result.size(); ++i) {
            uint br = (*img_ptr).pixels[result[i].first][result[i].second];
            std::cout << (*img_ptr).name << ": min at (" << result[i].first << ", " << result[i].second << "): (" << br << ")" << std::endl;
        }*/

        return result;
    }
};

class find_val {
    int b;
public:
    find_val(int a): b(a) {}

    vector<pair<int, int>> operator()(std::shared_ptr<image> img) {
        //std::cout << "find_val" << std::endl;

        vector<pair<int, int>> result;
        image* img_ptr = img.get();

        for(int i = 0; i < IMAGE_WIDTH; ++i) {
            for(int j = 0; j < IMAGE_HEIGHT; ++j) {
                uint br = (*img_ptr).pixels[i][j];
                if(b == br) {
                    result.push_back(std::make_pair(i, j));
                }
            }
        }

       /* for(int i = 0; i < result.size(); ++i) {
            uint br = (*img_ptr).pixels[result[i].first][result[i].second];
            std::cout << (*img_ptr).name << ": val " << b << " at (" << result[i].first << ", " << result[i].second << "): (" << br << ")" << std::endl;
        }*/

        return result;
    }
};

struct invert_img {
private:
    void invert_img_helper(image* img_ptr, const vector<pair<int, int>>& pixels) {
        for(auto p: pixels) {
            int pi = p.first;
            int pj = p.second;

            for(int i = -MARGIN; i <= MARGIN; ++i) {
                for(int j = -MARGIN; j <= MARGIN; ++j) {
                    if(pi + i >= 0 && pi + i < IMAGE_WIDTH && pj + j >= 0 && pj + j < IMAGE_HEIGHT) {
                        (*img_ptr).pixels[pi + i][pj + j] = MAX_BRIGHT - (*img_ptr).pixels[pi + i][pj + j];
                    }
                }
            }
        }
    }

public:
    bool operator()(std::tuple<std::shared_ptr<image>, const vector<pair<int, int>>&, const vector<pair<int, int>>&, const vector<pair<int, int>>&> v) {
        //std::cout << "invert_img" << std::endl;

        std::shared_ptr<image> img = std::get<0>(v);
        image new_img = *img.get();
        invert_img_helper(&new_img, std::get<1>(v));
        invert_img_helper(&new_img, std::get<2>(v));
        invert_img_helper(&new_img, std::get<3>(v));
        return true;
    }
};

class average_img {
    std::string average_img_helper(const image* img_ptr, const vector<pair<int, int>>& pixels) {
        for(auto p: pixels) {
            int pi = p.first;
            int pj = p.second;
            double sum = 0;
            int cnt = 0;

            for(int i = -MARGIN; i <= MARGIN; ++i) {
                for(int j = -MARGIN; j <= MARGIN; ++j) {
                    if(pi + i >= 0 && pi + i < IMAGE_WIDTH && pj + j >= 0 && pj + j < IMAGE_HEIGHT) {
                        sum += (*img_ptr).pixels[pi + i][pj + j];
                        ++cnt;
                    }
                }
            }

            sum /= cnt;

			std::string str = (*img_ptr).name;
			str.append(": average in (");
			str.append(std::to_string(pi));
			str.append(", ");
			str.append(std::to_string(pj));
			str.append(") is ");
			str.append(std::to_string(sum));
			str.append("\n");
			std::cout << str;
			return str;
        }
    }

public:
    average_img() {}

    std::string operator()(std::tuple<std::shared_ptr<image>, const vector<pair<int, int>>&, const vector<pair<int, int>>&, const vector<pair<int, int>>&> v) {
        //std::cout << "average_img" << std::endl;

        std::shared_ptr<image> img = std::get<0>(v);
		std::string output = average_img_helper(img.get(), std::get<1>(v));
        output += average_img_helper(img.get(), std::get<2>(v));
        output += average_img_helper(img.get(), std::get<3>(v));
		output += "\n";

        return output;
    }
};


class print_log {
	ofstream& out;

public:
	print_log(ofstream& o) : out(o) {}

	bool operator()(std::string str) {
		//std::cout << "print" << std::endl;
		out << str;

		return true;
	}
};
#endif
