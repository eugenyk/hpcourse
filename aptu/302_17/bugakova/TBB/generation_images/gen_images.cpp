//
// Created by Nadezhda Bugakova on 30/05/2017.
//

#include<cstdio>
#include<iostream>
#include<stdlib.h>
#include<ctime>
#include <dirent.h>

const int n = 50;
const int m = 60;
const std::string prefix = "img";

void gen_images(int cnt_images) {
    for (int k = 0; k < cnt_images; k++) {
        std::string file_name = prefix + std::to_string(k);
        freopen(file_name.c_str(), "w", stdout);
        std::cout << n << " " << m << std::endl;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int cur_value = rand()%256;
                std::cout << cur_value << " ";
            }
            std::cout << std::endl;
        }
    }
}

void remove_images() {
    DIR* cur_dir = opendir("./");
    dirent* ent;
    if (cur_dir != NULL) {
        while ((ent = readdir(cur_dir)) != NULL) {
            char* file_name = ent->d_name;
            std::string file_name_string = file_name;
            if (file_name_string.substr(0, 3) == prefix) {
                remove(file_name);
            }
        }
    }
}

int main(int args, char* argv[]) {
    if (args <= 1) {
        std::cerr << "Use man_images <cnt_image_to_generate>" << std::endl;
    }

    srand(time(NULL));

    int cnt_image = atoi(argv[1]);
    remove_images();
    gen_images(cnt_image);

    return 0;
}