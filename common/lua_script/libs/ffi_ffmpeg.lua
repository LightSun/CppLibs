-- FFI binding to FFmpeg

-- get current dir of this file
local rel_dir = assert ( debug.getinfo ( 1 , "S" ).source:match ( [=[^@(.-[/\]?)[^/\]*$]=] ) , "Current directory unknown" ) .. "./"
print("rel_dir: ", rel_dir)
-- for test. from github ffmpeg-builds(win/linux/mac)
rel_dir = "E:/study/tools/ffmpeg/win64-lgpl-shared/"

local assert , error = assert , error
local setmetatable = setmetatable
local tonumber , tostring = tonumber , tostring
local tblinsert = table.insert

local ffi                 = require "ffi"
local ffi_util            = require "libs.ffi_util"
local ffi_add_include_dir = ffi_util.ffi_add_include_dir
local ffi_defs            = ffi_util.ffi_defs

ffi_add_include_dir ( rel_dir .. "include" )

ffi_defs ( [[ffmpeg_funcs.h]] , [[ffmpeg_defs.h]] , {
		[[libavutil/avstring.h]] ;
		[[libavcodec/avcodec.h]] ;
		[[libavformat/avformat.h]] ;
	} )

-- must load in order
local avutil, swresample, swscale, avcodec, avformat, avfilter, avdevice
local osname = require "libs.platform"
-- assert ( jit , "jit table unavailable" )
-- Windows binaries from http://ffmpeg.zeranoe.com/builds/  not avaliable
if osname == "Windows" then 
	avutil 		= ffi.load ( rel_dir .. [[bin/avutil-57]] )
	--postproc	= ffi.load ( rel_dir .. [[bin/postproc-56]] )
	swresample  = ffi.load ( rel_dir .. [[bin/swresample-4]])
	swscale     = ffi.load ( rel_dir .. [[bin/swscale-6]])
	avcodec 	= ffi.load ( rel_dir .. [[bin/avcodec-59]] )
	avformat 	= ffi.load ( rel_dir .. [[bin/avformat-59]] )
	avfilter 	= ffi.load ( rel_dir .. [[bin/avfilter-8]] )
	avdevice 	= ffi.load ( rel_dir .. [[bin/avdevice-59]] )
elseif osname == "Linux" or osname == "OSX" or osname == "POSIX" or osname == "BSD" then
	avutil 		= ffi.load ( [[libavutil]] )
	--postproc 	= ffi.load ( [[libpostproc]] )
	swresample 	= ffi.load ( [[libswresample]] )
	swscale 	= ffi.load ( [[libswscale]] )
	avcodec 	= ffi.load ( [[libavcodec]] )
	avformat 	= ffi.load ( [[libavformat]] )
	avfilter 	= ffi.load ( [[libavfilter]] )
	avdevice 	= ffi.load ( [[libavdevice]] )
else
	error ( "Unknown platform" )
end

local ffmpeg = {
	avutil = avutil ;
	avdevice = avdevice;
	swresample = swresample;
	swscale = swscale;
	avcodec = avcodec ;
	avformat = avformat ;
	avfilter = avfilter ;

	AV_TIME_BASE = 1000000 ;
}

ffmpeg.format_to_type = {
	--[avutil.AV_SAMPLE_FMT_NONE] 	= nil ;
	[avutil.AV_SAMPLE_FMT_U8] 	= "uint8_t" ;
	[avutil.AV_SAMPLE_FMT_S16] 	= "int16_t" ;
	[avutil.AV_SAMPLE_FMT_S32] 	= "int32_t" ;
	[avutil.AV_SAMPLE_FMT_FLT] 	= "float" ;
	[avutil.AV_SAMPLE_FMT_DBL] 	= "double" ;
	--[avutil.AV_SAMPLE_FMT_NB] 	= "" ;
}

function ffmpeg.avInit()
	local avPkt = assert(avcodec.av_packet_alloc(), "av_packet_alloc() failed.");
	print("avPkt: ", avPkt)
end

function ffmpeg.avAssert ( err )
	if err < 0 then
		local errbuff = ffi.new ( "uint8_t[256]" )
		local ret = avutil.av_strerror ( err , errbuff , 256 )
		if ret ~= -1 then
			error ( ffi.string ( errbuff ) , 2 )
		else
			error ( "Unknown AV error: " .. tostring ( ret ) , 2 )
		end
	end
	return err
