package app.organicmaps.sdk.routing;

import androidx.annotation.NonNull;
import app.organicmaps.sdk.Router;
import app.organicmaps.sdk.settings.RoadType;
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
      return false;
    int mode = nativeGetTransportSubMode(router.ordinal());
    switch (mode)
    {
    case 0: return TransportSubMode.CyclingDefault;
    case 1 << 11: return TransportSubMode.CyclingRoad;
    case 2 << 11: return TransportSubMode.CyclingGravel;
    case 3 << 11: return TransportSubMode.CyclingMountainBike;
    }
  }

  public static void setTransportSubMode(@NonNull TransportSubMode mode, @NonNull Router router)
  {
    if (router == Router.Ruler)
      return false;
    int m;
    switch (mode)
    {
    default:
    case TransportSubMode.CyclingDefault: m = 0; break;
    case TransportSubMode.CyclingRoad: m = 1 << 11; break;
    case TransportSubMode.CyclingGravel: m = 2 << 11; break;
    case TransportSubMode.CyclingMountainBike: m = 3 << 11; break;
    }
    nativeSetTransportSubMode(m, router.ordinal());
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
