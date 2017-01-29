#include <cstdlib>
#include <utility>
#include <iostream>

class RandomImage {
	size_t height_;
	size_t width_;
	size_t id_;
	size_t* array_;
	static size_t number_of_instances_;

public:
	RandomImage(size_t height, size_t width) :
		height_(height),
		width_(width),
		id_(number_of_instances_++),
		array_(new size_t[height_ * width_])
	{
	    for (size_t i = 0; i < height * width; ++i) {
	        array_[i] = static_cast<size_t>(std::rand()) % 256;
	    }
	}
	RandomImage(const RandomImage& oth) :
		height_(oth.height_),
		width_(oth.width_)
	{
		array_ = new size_t[height_ * width_];
	    for (size_t i = 0; i < height_; ++i) {
			for (size_t j = 0; j < width_; ++j) {
				at(i,j) = oth.at(i,j);
			}
		}
	}
	size_t get_id() { return id_; }
	size_t& at(size_t i, size_t j) {
		return array_[i * width_ + j];
	}
	size_t at(size_t i, size_t j) const {
		return array_[i * width_ + j];
	}

	std::vector<std::pair<size_t, size_t>> find_eq_elements(size_t needed_brightness) const {
		std::vector<std::pair<size_t, size_t>> res;
		for (size_t i = 0; i < height_; ++i) {
			for (size_t j = 0; j < width_; ++j) {
				if (at(i,j) == needed_brightness) {
					res.push_back(std::make_pair(i,j));
				}
			}
		}
		return res;
	}
	std::vector<std::pair<size_t, size_t>> find_max_elements() const {
		std::vector<std::pair<size_t, size_t>> res;
		size_t max_brightness = 0;
		for (size_t i = 0; i < height_; ++i) {
			for (size_t j = 0; j < width_; ++j) {
				max_brightness =std::max(max_brightness, at(i,j));
			}
		}
		return find_eq_elements(max_brightness);
	}
	std::vector<std::pair<size_t, size_t>> find_min_elements() const {
		std::vector<std::pair<size_t, size_t>> res;
		size_t min_brightness = 256;
		for (size_t i = 0; i < height_; ++i) {
			for (size_t j = 0; j < width_; ++j) {
				min_brightness =std::min(min_brightness, at(i,j));
			}
		}
		return find_eq_elements(min_brightness);
	}

	void mark_elements(std::vector<std::pair<size_t, size_t>> elements) {
		for (auto p : elements) {
			size_t center_i = p.first;
			size_t center_j = p.second;
			size_t center_brightness = at(center_i, center_j);
			for (size_t i : {center_i - 1, center_i, center_i + 1}) {
				for (size_t j : {center_j - 1, center_j, center_j + 1}) {
					if (i < height_ && j < width_) {
						at(i,j) = 255;
					}
				}
			}
			at(center_i, center_j) = center_brightness;
		}
	}

	double mean() const {
		size_t sum = 0;
		for (size_t i = 0; i < height_; ++i) {
			for (size_t j = 0; j < width_; ++j) {
				sum += at(i,j);
			}
		}
		return static_cast<double>(sum)/(height_ * width_);
	}

	void inverse() {
		for (size_t i = 0; i < height_; ++i) {
			for (size_t j = 0; j < width_; ++j) {
				at(i,j) = 255 - at(i,j);
			}
		}
	}

	void write_to(std::ostream& otp) const {
		for (size_t i = 0; i < height_; ++i) {
			for (size_t j = 0; j < width_; ++j) {
				otp << at(i,j) << " ";
			}
			otp << std::endl;
		}
	}
	

	~RandomImage()
	{
		delete [] array_;
	}
};