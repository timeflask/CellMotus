package org.sen.lib;

import android.content.Context;
import android.content.res.Configuration;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.view.Display;
import android.view.Surface;
import android.view.WindowManager;


public class SENSensor implements SensorEventListener {

    private final Context       m_context;
    private final SensorManager m_sensorManager;
    private final Sensor        m_sensor;
    private final int           m_rotation;
	
	public SENSensor(final Context pContext) {
	    m_context = pContext;
	
	    m_sensorManager = (SensorManager) m_context.getSystemService(Context.SENSOR_SERVICE);
	    m_sensor = m_sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
	
	    final Display display = ((WindowManager) m_context.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();
	    m_rotation = display.getRotation();
	}
	
    public void enable() {
    	m_sensorManager.registerListener(this, m_sensor, SensorManager.SENSOR_DELAY_GAME);
    }
    public void disable() {
        m_sensorManager.unregisterListener(this);
    }
    
    public void setInterval(float interval) {
    	if(android.os.Build.VERSION.SDK_INT < 11) {
    		m_sensorManager.registerListener(this, m_sensor, SensorManager.SENSOR_DELAY_GAME);
    	} 
    	else 
    	{
    		m_sensorManager.registerListener(this, m_sensor, (int)(interval*100000));
    	}
    }
    
    public static native void nativeSensorChanged(final float pX, final float pY, final float pZ, final long pTimestamp);

    @Override
    public void onSensorChanged(final SensorEvent pSensorEvent) {
        if (pSensorEvent.sensor.getType() != Sensor.TYPE_ACCELEROMETER) {
            return;
        }

        float x = pSensorEvent.values[0];
        float y = pSensorEvent.values[1];
        final float z = pSensorEvent.values[2];

        final int rotation = m_context.getResources().getConfiguration().orientation;

        if ((rotation == Configuration.ORIENTATION_LANDSCAPE) && (m_rotation != Surface.ROTATION_0)) {
            final float tmp = x; x = -y; y = tmp;
        } 
        else if ((rotation == Configuration.ORIENTATION_PORTRAIT) && (m_rotation != Surface.ROTATION_0)) {
            final float tmp = x; x = y; y = -tmp;
        }       
        
        SENGLSurfaceView.queueSensor(x,y,z,pSensorEvent.timestamp);
        
    }
	
    @Override
    public void onAccuracyChanged(final Sensor pSensor, final int pAccuracy) {
    	
    }
	
}
