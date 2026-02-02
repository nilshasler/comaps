package app.organicmaps.settings;

import android.content.res.XmlResourceParser;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatDelegate;
import androidx.core.os.LocaleListCompat;
import androidx.fragment.app.Fragment;

import app.organicmaps.R;
import app.organicmaps.base.BaseMwmRecyclerFragment;
import app.organicmaps.editor.LanguagesAdapter;
import app.organicmaps.sdk.editor.data.Language;

import org.xmlpull.v1.XmlPullParser;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;
import java.util.Locale;

/**
 * Fragment for selecting app UI language from available translations.
 * Uses locales defined in res/xml/locales_config.xml.
 */
public class AppLanguagesFragment extends BaseMwmRecyclerFragment<LanguagesAdapter>
    implements LanguagesAdapter.OnLanguageSelectedListener
{
  public interface Listener
  {
    void onAppLanguageSelected(Language language);
  }

  private Listener mListener;

  @NonNull
  @Override
  protected LanguagesAdapter createAdapter()
  {
    List<String> supportedLocaleTags = parseSupportedLocales();

    List<Language> languages = new ArrayList<>();
    for (String localeTag : supportedLocaleTags)
    {
      Locale locale = Locale.forLanguageTag(localeTag);
      String displayName = locale.getDisplayName(locale);
      languages.add(new Language(localeTag, displayName));
    }

    languages.sort(Comparator.comparing(lhs -> lhs.name));
    languages.add(0, createSystemDefault());

    Language currentLanguage = getCurrentAppLanguage();
    return new LanguagesAdapter(this, languages.toArray(new Language[0]), currentLanguage);
  }

  private Language createSystemDefault() {
    return new Language(Language.DEFAULT_LANG_CODE, getString(R.string.setting_value_system_default));
  }

  @Nullable
  private Language getCurrentAppLanguage()
  {
    LocaleListCompat appLocales = AppCompatDelegate.getApplicationLocales();
    if (appLocales.isEmpty())
      return createSystemDefault();
    Locale currentLocale = appLocales.get(0);
    if (currentLocale == null)
      throw new IllegalStateException("Cannot be null");
    String localeTag = currentLocale.toLanguageTag();
    String displayName = currentLocale.getDisplayName(currentLocale);
    return new Language(localeTag, displayName);
  }

  /**
   * Parse supported locales from res/xml/locales_config.xml.
   */
  private List<String> parseSupportedLocales()
  {
    List<String> locales = new ArrayList<>();
    try (XmlResourceParser parser = getResources().getXml(R.xml.locales_config))
    {
      int eventType = parser.getEventType();
      while (eventType != XmlPullParser.END_DOCUMENT)
      {
        if (eventType == XmlPullParser.START_TAG && "locale".equals(parser.getName()))
        {
          String localeName = parser.getAttributeValue(
              "http://schemas.android.com/apk/res/android", "name");
          if (localeName != null && !localeName.isEmpty())
            locales.add(localeName);
        }
        eventType = parser.next();
      }
    }
    catch (Exception e)
    {
      // Fallback: return empty list, which means no languages available
    }
    return locales;
  }

  public void setListener(Listener listener)
  {
    mListener = listener;
  }

  @Override
  public void onLanguageSelected(Language language)
  {
    Fragment parent = getParentFragment();
    if (parent instanceof Listener)
      ((Listener) parent).onAppLanguageSelected(language);
    if (mListener != null)
      mListener.onAppLanguageSelected(language);
  }
}
