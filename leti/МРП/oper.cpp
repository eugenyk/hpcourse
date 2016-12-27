+#include "oper.h"
+#include <stdexcept>
+
+bool parse(int argc, char** argv, oper& args)
+{
+    int i = 1;
+    std::string s, s_;
+    while(i < argc) {
+        s = argv[i];
+        i++;
+        if(i == argc)
+            return false;
+        s_ = argv[i];
+        i++;
+        try {
+            if(s == "-h")
+                args.height = std::stoi(s_);
+            else if(s == "-w")
+                args.width = std::stoi(s_);
+            else if(s == "-b")
+                args.spec = std::stoi(s_);
+            else if (s == "-l")
+                args.imagesLimit = std::stoi(s_);
+            else if (s == "-f") {
+                args.logging = true;
+                if(s_ != "")
+                    args.filename = s_;
+                else
+                    return false;
+            }
+            else
+                return false;
+        } catch(std::invalid_oper& e) {
+                return false;
+        }
+    }
+    if(args.height <= 0 || args.width <= 0
+            || args.imagesLimit <= 0)
+        return false;
+    return true;
+}