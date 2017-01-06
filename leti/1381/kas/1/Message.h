#ifndef MESSAGE_H
#define MESSAGE_H

template <class T>
class Message
{
public:
  Message()
  {
    m_key = -1;
  }

  Message(int key, T data)
  {
    m_key = key;
    m_data = data;
  }

  int getKey() const
  {
    return m_key;
  }

  T getData() const
  {
    return m_data;
  }

  void setKey(int key)
  {
    m_key = key;
  }

  void setData(const T &data)
  {
    m_data = data;
  }

private:
  int m_key;
  T m_data;
};

#endif // MESSAGE_H
