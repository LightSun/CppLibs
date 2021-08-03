

### ffmpeg for ffi
- 需要导出来的 ffmpeg_defs.h 需要去除一些不必要的复杂的宏。like
```C
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic pop
__attribute__((deprecated)) typedef struct...
```


### FAQ
- download: 'https://github.com/BtbN/FFmpeg-Builds'