end

function ffmpeg.openfile ( file )
	assert ( file , "No input file" )
	local formatContext_p = ffi.new ( "AVFormatContext*[1]" )
	avAssert ( avformat.avformat_open_input ( formatContext_p , file , nil , nil ) )
	local formatContext = ffi.gc ( formatContext_p[0] , avformat.av_close_input_stream )
	return formatContext
end

function ffmpeg.findaudiostreams ( formatContext )
	avAssert( avformat.av_find_stream_info ( formatContext ) )

	local audiostreams = { }
	local nStreams = tonumber ( formatContext.nb_streams )
	for i=0 , nStreams-1 do
		local ctx = formatContext.streams [ i ].codec
		if ctx.codec_type == avutil.AVMEDIA_TYPE_AUDIO then
			local codec = assert ( avcodec.avcodec_find_decoder ( ctx.codec_id ) , "Unsupported codec" )
			avAssert ( avcodec.avcodec_open ( ctx , codec ) )
			tblinsert ( audiostreams , ctx )
		end
	end

	return audiostreams
end

local packet = ffi.new ( "AVPacket" )
function ffmpeg.read_frames ( formatctx )
	return function ( formatctx , packet )
			if tonumber( avformat.url_feof ( formatctx.pb ) ) == 0 then
				avAssert ( avformat.av_read_frame ( formatctx , packet ) )
				return packet
			else
				return nil
			end
		end , formatctx , packet
end
		
---------------------------------------------------------------
function ffmpeg.avcodec_find_decoder(avcodec_val)
	return avcodec.avcodec_find_decoder(avcodec_val)
end

function ffmpeg.av_parser_init(avcodec_id)
	-- parser = av_parser_init(codec->id);
	return assert(avcodec.av_parser_init(avcodec_id), "av_parser_init() failed.");
end

function ffmpeg.avcodec_alloc_context3(avcodec0)
	-- parser = av_parser_init(codec->id);
	return assert(avcodec.avcodec_alloc_context3(avcodec0), "avcodec_alloc_context3() failed");
end

function ffmpeg.avcodec_open2(ctx, codec0, ops)
-- int avcodec_open2(AVCodecContext *avctx, const AVCodec *codec, AVDictionary **options);
return assert(avcodec.avcodec_open2(ctx, codec0, ops), "avcodec_open2() failed.")
end 

function ffmpeg.av_frame_alloc()
-- int avcodec_open2(AVCodecContext *avctx, const AVCodec *codec, AVDictionary **options);
return assert(avcodec.av_frame_alloc(), "av_frame_alloc() failed")
end 

function ffmpeg.av_packet_alloc()
return assert(avcodec.av_packet_alloc(), "av_packet_alloc() failed.");
end

function ffmpeg.av_parser_parse2()
return assert(avcodec.av_parser_parse2(), "av_parser_parse2() failed.");
end

------------------------- free methods ------------------------------
function ffmpeg.av_parser_close(parser)
	avcodec.av_parser_close(parser);
end
function ffmpeg.avcodec_free_context(ctx)
	-- &ctx
	local ctx_addr = ffi.getCAddr("AVCodecContext*", ctx)
	avcodec.avcodec_free_context(ctx_addr);
end
function ffmpeg.av_frame_free(frame)
	local addr = ffi.getCAddr("AVFrame*", frame)
	avcodec.av_frame_free(addr);
end
function ffmpeg.av_packet_free(av_pkt)
	local addr = ffi.getCAddr("AVPacket*", av_pkt)
	avcodec.av_packet_free(addr);
end
----------------------- packet handle methods ------------------------
function ffmpeg.avcodec_receive_frame(dec_ctx, frame)
	return assert(avcodec.avcodec_receive_frame(dec_ctx, frame), "avcodec_receive_frame() failed");
end
function ffmpeg.avcodec_send_packet(dec_ctx, pkt)
	return assert(avcodec.avcodec_send_packet(dec_ctx, pkt), "avcodec_send_packet() failed");
end

--TODO  ffmpeg changed. the old may not right.
--avcodec.avcodec_init()
--avcodec.avcodec_register_all()
--avformat.av_register_all()

--ffmpeg.avInit();

return ffmpeg