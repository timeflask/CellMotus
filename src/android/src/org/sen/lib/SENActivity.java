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
//import android.view.View;
import android.view.WindowManager;
//import android.view.Display;

//import android.widget.Toast;
//import com.splunk.mint.Mint;
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
		//Mint.initAndStartSession(SENActivity.this, "9df46a6d");
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
            
            
			@SuppressWarnings("unused")
			public ConfigChooser(int redSize, int greenSize, int blueSize, int alphaSize, int depthSize, int stencilSize)
			{
				attribs = new int[] {redSize, greenSize, blueSize, alphaSize, depthSize, stencilSize};
			}
			public ConfigChooser(int[] _attribs)
			{
				attribs = _attribs;
			}
            
			public EGLConfig selectConfig(EGL10 egl, EGLDisplay display, EGLConfig[] configs, int[] attribs)
			{
			    for (EGLConfig config : configs) {
			        int d = findConfigAttrib(egl, display, config,
			                EGL10.EGL_DEPTH_SIZE, 0);
			        int s = findConfigAttrib(egl, display, config,
			                EGL10.EGL_STENCIL_SIZE, 0);
			        if ((d >= attribs[4]) && (s >= attribs[5])) {
			            int r = findConfigAttrib(egl, display, config,
			                    EGL10.EGL_RED_SIZE, 0);
			            int g = findConfigAttrib(egl, display, config,
			                     EGL10.EGL_GREEN_SIZE, 0);
			            int b = findConfigAttrib(egl, display, config,
			                      EGL10.EGL_BLUE_SIZE, 0);
			            int a = findConfigAttrib(egl, display, config,
			                    EGL10.EGL_ALPHA_SIZE, 0);
			            if ((r >= attribs[0]) && (g >= attribs[1])
			                    && (b >= attribs[2]) && (a >= attribs[3])) {
			                return config;
			            }
			        }
			    }
			    return null;
			}
			
			private int findConfigAttrib(EGL10 egl, EGLDisplay display,
			        EGLConfig config, int attribute, int defaultValue) {
				int[] value = new int[1];
			    if (egl.eglGetConfigAttrib(display, config, attribute, value)) {
			        return value[0];
			    }
			    return defaultValue;
			}
			
			
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
                            EGL10.EGL_RENDERABLE_TYPE, 4, 
                            
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
                                EGL10.EGL_RENDERABLE_TYPE, 4, //EGL_OPENGL_ES2_BIT
                                EGL10.EGL_NONE
                        
                        };
                        
                        int[] defaultEGLattribsAlpha = {
                                EGL10.EGL_RED_SIZE, 4, 
                                EGL10.EGL_GREEN_SIZE, 4,
                                EGL10.EGL_BLUE_SIZE, 4,
                                EGL10.EGL_ALPHA_SIZE, 4,
                                EGL10.EGL_DEPTH_SIZE, 0,
                                EGL10.EGL_STENCIL_SIZE,0,
                                EGL10.EGL_RENDERABLE_TYPE, 4, //EGL_OPENGL_ES2_BIT
                                EGL10.EGL_NONE

                        };
                        
                        int[] attrs = null;

                        if(this.attribs[3] == 0) {
                            egl.eglChooseConfig(display, defaultEGLattribs, configs, numConfigs[0], choosedConfigNum);
                            attrs=new int[]{5,6,5,0,0,0};
                        }
                        else
                        {
                            egl.eglChooseConfig(display, defaultEGLattribsAlpha, configs, numConfigs[0], choosedConfigNum);
                            attrs=new int[]{4,4,4,4,0,0};
                        }
                        
                        if(choosedConfigNum[0] > 0)
                        {
                            return selectConfig(egl, display, configs, attrs);
                        }
                        else
                        {
                            Log.e(DEVICE_POLICY_SERVICE, "Can not select an EGLConfig.");
                            return null;
                        }
                    }
                }
                Log.e(DEVICE_POLICY_SERVICE, "Can not select an EGLConfig.");
                return null;
            }
        }
        ConfigChooser chooser = new ConfigChooser(this.m_glContextAttrs);
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
//	  String model = Build.MODEL;
	  String product = Build.PRODUCT;
	  boolean res = product != null && ( product.equals("sdk") || product.contains("_sdk") || product.contains("sdk_") );
	  //Log.d(__class, "model[" + model+ "]  product[" +product+"] emulator["+ res +"]" );
	  return res;
  }
   
  public void doExit() {
	  moveTaskToBack(true);
  }

  public void setKeepScreenOn(boolean value, boolean showToast) {
      final boolean newValue = value;
     // final boolean _showToast = showToast; 
    //  final String message = "[Keep screen on]"; 
      runOnUiThread(new Runnable() {
          @Override
          public void run() {
        	  
       	    if (newValue) 
        	  getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        	else	
          	  getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
          //	if (_showToast && newValue)  
	          //Toast.makeText(getApplicationContext(), message, Toast.LENGTH_SHORT).show();
          }
      });
  }
  
  @Override
  public void onAttachedToWindow() {
     super.onAttachedToWindow();

    setKeepScreenOn(SENHandler.s_IsKeepScreenOn, false);
  }  
  
}