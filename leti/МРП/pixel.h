+#ifndef PIXEL_H
+#define PIXEL_H
+
+template <class T>
+struct pixel {
+    T value;
+    int hInd;
+    int wInd;
+};
+
+#endif // PIXEL_H