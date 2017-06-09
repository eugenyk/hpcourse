#include <algorithm>
#include <atomic>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <limits>
#include <memory>
#include <random>
#include <set>

#include <tbb/flow_graph.h>
#include <tbb/tbb.h>

namespace flow = tbb::flow;

using coord_t = size_t;
using value_t = uint8_t;
constexpr coord_t N{400}, M{400};
using image_tt = std::array<std::array<value_t, M>, N>;
using image_t = std::tuple<std::string, std::shared_ptr<image_tt>>;
using point_t = std::pair<value_t, value_t>;
using map_t = std::tuple<image_t, value_t>;
using stat_tt = std::vector<point_t>;
using stat_t = std::tuple<value_t, std::shared_ptr<stat_tt>, image_t>;
using save_t = std::tuple<image_t, std::string>;

using range_t = tbb::blocked_range2d<coord_t, coord_t>;

struct generator_t {
	image_t operator()(size_t id) {
		std::uniform_int_distribution<value_t> dist(std::numeric_limits<value_t>::min(), std::numeric_limits<value_t>::max() - 1);
		image_t result{std::to_string(id), std::make_shared<image_tt>()};
		parallel_for(range_t(0, N, 0, M), [&](range_t const& range) -> void {
				std::default_random_engine re;
				for (coord_t row{range.rows().begin()}; row != range.rows().end(); ++row)
					for (coord_t col{range.cols().begin()}; col != range.cols().end(); ++col)
						(*std::get<1>(result))[row][col] = dist(re);
			});
		return result;
	}
};

template<bool isMax>
struct extreme_t {
	map_t operator()(image_t image) {
		std::atomic<value_t> result(isMax ? std::numeric_limits<value_t>::min() : std::numeric_limits<value_t>::max());
		parallel_for(range_t(0, N, 0, M), [&](range_t const& range) -> void {
				value_t local_value{isMax ? std::numeric_limits<value_t>::min() : std::numeric_limits<value_t>::max()};
				for (coord_t row{range.rows().begin()}; row != range.rows().end(); ++row)
					for (coord_t col{range.cols().begin()}; col != range.cols().end(); ++col) {
						value_t pixel{(*std::get<1>(image))[row][col]};
						if (isMax)
							local_value = std::max(local_value, pixel);
						else
							local_value = std::min(local_value, pixel);
					}
				while (true) {
					value_t e(result), t;
					if (isMax)
						t = std::max(e, local_value);
					else
						t = std::min(e, local_value);
					if (result.compare_exchange_strong(e, t))
						break;
				}
			});
		return map_t{image, result};
	}
};

struct finder_t {
	stat_t operator()(map_t map) {
		image_t image(std::get<0>(map));
		value_t value{std::get<1>(map)};
		tbb::concurrent_unordered_set<point_t> res;
		parallel_for(range_t(0, N, 0, M), [&](range_t const& range) -> void {
				for (coord_t row{range.rows().begin()}; row != range.rows().end(); ++row)
					for (coord_t col{range.cols().begin()}; col != range.cols().end(); ++col)
						if ((*std::get<1>(image))[row][col] == value) {
							res.emplace(row, col);
						}
			});
		stat_t result;
		std::get<0>(result) = value;
		auto& resvec{std::get<1>(result)};
		std::get<2>(result) = image;
		resvec = std::make_shared<stat_tt>();
		std::move(res.begin(), res.end(), std::insert_iterator<std::vector<point_t>>(*resvec, resvec->end()));
		return result;
	}
};

struct painter_t {
	save_t operator()(stat_t stat) {
		image_t const& src_image{std::get<2>(stat)};
		image_t image{std::get<0>(src_image), std::make_shared<image_tt>(*std::get<1>(src_image))};
		std::shared_ptr<stat_tt> points{std::get<1>(stat)};
		parallel_for(tbb::blocked_range<size_t>(0, points->size()), [&](tbb::blocked_range<size_t> const& range) -> void {
				for (size_t ind{range.begin()}; ind != range.end(); ++ind) {
					for (coord_t dy{0}; dy != 3; ++dy) {
						coord_t ty{(*points)[ind].first + dy};
						if (ty < 1 || ty + 1 >= N)
							continue;
						for (coord_t dx{0}; dx != 3; ++dx) {
							coord_t tx{(*points)[ind].second + dx};
							if (tx < 1 || tx + 1 >= M)
								continue;
							if (dx == 1 && dy == 1)
								continue;
							(*std::get<1>(image))[ty - 1][tx - 1] = std::numeric_limits<value_t>::max();
						}
					}
				}
			});
		save_t result;
		std::get<1>(result) = std::get<0>(image) + "-" + std::to_string(std::get<0>(stat));
		std::get<0>(result) = image;
		return result;
	}
};

