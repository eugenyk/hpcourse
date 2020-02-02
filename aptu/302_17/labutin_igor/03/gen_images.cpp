
#include <cstdlib>
#include <iostream>
#include <dirent.h>
#include <fstream>

void print_usage() {
    std::cerr << "Usage:\n";
    std::cerr << "gen_images images_cnt max_h max_w\n";
}


void gen_images(int cnt, int max_h, int max_w) {
    for (int file_id = 0; file_id < cnt; file_id++) {
        std::ofstream image_file;
        image_file.open("../data/image_" + std::to_string(file_id) + ".test");
        int h = 1 + rand() % max_h;
        int w = 1 + rand() % max_w;
        image_file << h << " " << w << "\n";
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                image_file << rand() % 256 << " ";
            }
            image_file << "\n";
        }
        image_file.close();
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        print_usage();
        exit(1);
    }

    gen_images(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));

    return 0;
}