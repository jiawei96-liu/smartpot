package com.alexios.florid;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import java.util.Set;


public class Main7Activity extends AppCompatActivity implements AdapterView.OnItemSelectedListener{

    BluetoothAdapter mAdapter;      //蓝牙适配器
    BluetoothDevice mDevice;        //花盆蓝牙设备
    TextView show_state;        //状态文本
    boolean is_connected;           //指示是否连接
    ArrayAdapter<String> bt_list;       //spinner相关
    Spinner sp_list;        //spinner对象
    String bt_name;         //当前设备蓝牙名称
    String[] paired_list;       //配对蓝牙名称数组
    Set<BluetoothDevice> pairedDevices;     //配对蓝牙设备
    int num;        //配对数目

    @Override
    protected void onCreate(Bundle savedInstanceState)  //绘制界面
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main7);
        //this.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);//限制为竖屏

        is_connected = false;
        mAdapter = BluetoothAdapter.getDefaultAdapter();    //本机蓝牙
        if(mAdapter==null) {
            Toast.makeText(this, "该设备不支持蓝牙", Toast.LENGTH_SHORT).show();
            finish();
        }
        show_state = (TextView) findViewById(R.id.text12);
        show_state.setText("未连接");
        sp_list = (Spinner) findViewById(R.id.bt_list);
        sp_list.setOnItemSelectedListener(this);
    }

    public void connect(View v)     //连接
    {
        if (!mAdapter.isEnabled())      //若未打开蓝牙，则打开蓝牙
        {
            Intent turnOn = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(turnOn, 1);
            //Toast.makeText(getApplicationContext(),"成功打开蓝牙",Toast.LENGTH_SHORT).show();
        }
        else
            Toast.makeText(getApplicationContext(),"已经打开蓝牙", Toast.LENGTH_SHORT).show();

        pairedDevices = mAdapter.getBondedDevices();  //获取配对设备列表
        num = pairedDevices.size();

        paired_list = new String[num + 1]; //配对设备名称数组，用于显示

        int i = 0;      //序数变量
        for (BluetoothDevice device : pairedDevices)
            paired_list[i++] = device.getName();

        paired_list[num] = "其它设备";  //未配对的设备

        //设置显示内容
        bt_list = new ArrayAdapter<>(this,android.R.layout.simple_spinner_item,paired_list);
        //设置spinner
        bt_list.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        //将显示内容与spinner绑定
        sp_list.setAdapter(bt_list);
    }

    public void disconnect(View v)      //关闭连接
    {
        mAdapter.disable();
        Toast.makeText(getApplicationContext(),"成功关闭蓝牙" , Toast.LENGTH_SHORT).show();
        show_state.setText("未连接");
        is_connected = false;
    }

    public void showSituation(View v)       //展示盆栽情况
    {
        if(!is_connected)
            Toast.makeText(this, "请先打开蓝牙，并选择花盆设备", Toast.LENGTH_SHORT).show();
        else
        {
            Intent it = new Intent(this, Main3Activity.class);
            it.putExtra("bluetooth_device",mDevice);    //向下一活动传输蓝牙设备对象
            startActivity(it);
        }

    }

    public void remoteControl(View v)   //远程控制
    {
        if(!is_connected)
            Toast.makeText(this, "请先打开蓝牙，并选择花盆设备", Toast.LENGTH_SHORT).show();

        else
        {
            Intent it = new Intent(this, Main4Activity.class);
            it.putExtra("bluetooth_device",mDevice);
            startActivity(it);
        }
    }

    public void enterCommandLine(View v)    //进入命令行
    {
        if(!is_connected)
            Toast.makeText(this, "请先打开蓝牙，并选择花盆设备", Toast.LENGTH_SHORT).show();

        else
        {
            Intent it = new Intent(this, Main5Activity.class);
            it.putExtra("bluetooth_device",mDevice);
            startActivity(it);
        }
    }

    public void goAbout(View v)     //关于页面
    {
        Intent it = new Intent(this, Main2Activity.class);
        startActivity(it);
    }

    public void exit(View v)        //退出程序
    {
        if(mAdapter.isEnabled())
            mAdapter.disable();
        finish();
    }

    @Override
    //spinner单击事件处理
    public void onItemSelected(AdapterView<?> parent, View v, int position, long id)
    {
        if(position == num && num != 0)     //未配对设备
            Toast.makeText(this, "请先在蓝牙设置菜单中与花盆设备配对", Toast.LENGTH_SHORT).show();

        else        //出现在列表中
        {
            bt_name = paired_list[position];

            for (BluetoothDevice device : pairedDevices)
            {
                if(device.getName().equals(bt_name))
                {
                    mDevice = mAdapter.getRemoteDevice(device.getAddress());
                    show_state.setText("成功找到配对花盆设备");
                    is_connected = true;
                }
            }
        }
    }

    @Override
    public void onNothingSelected(AdapterView<?> parent){}  //空方法体
}
