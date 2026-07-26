package app.organicmaps.routing;

import android.content.Context;
import android.graphics.Canvas;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout; // Match your root layout type
import app.organicmaps.sdk.util.log.Logger;


public class SpyView extends LinearLayout {

    public SpyView(Context context) {
        super(context);
    }

    public SpyView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public SpyView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    @Override
    public void setVisibility(int visibility) {
        super.setVisibility(visibility);
        String visibilityString;
        switch (visibility) {
            case View.VISIBLE:      visibilityString = "VISIBLE"; break;
            case View.INVISIBLE:    visibilityString = "INVISIBLE"; break;
            case View.GONE:         visibilityString = "GONE"; break;
            default:                visibilityString = "UNKNOWN"; break;
        }

        // Passing a new Exception() forces Logcat to print the full execution stack trace
        Logger.d("SpyView", "Root view visibility changed to " + visibilityString, new Exception("Visibility Stacktrace"));
    }

    @Override
    protected void onDetachedFromWindow() {
        super.onDetachedFromWindow();
        Logger.d("SpyView", "Root view was DETACHED from window!", new Exception("Detachment Stacktrace"));
    }

  @Override
  protected void onDraw(Canvas canvas) {
      super.onDraw(canvas);
      
      // Check if the view actually has dimensions
      int width = getWidth();
      int height = getHeight();
      float alpha = getAlpha();
  
      Logger.d("SpyView", "onDraw called. Dimensions: " + width + "x" + height + " | Alpha: " + alpha);
  
      if (getParent() instanceof ViewGroup) {
          ViewGroup parent = (ViewGroup) getParent();
          int myIndex = parent.indexOfChild(this);
          int totalChildren = parent.getChildCount();
          
          Logger.d("SpyView", "My position in parent: " + myIndex + " out of " + totalChildren);
          
          // If myIndex is less than (totalChildren - 1), siblings are drawn AFTER (on top of) this view
          if (myIndex < totalChildren - 1) {
              for (int i = myIndex + 1; i < totalChildren; i++) {
                  View sibling = parent.getChildAt(i);
                  Logger.d("SpyView", "Sibling above me: " + sibling.getClass().getSimpleName() 
                          + " | ID: " + sibling.getId() 
                          + " | Visibility: " + sibling.getVisibility());
              }
          }
      }
  }
}
