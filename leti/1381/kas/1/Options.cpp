#include "Options.h"

#include <cstring>
#include <cstdlib>

Options::Options()
{
  m_findBrightness = 128;
  m_maxParallelFlow = 4;
  m_amountImages = 10;
  m_logFilePath = "./brightness.log";
  m_imageHeight = 64;
  m_imageWidth = 64;
  m_isDebug = false;
  m_isError = false;
}

bool Options::parseArgs(int argc, char *argv[])
{
  if (argc == 1)
    return false;

  for (int i = 1; i < argc; i++)
  {
    if (!std::strcmp(argv[i], "-b") && i+1 < argc)
    {
      m_findBrightness = std::atoi(argv[++i]);
    }
    else if (!std::strcmp(argv[i], "-l") && i+1 < argc)
    {
      m_maxParallelFlow = std::atoi(argv[++i]);
    }
    else if (!std::strcmp(argv[i], "-f") && i+1 < argc)
    {
      m_logFilePath = argv[++i];
    }
    else if (!std::strcmp(argv[i], "-n") && i+1 < argc)
    {
      m_amountImages = std::atoi(argv[++i]);
    }
    else if (!std::strcmp(argv[i], "-h") && i+1 < argc)
    {
      m_imageHeight = std::atoi(argv[++i]);
    }
    else if (!std::strcmp(argv[i], "-w") && i+1 < argc)
    {
      m_imageWidth = std::atoi(argv[++i]);
    }
    else if (!std::strcmp(argv[i], "-d"))
    {
      m_isDebug = true;
    }
    else if (!std::strcmp(argv[i], "--help"))
    {
      m_isError = true;
    }
    else
    {
      m_isError = true;
    }
  }

  if (m_isError)
  {
    return false;
  }

  return true;
}

void Options::printUsage(std::ostream &out)
{
  out << "Usage:\n"
         "  kas_1 [options]\n\n"
         "Parallel Programing: Laboratory work #1\n\n"
         "Options:\n"
         "  -b <brightness>          = Find brightness [128]\n"
         "  -l <max_parallel_flow>   = Max of parallel flow [4]\n"
         "  -f <path_log_file>       = Path to log file with brightness [brightness.log]\n"
         "  -n <amount_images>       = Amount of images [10]\n"
         "  -h <height_images>       = Height of images [64]\n"
         "  -w <width_images>        = Width of images [64]\n"
         "  -d                       = Print debug information\n"
         "  --help                   = Help\n\n";
}

unsigned char Options::getFindValue() const
{
  return m_findBrightness;
}

int Options::getMaxParallelFlow() const
{
  return m_maxParallelFlow;
}

int Options::getAmountImages() const
{
  return m_amountImages;
}

std::string Options::getLogFilePath() const
{
  return m_logFilePath;
}

unsigned int Options::getImageHeight() const
{
  return m_imageHeight;
}

unsigned int Options::getImageWidth() const
{
  return m_imageWidth;
}

bool Options::IsDebug() const
{
  return m_isDebug;
}
