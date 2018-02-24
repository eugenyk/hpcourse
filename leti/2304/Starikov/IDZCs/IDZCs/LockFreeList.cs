using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IDZCs
{
    class LockFreeList{
        public Item Head;   
        public void Add(string text){
            var item = new Item() {Next = null, Text = text };
            if (Head == null){
                if (item == System.Threading.Interlocked.CompareExchange(ref Head, item, null)){
                    return;
                }
            }
            var prev = Head;
            while (true){
                if (prev.Next == null){
                    if (item == System.Threading.Interlocked.CompareExchange(ref prev.Next, item, null)){
                        return;
                    }
                }
                prev = prev.Next;
            }
        }
    }

    class Item{
        public Item Next;
        public string Text;
    }
}
