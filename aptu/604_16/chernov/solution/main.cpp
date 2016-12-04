#include <cstdio>
#include <random>
#include <thread>
#include <iostream>
#include <fstream>
#include "tbb/flow_graph.h"
#include "cxxopts.hpp"

using namespace tbb::flow;
using namespace std;

const size_t DEFAULT_IMAGE_H = 3;
const size_t DEFAULT_IMAGE_W = 3;

struct pixel
{
    size_t R = 0, G = 0, B = 0;

    pixel() = default;

    pixel( size_t r_, size_t g_, size_t b_ )
        : R( r_ )
        , G( g_ )
        , B( b_ )
    {}

    void invert( size_t  base = 255 )
    {
        R = base - R;
        G = base - G;
        B = base - B;
    }
};

struct image
{
        using Mat = std::vector<std::vector<pixel>>;

        image( size_t w = DEFAULT_IMAGE_W, size_t h = DEFAULT_IMAGE_H )
            : w( w )
            , h( h )
        {
            mat = Mat( h, vector<pixel>( w ) );
        }

        void set( size_t i, size_t j, pixel p )
        {
            mat[i][j] = p;
        }

        pixel get( size_t i, size_t j ) const
        {
            return mat[i][j];
        }

        pair<size_t, size_t> shape() const
        {
            return std::make_pair( w, h );
        }

        float luma( pixel const & pix ) const
        {
            size_t R = pix.R;
            size_t G = pix.G;
            size_t B = pix.B;
            return ( R + R + R + B + G + G + G + G ) >> 3;
        }

        void equal_or_nearest( size_t val, size_t & row, size_t & col ) const
        {
            bool found = false;
            for( size_t i = 0; i < h; ++i )
            {
                for( size_t j = 0; j < w; ++j )
                {
                    size_t Y = static_cast<size_t>( luma( get( i, j ) ) );
                    if( Y == val )
                    {
                        row = i;
                        col = j;
                        found = true;
                    }
                }
            }

            if( !found ) // find pixel with closest Luma value
            {
                size_t min_diff = 255 - val;
                for( size_t i = 0; i < h; ++i )
                {
                    for( size_t j = 0; j < w; ++j )
                    {
                        size_t Y = static_cast<size_t>( luma( get( i, j ) ) );
                        size_t diff = std::abs( Y - val );
                        if( diff <= min_diff )
                        {
                            min_diff = diff;
                            row = i;
                            col = j;
                        }
                    }
                }
            }
        }

        void max_luminance( size_t & row , size_t & col , float & val ) const
        {
            val = std::numeric_limits<float>::min();
            for( size_t i = 0; i < h; ++i )
            {
                for( size_t j = 0; j < w; ++j )
                {
                    size_t Y = luma( get( i, j ) );
                    if( Y >= val )
                    {
                        val = Y;
                        row = i;
                        col = j;
                    }
                }
            }
        }

        void min_luminance( size_t & row , size_t & col , float & val ) const
        {
            val = std::numeric_limits<float>::max();
            for( size_t i = 0; i < h; ++i )
            {
                for( size_t j = 0; j < w; ++j )
                {
                    size_t Y = luma( get( i, j ) );
                    if( Y <= val )
                    {
                        val = Y;
                        row = i;
                        col = j;
                    }
                }
            }
        }

        pixel invert_pixel( pixel p, size_t val = 255 )
        {
            p.R = val - p.R;
            p.G = val - p.G;
            p.B = val - p.B;
            return p;
        }

        float mean_luminance() const
        {
            float result = 0.f;
            for( size_t i = 0; i < h; ++i )
                for( size_t j = 0; j < w; ++j )
                    result += luma( get( i, j ) );
            return result / static_cast<float>( w * h );
        }

        void invert( size_t base = 255 )
        {
            for( size_t i = 0; i < h; ++i )
                for( size_t j = 0; j < w; ++j )
                    get( i, j ).invert( base );
        }

        image cut_3x3_around( size_t row, size_t col ) const
        {
            image result( 3, 3 );
            for( size_t i = row - 1, ii = 0; i < row + 1; ++i, ++ii )
                for( size_t j = col - 1, jj = 0; j < col + 1; ++j, ++jj )
                    result.set( ii, jj, get( i, j ) );

            return result;
        }

    private:
        size_t w;
        size_t h;
        Mat mat;
};

using img_tuple = tuple<image, image, image>;
using flt_tuple = tuple<float, float, float>;

struct eq_elements
{
    size_t target_value;

    eq_elements( size_t val )
        : target_value( val )
    {}

    image operator()( image const & input )
    {
        size_t row = 0, col = 0;
        input.equal_or_nearest( target_value, row, col );
        return input.cut_3x3_around( row, col );
    }
};

struct max_luminance
{
    image operator()( image const & input )
    {
        float max_luma = std::numeric_limits<float>::min();
        size_t row = 0, col = 0;
        input.max_luminance( row, col, max_luma );
        return input.cut_3x3_around( row, col );
    }
};

struct min_luminance
{
    image operator()( image const & input )
    {
        float min_luma = std::numeric_limits<float>::max();
        size_t row = 0, col = 0;
        input.min_luminance( row, col, min_luma );
        return input.cut_3x3_around( row, col );
    }
};

