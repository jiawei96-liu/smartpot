package com.alexios.florid;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.UUID;

public class Main5Activity extends AppCompatActivity {

    BluetoothSocket mSocket;
    Intent from;
    BluetoothDevice mDevice;
    EditText InputText;
    TextView ResultText;

    static final int buffer_size = 1024;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main5);
        //this.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT); //限制为竖屏

        InputText = (EditText) findViewById(R.id.edit51);
        ResultText = (TextView) findViewById(R.id.text53);
        from = getIntent();
        mDevice = from.getParcelableExtra("bluetooth_device");  //获取蓝牙设备对象

        try {       //找到花盆设备
            mSocket = mDevice.createRfcommSocketToServiceRecord(UUID.fromString("00001101-0000-1000-8000-00805F9B34FB"));
        } catch (IOException e) {
            e.printStackTrace();
        }

        try {        //建立连接
            mSocket.connect();
            Toast.makeText(getApplicationContext(),"成功连接花盆设备", Toast.LENGTH_SHORT).show();
        } catch (IOException e) {
            Toast.makeText(getApplicationContext(),"连接花盆设备失败，确保花盆蓝牙已打开并在通信范围内", Toast.LENGTH_SHORT).show();
            e.printStackTrace();
        }
    }

    public void enterCommand(View v)
    {
        try {
            mSocket.getOutputStream().write(StringToByte(InputText.getText().toString()));   //写命令
        }catch (Exception e) {
            e.printStackTrace();
        }

        try {                                   //延时
            Thread.currentThread().sleep(1000);//毫秒
        } catch (Exception e) {
            e.printStackTrace();
        }

        byte[] buffer = new byte[buffer_size];

        if(!InputText.getText().toString().equals("A") && !InputText.getText().toString().equals("a") && !InputText.getText().toString().equals("b"))
        {
            try {
                mSocket.getInputStream().read(buffer); //读返回的数据
            } catch (IOException e) {
                e.printStackTrace();
            }

            try {
                ResultText.setText(byteTOString(buffer));
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        try {
            Thread.currentThread().sleep(500);//毫秒
        } catch (Exception e) {
            e.printStackTrace();
        }
        Toast.makeText(getApplicationContext(),"指令已发送", Toast.LENGTH_SHORT).show();
    }

    public void goBack(View v)      //退出该活动
    {
        try {
            mSocket.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        finish();
    }

    public static InputStream byteTOInputStream(byte[] in) throws Exception{

        ByteArrayInputStream is = new ByteArrayInputStream(in);
        return is;
    }

    public static String InputStreamTOString(InputStream in,String encoding) throws Exception{

        ByteArrayOutputStream outStream = new ByteArrayOutputStream();
        byte[] data = new byte[buffer_size];
        int count = -1;
        while((count = in.read(data,0,buffer_size)) != -1)
            outStream.write(data, 0, count);

        data = null;
        return new String(outStream.toByteArray(),encoding);
    }

    public static String byteTOString(byte[] in) throws Exception{

        InputStream is = byteTOInputStream(in);
        return InputStreamTOString(is, "UTF-8");
    }

    public static InputStream StringTOInputStream(String in) throws Exception{

        ByteArrayInputStream is = new ByteArrayInputStream(in.getBytes("UTF-8"));
        return is;
    }

    public static byte[] InputStreamTOByte(InputStream in) throws IOException{

        ByteArrayOutputStream outStream = new ByteArrayOutputStream();
        byte[] data = new byte[buffer_size];
        int count = -1;
        while((count = in.read(data,0,buffer_size)) != -1)
            outStream.write(data, 0, count);

        data = null;
        return outStream.toByteArray();
    }

    public static byte[] StringToByte(String in) throws Exception
    {
        InputStream input = StringTOInputStream(in);
        return InputStreamTOByte(input);
    }
}
