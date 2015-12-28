package com.example.lightwave.messageclient.Views;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.RectF;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;

/**
 * Created by lightwave on 16.11.15.
 */
public class ConnectionIndicator extends View {
    Paint paint;
    public ConnectionIndicator(Context context, AttributeSet set, int style) {
        super(context, set, style);
        initValues();

    }
    public ConnectionIndicator(Context context) {
        super(context);
        initValues();
    }

    public ConnectionIndicator(Context context, AttributeSet attrs) {
        super(context, attrs); // This should be first line of constructor
        initValues();
    }

    void initValues() {
        paint = new Paint();
        paint.setColor(Color.RED);
        paint.setStrokeWidth(3);
    }

    public void setPaint(Paint nPaint) {
        paint = nPaint;
    }


    @Override
    protected void onDraw(Canvas canvas) {
        Log.e("MessageClient", "Drawing indicator");
        canvas.drawColor(paint.getColor());
        canvas.drawText("Not connected",(float)10.0, (float)10.0, new Paint(Color.WHITE));

    }
}
