
local ffi = require "ffi"
local ffi_util = require "libs.ffi_util"
local ffmpeg = require "libs.ffi_ffmpeg"
local INBUF_SIZE=4096
local C = ffi.C

local function pgm_save(buf, wrap, xsize, ysize, filename)
	local f = C.fopen(filename,"wb");
    C.fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);
    for i = 0, ysize - 1, 1 do
        C.fwrite(buf + i * wrap, 1, xsize, f);
	end
    C.fclose(f);
end

local function decode(dec_ctx, frame, pkt, filename)
	--char buf[1024];
    --int ret;
	local buf0 = ffi.new("char[?]", 1024)
	local buf = ffi_util.getCAddr("char", buf0)

    local ret = ffmpeg.avcodec_send_packet(dec_ctx, pkt);
    if (ret < 0) then
        error("Error sending a packet for decoding\n");
    end
	local end_code;
	if(C.EAGAIN > 0) then
		end_code = - C.EAGAIN;
    else 
		end_code = C.EAGAIN;
    end
	
	local frame_data, frame_linesize
    while (ret >= 0) do
        ret = ffmpeg.avcodec_receive_frame(dec_ctx, frame);
        if (ret == end_code or ret == ffmpeg.avcodec.AVERROR_EOF) then
            return;
		elseif (ret < 0) then		
			error("Error during decoding\n"); 
		end

        C.printf("saving frame %3d\n", dec_ctx.frame_number);
        C.fflush(C.stdout);

        --/* the picture is allocated by the decoder. no need to free it */
        C.snprintf(buf, ffi.sizeof(buf0), "%s-%d", filename, dec_ctx.frame_number);
	    frame_data = frame.data;		 --frame.data[0]
		frame_linesize = frame.linesize; -- frame.linesize[0]
        pgm_save(frame_data, frame_linesize,
			frame.width, frame.height, buf);
    end
end

local function main(filename, outfilename)
   local pkt = ffmpeg.av_packet_alloc();
   --  uint8_t inbuf[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
   local inbuf0 = ffi.new("uint8_t[?]", INBUF_SIZE + ffmpeg.avcodec.AV_INPUT_BUFFER_PADDING_SIZE)
   local inbuf = ffi.new("uint8_t[1]", inbuf0)
   --  /* set end of buffer to 0 (this ensures that no overreading happens for damaged MPEG streams) */
   C.memset(inbuf + INBUF_SIZE, 0, ffmpeg.avcodec.AV_INPUT_BUFFER_PADDING_SIZE)
   
    -- /* find the MPEG-1 video decoder */
    local codec = ffmpeg.avcodec_find_decoder(ffmpeg.avcodec.AV_CODEC_ID_MPEG2VIDEO);
	if (not codec) then
		error("Codec not found\n");
	end
	local parser = ffmpeg.av_parser_init(codec.id);
	if (not parser) then
		error("parser not found\n");
	end
	local c = ffmpeg.avcodec_alloc_context3(codec);
	if (not parser) then
		error("Could not allocate video codec context");
	end
	
	if (ffmpeg.avcodec_open2(c, codec, NULL) < 0) then
		error("Could not open codec");
	end
	
	local f = C.fopen(filename, "rb");
	if(not f) then 
		print("Could not open file = ", filename)
		error("open file")
	end
	local frame = ffmpeg.av_frame_alloc();
	if (not frame) then
		error("Could not allocate video frame");
	end
	
	local data_size, data;
	local pkt_data = ffi_util.getCAddr("uint8_t*", pkt.data)
	local pkt_size = ffi_util.getCAddr("int", pkt.size)
	while (C.feof(f) == 0) do
		--/* read raw data from the input file */
        data_size = C.fread(inbuf, 1, INBUF_SIZE, f);
        if (data_size == 0) then
            break;
		end;

        --/* use the parser to split the data into frames */
        data = inbuf;
        while (data_size > 0) do
            ret = ffmpeg.av_parser_parse2(parser, c, pkt_data, pkt_size,
                                   data, data_size, ffmpeg.avcodec.AV_NOPTS_VALUE, ffmpeg.avcodec.AV_NOPTS_VALUE, 0);
            if (ret < 0) then
                error("Error while call av_parser_parse2()");
            end
            data      = data + ret;
            data_size = data_size - ret;
			-- *p=y -> p[0] = y
            if (pkt_size[0]) then
                decode(c, frame, pkt, outfilename);
			end
        end
	end
	 --/* flush the decoder */
    decode(c, frame, nil, outfilename);

    C.fclose(f);

    ffmpeg.av_parser_close(parser);
    ffmpeg.avcodec_free_context(ffi_util.getCAddr("AVCodecContext*", c));
    ffmpeg.av_frame_free(ffi_util.getCAddr("AVFrame*", frame));
    ffmpeg.av_packet_free(ffi_util.getCAddr("AVPacket*", pkt));
	return 0;
end

main("E:/study/android_sdk/docs/design/media/scroll_index.mp4", "d:/scroll_index.data")

