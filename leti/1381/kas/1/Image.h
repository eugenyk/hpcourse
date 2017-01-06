#ifndef IMAGE_H
#define IMAGE_H

#include <vector>

/**
 * @brief The Image class Класс для изображения
 */
class Image
{
public:
  /**
   * @brief Конструктор класса Image
   * @param height Высота
   * @param width Ширина
   */
  Image(unsigned int height, unsigned int width);

  /**
   * @brief Конструктор класса Image по умолчанию
   */
  Image();

  /**
   * @brief Возвращение яркости пикселя с указанными координатами
   * @param indexHeight Индекс пикселя по высоте
   * @param indexWidth Индекс пикселя по ширине
   * @return Яркость указанного пикселя
   */
  unsigned char at(unsigned int indexHeight, unsigned int indexWidth) const;

  /**
   * @brief Установка яркости для пикселя
   * @param indexHeight Индекс пикселя по высоте
   * @param indexWidth Индекс пикселя по ширине
   * @param brightness Значение яркости
   */
  void setBrightness(unsigned int indexHeight, unsigned int indexWidth, unsigned char brightness);

  /**
   * @brief Подсветка области пикселей
   * @param indexes Вектор пикселей, которые необходимо подсветить
   * @param brightness Значение яркости
   */
  void highlightPixels(std::vector<std::pair<unsigned int, unsigned int> > indexes, unsigned char brightness);

  /**
   * @brief Поиск максимальной яркости на изображении
   * @return Максимальная яркость
   */
  unsigned char getMaxBrightness() const;

  /**
   * @brief Поиск минимальной яркости на изображении
   * @return Минимальная яркость
   */
  unsigned char getMinBrightness() const;

  /**
   * @brief Поиск всех элементов с заданной яркостью
   * @param brightness Яркость
   * @return Вектор индексов всех найденных элементов
   */
  std::vector<std::pair<unsigned int, unsigned int> > getIndexOfBrightness(unsigned char brightness) const;

  /**
   * @brief Вычисление среднего значения яркости
   * @return Среднее значение яркости
   */
  double getAverageBrightness() const;

  /**
   * @brief Создание обратного изображения (инверсия яркости)
   */
  void invert();

  unsigned int getHeight() const;

  unsigned int getWidth() const;

  void setHeight(unsigned int height);

  void setWidth(unsigned int width);

private:
  unsigned int m_height; /// Высота изображения
  unsigned int m_width; /// Ширина изображения
  std::vector<unsigned char> m_image; /// Массив яркостей изображения
}; // Image

#endif // IMAGE_H
