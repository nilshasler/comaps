package app.organicmaps.sdk.routing;

import androidx.annotation.NonNull;
import app.organicmaps.sdk.Router;
import app.organicmaps.sdk.settings.RoadType;
import app.organicmaps.sdk.settings.TransportSubMode;
import app.organicmaps.sdk.util.log.Logger;
import java.util.HashSet;
import java.util.Set;

public final class RoutingOptions
{
  private static final String TAG = RoutingOptions.class.getSimpleName();

  public static void addOption(@NonNull RoadType roadType, @NonNull Router router)
  {
    nativeAddOption(roadType.ordinal(), router.ordinal());
  }

  public static void removeOption(@NonNull RoadType roadType, @NonNull Router router)
  {
    nativeRemoveOption(roadType.ordinal(), router.ordinal());
  }

  public static boolean hasOption(@NonNull RoadType roadType, @NonNull Router router)
  {
    if (router == Router.Ruler)
      return false;
    return nativeHasOption(roadType.ordinal(), router.ordinal());
  }

  public static TransportSubMode getTransportSubMode(@NonNull Router router)
  {
    if (router == Router.Ruler)
      return TransportSubMode.RulerDefault;
    if (router == Router.Vehicle)
      return TransportSubMode.DrivingDefault;

    int mode = nativeGetTransportSubMode(router.ordinal());

    if (router == Router.Bicycle)
    {
      switch (mode)
      {
      case 0: return TransportSubMode.CyclingDefault;
      case 1 << 11: return TransportSubMode.CyclingRoad;
      case 2 << 11: return TransportSubMode.CyclingGravel;
      case 3 << 11: return TransportSubMode.CyclingMountainBike;
      }
    }
    else // walking, transit
    {
      switch (mode)
      {
      case 0: return TransportSubMode.WalkingDefault;
      case 1 << 11: return TransportSubMode.WalkingHiking;
      case 2 << 11: return TransportSubMode.WalkingHardHiking;
      case 3 << 11: return TransportSubMode.WalkingStrolling;
      }
    }
  }

  public static void setTransportSubMode(@NonNull TransportSubMode mode)
  {
    int m;
    int router = -1;
    switch (mode)
    {
    default:
    case CyclingDefault: m = 0; router = 1; break;
    case CyclingRoad: m = 1 << 11; router = 1; break;
    case CyclingGravel: m = 2 << 11; router = 1; break;
    case CyclingMountainBike: m = 3 << 11; router = 1; break;
    case WalkingDefault: m = 0; router = 0; break;
    case WalkingHiking: m = 1 << 11; router = 0; break;
    case WalkingHardHiking: m = 2 << 11; router = 0; break;
    case WalkingStrolling: m = 3 << 11; router = 0; break;
    }
    if (router != -1)
      nativeSetTransportSubMode(m, router);
  }

  public static boolean hasAnyOptions(@NonNull Router router)
  {
    for (RoadType each : RoadType.values())
    {
      if (hasOption(each, router))
        return true;
    }
    return false;
  }

  @NonNull
  public static Set<RoadType> getActiveRoadTypes(@NonNull Router router)
  {
    Set<RoadType> roadTypes = new HashSet<>();
    for (RoadType each : RoadType.values())
    {
      if (hasOption(each, router))
        roadTypes.add(each);
    }
    return roadTypes;
  }

  private RoutingOptions() throws IllegalAccessException
  {
    throw new IllegalAccessException("RoutingOptions is a utility class and should not be instantiated");
  }
  private static native void nativeAddOption(int option, int vehicle);

  private static native void nativeRemoveOption(int option, int vehicle);

  private static native boolean nativeHasOption(int option, int vehicle);
  
  private static native int nativeGetTransportSubMode(int vehicle);
  
  private static native void nativeSetTransportSubMode(int vehicle, int mode);
}
