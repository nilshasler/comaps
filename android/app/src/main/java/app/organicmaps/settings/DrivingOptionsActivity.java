package app.organicmaps.settings;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import app.organicmaps.base.BaseMwmFragmentActivity;
import app.organicmaps.settings.RoutingOptionsFragment;
import androidx.activity.result.ActivityResultLauncher;

public class DrivingOptionsActivity extends BaseMwmFragmentActivity
{
  public static void start(@NonNull Activity activity, ActivityResultLauncher<Intent> startDrivingOptionsForResult)
  {
    Intent intent = new Intent(activity, DrivingOptionsActivity.class);
    startDrivingOptionsForResult.launch(intent);
  }

  @Override
  protected Class<? extends Fragment> getFragmentClass()
  {
    return RoutingOptionsFragment.class;
  }
}
