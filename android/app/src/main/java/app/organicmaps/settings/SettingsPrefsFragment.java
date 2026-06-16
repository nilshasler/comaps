package app.organicmaps.settings;

<<<<<<< HEAD
=======
import static app.organicmaps.leftbutton.LeftButtonsHolder.DISABLE_BUTTON_CODE;
import static app.organicmaps.sdk.editor.data.Language.AUTO_LANG_CODE;
import static app.organicmaps.sdk.editor.data.Language.DEFAULT_LANG_CODE;
import static app.organicmaps.util.Utils.isAndroidAutoSupported;

import android.annotation.SuppressLint;
>>>>>>> 2c37e7393 ([routing] [android] options per transport type)
import android.content.Intent;
import android.os.Bundle;
import android.view.View;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.preference.Preference;

import app.organicmaps.BuildConfig;
import app.organicmaps.R;
import app.organicmaps.editor.ProfileActivity;
<<<<<<< HEAD
import app.organicmaps.help.HelpActivity;
import app.organicmaps.sdk.editor.OsmOAuth;
=======
import app.organicmaps.leftbutton.LeftButton;
import app.organicmaps.leftbutton.LeftButtonsHolder;
import app.organicmaps.sdk.Framework;
import app.organicmaps.sdk.Router;
import app.organicmaps.sdk.downloader.MapManager;
import app.organicmaps.sdk.editor.OsmOAuth;
import app.organicmaps.sdk.editor.data.Language;
import app.organicmaps.sdk.location.LocationHelper;
import app.organicmaps.sdk.routing.RoutingController;
import app.organicmaps.sdk.routing.RoutingOptions;
import app.organicmaps.sdk.search.SearchRecents;
import app.organicmaps.sdk.settings.MapLanguageCode;
import app.organicmaps.sdk.settings.UnitLocale;
import app.organicmaps.sdk.util.Config;
import app.organicmaps.sdk.util.NetworkPolicy;
import app.organicmaps.sdk.util.PowerManagment;
import app.organicmaps.sdk.util.SharedPropertiesUtils;
import app.organicmaps.sdk.util.log.LogsManager;
import app.organicmaps.util.ThemeSwitcher;
import app.organicmaps.util.Utils;
import com.google.android.material.dialog.MaterialAlertDialogBuilder;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Locale;
>>>>>>> 2c37e7393 ([routing] [android] options per transport type)

public class SettingsPrefsFragment extends BaseXmlSettingsFragment
{
  @Override
  protected int getXmlResources()
  {
    return R.xml.prefs_main;
  }

