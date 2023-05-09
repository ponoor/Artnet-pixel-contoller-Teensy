# config項目
- network
    - ip i i i i
    - gateway i i i i
    - subnet i i i i
    - mac i i i i i i
    - ipAddId i(b)
    - macAddId i(b)
- artnet
    - port i
    - net i
    - subnet i
    - universe i
- osc
    - dstPort i
    - dstIp i i i i
    - srcPort i
    - dstPortAddId i(b)
- led
    - numPixels i
    - protocol i
    - colorOrder i
    - initTest i(b)
## OSC送り方
/config/network/ip 10 0 0 0
のように送ると値を保存してリセット後に適応される
## colorOrder
'''
OctoWS2811.h:39
#define WS2811_RGB	0	// The WS2811 datasheet documents this way
#define WS2811_RBG	1
#define WS2811_GRB	2	// Most LED strips are wired this way
#define WS2811_GBR	3
#define WS2811_BRG	4
#define WS2811_BGR	5
'''
## protocol
'''
OctoWS2811.h:70
#define WS2811_800kHz 0x00	// Nearly all WS2811 are 800 kHz
#define WS2811_400kHz 0x40	// Adafruit's Flora Pixels
#define WS2813_800kHz 0x80	// WS2813 are close to 800 kHz but has 300 us frame set delay
'''
#その他OSCコマンド
/rst
リセットする　シリアルをつなげてると起動しないxx
/print
シリアルにconfigの値を出力する
/setDstIp
oscの送信先を変えられるけど送るものがあまり無い
/ledTest
r g b
