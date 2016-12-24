#include "myflowgraph.h"

MyFlowGraph::MyFlowGraph(graph_options opt)
{
    image img;
    images_limit = opt.max_images;
    for(int i = 0; i < opt.max_images; i++)
    {
        img.id = i;
        img.generate(opt.image_w, opt.image_h);
        imgs.push_back(img);
    }

    br = opt.brightness;
    logging = opt.logging;
    log_file = opt.filename;
}

MyFlowGraph::~MyFlowGraph()
{
    for(int i = 0; i < imgs.size(); i++)
        delete[] imgs[i].data;
}

void MyFlowGraph::run()
{
    tbb_graph g;

    tbb::flow::broadcast_node<image> input_img(g);
    tbb::flow::function_node<image, minmax>
            find_min_max(g, images_limit, [](image img)
    {
        return find_minmax_value(img);
    });
    int br_ = br;
    tbb::flow::function_node<minmax, selected_pixels>
            select_elements(g, images_limit, [br_](minmax val)
    {
       return find_elements(val, br_);
    });
    tbb::flow::function_node<selected_pixels, image>
            ext_min(g, images_limit, [](selected_pixels pixs)
    {
        return extend_min(pixs);
    });
    tbb::flow::function_node<selected_pixels, image>
            ext_max(g, images_limit, [](selected_pixels pixs)
    {
       return extend_max(pixs);
    });
    tbb::flow::function_node<selected_pixels, image>
            ext_br(g, images_limit, [](selected_pixels pixs)
    {
       return extend_br(pixs);
    });
    tbb::flow::join_node<std::tuple<image, image, image>, tbb::flow::queueing >
            join(g);
    tbb::flow::function_node<std::tuple<image, image, image>, img_avgbr>
            inverse_and_avg(g, tbb::flow::unlimited, [](std::tuple<image, image, image> tup)
    {
        img_avgbr res;
        res.img = std::get<0>(tup);
        res.avg_br = inverse_and_avgbr(res.img);
        return res;
    });
    tbb::flow::buffer_node<img_avgbr> avg_br(g);

    tbb::flow::make_edge(input_img, find_min_max);
    tbb::flow::make_edge(find_min_max, select_elements);
    tbb::flow::make_edge(select_elements, ext_min);
    tbb::flow::make_edge(select_elements, ext_max);
    tbb::flow::make_edge(select_elements, ext_br);
    tbb::flow::make_edge(ext_min, tbb::flow::input_port<0>(join));
    tbb::flow::make_edge(ext_max, tbb::flow::input_port<1>(join));
    tbb::flow::make_edge(ext_br, tbb::flow::input_port<2>(join));
    tbb::flow::make_edge(join, inverse_and_avg);
    tbb::flow::make_edge(inverse_and_avg, avg_br);


    for(int i = 0; i < imgs.size(); i++)
        input_img.try_put(imgs[i]);
    g.wait_for_all();

    if(logging)
        write_avgs_to_file(avg_br);
}

minmax MyFlowGraph::find_minmax_value(image img)
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
    minmax res;
    res.minmax_.first = min;
    res.minmax_.second = max;
    res.img = img;
    return res;
}

selected_pixels MyFlowGraph::find_elements(minmax val, int br)
{
    selected_pixels result;
    uchar min = val.minmax_.first;
    uchar max = val.minmax_.second;

    result.img = val.img;
    tbb::parallel_for(size_t(0), size_t(result.img.height), size_t(1),
        [min, max, br, &result](size_t i) {
            pixel p;
            p.h_index = i;
            uchar* arr = result.img.data + i*result.img.width;
            for(int j = 0; j < result.img.width; j++)
            {
                if(arr[j] == min)
                {
                    p.value = min;
                    p.w_index = j;
                    result.min.push_back(p);
                }
                if(arr[j] == max)
                {
                    p.value = max;
                    p.w_index = j;
                    result.max.push_back(p);
                }
                if(arr[j] == br)
                {
                    p.value = br;
                    p.w_index = j;
                    result.br.push_back(p);
                }
            }
    });
    return result;
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

image MyFlowGraph::extend_min(selected_pixels pixs)
{
    tbb::parallel_for(size_t(0), size_t(pixs.min.size()), size_t(1),
        [&pixs](size_t i) {
            extend_pix(pixs.img, pixs.min[i]);
    });
    return pixs.img;
}

image MyFlowGraph::extend_max(selected_pixels pixs)
{
    tbb::parallel_for(size_t(0), size_t(pixs.max.size()), size_t(1),
        [&pixs](size_t i) {
            extend_pix(pixs.img, pixs.max[i]);
    });
    return pixs.img;
}

image MyFlowGraph::extend_br(selected_pixels pixs)
{
    tbb::parallel_for(size_t(0), size_t(pixs.br.size()), size_t(1),
        [&pixs](size_t i) {
            extend_pix(pixs.img, pixs.br[i]);
    });
    return pixs.img;
}

double MyFlowGraph::inverse_and_avgbr(image img)
{
    double avg_br = 0;
    double* buffer = new double[img.height];
    tbb::parallel_for(size_t(0), size_t(img.height), size_t(1),
        [buffer, &img](size_t i) {
            buffer[i] = 0;
            for(int j = 0; j < img.width; j++)
            {
                uchar value = img.data[i*img.width + j];
                img.data[i*img.width + j] = 255 - value;
                buffer[i] += value;
            }
            buffer[i] /= img.width;
    });
    for(int i = 0; i < img.height; i++)
        avg_br += buffer[i];
    delete[] buffer;
    return avg_br / img.height;
}

void MyFlowGraph::write_avgs_to_file(tbb::flow::buffer_node<img_avgbr>& node)
{
    std::ofstream of(log_file);
    for(int i = 0; i < imgs.size(); i++)
    {
        img_avgbr res;
        node.try_get(res);
        of << "Avg brightness for image[" << res.img.id << "] = " << res.avg_br << std::endl;
    }
    of.close();
}
