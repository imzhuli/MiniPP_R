./_build/cpp/bin/_pa_embedded_client -r 127.0.0.1:17002 -t 240e:ff:e020:966:0:ff:b042:f296:80

killall server_config_center
killall server_relay_device

nohup ./_build/cpp/bin/server_config_center -c "./test_assets/cc.ini" > ./logs/cc.log &
nohup ./_build/cpp/bin/server_relay_device -c "./test_assets/rd.ini" > ./logs/rd.log &

