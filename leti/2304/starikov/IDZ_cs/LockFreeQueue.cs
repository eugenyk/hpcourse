using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Tutorial;

namespace IDZCs
{
    static class LockFreeQueue{
        public static Item Head;   
        public static void Push(Message message){
            var item = new Item() {Next = null, Message = message };  
           if (Head == null){
                if (Head == System.Threading.Interlocked.CompareExchange(ref Head, item, null))return;                
            }
            var prev = Head;
            while (true){
               if (prev != null && prev.Next == null){
                   if (prev.Next == System.Threading.Interlocked.CompareExchange(ref prev.Next, item, null)){
                       return;
                   }
               }
               prev = prev.Next;
           }
        }

        public static Message Pop(){
            while (true){
                var item = Head;
                if (Head == null){
                    return null;
                }
                if (item.Next == null){
                    if (Head == System.Threading.Interlocked.CompareExchange(ref Head, null, item))
                        return item.Message;
                }
                else{
                    if (Head == System.Threading.Interlocked.CompareExchange(ref Head, Head.Next, item))
                        return item.Message;
                }
            }
        }
    }

    class Item{
        public Item Next;
        public Message Message;
    }
}
