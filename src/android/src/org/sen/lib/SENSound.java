package org.sen.lib;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map.Entry;
import java.util.concurrent.Semaphore;

import android.content.Context;
import android.media.AudioManager;
import android.media.SoundPool;
import android.util.Log;
import android.os.Build;

public class SENSound {
  private final Context m_context;
  private SoundPool     m_pool;
  private float         m_leftVol;
  private float         m_rightVol;
  private Semaphore     m_semaphore;
  private int           m_stream_id;


  private final HashMap<String, ArrayList<Integer>> m_map_pathIds = new HashMap<String, ArrayList<Integer>>();
  private final HashMap<String, Integer>            m_map_pathId  = new HashMap<String, Integer>();
  
  
  public class SoundInfo {
      public int soundID;
      public boolean isLoop;
      public float pitch;
      public float pan;
      public float gain;
      public String path;

      public SoundInfo(String path, int soundId, boolean isLoop,
                       float pitch, float pan, float gain) 
      {
          this.path = path;
          this.soundID = soundId;
          this.isLoop = isLoop;
          this.pitch = pitch;
          this.pan = pan;
          this.gain = gain;
      }
  }

  private final ArrayList<SoundInfo>  m_playList    = new ArrayList<SoundInfo>();
  
  public SENSound (final Context c) {
    m_context = c;
    init();
  }

  
  public void onEnterBackground(){
	  m_pool.autoPause();
  }
    
  public void onEnterForeground(){
	  m_pool.autoResume();
  }
    
    
  private void init() {
    m_pool = new SoundPool(
    		Build.MODEL.indexOf("GT-I9100") != -1 ? 3 : 5, 
    		AudioManager.STREAM_MUSIC, 
    		5);
    m_pool.setOnLoadCompleteListener(new OnLoadCompletedListener());
	
    m_leftVol  = 0.5f;
	m_rightVol = 0.5f;
	
	m_semaphore = new Semaphore(0, true);
  }
  
  public int preload(final String pPath) {
      Integer soundID = m_map_pathId.get(pPath);
      if (soundID == null) {
          soundID = createSoundIDFromAsset(pPath);
          if (soundID != -1)
              m_map_pathId.put(pPath, soundID);
      }
      return soundID;
  }

  public void unload(final String pPath) {
      final ArrayList<Integer> streamIDs = m_map_pathIds.get(pPath);
      
      if (streamIDs != null)
          for (final Integer steamID : streamIDs) 
              m_pool.stop(steamID);

      m_map_pathIds.remove(pPath);
      final Integer soundID = m_map_pathId.get(pPath);
      if(soundID != null){
          m_pool.unload(soundID);
          m_map_pathId.remove(pPath);
      }
  }

  public int play(final String pPath, 
		          final boolean pLoop, 
		          float pitch, 
		          float pan, 
		          float gain)
  {
      Integer soundID = m_map_pathId.get(pPath);
      int streamID = -1;

      if (soundID != null) 
          streamID = doPlayEffect(pPath, soundID.intValue(), pLoop, pitch, pan, gain);
      else {
          soundID = preload(pPath);
          if (soundID == -1) 
              return -1;
          
          synchronized(m_pool) {
              m_playList.add(new SoundInfo(pPath, 
            		                       soundID.intValue(), 
            		                       pLoop,
                                           pitch, 
                                           pan, 
                                           gain));

              try {
                  m_semaphore.acquire();
                  streamID = m_stream_id;
              } catch(Exception e) {
                  return -1;
              }
          }
      }
      return streamID;
  }

  public void stop(final int steamID) {
      m_pool.stop(steamID);
      for (final String pPath : m_map_pathIds.keySet()) 
          if (m_map_pathIds.get(pPath).contains(steamID)) {
              m_map_pathIds.get(pPath).remove(m_map_pathIds.get(pPath).indexOf(steamID));
              break;
          }
  }

  public void pause(final int steamID) {
      m_pool.pause(steamID);
  }

  public void resume(final int steamID) {
      m_pool.resume(steamID);
  }

  public void pauseAll() {
	  if (m_map_pathIds.isEmpty()) return;
      final Iterator<Entry<String, ArrayList<Integer>>> iter = m_map_pathIds.entrySet().iterator();
      while (iter.hasNext()) {
          final Entry<String, ArrayList<Integer>> entry = iter.next();
          for (final int steamID : entry.getValue()) 
              m_pool.pause(steamID);
      }
  }

