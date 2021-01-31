package com.alexios.florid;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.Toast;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.UUID;

public class Main3Activity extends AppCompatActivity {

    Intent from;
    TextView env_temp, env_humi, soil_humi, light;
    BluetoothDevice mDevice;
    BluetoothSocket mSocket;
    RadioGroup ra;
    static final int buffer_size = 8;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main3);
        //this.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT); //限制为竖屏

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
        env_temp = (TextView) findViewById(R.id.text32);//环境温度
        //soil_temp = (TextView) findViewById(R.id.text34);
        env_humi = (TextView) findViewById(R.id.text36);//环境湿度
        soil_humi = (TextView) findViewById(R.id.text38);//土壤湿度
        light = (TextView) findViewById(R.id.text3x);//光照
        ra=(RadioGroup)findViewById(R.id.ra);
        ra.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(RadioGroup group, int checkedId) {
                if(checkedId==R.id.ra1){
                    try {
                        mSocket.getOutputStream().write('M');  //写指令
                    } catch (IOException e) {
                        e.printStackTrace();
                    }

                    try {
                        Thread.currentThread().sleep(1000);//延时毫秒
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                    Toast.makeText(getApplicationContext(),"已设置为手动模式", Toast.LENGTH_SHORT).show();
                }
                if(checkedId==R.id.ra2){
                    try {
                        mSocket.getOutputStream().write('m');  //写指令
                    } catch (IOException e) {
                        e.printStackTrace();
                    }

                    try {
                        Thread.currentThread().sleep(1000);//延时毫秒
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                    Toast.makeText(getApplicationContext(),"已设置为自动模式", Toast.LENGTH_SHORT).show();
                }
            }
        });
    }

    public void getData(View v)     //获取数据
    {
        char[] command = {'G', 'h', 'H', 'i'};  //查询命令
        TextView[] Datas = {env_temp, env_humi, soil_humi, light};

        for (int i = 0; i < 4; ++i) {
            byte[] buffer = new byte[buffer_size];
            try {
                mSocket.getOutputStream().write(command[i]);  //写指令
            } catch (IOException e) {
                e.printStackTrace();
            }

            try {
                Thread.currentThread().sleep(1000);//延时毫秒
            } catch (Exception e) {
                e.printStackTrace();
            }

            try {
                mSocket.getInputStream().read(buffer); //读返回的数据
            } catch (IOException e) {
                e.printStackTrace();
            }

            try {
                Datas[i].setText(byteTOString(buffer));//显示数据
            } catch (Exception e) {
                e.printStackTrace();
            }

            try {
                Thread.currentThread().sleep(500);//毫秒
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    public static InputStream byteTOInputStream(byte[] in) throws Exception{
    //字节数组转化为输入流
        ByteArrayInputStream is = new ByteArrayInputStream(in);
        return is;
    }

    public static String InputStreamTOString(InputStream in,String encoding) throws Exception{
    //输入流转化为字符串
        ByteArrayOutputStream outStream = new ByteArrayOutputStream();
        byte[] data = new byte[buffer_size];
        int count = -1;
        while((count = in.read(data,0,buffer_size)) != -1)
            outStream.write(data, 0, count);

        data = null;
        return new String(outStream.toByteArray(),encoding);
    }

    public static String byteTOString(byte[] in) throws Exception{
    //字节数组转化为字符串
        InputStream is = byteTOInputStream(in);
        return InputStreamTOString(is, "UTF-8");
    }

    public void getAdvice(View v)   //获取建议
    {
        //int []Datas = {Integer.valueOf(env_temp.getText().toString()),Integer.valueOf(env_humi.getText().toString()),Integer.valueOf(soil_humi.getText().toString()),Integer.valueOf(light.getText().toString())};

        try {
            mSocket.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        Intent it = new Intent(this, Main6Activity.class);
        //it.putExtra("plant_datas",Datas);
        it.putExtra("bluetooth_device",mDevice);
        startActivity(it);
    }

    public void goBack(View v)      //退出该活动
    {
        try {
            mSocket.close();
        } catch (IOException e) {
            e.printStackTrace();
        }

        Intent it = new Intent(this, Main7Activity.class);
        //it.putExtra("bluetooth_device",mDevice);
        startActivity(it);
    }
}