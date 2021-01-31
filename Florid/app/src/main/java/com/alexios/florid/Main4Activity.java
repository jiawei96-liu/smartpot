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

public class Main4Activity extends AppCompatActivity {

    BluetoothSocket mSocket;
    Intent from;
    BluetoothDevice mDevice;
    char last_open_or_close_cmd;
    TextView ResultText;
    static final int buffer_size = 38;
    EditText ed1,ed2,ed3,ed4,ed5,ed6,ed7;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main4);
        //this.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT); //限制为竖屏

        last_open_or_close_cmd = 'A';
        from = getIntent();
        mDevice = from.getParcelableExtra("bluetooth_device");  //获取蓝牙设备对象
        ResultText = (TextView) findViewById(R.id.text42);
        ed1=(EditText)findViewById(R.id.text4721);
        ed2=(EditText)findViewById(R.id.text4722);
        ed3=(EditText)findViewById(R.id.text4723);
        ed4=(EditText)findViewById(R.id.text4724);
        ed5=(EditText)findViewById(R.id.text4725);
        ed6=(EditText)findViewById(R.id.text4726);
        ed7=(EditText)findViewById(R.id.text4727);



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

    public void SendStr(String str) {
        byte[] bf = new byte[32];
        bf = str.getBytes();
        if((!str.equals("")) && (mSocket!=null)) {
            try {

                mSocket.getOutputStream().write(bf);
                mSocket.getOutputStream().write('\0');

            } catch (IOException e) {
                e.printStackTrace();
            }

            try {
                Thread.currentThread().sleep(1000);//延时毫秒
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    public  void openLCD (View v){
        try {
            mSocket.getOutputStream().write('A');   //写命令
        } catch (IOException e) {
            e.printStackTrace();
        }

        try {                                   //延时
            Thread.currentThread().sleep(1000);//毫秒
        } catch (Exception e) {
            e.printStackTrace();
        }
        Toast.makeText(getApplicationContext(),"LCD显示屏已开启", Toast.LENGTH_SHORT).show();
    }

    public  void closeLCD (View v){
        try {
            mSocket.getOutputStream().write('a');   //写命令
        } catch (IOException e) {
            e.printStackTrace();
        }

        try {                                   //延时
            Thread.currentThread().sleep(1000);//毫秒
        } catch (Exception e) {
            e.printStackTrace();
        }
        Toast.makeText(getApplicationContext(),"LCD显示屏已关闭", Toast.LENGTH_SHORT).show();

    }




    /*public void closeOrOpenLcd(View v)
    {
        if(last_open_or_close_cmd == 'A')      //此时LCD开启
        {
            try {
                mSocket.getOutputStream().write('a');   //关闭LCD
            } catch (IOException e) {
                e.printStackTrace();
            }
            last_open_or_close_cmd = 'a';
        }
        else
        {
            try {
                mSocket.getOutputStream().write('A');   //打开LCD
            } catch (IOException e) {
                e.printStackTrace();
            }
            last_open_or_close_cmd = 'A';
        }
    }*/

    public void getenvtemp(View v){
        try {
            mSocket.getOutputStream().write('I');   //写命令
        } catch (IOException e) {
            e.printStackTrace();
        }

        try {                                   //延时
            Thread.currentThread().sleep(1000);//毫秒
        } catch (Exception e) {
            e.printStackTrace();
        }

        byte[] buffer = new byte[buffer_size];

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

        try {
            Thread.currentThread().sleep(500);//毫秒
        } catch (Exception e) {
            e.printStackTrace();
        }
        Toast.makeText(getApplicationContext(),"已获取环境温度阈值", Toast.LENGTH_SHORT).show();

    }

    public void getenvhumi(View v){
        try {
            mSocket.getOutputStream().write('l');   //写命令
        } catch (IOException e) {
            e.printStackTrace();
        }

        try {                                   //延时
            Thread.currentThread().sleep(1000);//毫秒
        } catch (Exception e) {
            e.printStackTrace();
        }

        byte[] buffer = new byte[buffer_size];

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

        try {
            Thread.currentThread().sleep(500);//毫秒
        } catch (Exception e) {
            e.printStackTrace();
        }
        Toast.makeText(getApplicationContext(),"已获取环境湿度阈值", Toast.LENGTH_SHORT).show();

    }
    public void getsoilhumi(View v){
        try {
            mSocket.getOutputStream().write('L');   //写命令
        } catch (IOException e) {
            e.printStackTrace();
        }

        try {                                   //延时
            Thread.currentThread().sleep(1000);//毫秒
        } catch (Exception e) {
            e.printStackTrace();
        }

        byte[] buffer = new byte[buffer_size];

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

        try {
            Thread.currentThread().sleep(500);//毫秒
        } catch (Exception e) {
            e.printStackTrace();
        }
        Toast.makeText(getApplicationContext(),"已获取土壤湿度阈值", Toast.LENGTH_SHORT).show();

    }
    public void getlight(View v){
        try {
            mSocket.getOutputStream().write('o');   //写命令
        } catch (IOException e) {
            e.printStackTrace();
        }

        try {                                   //延时
            Thread.currentThread().sleep(1000);//毫秒
        } catch (Exception e) {
            e.printStackTrace();
        }

        byte[] buffer = new byte[buffer_size];

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

        try {
            Thread.currentThread().sleep(500);//毫秒
        } catch (Exception e) {
            e.printStackTrace();
        }
        Toast.makeText(getApplicationContext(),"已获取光照强度阈值", Toast.LENGTH_SHORT).show();

    }


    public void setTime(View v){
        String y="0000",m="00",d="00",h="00",mi="00",s="00",w="0";
        y=ed1.getText().toString();
        m=ed2.getText().toString();
        d=ed3.getText().toString();
        h=ed4.getText().toString();
        mi=ed5.getText().toString();
        s=ed6.getText().toString();
        w=ed7.getText().toString();
        y="d"+":"+y;
        m="D"+":"+m;
        d="e"+":"+d;
        h="E"+":"+h;
        mi="f"+":"+mi;
        s="F"+":"+s;
        w="g"+":"+w;
        SendStr(y);//指令发送
        SendStr(m);
        SendStr(d);
        SendStr(h);
        SendStr(mi);
        SendStr(s);
        SendStr(w);
        Toast.makeText(getApplicationContext(),"时间设置成功", Toast.LENGTH_SHORT).show();
    }















    public void getTime(View v)
    {
        try {
            mSocket.getOutputStream().write('b');   //写命令
        } catch (IOException e) {
            e.printStackTrace();
        }

        try {                                   //延时
            Thread.currentThread().sleep(1000);//毫秒
        } catch (Exception e) {
            e.printStackTrace();
        }

        byte[] buffer = new byte[buffer_size];

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

        try {
            Thread.currentThread().sleep(500);//毫秒
        } catch (Exception e) {
            e.printStackTrace();
        }
        Toast.makeText(getApplicationContext(),"已获取时间", Toast.LENGTH_SHORT).show();
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
}
