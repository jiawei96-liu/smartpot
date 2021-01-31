package com.alexios.florid;

import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        //this.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT); //限制为竖屏
    }

    public void goMainAcitivity(View v)
    {
        Intent it = new Intent(this, Main7Activity.class);
        startActivity(it);
    }

    public void goAbout(View v)     //关于页面
    {
        Intent it = new Intent(this, Main2Activity.class);
        startActivity(it);
    }

    public void exit(View v)
    {
        finish();
    }
}
