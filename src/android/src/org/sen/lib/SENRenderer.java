package org.sen.lib;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;
import android.opengl.GLSurfaceView.Renderer;

public class SENRenderer implements Renderer {
	
    private final static long nano_1sec = 1000000000L;
    private final static long nano_1ms  = 1000000;

   	private int     m_width;
   	private int     m_height;
   	private boolean m_bInitialized = false;
   	private long    m_lastTick;
   	
    private static int  maxFps = 60;   
    private static long frameTime =  (long) (1.0 / maxFps * nano_1sec);
	
	 
    public static void setMaxFps(final int _maxFps) {
        SENRenderer.maxFps = _maxFps;
        SENRenderer.frameTime =  (long) (1.0 / maxFps * nano_1sec);
    }
    public static int getMaxFps() {
        return SENRenderer.maxFps;
    }
	
	
	public void setScreenSize(final int w, final int h){
		m_height = h; m_width = w;
	}
	
    //---------------------------------------------------------------- onSurfaceCreated	
    private static native void nativeInit(final int w, final int h);
	@Override
	public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        nativeInit(m_width, m_height);
        m_lastTick = System.nanoTime();
        m_bInitialized = true;
	}
	
    //---------------------------------------------------------------- onSurfaceChanged	
    private static native void nativeResize(final int w, final int h);
	@Override
	public void onSurfaceChanged(GL10 gl, int width, int height) {
		nativeResize(width, height);
	}
	
	
    //---------------------------------------------------------------- onDrawFrame	
	private static native void nativeRender();
	@Override
	public void onDrawFrame(GL10 gl) {
		if (maxFps >= 60) {
			nativeRender(); 
		}
		else {
  		  final long dt  = System.nanoTime() - m_lastTick;
 		  if (dt < frameTime)  try {
 			  Thread.sleep( (frameTime - dt)/nano_1ms );
  		  }
  		  catch(final Exception e)
  		  {
  			  
  		  }
  		  m_lastTick = System.nanoTime();
  		  nativeRender();
		}
	}

    //---------------------------------------------------------------- onPause
    private static native void nativeSuspend();
	public void onPause() {
      if (m_bInitialized) {
    	  SENHandler.onEnterBackground(); 
    	  nativeSuspend();
      }
	}
	
    //---------------------------------------------------------------- onResume
    private static native void nativeResume();
	public void onResume() {
   	  SENHandler.onEnterForeground(); 
	  nativeResume();
	}
	
    //---------------------------------------------------------------- onKeyDown
    private static native boolean nativeKeyDown(final int pKeyCode);
    public void onKeyDown(final int pKeyCode) {
        nativeKeyDown(pKeyCode);
    }

    //---------------------------------------------------------------- onActionDown
    private static native void nativeTouchesBegin(final int pID, final float pX, final float pY);
    public void onActionDown(final int pID, final float pX, final float pY) {
      nativeTouchesBegin(pID, pX, pY);
    }
    
    //---------------------------------------------------------------- onActionUP
    private static native void nativeTouchesEnd(final int pID, final float pX, final float pY);
    public void onActionUp(final int pID, final float pX, final float pY) {
        nativeTouchesEnd(pID, pX, pY);
    }

    //---------------------------------------------------------------- onActionMove
    private static native void nativeTouchesMove(final int[] pIDs, final float[] pXs, final float[] pYs);
    public void onActionMove(final int[] pIDs, final float[] pXs, final float[] pYs) {
        nativeTouchesMove(pIDs, pXs, pYs);
    }
    
    //---------------------------------------------------------------- onActionCancel
    private static native void nativeTouchesCancel(final int[] pIDs, final float[] pXs, final float[] pYs);
    public void onActionCancel(final int[] pIDs, final float[] pXs, final float[] pYs) {
        nativeTouchesCancel(pIDs, pXs, pYs);
    }
    
    
    
}
