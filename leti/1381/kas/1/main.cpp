#include "Image.h"
#include "Options.h"

#include "tbb/flow_graph.h"

#include <iostream>
#include <fstream>

int main(int argc, char *argv[])
{
  Options opt;
  if (!opt.parseArgs(argc, argv))
  {
    opt.printUsage(std::cout);
    return 1;
  }

  srand(time(0));
  std::ofstream fout(opt.getLogFilePath());

  tbb::flow::graph flowGraph;

  // Создание изображений
  int currImage = 0;
  tbb::flow::source_node<Image> sourceNode(flowGraph, [&currImage, &opt](Image &image)
    {
      if (currImage >= opt.getAmountImages())
      {
        return false;
      }

      image = Image(opt.getImageHeight(), opt.getImageWidth());
      currImage++;

      if (opt.IsDebug())
      {
        std::cout << std::endl << "Start Image " << currImage - 1 << std::endl;
        for (unsigned int i = 0; i < image.getHeight(); i++)
        {
          for (unsigned int j = 0; j < image.getWidth(); j++)
          {
            std::cout << (int)image.at(i, j) << "   ";
          }
          std::cout << std::endl;
        }
      }

      return true;
    }, false);

  // Ограничение одновреммено обрабатываемых изображений
  tbb::flow::limiter_node<Image> limiterNode(flowGraph, opt.getMaxParallelFlow());

  // Поиск максимального значения яркости
  tbb::flow::function_node<Image, std::vector<std::pair<unsigned int, unsigned int> > > searchMaxValueNode(flowGraph, tbb::flow::unlimited, [](const Image &image)
    {
      return image.getIndexOfBrightness(image.getMaxBrightness());
    });

  // Поиск минимального значения яркости
  tbb::flow::function_node<Image, std::vector<std::pair<unsigned int, unsigned int> > > searchMinValueNode(flowGraph, tbb::flow::unlimited, [](const Image &image)
    {
      return image.getIndexOfBrightness(image.getMinBrightness());
    });

  // Поиск заданного в командной строке значения яркости
  tbb::flow::function_node<Image, std::vector<std::pair<unsigned int, unsigned int> > > searchSpecifiedValueNode(flowGraph, tbb::flow::unlimited, [&opt](const Image &image)
    {
      return image.getIndexOfBrightness(opt.getFindValue());
    });

  // Сбор всех данных перед следующим этапом
  using HighlightArgs = tbb::flow::tuple<Image, std::vector<std::pair<unsigned int, unsigned int> >, std::vector<std::pair<unsigned int, unsigned int> >, std::vector<std::pair<unsigned int, unsigned int> > >;
  tbb::flow::join_node<HighlightArgs> joinNode(flowGraph);

  // Подсветка всех найденных объектов
  tbb::flow::function_node<HighlightArgs, Image> highlightNode(flowGraph, tbb::flow::unlimited, [&opt](HighlightArgs highlightArgs)
    {
      Image image = tbb::flow::get<0>(highlightArgs);
      image.highlightPixels(tbb::flow::get<1>(highlightArgs), image.getMaxBrightness());
      image.highlightPixels(tbb::flow::get<2>(highlightArgs), image.getMinBrightness());
      image.highlightPixels(tbb::flow::get<3>(highlightArgs), opt.getFindValue());
      return image;
    });

  // Инвертирование изображения
  tbb::flow::function_node<Image, Image> invertNode(flowGraph, tbb::flow::unlimited, [](Image image)
    {
      image.invert();
      return image;
    });

  // Поиск средней яркости изображения
  tbb::flow::function_node<Image, double> searchAvgNode(flowGraph, tbb::flow::unlimited, [](const Image &image)
    {
      return image.getAverageBrightness();
    });

  // Сбор всех данных перед финальным этапом
  tbb::flow::join_node<tbb::flow::tuple<Image, double> > finalJoinNode(flowGraph);

  // Вывод информации
  tbb::flow::function_node<tbb::flow::tuple<Image, double>, tbb::flow::continue_msg> outputNode(flowGraph, tbb::flow::serial, [&opt, &fout](const tbb::flow::tuple<Image, double> &info)
    {
      fout << tbb::flow::get<1>(info) << std::endl;
      if (opt.IsDebug())
      {
        Image image = tbb::flow::get<0>(info);
        std::cout << std::endl << "Final Image" << std::endl;
        for (unsigned int i = 0; i < image.getHeight(); i++)
        {
          for (unsigned int j = 0; j < image.getWidth(); j++)
          {
            std::cout << (int)image.at(i, j) << "   ";
          }
          std::cout << std::endl;
        }
      }
      return tbb::flow::continue_msg();
    });

  tbb::flow::make_edge(sourceNode, limiterNode);
  tbb::flow::make_edge(limiterNode, searchMaxValueNode);
  tbb::flow::make_edge(limiterNode, searchMinValueNode);
  tbb::flow::make_edge(limiterNode, searchSpecifiedValueNode);
  tbb::flow::make_edge(limiterNode, tbb::flow::input_port<0>(joinNode));
  tbb::flow::make_edge(searchMaxValueNode, tbb::flow::input_port<1>(joinNode));
  tbb::flow::make_edge(searchMinValueNode, tbb::flow::input_port<2>(joinNode));
  tbb::flow::make_edge(searchSpecifiedValueNode, tbb::flow::input_port<3>(joinNode));
  tbb::flow::make_edge(joinNode, highlightNode);
  tbb::flow::make_edge(highlightNode, invertNode);
  tbb::flow::make_edge(highlightNode, searchAvgNode);
  tbb::flow::make_edge(invertNode, tbb::flow::input_port<0>(finalJoinNode));
  tbb::flow::make_edge(searchAvgNode, tbb::flow::input_port<1>(finalJoinNode));
  tbb::flow::make_edge(finalJoinNode, outputNode);
  tbb::flow::make_edge(outputNode, limiterNode.decrement);

  sourceNode.activate();

  flowGraph.wait_for_all();

  fout.close();

  return 0;
}
