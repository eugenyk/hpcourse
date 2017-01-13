#include "main_window.h"
#include <QApplication>
#include "chat_client.h"
#include "message/message.pb.h"
#include "iostream"
#include <sys/types.h>
#include <netinet/in.h>
#include <stdint.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
using namespace std;
using namespace google::protobuf::io;
int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    // ChatClient chatClient;
    /*/
ChatMessage msg;
msg.set_sender("Sender");
msg.set_text("Message");
msg.set_data("Date");

static uint8_t buffer[4];
  for (uint32_t i = 0; i < 0xf; ++i) {
    uint8_t* end = CodedOutputStream::WriteVarint32ToArray(i, buffer);
    uint32_t value;
   cout<<"Uint_32:"<<i<<"--> Uint8"<<end<<"\n";
   cout<<"Value:"<<value<<"\n";
    //CHECK_EQ(i, value);
  }
  /*/
    /*/
    static quint8 buffer[4];
    string buf;
    CodedOutputStream* coded_output = new CodedOutputStream(buf);

    coded_output->WriteVarint32(255);
    /*/

    quint8 buf[4];
/*/
    QByteArray ba;
    QDataStream outBA(&ba,QIODevice::WriteOnly);

    ZeroCopyOutputStream* raw_output = new ArrayOutputStream(buf,sizeof(buf));
    CodedOutputStream* coded_output = new CodedOutputStream(raw_output);
    coded_output->WriteVarint32(150);

    int countByte=coded_output->ByteCount();
    for(int i=0;i<countByte;i++){
        outBA<<buf[i];
    }
    cout<<"Encode_length:"<<ba.data()<<" "<<coded_output->ByteCount()<<endl;
    qDebug()<<"Ba:"<<ba<<" "<<ba.size()<<endl;
//*********************************
    QByteArray ba1=ba;
    quint32 length;
    quint8 bufDecod[4];

    int countByteBa=ba1.size();
    for(int i=0;i<countByteBa;i++){
        bufDecod[i]=ba1[i];
    }

    ZeroCopyInputStream* raw_input = new ArrayInputStream(bufDecod,sizeof(bufDecod));
    CodedInputStream *coded_input = new CodedInputStream(raw_input);
    coded_input->ReadVarint32(&length);
    qDebug()<<"Length:"<<length;
/*/


    return a.exec();
}
