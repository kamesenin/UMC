package tinyblue.com.umcandroid;

import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.widget.Toast;
import android.view.View;
import android.content.Context;
import android.provider.Settings;

import java.net.InetAddress;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;
import java.io.IOException;

import tinyblue.com.common.helpers.CameraPermissionHelper;
import tinyblue.com.common.helpers.DisplayRotationHelper;
import tinyblue.com.common.helpers.FullScreenHelper;
import tinyblue.com.common.helpers.SnackbarHelper;
import tinyblue.com.common.rendering.BackgroundRenderer;

import com.google.ar.core.exceptions.CameraNotAvailableException;
import com.google.ar.core.exceptions.UnavailableApkTooOldException;
import com.google.ar.core.exceptions.UnavailableArcoreNotInstalledException;
import com.google.ar.core.exceptions.UnavailableDeviceNotCompatibleException;
import com.google.ar.core.exceptions.UnavailableSdkTooOldException;
import com.google.ar.core.exceptions.UnavailableUserDeclinedInstallationException;
import com.google.ar.core.ArCoreApk;
import com.google.ar.core.Camera;
import com.google.ar.core.Frame;
import com.google.ar.core.Session;
import com.google.ar.core.TrackingState;

public class UMCActivity extends AppCompatActivity implements GLSurfaceView.Renderer {
    private static final String TAG = UMCActivity.class.getSimpleName();

    private GLSurfaceView surfaceView;

    private boolean installRequested;

    private Session session;
    private final SnackbarHelper messageSnackbarHelper = new SnackbarHelper();
    private DisplayRotationHelper displayRotationHelper;

    private final BackgroundRenderer backgroundRenderer = new BackgroundRenderer();
    final Handler handler = new Handler();

    private DatagramPacket UMCDatagramPacket = null;
    private DatagramSocket UMCDatagramSocket = null;

