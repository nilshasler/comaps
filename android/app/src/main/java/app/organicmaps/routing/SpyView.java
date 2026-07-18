package app.organicmaps.routing;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import androidx.linearlayout.widget.LinearLayout; // Match your root layout type

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
        Log.d("SpyView", "Root view visibility changed to " + visibilityString, new Exception("Visibility Stacktrace"));
    }

    @Override
    protected void onDetachedFromWindow() {
        super.onDetachedFromWindow();
        Log.d("SpyView", "Root view was DETACHED from window!", new Exception("Detachment Stacktrace"));
    }
}