  @Override
  public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState)
  {
    super.onViewCreated(view, savedInstanceState);
<<<<<<< HEAD
=======

    initStoragePrefCallbacks();
    initMeasureUnitsPrefsCallbacks();
    initZoomPrefsCallbacks();
    initMapStylePrefsCallbacks();
    initAutoDownloadPrefsCallbacks();
    initLargeFontSizePrefsCallbacks();
    initTransliterationPrefsCallbacks();
    initAlternativeMapLanguageHandlingCallbacks();
    init3dModePrefsCallbacks();
    initPerspectivePrefsCallbacks();
    initAutoZoomPrefsCallbacks();
    initLoggingEnabledPrefsCallbacks();
    initEmulationBadStorage();
    initUseMobileDataPrefsCallbacks();
    initPowerManagementPrefsCallbacks();
    initPlayServicesPrefsCallbacks();
    initSearchPrivacyPrefsCallbacks();
    initScreenSleepEnabledPrefsCallbacks();
    initShowOnLockScreenPrefsCallbacks();
    initLeftButtonPrefs();
    initCustomMapDownloadUrlPrefsCallbacks();
    initOpenExternalLinksPrefsCallback();
    initIncognitoModePrefsCallback();
    initAndroidAutoSupportPrefsCallback();
  }

  private void initLeftButtonPrefs()
  {
    final String leftButtonPreferenceKey = getString(R.string.pref_left_button);
    final ListPreference pref = getPreference(leftButtonPreferenceKey);
    LeftButtonsHolder holder = LeftButtonsHolder.getInstance(requireContext());

    LeftButton currentButton = holder.getActiveButton();
    Collection<LeftButton> buttons = holder.getAllButtons();

    List<String> entryList = new ArrayList<>(buttons.size());
    List<String> valueList = new ArrayList<>(buttons.size());

    for (LeftButton button : buttons)
    {
      entryList.add(button.getPrefsName());
      valueList.add(button.getCode());
    }

    pref.setEntries(entryList.toArray(new CharSequence[0]));
    pref.setEntryValues(valueList.toArray(new CharSequence[0]));

    if (currentButton != null)
    {
      pref.setSummary(currentButton.getPrefsName());
      pref.setValue(currentButton.getCode());
    }
    else
    {
      pref.setSummary(R.string.pref_left_button_disable);
      pref.setValue(DISABLE_BUTTON_CODE);
    }

    pref.setOnPreferenceChangeListener((preference, newValue) -> {
      int index = pref.findIndexOfValue(newValue.toString());
      if (index >= 0)
      {
        pref.setSummary(pref.getEntries()[index]);
      }

      Intent intent = new Intent();
      intent.putExtra(leftButtonPreferenceKey, newValue.toString());

      requireActivity().setResult(-1, intent);

      return true;
    });
  }

  private void updateVoiceInstructionsPrefsSummary()
  {
    final Preference pref = getPreference(getString(R.string.pref_tts_screen));
    pref.setSummary(Config.TTS.isEnabled() ? R.string.on : R.string.off);
  }

  private void updateMapLanguageCodeSummary()
  {
    final Preference pref = getPreference(getString(R.string.pref_map_locale));
    String mapLanguageCode = MapLanguageCode.getMapLanguageCode();
    if (mapLanguageCode.equals(AUTO_LANG_CODE))
    {
      pref.setSummary(R.string.auto);
    }
    else if (mapLanguageCode.equals(DEFAULT_LANG_CODE))
    {
      pref.setSummary(R.string.pref_maplanguage_local);
    }
    else
    {
      Locale locale = new Locale(mapLanguageCode);
      pref.setSummary(locale.getDisplayLanguage());
    }
  }

  private void updateAppLanguageCodeSummary()
  {
    final Preference pref = getPreference(getString(R.string.pref_app_locale));
    pref.setVisible(true);
    LocaleListCompat appLocales = AppCompatDelegate.getApplicationLocales();
    Locale currentLocale = appLocales.get(0);
    if (appLocales.isEmpty())
    {
      pref.setSummary(getString(R.string.setting_value_system_default));
    } else if (currentLocale != null)
    {
      pref.setSummary(currentLocale.getDisplayLanguage());
    }
  }

  private void updateRoutingSettingsPrefsSummary()
  {
    final Preference pref = getPreference(getString(R.string.prefs_routing));
    Router routerType = RoutingController.get().getLastRouterType();
    pref.setSummary(RoutingOptions.hasAnyOptions(routerType) ? R.string.on : R.string.off);
  }

  private void updateProfileSettingsPrefsSummary()
  {
    final Preference pref = getPreference(getString(R.string.pref_osm_profile));
    if (OsmOAuth.isAuthorized())
    {
      final String username = OsmOAuth.getUsername();
      pref.setSummary(username);
    }
    else
      pref.setSummary(R.string.not_signed_in);
>>>>>>> 2c37e7393 ([routing] [android] options per transport type)
  }

  @Override
  public void onResume()
  {
    super.onResume();
    updateProfileSettingsPrefsSummary();
    updateAboutSummary();
  }

  @Override
  public boolean onPreferenceTreeClick(Preference preference)
  {
    final String key = preference.getKey();
    if (key != null && key.equals(getString(R.string.pref_osm_profile)))
    {
      startActivity(new Intent(requireActivity(), ProfileActivity.class));
    }
    else if (key != null && key.equals(getString(R.string.pref_about)))
    {
      startActivity(new Intent(requireActivity(), HelpActivity.class));
    }
    return super.onPreferenceTreeClick(preference);
  }

  private void updateProfileSettingsPrefsSummary()
  {
    final Preference pref = getPreference(getString(R.string.pref_osm_profile));
    if (OsmOAuth.isAuthorized())
      pref.setSummary(OsmOAuth.getUsername());
    else
      pref.setSummary(R.string.not_signed_in);
  }

  private void updateAboutSummary()
  {
    final Preference pref = getPreference(getString(R.string.pref_about));
    pref.setSummary(getString(R.string.pref_about_summary, BuildConfig.VERSION_NAME));
  }
}