    private int TouchesCount = 0;
    private Context context = this;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_umc);
        surfaceView = findViewById(R.id.surfaceview);
        displayRotationHelper = new DisplayRotationHelper(this);

        surfaceView.setOnTouchListener(new View.OnTouchListener()
        {
            @Override
            public boolean onTouch(View v, MotionEvent event)
            {
                TouchesCount = event.getPointerCount();
                return true;
            }});

        surfaceView.setPreserveEGLContextOnPause(true);
        surfaceView.setEGLContextClientVersion(2);
        surfaceView.setEGLConfigChooser(8, 8, 8, 8, 16, 0);
        surfaceView.setRenderer(this);
        surfaceView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);

        installRequested = false;

        CreateARSession();
    }

    private void CreateARSession()
    {
        Exception exception = null;
        String message = null;
        try
        {
            switch (ArCoreApk.getInstance().requestInstall(this, !installRequested))
            {
                case INSTALL_REQUESTED:
                    installRequested = true;
                    return;
                case INSTALLED:
                    break;
            }

            if (!CameraPermissionHelper.hasCameraPermission(this))
            {
                CameraPermissionHelper.requestCameraPermission(this);
                return;
            }

            session = new Session(this);

        }
        catch (UnavailableArcoreNotInstalledException
                | UnavailableUserDeclinedInstallationException e)
        {
            message = "Please install ARCore";
            exception = e;
        }
        catch (UnavailableApkTooOldException e)
        {
            message = "Please update ARCore";
            exception = e;
        }
        catch (UnavailableSdkTooOldException e)
        {
            message = "Please update this app";
            exception = e;
        }
        catch (UnavailableDeviceNotCompatibleException e)
        {
            message = "This device does not support AR";
            exception = e;
        }
        catch (Exception e)
        {
            message = "Failed to create AR session";
            exception = e;
        }

        if (message != null)
        {
            messageSnackbarHelper.showError(this, message);
            Log.e(TAG, "Exception creating session", exception);
            return;
        }
    }

    @Override
    protected void onResume()
    {
        super.onResume();

        if (session == null)
        {
            CreateARSession();
            return;
        }

        try
        {
            session.resume();
        }
        catch (CameraNotAvailableException e)
        {
            messageSnackbarHelper.showError(this, "Camera not available. Please restart the app.");
            session = null;
            return;
        }

        surfaceView.onResume();
        displayRotationHelper.onResume();
    }

    @Override
    public void onPause()
    {
        super.onPause();
        if (session != null)
        {
            displayRotationHelper.onPause();
            surfaceView.onPause();
            session.pause();
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] results)
    {
        if (!CameraPermissionHelper.hasCameraPermission(this))
        {
            Toast.makeText(this, "Camera permission is needed to run this application", Toast.LENGTH_LONG)
                    .show();
            if (!CameraPermissionHelper.shouldShowRequestPermissionRationale(this))
            {
                CameraPermissionHelper.launchPermissionSettings(this);
            }
            finish();
        }
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus)
    {
        super.onWindowFocusChanged(hasFocus);
        FullScreenHelper.setFullScreenOnWindowFocusChanged(this, hasFocus);
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config)
    {
        GLES20.glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        try
        {
            backgroundRenderer.createOnGlThread(this);

        }
        catch (Throwable e)
        {
            Log.e(TAG, "Failed to read an asset file", e);
        }
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height)
    {
        displayRotationHelper.onSurfaceChanged(width, height);
        GLES20.glViewport(0, 0, width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl)
    {
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT);

        if (session == null){return;}

        displayRotationHelper.updateSessionIfNeeded(session);

        try
        {
            session.setCameraTextureName(backgroundRenderer.getTextureId());

            Frame frame = session.update();
            Camera camera = frame.getCamera();

            if (camera.getTrackingState() == TrackingState.TRACKING)
            {
                float[] Location = frame.getAndroidSensorPose().getTranslation();
                float[] Rotation = frame.getAndroidSensorPose().getRotationQuaternion();
                backgroundRenderer.draw(frame);
                //Log.d("UMCLog","Tracking");
                Network(Location, Rotation);
            }

        }
        catch (Throwable t)
        {
            Log.e(TAG, "Exception on the OpenGL thread", t);
        }
    }

    private void Network(float[] Location, float[] Rotation)
    {
        //Log.d("UMCLog","befor network");
        try
        {
            if(UMCDatagramSocket == null)
            {
                UMCDatagramSocket = new DatagramSocket(2492, InetAddress.getByName("0.0.0.0"));
                UMCDatagramSocket.setReuseAddress(true);

                UMCDatagramSocket.setSoTimeout(1000);
            }
            //Log.d("UMCLog","in network");

            String android_id = Settings.Secure.getString(context.getContentResolver(),
                    Settings.Secure.ANDROID_ID);
            String UMCMessage =
                    "Android"
                            + "#"
                            + "ID"
                            + "#"
                            + android_id //id
                            + "#"
                            + "Location"
                            + "#"
                            + Float.toString(Location[0]) + "," + Float.toString(Location[1]) + "," + Float.toString(Location[2])
                            + "#"
                            + "Rotation"
                            + "#"
                            + Float.toString(Rotation[0]) + "," + Float.toString(Rotation[1]) + "," + Float.toString(Rotation[2]) + "," + Float.toString(Rotation[3])
                            + "#"
                            + "Touches"
                            + "#"
                            + Integer.toString(TouchesCount) //touches
                    ;
            //Log.d("UMCLog","sending: "+UMCMessage);

            UMCDatagramPacket = new DatagramPacket(UMCMessage.getBytes(), UMCMessage.length(), InetAddress.getByName("255.255.255.255"), 2492);
            UMCDatagramSocket.send(UMCDatagramPacket);
            //Log.d("UMCLog","message sent");

        }
        catch (IOException e)
        {
            e.printStackTrace();
        }
    }
}
