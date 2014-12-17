package org.sen.lib;

//import android.app.Activity;
import android.util.DisplayMetrics;
import android.view.Display;
import android.view.WindowManager;
import android.content.res.Configuration;
import android.content.SharedPreferences;
import android.util.Log;
public class SENHandler {

  private static SENActivity s_activity = null;
  private static boolean  s_bInitialized = false;
  private static SENMusic s_music;
  private static SENSound s_sound;

  public static void init(final SENActivity activity) {
    if (s_bInitialized) return;
    
    s_activity = activity;
    s_music = new SENMusic(activity);
    s_sound = new SENSound(activity);
    
    
    s_bInitialized = true;
  }

  public static void doExit()
  {
	  s_activity.doExit();
  }
  
  public static int getDPI()
  {
    if (s_activity != null)
    {
      DisplayMetrics metrics = new DisplayMetrics();
      WindowManager wm = s_activity.getWindowManager();
      if (wm != null)
      {
        Display d = wm.getDefaultDisplay();
        if (d != null)
        {
          d.getMetrics(metrics);
          return (int)(metrics.density*160.0f);
        }
      }
    }
    return -1;
  }
  
  public static int getSizeIdent() {
    int screenLayout = s_activity.getResources().getConfiguration().screenLayout;
    screenLayout &= Configuration.SCREENLAYOUT_SIZE_MASK;

    switch (screenLayout) {
    case Configuration.SCREENLAYOUT_SIZE_SMALL:
        return 0;
    case Configuration.SCREENLAYOUT_SIZE_NORMAL:
        return 1;
    case Configuration.SCREENLAYOUT_SIZE_LARGE:
        return 2;
    case 4: 
        return 3;
    default:
        return 0;
    }
  }  
  
  public static void onEnterBackground(){
	    s_music.onEnterBackground();
	    s_sound.onEnterBackground();
  }

  public static void onEnterForeground(){
	    s_music.onEnterForeground();
	    s_sound.onEnterForeground();
	  
  }

  public static void end() {
      s_music.end();
      s_sound.end();
  }
  
  public static void preloadMusic(final String path) {
      s_music.preload(path);
  }

  public static void playMusic(final String path, final boolean loop) {
      s_music.play(path, loop);
  }

  public static void resumeMusic() {
	  s_music.resume();
  }

  public static void pauseMusic() {
	  s_music.pause();
  }

  public static void stopMusic() {
	  s_music.stop();
  }

  public static void rewindMusic() {
	  s_music.rewind();
  }

  public static boolean isMusicPlaying() {
	 return  s_music.isPlaying();
  }

  public static float getMusicVol() {
      return s_music.getVolume();
  }

  public static void setMusicVol(final float volume) {
      s_music.setVolume(volume);
  }
  
  public static void preloadSound(final String path) {
      s_sound.preload(path);
  }

  public static int playSound(final String path, 
		                      final boolean isLoop, 
		                      final float pitch, 
		                      final float pan, 
		                      final float gain) 
  {
      return s_sound.play(path, isLoop, pitch, pan, gain);
  }

  public static void resumeSound(final int soundId) {
      s_sound.resume(soundId);
  }

  public static void pauseSound(final int soundId) {
      s_sound.pause(soundId);
  }

  public static void stopSound(final int soundId) {
      s_sound.stop(soundId);
  }

  public static float getSoundsVol() {
      return s_sound.getVolume();
  }

  public static void setSoundsVol(final float volume) {
      s_sound.setVolume(volume);
  }

  public static void unloadSound(final String path) {
      s_sound.unload(path);
  }

  public static void pauseAllSounds() {
      s_sound.pauseAll();
  }

  public static void resumeAllSounds() {
      s_sound.resumeAll();
  }

  public static void stopAllSounds() {
      s_sound.stopAll();
  }
  

  private static final String SENPREFS = "senPrefs";
  
  public static void setKeyString(String key, String value) {
      SharedPreferences settings = s_activity.getSharedPreferences(SENPREFS, 0);
      SharedPreferences.Editor editor = settings.edit();
      editor.putString(key, value);
	  Log.d("SEN:Android:Settings", "--->  "+ value); 
      editor.commit();
  }
  public static String getKeyString(String key, String defaultValue) {
      SharedPreferences settings = s_activity.getSharedPreferences(SENPREFS, 0);
	  Log.d("SEN:Android:Settings", "<---  "+ settings.getString(key, defaultValue)); 
      return settings.getString(key, defaultValue);
  }
  
}
