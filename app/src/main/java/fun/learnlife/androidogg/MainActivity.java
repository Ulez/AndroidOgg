package fun.learnlife.androidogg;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import fun.learnlife.ogglib.OggLib;

import android.Manifest;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {


    private Button btBuffer;
    private Button btFile;
    private EditText etIn;
    private EditText etOut;
    private EditText etOut2;
    private OggLib oggLib;
    private static final String TAG = "MainActivity";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        ActivityCompat.requestPermissions(this, new String[]{
                Manifest.permission.WRITE_EXTERNAL_STORAGE,
                Manifest.permission.READ_EXTERNAL_STORAGE,}, 1
        );

        etIn = findViewById(R.id.et_in);
        etOut = findViewById(R.id.et_out);
        etOut2 = findViewById(R.id.et_out2);

        btBuffer = findViewById(R.id.bt_buffer);
        btFile = findViewById(R.id.bt_file);
        btBuffer.setOnClickListener(this);
        btFile.setOnClickListener(this);

        oggLib = new OggLib();
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.bt_buffer:
                toOggStream();
                break;
            case R.id.bt_file:
                toOggFile();
                break;
        }
    }

    private void toOggFile() {
        String oggPath = oggLib.wavToOgg(etIn.getText().toString(), etOut2.getText().toString());
        Log.e(TAG, "wav to ogg file path = " + oggPath);
    }

    private void toOggStream() {
        byte[] data = new byte[3200];
        InputStream inputStream = null;
        FileOutputStream outputStream = null;
        try {

            outputStream = new FileOutputStream(etOut.getText().toString());

            inputStream = new FileInputStream(etIn.getText().toString());
            inputStream.read(new byte[44]);

            byte[] encode1 = oggLib.encode(new byte[1], 1);
            outputStream.write(encode1);
            while ((inputStream.read(data) != -1)) {
                byte[] encode = oggLib.encode(data, data.length);
                outputStream.write(encode);
            }
            oggLib.encode(new byte[2], 2);
            outputStream.close();
            inputStream.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