struct average_t {
	std::fstream& log;

	double operator()(save_t source) {
		std::atomic<uint64_t> sum(0);
		image_t const& image{std::get<0>(source)};
		parallel_for(range_t(0, N, 0, M), [&](range_t const& range) -> void {
				uint64_t lsum{0};
				for (coord_t row{range.rows().begin()}; row != range.rows().end(); ++row)
					for (coord_t col{range.cols().begin()}; col != range.cols().end(); ++col)
						lsum += (*std::get<1>(image))[row][col];
				sum += lsum;
			});
		double result{sum / double{N * M}};
		log << std::get<1>(source) << ": avg=" << result << std::endl;
		return result;
	}
};

struct inverse_t {
	save_t operator()(save_t source) {
		constexpr value_t max_val{std::numeric_limits<value_t>::max()};
		image_t const& src_image{std::get<0>(source)};
		image_t image{std::get<0>(src_image), std::make_shared<image_tt>(*std::get<1>(src_image))};
		parallel_for(range_t(0, N, 0, M), [&](range_t const& range) -> void {
				for (coord_t row{range.rows().begin()}; row != range.rows().end(); ++row)
					for (coord_t col{range.cols().begin()}; col != range.cols().end(); ++col)
						(*std::get<1>(image))[row][col] = max_val - (*std::get<1>(src_image))[row][col];
			});
		return save_t{image, "inv-" + std::get<1>(source)};
	}
};

struct saver_t {
	flow::continue_msg operator()(save_t stat) {
		image_t const& image{std::get<0>(stat)};
		std::string name(std::get<1>(stat) + ".out");
		std::fstream f(name, std::fstream::out);
		for (auto const& row: *std::get<1>(image)) {
			for (value_t v: row)
				f << std::setw(3) << size_t{v} << ' ';
			f << '\n';
		}
		return flow::continue_msg{};
	}
};

int main(int argc, char const* argv[]) {
	int argi(1);

	size_t images{10};
	size_t limit{5};
	value_t value{13};
	std::string log_name("log.txt");
	while (argi < argc) {
		std::string arg(argv[argi]);
		if (arg == "-b") {
			sscanf(argv[argi + 1], "%hhu", &value);
			argi += 2;
		} else if (arg == "-l") {
			sscanf(argv[argi + 1], "%zu", &limit);
			argi += 2;
		} else if (arg == "-c") {
			sscanf(argv[argi + 1], "%zu", &images);
			argi += 2;
		} else if (arg == "-f") {
			log_name = argv[argi + 1];
			argi += 2;
		} else {
			argi += 1;
		}
	}

	std::fstream log(log_name, std::fstream::out);

	flow::graph g;
	flow::function_node<size_t, image_t> generator(g, limit, generator_t{});
	flow::function_node<image_t, map_t> min(g, limit, extreme_t<false>{});
	flow::function_node<image_t, map_t> max(g, limit, extreme_t<true >{});
	flow::function_node<image_t, map_t> val(g, limit, [value](image_t img) -> map_t {return make_tuple(img, value);});
	flow::function_node<map_t, stat_t> find(g, 3 * limit, finder_t{});
	flow::function_node<stat_t, save_t> paint(g, 3 * limit, painter_t{});
	flow::function_node<save_t, save_t> inverse(g, 3 * limit, inverse_t{});
	flow::function_node<save_t, double> avg(g, 3 * limit, average_t{log});
	flow::function_node<save_t, flow::continue_msg> save(g, 6 * limit, saver_t{});

	flow::make_edge(generator, min);
	flow::make_edge(generator, max);
	flow::make_edge(generator, val);
	flow::make_edge(min, find);
	flow::make_edge(max, find);
	flow::make_edge(val, find);
	flow::make_edge(find, paint);
	flow::make_edge(paint, avg);
	flow::make_edge(paint, inverse);
	flow::make_edge(paint, save);
	flow::make_edge(inverse, save);

	for (size_t i{0}; i < images; ++i)
		generator.try_put(i);
	g.wait_for_all();
	return 0;
}
