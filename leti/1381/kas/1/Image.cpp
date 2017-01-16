#include "Image.h"

#include <iostream>

Image::Image(unsigned int height, unsigned int width)
{
  m_height = height;
  m_width = width;
  unsigned int size = height*width;

  for (unsigned int i = 0; i < size; i++)
  {
    m_image.push_back(rand() % 256);
  }
}

Image::Image()
{
  m_height = 0;
  m_width = 0;
}

unsigned char Image::at(unsigned int indexHeight, unsigned int indexWidth) const
{
  return m_image[indexHeight*m_width + indexWidth];
}

void Image::setBrightness(unsigned int indexHeight, unsigned int indexWidth, unsigned char brightness)
{
  m_image[indexHeight*m_width + indexWidth] = brightness;
}

void Image::highlightPixels(std::vector<std::pair<unsigned int, unsigned int> > indexes, unsigned char brightness)
{
  for (std::vector<std::pair<unsigned int, unsigned int> >::iterator it = indexes.begin(); it != indexes.end(); it++)
  {
    for (int i = it->first == 0 ? 0 : it->first - 1; i <= it->first + 1 && i < m_height; i++)
    {
      for (int j = it->second == 0 ? 0 : it->second - 1; j <= it->second + 1 && j < m_width; j++)
      {
        setBrightness(i, j, brightness);
      }
    }
  }
}

unsigned char Image::getMaxBrightness() const
{
  unsigned char max = m_image[0];
  for (unsigned int i = 0; i < m_height*m_width; i++)
  {
    if (max < m_image[i])
    {
      max = m_image[i];
    }
  }
  return max;
}

unsigned char Image::getMinBrightness() const
{
  unsigned char min = m_image[0];
  for (unsigned int i = 0; i < m_height*m_width; i++)
  {
    if (min > m_image[i])
    {
      min = m_image[i];
    }
  }
  return min;
}

std::vector<std::pair<unsigned int, unsigned int> > Image::getIndexOfBrightness(unsigned char brightness) const
{
  std::vector<std::pair<unsigned int, unsigned int> > indexes;
  for (unsigned int i = 0; i < m_height*m_width; i++)
  {
    if (brightness == m_image[i])
    {
      indexes.push_back(std::pair<unsigned int, unsigned int>(i / m_width, i % m_width));
    }
  }
  return indexes;
}

double Image::getAverageBrightness() const
{
  double avg = 0.0;
  unsigned int size = m_height*m_width;
  for (unsigned int i = 0; i < size; i++)
  {
    avg += (double)m_image[i] / (double)size;
  }
  return avg;
}

void Image::invert()
{
  for (unsigned int i = 0; i < m_height*m_width; i++)
  {
    m_image[i] = 255 - m_image[i];
  }
}

unsigned int Image::getHeight() const
{
  return m_height;
}

unsigned int Image::getWidth() const
{
  return m_width;
}

void Image::setHeight(unsigned int height)
{
  m_height = height;
}

void Image::setWidth(unsigned int width)
{
  m_width = width;
}
