#ifndef OPTIONS_H
#define OPTIONS_H

#include <iostream>

/**
 * @brief The Options class Класс опций для обработки аргументов командной строки
 */
class Options
{
public:
  /**
   * @brief Конструктор по умолчанию класса Options
   */
  Options();

  /**
   * @brief Разбор аргументов командной строки
   * @param argc Количество аргументов
   * @param argv Массив аргументов
   * @return Статус: 1 - нет ошибок, 0 - есть ошибки
   */
  bool parseArgs(int argc, char *argv[]);

  /**
   * @brief Вывод информации по приложению в поток
   * @param out Выходной поток
   */
  void printUsage(std::ostream& out);

  unsigned char getFindValue() const;

  int getMaxParallelFlow() const;

  int getAmountImages() const;

  std::string getLogFilePath() const;

  unsigned int getImageHeight() const;

  unsigned int getImageWidth() const;

  bool IsDebug() const;

private:
  unsigned char m_findBrightness; /// Искомое значение яркости
  int m_maxParallelFlow; /// Предел одновременно обрабатываемых приложением изображений
  int m_amountImages; /// Общее количество изображений
  std::string m_logFilePath; /// Путь до файла журнала яркостей
  unsigned int m_imageHeight; /// Высота изображений
  unsigned int m_imageWidth; /// Ширина изображений
  bool m_isDebug; /// Флаг вывода дополнительной информации
  bool m_isError; /// Флаг ошибки
};









#endif // OPTIONS_H
