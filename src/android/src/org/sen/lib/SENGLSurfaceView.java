package org.sen.lib;

import android.content.Context;
import android.opengl.GLSurfaceView;
//import android.os.Handler;
//import android.os.Message;
import android.util.AttributeSet;
//import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;

public class SENGLSurfaceView extends GLSurfaceView {
    
    public SENGLSurfaceView(final Context context) { super(context); init(); }
    public SENGLSurfaceView(final Context context, final AttributeSet attrs) { super(context, attrs); init(); }

    private        SENRenderer      m_renderer;
    private static SENGLSurfaceView st_SENGLSurfaceView;

    protected void init()
    {
        setEGLContextClientVersion(2);
        setFocusableInTouchMode(true);
        SENGLSurfaceView.st_SENGLSurfaceView = this;
    }
    
    public static void queueSensor(final float x, final float y, final float z, final long ts) {   
    	st_SENGLSurfaceView.queueEvent(
          new Runnable() {
            @Override
            public void run() {
              SENSensor.nativeSensorChanged(x, y, z, ts);
            }
        });
    }
    
    public void setSENRenderer(final SENRenderer renderer) {
    	m_renderer = renderer;
        setRenderer(m_renderer);
    }
 
    @Override
    public void onResume() {
        super.onResume();
        setRenderMode(RENDERMODE_CONTINUOUSLY);
        queueEvent(new Runnable() {
            @Override
            public void run() {
            	m_renderer.onResume();
            }
        });
    }
    
    @Override
    public void onPause() {
        this.queueEvent(new Runnable() {
            @Override
            public void run() {
            	m_renderer.onPause();
            }
        });
        this.setRenderMode(RENDERMODE_WHEN_DIRTY);
    }
    
    @Override
    protected void onSizeChanged(final int w, final int h, final int old_w, final int old_h) {
    	m_renderer.setScreenSize(w, h);
    }
    
    
    @Override
    public boolean onKeyDown(final int pKeyCode, final KeyEvent pKeyEvent) {
        switch (pKeyCode) {
            case KeyEvent.KEYCODE_BACK:
            case KeyEvent.KEYCODE_MENU:
            case KeyEvent.KEYCODE_DPAD_LEFT:
            case KeyEvent.KEYCODE_DPAD_RIGHT:
            case KeyEvent.KEYCODE_DPAD_UP:
            case KeyEvent.KEYCODE_DPAD_DOWN:
            case KeyEvent.KEYCODE_ENTER:
            case KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE:
            case KeyEvent.KEYCODE_DPAD_CENTER:
                this.queueEvent(new Runnable() {
                    @Override
                    public void run() {
                        m_renderer.onKeyDown(pKeyCode);
                    }
                });
                return true;
            default:
                return super.onKeyDown(pKeyCode, pKeyEvent);
        }
    }
    
  
    @Override
    public boolean onTouchEvent(final MotionEvent pMotionEvent) {
        
        final int pointerNumber = pMotionEvent.getPointerCount();
        final int[] ids = new int[pointerNumber];
        final float[] xs = new float[pointerNumber];
        final float[] ys = new float[pointerNumber];

        for (int i = 0; i < pointerNumber; i++) {
            ids[i] = pMotionEvent.getPointerId(i);
            xs[i] = pMotionEvent.getX(i);
            ys[i] = pMotionEvent.getY(i);
        }

        switch (pMotionEvent.getAction() & MotionEvent.ACTION_MASK) {
            case MotionEvent.ACTION_POINTER_DOWN:
                final int indexPointerDown = pMotionEvent.getAction() >> MotionEvent.ACTION_POINTER_INDEX_SHIFT;
                final int idPointerDown = pMotionEvent.getPointerId(indexPointerDown);
                final float xPointerDown = pMotionEvent.getX(indexPointerDown);
                final float yPointerDown = pMotionEvent.getY(indexPointerDown);

                this.queueEvent(new Runnable() {
                    @Override
                    public void run() {
                        m_renderer.onActionDown(idPointerDown, xPointerDown, yPointerDown);
                    }
                });
                break;

            case MotionEvent.ACTION_DOWN:
                final int idDown = pMotionEvent.getPointerId(0);
                final float xDown = xs[0];
                final float yDown = ys[0];

                this.queueEvent(new Runnable() {
                    @Override
                    public void run() {
                        m_renderer.onActionDown(idDown, xDown, yDown);
                    }
                });
                break;

            case MotionEvent.ACTION_MOVE:
                this.queueEvent(new Runnable() {
                    @Override
                    public void run() {
                        m_renderer.onActionMove(ids, xs, ys);
                    }
                });
                break;

            case MotionEvent.ACTION_POINTER_UP:
                final int indexPointUp = pMotionEvent.getAction() >> MotionEvent.ACTION_POINTER_INDEX_SHIFT;
                final int idPointerUp = pMotionEvent.getPointerId(indexPointUp);
                final float xPointerUp = pMotionEvent.getX(indexPointUp);
                final float yPointerUp = pMotionEvent.getY(indexPointUp);

                this.queueEvent(new Runnable() {
                    @Override
                    public void run() {
                        m_renderer.onActionUp(idPointerUp, xPointerUp, yPointerUp);
                    }
                });
                break;

            case MotionEvent.ACTION_UP:
                final int idUp = pMotionEvent.getPointerId(0);
                final float xUp = xs[0];
                final float yUp = ys[0];
                performClick();
                this.queueEvent(new Runnable() {
                    @Override
                    public void run() {
                        m_renderer.onActionUp(idUp, xUp, yUp);
                    }
                });
                break;

            case MotionEvent.ACTION_CANCEL:
                this.queueEvent(new Runnable() {
                    @Override
                    public void run() {
                        m_renderer.onActionCancel(ids, xs, ys);
                    }
                });
                break;
        }

        return true;
    }
    
    @Override
    public boolean performClick() {
      super.performClick();
      return true;
    }    
  
}