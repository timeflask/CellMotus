package org.sen.lib;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLDisplay;

import android.util.Log;
import android.app.Activity;
//import android.app.ActivityManager;
import android.content.Context;
//import android.content.pm.ConfigurationInfo;
import android.content.res.AssetManager;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.Bundle;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.graphics.PixelFormat;

public class SENActivity extends Activity {

	private final static String __class = SENActivity.class.getSimpleName();

    private static SENActivity m_context = null;
	public static Context getContext() { return m_context; }
	
	private SENGLSurfaceView m_surface;
    private int[]            m_glContextAttrs = {8,8,8,8,24,8};
    private SENSensor        m_sensor;
	
	
   static {
	  System.loadLibrary("game");
	}
	  
    protected void LoadLibrary(final String name) 
    {
        try {
            System.loadLibrary(name);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
	
	//------------------------------------------------------------------------- onCreate
	private static native void  nativeInitAssets(AssetManager assetManager);
	private static native void  nativeInitContext(Activity context);

	@Override
	protected void onCreate(Bundle savedInstanceState) 
	{
		super.onCreate(savedInstanceState);

		m_context = this;
        
        SENHandler.init(this);

        nativeInitContext(m_context);
        nativeInitAssets(m_context.getAssets());
        
        m_sensor = new SENSensor(m_context);
        init();
	}

	@Override
	protected void onResume() { 
		super.onResume();
	//	m_sensor.enable();
		m_surface.onResume(); 
	}	
	
	@Override
	protected void onPause() { 
		super.onPause();
   	   // m_sensor.disable();
		m_surface.onPause(); 
	}	 

	@Override
	protected void onDestroy() { super.onDestroy();	}	


    public SENGLSurfaceView createGLSurface() {
    	SENGLSurfaceView surf = new SENGLSurfaceView(this);

    	if(m_glContextAttrs[3] > 0) 
    		surf.getHolder().setFormat(PixelFormat.TRANSLUCENT);
        
        class ConfigChooser implements GLSurfaceView.EGLConfigChooser
        {
            public int[] attribs;
            @Override
            public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display) 
            {
                int[] numConfigs = new int[1];
                if(egl.eglGetConfigs(display, null, 0, numConfigs))
                {
                    EGLConfig[] configs = new EGLConfig[numConfigs[0]];
                    int[] EGLattribs = {
                            EGL10.EGL_RED_SIZE, attribs[0], 
                            EGL10.EGL_GREEN_SIZE, attribs[1],
                            EGL10.EGL_BLUE_SIZE, attribs[2],
                            EGL10.EGL_ALPHA_SIZE, attribs[3],
                            EGL10.EGL_DEPTH_SIZE, attribs[4],
                            EGL10.EGL_STENCIL_SIZE,attribs[5],
                            EGL10.EGL_NONE
                                        };
                    int[] choosedConfigNum = new int[1];
                    
                    egl.eglChooseConfig(display, EGLattribs, configs, numConfigs[0], choosedConfigNum);
                    if(choosedConfigNum[0]>0)
                    {
                        return configs[0];
                    }
                    else
                    {
                        int[] defaultEGLattribs = {
                                EGL10.EGL_RED_SIZE, 5, 
                                EGL10.EGL_GREEN_SIZE, 6,
                                EGL10.EGL_BLUE_SIZE, 5,
                                EGL10.EGL_ALPHA_SIZE, 0,
                                EGL10.EGL_DEPTH_SIZE, 0,
                                EGL10.EGL_STENCIL_SIZE,0,
                                EGL10.EGL_NONE
                                            };
                        int[] defaultEGLattribsAlpha = {
                                EGL10.EGL_RED_SIZE, 4, 
                                EGL10.EGL_GREEN_SIZE, 4,
                                EGL10.EGL_BLUE_SIZE, 4,
                                EGL10.EGL_ALPHA_SIZE, 4,
                                EGL10.EGL_DEPTH_SIZE, 0,
                                EGL10.EGL_STENCIL_SIZE,0,
                                EGL10.EGL_NONE
                                            };

                        if(this.attribs[3] == 0)
                            egl.eglChooseConfig(display, defaultEGLattribs, configs, numConfigs[0], choosedConfigNum);
                        else
                            egl.eglChooseConfig(display, defaultEGLattribsAlpha, configs, numConfigs[0], choosedConfigNum);
                        if(choosedConfigNum[0] > 0)
                        {
                            Log.w(DEVICE_POLICY_SERVICE, "The EGLConfig can not be used for rendering, use a default one");
                            return configs[0];
                        }
                        else
                        {
                            Log.e(DEVICE_POLICY_SERVICE, "Can not select an EGLConfig for rendering.");
                            return null;
                        }
                    }
                }
                Log.e(DEVICE_POLICY_SERVICE, "Can not select an EGLConfig for rendering.");
                return null;
            }
        }
        ConfigChooser chooser = new ConfigChooser();
        chooser.attribs = this.m_glContextAttrs;
        surf.setEGLConfigChooser(chooser);
        return surf;
    }
	
	protected FrameLayout mFrameLayout = null;
    public void init() {
        ViewGroup.LayoutParams framelayout_params =
            new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT,
                                       ViewGroup.LayoutParams.MATCH_PARENT);
        mFrameLayout = new FrameLayout(this);
        mFrameLayout.setLayoutParams(framelayout_params);

        m_surface = createGLSurface();

        mFrameLayout.addView(m_surface);

        if (is_emulator())
           m_surface.setEGLConfigChooser(8, 8, 8, 8, 16, 0);

        m_surface.setSENRenderer(new SENRenderer());

        setContentView(mFrameLayout);
    }
    
	
	
   private final static boolean is_emulator() {
	  String model = Build.MODEL;
	  String product = Build.PRODUCT;
	  boolean res = product != null && ( product.equals("sdk") || product.contains("_sdk") || product.contains("sdk_") );
	  Log.d(__class, "model[" + model+ "]  product[" +product+"] emulator["+ res +"]" );
	  return res;
  }
   
  public void doExit() {
	  moveTaskToBack(true);
  }
	
}