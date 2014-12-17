package org.sen.lib;
import java.io.FileInputStream;

import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.media.MediaPlayer;
import android.util.Log;

public class SENMusic {
    private final Context m_context;
    private MediaPlayer   m_mediaPlayer;
    private String        m_current;
    private boolean       m_paused;
    private boolean       m_pausedByUser = false;
    private float         m_leftVol;
    private float         m_rightVol;

    public SENMusic(final Context context) {
      m_context = context;
      init();
    }
    
    
    public void preload(final String path) {
      if (m_current== null || !m_current.equals(path))
      {
		if (m_mediaPlayer != null) {
			m_mediaPlayer.release();
		}
		
		m_mediaPlayer = createMediaPlayer(path);
        m_current = path;
      }
    }

    
    public void play(final String path, final boolean loop) {
      if (m_current == null) {
        m_mediaPlayer = createMediaPlayer(path);
        m_current = path;
      } 
      else {
        if (!m_current.equals(path)) {
          if (m_mediaPlayer != null) {
            m_mediaPlayer.release();
          }
          m_mediaPlayer = createMediaPlayer(path);
          m_current = path;
        }
      }

      if (m_mediaPlayer == null) {
        Log.e("SEN:Android:Music", "media player is broken");
      } 
      else {
        m_mediaPlayer.stop();
        m_mediaPlayer.setLooping(loop);
        try {
          m_mediaPlayer.prepare();
          m_mediaPlayer.seekTo(0);
          m_mediaPlayer.start();
          m_paused = false;
        } 
        catch (final Exception e) {
          Log.e("SEN:Android:Music", "play: Error "+ e.getMessage(), e);
        }
      }
    }

    public void stop() {
      if (m_mediaPlayer != null) {
	    m_mediaPlayer.stop();
	    m_paused = false;
	  }
    }

    public void pause() {
      if (m_mediaPlayer!=null && m_mediaPlayer.isPlaying()) {
    	  m_mediaPlayer.pause();
          m_paused = true;
          m_pausedByUser = true;
      }
    }

    public void resume() {
      if (m_mediaPlayer!=null && m_paused) {
        m_mediaPlayer.start();
        m_paused = false;
        m_pausedByUser = false;
      }
    }

    public void rewind() {
	  if (m_mediaPlayer != null) {
	    m_mediaPlayer.stop();
        try {
          m_mediaPlayer.prepare();
          m_mediaPlayer.seekTo(0);
          m_mediaPlayer.start();
	
	      m_paused = false;
	     } 
        catch (final Exception e) {
          Log.e("SEN:Android:Music", "rewind: Error "+ e.getMessage(), e);
	    }
	  }
    }

    public boolean isPlaying() {
	  return m_mediaPlayer != null && m_mediaPlayer.isPlaying();
    }

    public void end() {
      if (m_mediaPlayer != null) {
        m_mediaPlayer .release();
      }
      init();
    }

    public float getVolume() {
	  return  m_mediaPlayer != null ? (m_leftVol + m_rightVol) / 2 : 0.0f;
    }

    public void setVolume(float volume) {
      if (volume < 0.0f) volume = 0.0f; else if (volume > 1.0f) volume = 1.0f; 
      m_leftVol = m_rightVol = volume;
      if (m_mediaPlayer != null) 
        m_mediaPlayer.setVolume(m_leftVol, m_rightVol);
    }
    
    public void onEnterBackground(){
      if (m_mediaPlayer!= null && m_mediaPlayer.isPlaying()) {
        m_mediaPlayer.pause();
        m_paused = true;
      }
    }
    
    public void onEnterForeground(){
      if(!m_pausedByUser){
        if (m_mediaPlayer!= null && m_paused) {
          m_mediaPlayer.start();
          m_paused = false;
        }
      }
    }
    
    
	private MediaPlayer createMediaPlayer(final String path) {
	  MediaPlayer mediaPlayer = new MediaPlayer();
	  try {
	    if (path.startsWith("/")) {
	       final FileInputStream fis = new FileInputStream(path);
	       mediaPlayer.setDataSource(fis.getFD());
	       fis.close();
	    } 
	    else {
	      final AssetFileDescriptor asset = m_context.getAssets().openFd(path);
	       mediaPlayer.setDataSource(asset.getFileDescriptor(), 
	    		                     asset.getStartOffset(), 
	    		                     asset.getLength());
	    }
	    mediaPlayer.prepare();
	    mediaPlayer.setVolume(m_leftVol, m_rightVol);
	  } 
	  catch (final Exception e) {
	    mediaPlayer = null;
	    Log.e("SEN:Android:Music", "Error: " + e.getMessage(), e);
	  }
	  return mediaPlayer;
	}
	
  private void init() {
    m_leftVol = 0.5f;
    m_rightVol = 0.5f;
    m_mediaPlayer = null;
    m_paused = false;
    m_current = null;
  }
  
}