struct mean_luminance
{
    tuple<float, float, float> operator()( tuple<image, image, image>const & input )
    {
        return std::make_tuple( std::get<0>( input ).mean_luminance(),
                                std::get<1>( input ).mean_luminance(),
                                std::get<2>( input ).mean_luminance() );
    }
};

struct inverse_image
{
    continue_msg operator()( tuple<image, image, image> input )
    {
        std::get<0>( input ).invert();
        std::get<1>( input ).invert();
        std::get<2>( input ).invert();
        return continue_msg();
    }
};

struct logger
{
    private:
        ofstream & logstream;

    public:
        logger( ofstream & stream )
            : logstream( stream )
        {
        }

        continue_msg operator()( flt_tuple const & input )
        {
            if( logstream.is_open() )
            {
                logstream << std::get<0>( input ) << endl;
                logstream << std::get<1>( input ) << endl;
                logstream << std::get<2>( input ) << endl;
            }
            return continue_msg();
        }
};

struct image_generator
{
    private:
        std::uniform_int_distribution<> dist;
        std::mt19937 twister;
        size_t w = DEFAULT_IMAGE_W;
        size_t h = DEFAULT_IMAGE_H;

    public:
        image_generator()
        {
            std::random_device r;
            dist = std::uniform_int_distribution<int>( 0, 255 );
            std::seed_seq s{r()};
            twister = std::mt19937{s};
        }

        image_generator( size_t w_, size_t h_ )
            : image_generator()

        {
            w = w_;
            h = h_;
        }

        pixel random_pixel()
        {
            return pixel( dist( twister ), dist( twister ), dist( twister ) );
        }

        bool operator()( image & input )
        {
            image img( w, h );
            for( size_t i = 0; i < h; ++i )
                for( size_t j = 0; j < w; ++j )
                    img.set( i, j, random_pixel() );

            input = img;
            return true;
        }
};

int main( int argc, char * argv[] )
{
    size_t target = 255;
    size_t queue_len = std::thread::hardware_concurrency() * 4;
    std::string logfile = "";
    try
    {
        cxxopts::Options options( argv[0], " -b <value in [0,255]>  -l <value in (0, inf)> -f <lofgile> " );
        options.add_options()
        ( "b,brightness", "Target Luma value", cxxopts::value<size_t>() )
        ( "l,limig", "Limit of simutanious image processing", cxxopts::value<size_t>() )
        ( "f,file", "File for logging", cxxopts::value<std::string>(), "FILE" )
        ( "h,help", "Help string" );

        options.parse( argc, argv );

        if( options.count( "help" ) )
        {
            std::cout << options.help() << std::endl;
            exit( 0 );
        }


        if( options.count( "b" ) > 0 )
        {
            target = options["b"].as<size_t>();
            if( target > 255 )
            {
                throw cxxopts::OptionException( "wrong -b target value" );
            }
        }

        if( options.count( "l" ) > 0 )
            queue_len = options["l"].as<size_t>();

        if( options.count( "f" ) > 0 )
            logfile = options["f"].as<string>();
    }
    catch( const cxxopts::OptionException & e )
    {
        std::cout << "error parsing options: " << e.what() << std::endl;
        exit( 1 );
    }

    ofstream stream;
    if( logfile != "" )
    {
        stream.open( logfile );
    }

    /* Default values */
    size_t M = 50;
    size_t N = 50;

    cout << "Runing with the following parameters: " << endl;
    cout << " - M x N = " << "( " << M << " x " << N << ")" << endl;
    cout << " - brightness(luma) = " << target << endl;
    cout << " - limit = " << queue_len << endl;
    cout << " - logfile = " << logfile << endl;

    graph g;
    source_node<image> source( g, image_generator( M, N ) );
    limiter_node<image> limiter( g, queue_len );

    broadcast_node<image> broadcaster( g );
    function_node<image, image, rejecting> equal_elements( g, unlimited, eq_elements{target} );
    function_node<image, image, rejecting> max_elements( g, unlimited, max_luminance{} );
    function_node<image, image, rejecting> min_elements( g, unlimited, min_luminance{} );
    join_node<img_tuple, queueing> join( g );
    function_node<img_tuple, flt_tuple, rejecting> mean( g, unlimited, mean_luminance{} );
    function_node<img_tuple, continue_msg, rejecting> inverser( g, unlimited, inverse_image{} );
    function_node<flt_tuple, continue_msg, rejecting> output( g, unlimited, logger( stream ) );

    make_edge( source, limiter );
    make_edge( limiter, broadcaster );
    make_edge( broadcaster, equal_elements );
    make_edge( broadcaster, max_elements );
    make_edge( broadcaster, min_elements );
    make_edge( equal_elements, get<0>( join.input_ports() ) );
    make_edge( max_elements, get<1>( join.input_ports() ) );
    make_edge( min_elements, get<2>( join.input_ports() ) );
    make_edge( join, mean );
    make_edge( join, inverser );
    make_edge( inverser, limiter.decrement );
    make_edge( mean, output );
    make_edge( output, limiter.decrement );


    source.activate();
    g.wait_for_all();
    if( stream.is_open() )
        stream.close();

    return 0;
}