  public void resumeAll() {
	  if (m_map_pathIds.isEmpty()) return;
	  
      final Iterator<Entry<String, ArrayList<Integer>>> iter = m_map_pathIds.entrySet().iterator();
      while (iter.hasNext()) {
          final Entry<String, ArrayList<Integer>> entry = iter.next();
          for (final int steamID : entry.getValue()) 
              m_pool.resume(steamID);
      }
  }

  
  public void stopAll() {
	  if (m_map_pathIds.isEmpty()) return;
	  
      final Iterator<Entry<String, ArrayList<Integer>>> iter = m_map_pathIds.entrySet().iterator();
      while (iter.hasNext()) {
          final Entry<String, ArrayList<Integer>> entry = iter.next();
          for (final int steamID : entry.getValue()) 
              m_pool.stop(steamID);
      }
      m_map_pathIds.clear();
  }

  public float getVolume() {
      return (m_leftVol + m_rightVol) / 2;
  }

  public void setVolume(float pVolume) {
      if (pVolume < 0) {
          pVolume = 0;
      }
      if (pVolume > 1) {
          pVolume = 1;
      }

      m_leftVol = m_rightVol = pVolume;

	  if (m_map_pathIds.isEmpty()) return;

	  final Iterator<Entry<String, ArrayList<Integer>>> iter = m_map_pathIds.entrySet().iterator();
      while (iter.hasNext()) {
          final Entry<String, ArrayList<Integer>> entry = iter.next();
          for (final int steamID : entry.getValue()) 
              m_pool.setVolume(steamID, m_leftVol, m_rightVol);
      }
  }

  public void end() {
      m_pool.release();
      m_map_pathIds.clear();
      m_map_pathId.clear();
      m_playList.clear();

      m_leftVol = 0.5f;
      m_rightVol = 0.5f;

      init();
  }

  public int createSoundIDFromAsset(final String pPath) {
      int soundID = -1;

      try {
        soundID = pPath.startsWith("/") ?  
        		  m_pool.load(pPath, 0) : 
        	      m_pool.load(m_context.getAssets().openFd(pPath), 0);
        		  
      } catch (final Exception e) {
          soundID = -1;
          Log.e("SEN:Android:Sounds", "Error: " + e.getMessage(), e);
      }

      if (soundID == 0) soundID = -1;
      return soundID;
  }

  
  public class OnLoadCompletedListener implements SoundPool.OnLoadCompleteListener {
      @Override
      public void onLoadComplete(SoundPool soundPool, int sampleId, int status) {
	      if (status == 0)
	          for ( SoundInfo info : m_playList) 
	              if (sampleId == info.soundID) {
	                  m_stream_id = doPlayEffect(info.path, 
	                		                     info.soundID, 
	                		                     info.isLoop, 
	                		                     info.pitch, 
	                		                     info.pan, 
	                		                     info.gain);
	                  m_playList.remove(info);
	                  break;
	              }
	       else 
	    	  m_stream_id = -1;
	
	      m_semaphore.release();
	  }
  }

  private int doPlayEffect(final String pPath, 
		                   final int soundId, 
		                   final boolean pLoop, 
		                   float pitch, 
		                   float pan, 
		                   float gain) 
  {
      float leftVolume = m_leftVol * gain * (1.0f - clamp(pan, 0.0f, 1.0f));
      float rightVolume = m_rightVol * gain * (1.0f - clamp(-pan, 0.0f, 1.0f));
      float soundRate = clamp(pitch, 0.5f, 2.0f);

      int streamID = m_pool.play(soundId, 
    		                     clamp(leftVolume, 0.0f, 1.0f), 
    		                     clamp(rightVolume, 0.0f, 1.0f), 
    		                     1, 
    		                     pLoop ? -1 : 0, 
    		                     soundRate);


      ArrayList<Integer> streamIDs = m_map_pathIds.get(pPath);
      if (streamIDs == null) {
          streamIDs = new ArrayList<Integer>();
          m_map_pathIds.put(pPath, streamIDs);
      }
      streamIDs.add(streamID);

      return streamID;
  }
  
  private float clamp(float value, float min, float max) {
      return Math.max(min, (Math.min(value, max)));
  }
}
