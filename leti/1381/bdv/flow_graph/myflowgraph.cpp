#include "myflowgraph.h"

MyFlowGraph::MyFlowGraph(graph_options opt)
{
    image img;
    images_limit = opt.max_images;
    imgs_h = opt.image_h;
    imgs_w = opt.image_w;
    br = opt.brightness;
    logging = opt.logging;
    log_file = opt.filename;
}

MyFlowGraph::~MyFlowGraph()
{
}

void MyFlowGraph::run()
{
    tbb_graph g;

    int w, h, num_imgs = 0, max_imgs = images_limit;
    h = imgs_h;
    w = imgs_w;
    tbb::flow::source_node<image>
            input_img(g, [&num_imgs, max_imgs, w, h](image& img)
    {
        if(num_imgs < max_imgs)
        {
            num_imgs++;
            img.id = num_imgs;
            img.generate(w, h);
            return true;
        }
        return false;
    }, false
    );
    tbb::flow::function_node<image, minmax>
            find_min_max(g, images_limit, [](image img)
    {
        minmax res;
        res.img = img;
        res.minmax_ = find_minmax_value(img);
        return res;
    });
    int bri = br;
    tbb::flow::function_node<image, selected_pixels>
            select_elements(g, images_limit, [bri](image img)
    {
        selected_pixels res;
        res.img = img;
        res.pixels = find_elements(img, bri);
        return res;
    });
    tbb::flow::function_node<minmax, selected_pixels>
            select_min_elements(g, images_limit, [](minmax val)
    {
        selected_pixels res;
        res.img = val.img;
        uchar min = val.minmax_.first;
        res.pixels = find_elements(res.img, min);
        return res;
    });
    tbb::flow::function_node<minmax, selected_pixels>
            select_max_elements(g, images_limit, [](minmax val)
    {
        selected_pixels res;
        res.img = val.img;
        uchar max = val.minmax_.second;
        res.pixels = find_elements(res.img, max);
        return res;
    });
    tbb::flow::join_node<std::tuple<selected_pixels, selected_pixels, selected_pixels> >
            pixs_join(g);
    tbb::flow::function_node<std::tuple<selected_pixels, selected_pixels, selected_pixels>, image>
            ext_min(g, images_limit, [](std::tuple<selected_pixels, selected_pixels, selected_pixels> pixs_tup)
    {
        selected_pixels pixs = std::get<0>(pixs_tup);
        extend_pixels(pixs.img, pixs.pixels);
        return pixs.img;
    });
    tbb::flow::function_node<std::tuple<selected_pixels, selected_pixels, selected_pixels>, image>
            ext_max(g, images_limit, [](std::tuple<selected_pixels, selected_pixels, selected_pixels> pixs_tup)
    {
        selected_pixels pixs = std::get<1>(pixs_tup);
        extend_pixels(pixs.img, pixs.pixels);
        return pixs.img;
    });
    tbb::flow::function_node<std::tuple<selected_pixels, selected_pixels, selected_pixels>, image>
            ext_br(g, images_limit, [](std::tuple<selected_pixels, selected_pixels, selected_pixels> pixs_tup)
    {
        selected_pixels pixs = std::get<2>(pixs_tup);
        extend_pixels(pixs.img, pixs.pixels);
        return pixs.img;
    });
    tbb::flow::join_node<std::tuple<image, image, image>, tbb::flow::queueing >
            join(g);
    tbb::flow::function_node<std::tuple<image, image, image>, image>
            inverse_br(g, images_limit, [](std::tuple<image, image, image> tup)
    {
        image res, img;
        img = std::get<0>(tup);
        res = inverse_brightness(img);
        return res;
    });
    tbb::flow::buffer_node<image> inversed_img(g);
    tbb::flow::function_node<std::tuple<image, image, image>, img_avgbr>
            calc_avgbr(g, images_limit, [](std::tuple<image, image, image> tup)
    {
        img_avgbr res;
        res.img = std::get<0>(tup);
        res.avg_br = calc_avg_br(res.img);
        return res;
    });
    tbb::flow::buffer_node<img_avgbr> avg_br(g);

    tbb::flow::make_edge(input_img, find_min_max);
    tbb::flow::make_edge(input_img, select_elements);
    tbb::flow::make_edge(find_min_max, select_min_elements);
    tbb::flow::make_edge(find_min_max, select_max_elements);
    tbb::flow::make_edge(select_min_elements, tbb::flow::input_port<0>(pixs_join));
    tbb::flow::make_edge(select_max_elements, tbb::flow::input_port<1>(pixs_join));
    tbb::flow::make_edge(select_elements, tbb::flow::input_port<2>(pixs_join));
    tbb::flow::make_edge(pixs_join, ext_min);
    tbb::flow::make_edge(pixs_join, ext_max);
    tbb::flow::make_edge(pixs_join, ext_br);
    tbb::flow::make_edge(ext_min, tbb::flow::input_port<0>(join));
    tbb::flow::make_edge(ext_max, tbb::flow::input_port<1>(join));
    tbb::flow::make_edge(ext_br, tbb::flow::input_port<2>(join));
    tbb::flow::make_edge(join, inverse_br);
    tbb::flow::make_edge(inverse_br, inversed_img);
    tbb::flow::make_edge(join, calc_avgbr);
    tbb::flow::make_edge(calc_avgbr, avg_br);

    input_img.activate();

    g.wait_for_all();

    if(logging)
        write_avgs_to_file(avg_br, max_imgs);
    else
    {
        for(int i = 0; i < max_imgs; i++)
        {
            img_avgbr res;
            avg_br.try_get(res);
            delete[] res.img.data;
        }
    }

    for(int i = 0; i < max_imgs; i++)
    {
        image inv_img;
        inversed_img.try_get(inv_img);
        delete[] inv_img.data;
    }
}

