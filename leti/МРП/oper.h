+#ifndef OPER_H
+#define OPER_H
+#include <string>
+
+struct oper {
+    int height;
+    int width;
+    int spec; //заданна€ €ркость
+    int imagesLimit;
+    bool logging;
+    std::string filename;
+};
+
+bool parse(int argc, char** argv, oper& args);
+
+#endif // OPER_H