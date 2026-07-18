package app.organicmaps.routing;

import android.animation.Animator;
import android.animation.ValueAnimator;
import android.content.Intent;
import android.os.Bundle;
import androidx.activity.result.ActivityResultLauncher;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import app.organicmaps.MwmActivity;
import app.organicmaps.R;
import app.organicmaps.util.UiUtils;
import app.organicmaps.sdk.util.log.Logger;

public class RoutingPlanInplaceController extends RoutingPlanController
{
  @NonNull
  private final RoutingPlanListener mRoutingPlanListener;

  @Nullable
  private Animator mAnimator;

  public RoutingPlanInplaceController(@NonNull MwmActivity activity,
                                      ActivityResultLauncher<Intent> startDrivingOptionsForResult,
                                      @NonNull RoutingPlanListener routingPlanListener,
                                      @NonNull RoutingBottomMenuListener listener)
  {
    super(activity.findViewById(R.id.routing_plan_frame), activity, startDrivingOptionsForResult, routingPlanListener,
          listener);
    mRoutingPlanListener = routingPlanListener;
  }

  private static final String TAG = RoutingPlanInplaceController.class.getSimpleName();
  public void show(final boolean show)
  {
    Logger.d(TAG, "show=" + show + " mAnimator=" + mAnimator);
    if (mAnimator != null)
    {
      mAnimator.cancel();
      mAnimator.removeAllListeners();
    }
    if (show)
      UiUtils.show(getFrame());

    mAnimator = animateFrame(show, () -> {
      if (!show)
        UiUtils.hide(getFrame());
    });
  }

  public void onSaveState(@NonNull Bundle outState)
  {
    saveRoutingPanelState(outState);
  }

  public void restoreState(@NonNull Bundle state)
  {
    restoreRoutingPanelState(state);
  }

  @Nullable
  private ValueAnimator animateFrame(final boolean show, final @Nullable Runnable completion)
  {
    Logger.d(TAG, "animateFrame show=" + show + " height=" + checkFrameHeight() +" vis=" + UiUtils.isVisible(getFrame()));
    if (!checkFrameHeight())
    {
      getFrame().post(() -> animateFrame(show, completion));
      return null;
    }

    mRoutingPlanListener.onRoutingPlanStartAnimate(show);

    ValueAnimator animator =
        ValueAnimator.ofFloat(show ? -getFrame().getHeight() : 0, show ? 0 : -getFrame().getHeight());
    animator.addUpdateListener(animation -> getFrame().setTranslationY((Float) animation.getAnimatedValue()));
    animator.addListener(new UiUtils.SimpleAnimatorListener() {
      @Override
      public void onAnimationEnd(Animator animation)
      {
        if (completion != null)
          completion.run();
      }
    });
    animator.setDuration(mAnimToggle);
    animator.start();
    return animator;
  }

  public interface RoutingPlanListener
  {
    void onRoutingPlanStartAnimate(boolean show);
  }
}