std::pair<uchar, uchar> MyFlowGraph::find_minmax_value(image img)
{
    uchar* min_buffer = new uchar[img.height];
    uchar* max_buffer = new uchar[img.height];
    tbb::parallel_for(size_t(0), size_t(img.height), size_t(1),
        [min_buffer, max_buffer, &img](size_t i) {
            uchar* arr = img.data + i*img.width;
            uchar min = arr[0];
            uchar max = arr[0];
            for(int j = 1; j < img.width; j++)
            {
                if(min > arr[j])
                    min = arr[j];
                if(max < arr[j])
                    max = arr[j];
            }
            min_buffer[i] = min;
            max_buffer[i] = max;
    });
    uchar min = min_buffer[0];
    uchar max = max_buffer[0];
    for(int i = 1; i < img.height; i++)
    {
        if(min > min_buffer[i])
            min = min_buffer[i];
        if(max < max_buffer[i])
            max = max_buffer[i];
    }
    delete[] min_buffer;
    delete[] max_buffer;
    std::pair<uchar, uchar> res;
    res.first = min;
    res.second = max;
    return res;
}

tbb::concurrent_vector<pixel> MyFlowGraph::find_elements(image img, int value)
{
    tbb::concurrent_vector<pixel> result;
    tbb::parallel_for(size_t(0), size_t(img.height), size_t(1),
        [&img, value ,&result](size_t i) {
            pixel p;
            p.h_index = i;
            uchar* arr = img.data + i*img.width;
            for(int j = 0; j < img.width; j++)
            {
                if(arr[j] == value)
                {
                    p.value = value;
                    p.w_index = j;
                    result.push_back(p);
                }
            }
    });
    return result;
}
void MyFlowGraph::extend_pixels(image img, tbb::concurrent_vector<pixel> pixels)
{
    tbb::parallel_for(size_t(0), size_t(pixels.size()), size_t(1),
                      [img, &pixels](size_t i)
    {
        extend_pix(img, pixels[i]);
    });
}

void MyFlowGraph::extend_pix(image img, pixel p)
{
    for(int ii = -1; ii <= 1; ii++)
        for(int jj = -1; jj <= 1; jj++)
        {
            uchar* pix_ptr = img.pix_ptr(p.w_index + jj, p.h_index + ii);
            if(pix_ptr != 0)
                pix_ptr[0] = p.value;
        }
}

void MyFlowGraph::write_avgs_to_file(tbb::flow::buffer_node<img_avgbr>& node, int num_images)
{
    std::ofstream of(log_file);
    for(int i = 0; i < num_images; i++)
    {
        img_avgbr res;
        node.try_get(res);
        of << "Avg brightness for image[" << res.img.id << "] = " << res.avg_br << std::endl;
        delete[] res.img.data;
    }
    of.close();
}

image MyFlowGraph::inverse_brightness(image img)
{
    image res;
    res.data = new uchar[img.width*img.height];
    tbb::parallel_for(size_t(0), size_t(img.height), size_t(1),
        [&res, &img](size_t i)
    {
            for(int j = 0; j < img.width; j++)
                res.data[i*img.width + j] = img.data[i*img.width + j];
    });
    return res;
}

double MyFlowGraph::calc_avg_br(image img)
{
    double avg_br = 0;
    double* buffer = new double[img.height];
    tbb::parallel_for(size_t(0), size_t(img.height), size_t(1),
        [buffer, &img](size_t i) {
            buffer[i] = 0;
            for(int j = 0; j < img.width; j++)
                buffer[i] += img.data[i*img.width + j];
            buffer[i] /= img.width;
    });
    for(int i = 0; i < img.height; i++)
        avg_br += buffer[i];
    delete[] buffer;
    return avg_br / img.height;
}
