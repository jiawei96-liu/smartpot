package com.alexios.florid;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.UUID;

public class Main6Activity extends AppCompatActivity {



    //int[] Datas;
    TextView t11,t12,t13,t14,t21,t22,t23,t24,t31,t32,t33,t34;

    Intent from;
    BluetoothDevice mDevice;
    BluetoothSocket mSocket;
    static final int buffer_size = 8;




    String[] someflower_env_temp={"20","20","25","20","25","暂无"};
    String[] someflower_env_humi={"70","70","70","40","65","暂无"};
    String[] someflower_soil_humi={"40","40","40","40","40","暂无"};
    String[] someflower_light={"20","20","40","20","20","暂无"};
    private String str;
    String flowername[]={"吊兰","君子兰","滴水观音","文竹","芦荟","其它"};
    Spinner sp;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main6);
        //this.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT); //限制为竖屏

        from = getIntent();
        mDevice = from.getParcelableExtra("bluetooth_device");  //获取蓝牙设备对象
        //Datas = from.getIntArrayExtra("plant_datas");


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



        t11=(TextView)findViewById(R.id.text642) ;
        t12=(TextView)findViewById(R.id.text645) ;
        t13=(TextView)findViewById(R.id.text652) ;
        t14=(TextView)findViewById(R.id.text655) ;




        t21=(TextView)findViewById(R.id.text672) ;
        t22=(TextView)findViewById(R.id.text677) ;
        t23=(TextView)findViewById(R.id.text682) ;
        t24=(TextView)findViewById(R.id.text687) ;

        t31=(TextView)findViewById(R.id.text674) ;
        t32=(TextView)findViewById(R.id.text679) ;
        t33=(TextView)findViewById(R.id.text684) ;
        t34=(TextView)findViewById(R.id.text689) ;

        sp = (Spinner) findViewById(R.id.spinner622);
        sp.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                str = sp.getSelectedItem().toString();
                int value=-1;
                for(int i=0;i<=5;i++){
                    if(flowername[i].equals(str)){
                        value=i;
                    }
                }
                if(value!=-1) {
                    t11.setText(someflower_env_temp[value]);
                    t12.setText(someflower_env_humi[value]);
                    t13.setText(someflower_soil_humi[value]);
                    t14.setText(someflower_light[value]);
                }
                else{
                    t11.setText("暂无");
                    t12.setText("暂无");
                    t13.setText("暂无");
                    t14.setText("暂无");
                }
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });
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




    public void setNumber(View v){
        String min_env_temp,min_env_humi,min_soil_humi,min_light,max_env_temp,max_env_humi,max_soil_humi,max_light;
        min_env_temp=t21.getText().toString();
        min_env_humi=t22.getText().toString();
        min_soil_humi=t23.getText().toString();
        min_light=t24.getText().toString();

        max_env_temp=t31.getText().toString();
        max_env_humi=t32.getText().toString();
        max_soil_humi=t33.getText().toString();
        max_light=t34.getText().toString();

        min_env_temp="j"+":"+min_env_temp;//指令合并
        min_env_humi="k"+":"+min_env_humi;
        min_soil_humi="n"+":"+min_soil_humi;
        min_light="p"+":"+min_light;

        max_env_temp="J"+":"+max_env_temp;
        max_env_humi="K"+":"+max_env_humi;
        max_soil_humi="N"+":"+max_soil_humi;
        max_light="P"+":"+max_light;

        SendStr(min_env_temp);//指令发送
        SendStr(min_env_humi);
        SendStr(min_soil_humi);
        SendStr(min_light);
        SendStr(max_env_temp);
        SendStr(max_env_humi);
        SendStr(max_soil_humi);
        SendStr(max_light);
        Toast.makeText(getApplicationContext(),"花卉阈值已设置成功", Toast.LENGTH_SHORT).show();
    }




    public  void openWater(View v){

        try {
            mSocket.getOutputStream().write('r');  //写指令
        } catch (IOException e) {
            e.printStackTrace();
        }

        try {
            Thread.currentThread().sleep(1000);//延时毫秒
        } catch (Exception e) {
            e.printStackTrace();
        }
        Toast.makeText(getApplicationContext(),"浇水开关已打开", Toast.LENGTH_SHORT).show();
    }
    public  void closeWater(View v){
        try {
            mSocket.getOutputStream().write('R');  //写指令
        } catch (IOException e) {
            e.printStackTrace();
        }

        try {
            Thread.currentThread().sleep(1000);//延时毫秒
        } catch (Exception e) {
            e.printStackTrace();
        }
        Toast.makeText(getApplicationContext(),"浇水开关已关闭", Toast.LENGTH_SHORT).show();

    }
    public  void openLed(View v){
        try {
            mSocket.getOutputStream().write('q');  //写指令
        } catch (IOException e) {
            e.printStackTrace();
        }

        try {
            Thread.currentThread().sleep(1000);//延时毫秒
        } catch (Exception e) {
            e.printStackTrace();
        }
        Toast.makeText(getApplicationContext(),"Led灯已开启", Toast.LENGTH_SHORT).show();

    }
    public  void closeLed(View v){
        try {
            mSocket.getOutputStream().write('Q');  //写指令
        } catch (IOException e) {
            e.printStackTrace();
        }

        try {
            Thread.currentThread().sleep(1000);//延时毫秒
        } catch (Exception e) {
            e.printStackTrace();
        }
        Toast.makeText(getApplicationContext(),"Led灯已关闭", Toast.LENGTH_SHORT).show();

    }
    public void goBack(View v) {
        try {
            mSocket.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        Intent it = new Intent(this, Main3Activity.class);
        it.putExtra("bluetooth_device", mDevice);
        startActivity(it);
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

}
