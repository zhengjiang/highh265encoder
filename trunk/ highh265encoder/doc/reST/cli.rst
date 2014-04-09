*********************
Command Line Options
*********************

Note that unless an option is listed as **CLI ONLY** the option is also
supported by x265_param_parse(). The CLI uses getopt to parse the
command line options so the short or long versions may be used and the
long options may be truncated to the shortest unambiguous abbreviation.
Users of the API must pass x265_param_parse() the full option name.

Preset and tune have special implications. The API user must call
x265_param_default_preset() with the preset and tune parameters they
wish to use, prior to calling x265_param_parse() to set any additional
fields. The CLI does this for the user implicitly, so all CLI options
are applied after the user's preset and tune choices, regardless of the
order of the arguments on the command line.

If there is an extra command line argument (not an option or an option
value) the CLI will treat it as the input filename.  This effectively
makes the :option:`--input` specifier optional for the input file. If
there are two extra arguments, the second is treated as the output
bitstream filename, making :option:`--output` also optional if the input
filename was implied. This makes :command:`x265 in.y4m out.hevc` a valid
command line. If there are more than two extra arguments, the CLI will
consider this an error and abort.

Generally, when an option expects a string value from a list of strings
the user may specify the integer ordinal of the value they desire. ie:
:option:`--log-level` 3 is equivalent to :option:`--log-level` debug.

Standalone Executable Options
=============================

.. option:: --help, -h

	Display help text

	**CLI ONLY**

.. option:: --version, -V

	Display version details

	**CLI ONLY**

.. option:: --asm <integer:false:string>, --no-asm

	x265 will use all detected CPU SIMD architectures by default. You can
	disable all assembly by using :option:`--no-asm` or you can specify
	a comma separated list of SIMD architectures to use, matching these
	strings: MMX2, SSE, SSE2, SSE3, SSSE3, SSE4, SSE4.1, SSE4.2, AVX, XOP, FMA4, AVX2, FMA3

	Some higher architectures imply lower ones being present, this is
	handled implicitly.

	One may also directly supply the CPU capability bitmap as an integer.

.. option:: --threads <integer>

	Number of threads for thread pool. Default 0 (detected CPU core
	count)

.. option:: --preset, -p <integer|string>

	Sets parameters to preselected values, trading off compression efficiency against 
	encoding speed. These parameters are applied before all other input parameters are 
	applied, and so you can override any parameters that these values control.

	0. ultrafast
	1. superfast
	2. veryfast
	3. faster
	4. fast
	5. medium **(default)**
	6. slow
	7. slower
	8. veryslow
	9. placebo

.. option:: --tune, -t <string>

	Tune the settings for a particular type of source or situation. The changes will
	be applied after :option:`--preset` but before all other parameters. Default none

	**Values:** psnr, ssim, zero-latency, fast-decode.

.. option:: --frame-threads, -F <integer>

	Number of concurrently encoded frames. Using a single frame thread
	gives a slight improvement in compression, since the entire reference
	frames are always available for motion compensation, but it has
	severe performance implications. Default is an autodetected count
	based on the number of CPU cores and whether WPP is enabled or not.

.. option:: --log-level <integer|string>

	Logging level. Debug level enables per-frame QP, metric, and bitrate
	logging. If a CSV file is being generated, debug level makes the log
	be per-frame rather than per-encode. Full level enables hash and
	weight logging. -1 disables all logging, except certain fatal
	errors, and can be specified by the string "none".

	0. error
	1. warning
	2. info **(default)**
	3. debug
	4. full

.. option:: --csv <filename>

	Writes encoding results to a comma separated value log file. Creates
	the file if it doesnt already exist, else adds one line per run.  if
	:option:`--log-level` is debug or above, it writes one line per
	frame. Default none

.. option:: --output, -o <filename>

	Bitstream output file name. If there are two extra CLI options, the
	first is implicitly the input filename and the second is the output
	filename, making the :option:`--output` option optional.

	The output file will always contain a raw HEVC bitstream, the CLI
	does not support any container file formats.

	**CLI ONLY**

.. option:: --no-progress

	Disable CLI periodic progress reports

	**CLI ONLY**

Input Options
=============

.. option:: --input <filename>

	Input filename, only raw YUV or Y4M supported. Use single dash for
	stdin. This option name will be implied for the first "extra"
	command line argument.

	**CLI ONLY**

.. option:: --y4m

	Parse input stream as YUV4MPEG2 regardless of file extension,
	primarily intended for use with stdin (ie: :option:`--input` -
	:option:`--y4m`).  This option is implied if the input filename has
	a ".y4m" extension

	**CLI ONLY**

.. option:: --input-depth <integer>

	YUV only: Bit-depth of input file or stream

	**Values:** any value between 8 and 16. Default is internal depth.

	**CLI ONLY**

.. option:: --dither

	Enable high quality downscaling. Dithering is based on the diffusion
	of errors from one row of pixels to the next row of pixels in a
	picture. Only applicable when the input bit depth is larger than
	8bits and internal bit depth is 8bits. Default disabled

	**CLI ONLY**

.. option:: --input-res <wxh>

	YUV only: Source picture size [w x h]

	**CLI ONLY**

.. option:: --input-csp <integer|string>

	YUV only: Source color space. Only i420 and i444 are supported at
	this time.

	0. i400
	1. i420 **(default)**
	2. i422
	3. i444
	4. nv12
	5. nv16

.. option:: --fps <integer|float|numerator/denominator>

	YUV only: Source frame rate

	**Range of values:** positive int or float, or num/denom

.. option:: --interlaceMode <false|tff|bff>, --no-interlaceMode

	**EXPERIMENTAL** Specify interlace type of source pictures. 
	
	0. progressive pictures **(default)**
	1. top field first 
	2. bottom field first

	HEVC encodes interlaced content as fields. Fields must be provided to
	the encoder in the correct temporal order. The source dimensions
	must be field dimensions and the FPS must be in units of fields per
	second. The decoder must re-combine the fields in their correct
	orientation for display.

.. option:: --seek <integer>

	Number of frames to skip at start of input file. Default 0

	**CLI ONLY**

.. option:: --frames, -f <integer>

	Number of frames to be encoded. Default 0 (all)

	**CLI ONLY**

Quad-Tree analysis
==================

.. option:: --wpp, --no-wpp

	Enable Wavefront Parallel Processing. The encoder may begin encoding
	a row as soon as the row above it is at least two LCUs ahead in the
	encode process. This gives a 3-5x gain in parallelism for about 1%
	overhead in compression efficiency. Default: Enabled

.. option:: --ctu, -s <64|32|16>

	Maximum CU size (width and height). The larger the maximum CU size,
	the more efficiently x265 can encode flat areas of the picture,
	giving large reductions in bitrate. However this comes at a loss of
	parallelism with fewer rows of CUs that can be encoded in parallel,
	and less frame parallelism as well. Because of this the faster
	presets use a CU size of 32. Default: 64

.. option:: --tu-intra-depth <1..4>

	The transform unit (residual) quad-tree begins with the same depth
	as the coding unit quad-tree, but the encoder may decide to further
	split the transform unit tree if it improves compression efficiency.
	This setting limits the number of extra recursion depth which can be
	attempted for intra coded units. Default: 1

.. option:: --tu-inter-depth <1..4>

	The transform unit (residual) quad-tree begins with the same depth
	as the coding unit quad-tree, but the encoder may decide to further
	split the transform unit tree if it improves compression efficiency.
	This setting limits the number of extra recursion depth which can be
	attempted for inter coded units. Default: 1


Temporal / motion search options
================================

.. option:: --me <integer|string>

	Motion search method. Generally, the higher the number the harder
	the ME method will try to find an optimal match. Diamond search is
	the simplest. Hexagon search is a little better. Uneven
	Multi-Hexegon is an adaption of the search method used by x264 for
	slower presets. Star is a three step search adapted from the HM
	encoder: a star-pattern search followed by an optional radix scan
	followed by an optional star-search refinement. Full is an
	exhaustive search; an order of magnitude slower than all other
	searches but not much better than umh or star.

	0. dia
	1. hex **(default)**
	2. umh
	3. star
	4. full

.. option:: --subme, -m <0..7>

	Amount of subpel refinement to perform. The higher the number the
	more subpel iterations and steps are performed. Default 2

	+----+------------+-----------+------------+-----------+-----------+
	| -m | HPEL iters | HPEL dirs | QPEL iters | QPEL dirs | HPEL SATD |
	+====+============+===========+============+===========+===========+
	|  0 | 1          | 4         | 0          | 4         | false     |
	+----+------------+-----------+------------+-----------+-----------+
	|  1 | 1          | 4         | 1          | 4         | false     |
	+----+------------+-----------+------------+-----------+-----------+
	|  2 | 1          | 4         | 1          | 4         | true      |
	+----+------------+-----------+------------+-----------+-----------+
	|  3 | 2          | 4         | 1          | 4         | true      |
	+----+------------+-----------+------------+-----------+-----------+
	|  4 | 2          | 4         | 2          | 4         | true      |
	+----+------------+-----------+------------+-----------+-----------+
	|  5 | 1          | 8         | 1          | 8         | true      |
	+----+------------+-----------+------------+-----------+-----------+
	|  6 | 2          | 8         | 1          | 8         | true      |
	+----+------------+-----------+------------+-----------+-----------+
	|  7 | 2          | 8         | 2          | 8         | true      |
	+----+------------+-----------+------------+-----------+-----------+

.. option:: --merange <integer>

	Motion search range. Default 57

	The default is derived from the default CTU size (64) minus the luma
	interpolation half-length (4) minus maximum subpel distance (2)
	minus one extra pixel just in case the hex search method is used. If
	the search range were any larger than this, another CTU row of
	latency would be required for reference frames.

	**Range of values:** an integer from 0 to 32768

.. option:: --rect, --no-rect

	Enable analysis of rectangular motion partitions Nx2N and 2NxN
	(50/50 splits, two directions). Default enabled

.. option:: --amp, --no-amp

	Enable analysis of asymmetric motion partitions (75/25 splits, four
	directions). This setting has no effect if rectangular partitions
	are disabled. Even though there are four possible AMP partitions,
	only the most likely candidate is tested, based on the results of
	the rectangular mode tests. Default enabled

.. option:: --max-merge <1..5>

	Maximum number of neighbor (spatial and temporal) candidate blocks
	that the encoder may consider for merging motion predictions. If a
	merge candidate results in no residual, it is immediately selected
	as a "skip".  Otherwise the merge candidates are tested as part of
	motion estimation when searching for the least cost inter option.
	The max candidate number is encoded in the SPS and determines the
	bit cost of signaling merge CUs. Default 2

.. option:: --early-skip, --no-early-skip

	Measure full CU size (2Nx2N) merge candidates first; if no residual
	is found the analysis is short circuited. Default disabled

.. option:: --fast-cbf, --no-fast-cbf

	Short circuit analysis if a prediction is found that does not set
	the coded block flag (aka: no residual was encoded).  It prevents
	the encoder from perhaps finding other predictions that also have no
	residual but require less signaling bits. Default disabled

.. option:: --ref <1..16>

	Max number of L0 references to be allowed. This number has a linear
	multiplier effect on the amount of work performed in motion search,
	but will generally have a beneficial affect on compression and
	distortion. Default 3

.. option:: --weightp, -w, --no-weightp

	Enable weighted prediction in P slices. This enables weighting
	analysis in the lookahead, which influences slice decisions, and
	enables weighting analysis in the main encoder which allows P
	reference samples to have a weight function applied to them prior to
	using them for motion compensation.  In video which has lighting
	changes, it can give a large improvement in compression efficiency.
	Default is enabled


.. option:: --weightb, --no-weightb

	Enable weighted prediction in B slices. Default disabled


Spatial/intra options
=====================

.. option:: --rdpenalty <0..2>

	Penalty for 32x32 intra TU in non-I slices. Default 0

	**Values:** 0:disabled 1:RD-penalty 2:maximum

.. option:: --tskip, --no-tskip

	Enable intra transform skipping (encode residual as coefficients)
	for intra coded blocks. Default disabled

.. option:: --tskip-fast, --no-tskip-fast

	Enable fast intra transform skip decisions. Only applicable if
	transform skip is enabled. Default disabled

.. option:: --strong-intra-smoothing, --no-strong-intra-smoothing

	Enable strong intra smoothing for 32x32 intra blocks. Default enabled

.. option:: --constrained-intra, --no-constrained-intra

	Constrained intra prediction (use only intra coded reference pixels)
	Default disabled


Slice decision options
======================

.. option:: --open-gop, --no-open-gop

	Enable open GOP, allow I-slices to be non-IDR. Default enabled

.. option:: --keyint, -I <integer>

	Max intra period in frames. A special case of infinite-gop (single
	keyframe at the beginning of the stream) can be triggered with
	argument -1. Use 1 to force all-intra. Default 250

.. option:: --min-keyint, -i <integer>

	Minimum GOP size. Scenecuts closer together than this are coded as I or P, not IDR. 

	**Range of values:** >=0 (0: auto)

.. option:: --scenecut <integer>, --no-scenecut

	How aggressively I-frames need to be inserted. The higher the
	threshold value, the more aggressive the I-frame placement.
	:option:`--scenecut` 0 or :option:`--no-scenecut` disables adaptive
	I frame placement. Default 40

.. option:: --rc-lookahead <integer>

	Number of frames for slice-type decision lookahead (a key
	determining factor for encoder latency). The longer the lookahead
	buffer the more accurate scenecut decisions will be, and the more
	effective cuTree will be at improving adaptive quant. Having a
	lookahead larger than the max keyframe interval is not helpful.
	Default 20

	**Range of values:** Between the maximum consecutive bframe count (:option:`--bframes`) and 250

.. option:: --b-adapt <integer>

	Adaptive B frame scheduling. Default 2

	**Values:** 0:none; 1:fast; 2:full(trellis)

.. option:: --bframes, -b <0..16>

	Maximum number of consecutive b-frames. Use :option:`--bframes` 0 to
	force all P/I low-latency encodes. Default 4. This parameter has a
	quadratic effect on the amount of memory allocated and the amount of
	work performed by the full trellis version of :option:`--b-adapt`
	lookahead.

.. option:: --bframe-bias <integer>

	Bias towards B frames in slicetype decision. The higher the bias the
	more likely x265 is to use B frames. Can be any value between -20
	and 100, but is typically between 10 and 30. Default 0

.. option:: --b-pyramid, --no-b-pyramid

	Use B-frames as references, when possible. Default enabled

Quality, rate control and rate distortion options
=================================================

.. option:: --bitrate <integer>

	Enables single-pass ABR rate control. Specify the target bitrate in
	kbps. Default is 0 (CRF)

	**Range of values:** An integer greater than 0

.. option:: --crf <0..51.0>

	Quality-controlled variable bitrate. CRF is the default rate control
	method; it does not try to reach any particular bitrate target,
	instead it tries to achieve a given uniform quality and the size of
	the bitstream is determined by the complexity of the source video.
	The higher the rate factor the higher the quantization and the lower
	the quality. Default rate factor is 28.0.

.. option:: --max-crf <0..51.0>

	Specify an upper limit to the rate factor which may be assigned to
	any given frame (ensuring a max QP).  This is dangerous when CRF is
	used in combination with VBV as it may result in buffer underruns.
	Default disabled

.. option:: --vbv-bufsize <integer>

	Specify the size of the VBV buffer (kbits). Enables VBV in ABR
	mode.  In CRF mode, :option:`--vbv-maxrate` must also be specified.
	Default 0 (vbv disabled)

.. option:: --vbv-maxrate <integer>

	Maximum local bitrate (kbits/sec). Will be used only if vbv-bufsize
	is also non-zero. Both vbv-bufsize and vbv-maxrate are required to
	enable VBV in CRF mode. Default 0 (disabled)

.. option:: --vbv-init <float>

	Initial buffer occupancy. The portion of the decode buffer which
	must be full before the decoder will begin decoding.  Determines
	absolute maximum frame size. Default 0.9

	**Range of values:** 0 - 1.0

.. option:: --qp, -q <integer>

	Specify base quantization parameter for Constant QP rate control.
	Using this option enables Constant QP rate control. The specified QP
	is assigned to P slices. I and B slices are given QPs relative to P
	slices using param->rc.ipFactor and param->rc.pbFactor.  Default 0
	(CRF)

	**Range of values:** an integer from 0 to 51

.. option:: --aq-mode <0|1|2>

	Adaptive Quantization operating mode. Raise or lower per-block
	quantization based on complexity analysis of the source image. The
	more complex the block, the more quantization is used. This offsets
	the tendency of the encoder to spend too many bits on complex areas
	and not enough in flat areas.

	0. disabled
	1. AQ enabled **(default)**
	2. AQ enabled with auto-variance

.. option:: --aq-strength <float>

	Adjust the strength of the adaptive quantization offsets. Setting
	:option:`--aq-strength` to 0 disables AQ. Default 1.0.

	**Range of values:** 0.0 to 3.0

.. option:: --cutree, --no-cutree

	Enable the use of lookahead's lowres motion vector fields to
	determine the amount of reuse of each block to tune adaptive
	quantization factors. CU blocks which are heavily reused as motion
	reference for later frames are given a lower QP (more bits) while CU
	blocks which are quickly changed and are not referenced are given
	less bits. This tends to improve detail in the backgrounds of video
	with less detail in areas of high motion. Default enabled

.. option:: --cbqpoffs <integer>

	Offset of Cb chroma QP from the luma QP selected by rate control.
	This is a general way to spend more or less bits on the chroma
	channel.  Default 0

	**Range of values:** -12 to 12

.. option:: --crqpoffs <integer>

	Offset of Cr chroma QP from the luma QP selected by rate control.
	This is a general way to spend more or less bits on the chroma
	channel.  Default 0

	**Range of values:**  -12 to 12

.. option:: --rd <0..6>

	Level of RDO in mode decision. The higher the value, the more
	exhaustive the analysis and the more rate distortion optimization is
	used. The lower the value the faster the encode, the higher the
	value the smaller the bitstream (in general). Default 3

	**Range of values:** 0: least .. 6: full RDO analysis

.. option:: --signhide, --no-signhide

	Hide sign bit of one coeff per TU (rdo). Default enabled
 
Loop filter
===========

.. option:: --lft, --no-lft

	Toggle deblocking loop filter, default enabled

.. option:: --sao, --no-sao

	Toggle Sample Adaptive Offset loop filter, default enabled

.. option:: --sao-lcu-bounds <0|1>

	How to handle depencency with deblocking filter

	0. right/bottom boundary areas skipped **(default)**
	1. non-deblocked pixels are used

.. option:: --sao-lcu-opt <0|1>

	Frame level or block level optimization

	0. SAO picture-based optimization (prevents frame parallelism,
	   effectively causes :option:`--frame-threads` 1)
	1. SAO LCU-based optimization **(default)**

Quality reporting metrics
=========================

.. option:: --ssim, --no-ssim

	Calculate and report Structural Similarity values. It is
	recommended to use :option:`--tune` ssim if you are measuring ssim,
	else the results should not be used for comparison purposes.
	Default disabled

.. option:: --psnr, --no-psnr

	Calculate and report Peak Signal to Noise Ratio.  It is recommended
	to use :option:`--tune` psnr if you are measuring PSNR, else the
	results should not be used for comparison purposes.  Default
	disabled

VUI (Video Usability Information) options
=========================================

By default x265 does not emit a VUI in the SPS, but if you specify any
of the VUI fields (:option:`--sar`, :option:`--range`, etc) the VUI is
implicitly enabled.

.. option:: --sar <integer|w:h>

	Sample Aspect Ratio, the ratio of width to height of an individual
	sample (pixel). The user may supply the width and height explicitly
	or specify an integer from the predefined list of aspect ratios
	defined in the HEVC specification.  Default undefined

	1. 1:1 (square)
	2. 12:11
	3. 10:11
	4. 16:11
	5. 40:33
	6. 24:11
	7. 20:11
	8. 32:11
	9. 80:33
	10. 18:11
	11. 15:11
	12. 64:33
	13. 160:99
	14. 4:3
	15. 3:2
	16. 2:1

.. option:: --crop-rect <left,top,right,bottom>

	Define the (overscan) region of the image that does not contain
	information because it was added to achieve certain resolution or
	aspect ratio. The decoder may be directed to crop away this region
	before displaying the images via the :option:`--overscan` option.
	Default undefined

.. option:: --overscan <show|crop>

	Specify whether it is appropriate for the decoder to display or crop
	the overscan area. Default unspecified

.. option:: --videoformat <integer|string>

	Specify the source format of the original analog video prior to
	digitizing and encoding. Default undefined

	0. component
	1. pal
	2. ntsc
	3. secam
	4. mac
	5. undefined

.. option:: --range <full|limited>

	Specify output range of black level and range of luma and chroma
	signals. Default undefined

.. option:: --colorprim <integer|string>

	Specify color primitive to use when converting to RGB. Default
	undefined

	1. bt709
	2. undef
	3. **reserved**
	4. bt470m
	5. bt470bg
	6. smpte170m
	7. smpte240m
	8. film
	9. bt2020

.. option:: --transfer <integer|string>

	Specify transfer characteristics. Default undefined

	1. bt709
	2. undef
	3. **reserved**
	4. bt470m
	5. bt470bg
	6. smpte170m
	7. smpte240m
	8. linear
	9. log100
	10. log316
	11. iec61966-2-4
	12. bt1361e
	13. iec61966-2-1
	14. bt2020-10
	15. bt2020-12

.. option:: --colormatrix <integer|string>

	Specify color matrix setting i.e set the matrix coefficients used in
	deriving the luma and chroma. Default undefined

	0. GBR
	1. bt709
	2. undef 
	3. **reserved**
	4. fcc
	5. bt470bg
	6. smpte170m
	7. smpte240m
	8. YCgCo
	9. bt2020nc
	10. bt2020c

.. option:: --chromalocs <0..5>

	Specify chroma sample location for 4:2:0 inputs. Default undefined
	Consult the HEVC specification for a description of these values.

.. option:: --timinginfo, --no-timinginfo

	Add timing information to the VUI. This is identical to the timing
	info reported in the PPS header but is sometimes required.  Default
	disabled

Bitstream options
=================

.. option:: --aud, --no-aud

	Emit an access unit delimiter NAL at the start of each slice access
	unit. If option:`--repeat-headers` is not enabled (indicating the
	user will be writing headers manually at the start of the stream)
	the very first AUD will be skipped since it cannot be placed at the
	start of the access unit, where it belongs. Default disabled

Debugging options
=================

.. option:: --hash <integer>

	Emit decoded picture hash SEI, to validate encoder state. Default None

	1. MD5
	2. CRC
	3. Checksum

.. option:: --recon, -r <filename>

	Output file containing reconstructed images in display order. If the
	file extension is ".y4m" the file will contain a YUV4MPEG2 stream
	header and frame headers. Otherwise it will be a raw YUV file in the
	encoder's internal bit depth.

	**CLI ONLY**

.. option:: --recon-depth <integer>

	Bit-depth of output file. This value defaults to the internal bit
	depth and currently cannot to be modified.

	**CLI ONLY**

API-only Options
================

These options are not exposed in the CLI because they are only useful to
applications which use libx265 as a shared library.  These are available
via x265_param_parse()

.. option:: --repeat-headers

	If enabled, x265 will emit VPS, SPS, and PPS headers with every
	keyframe. This is intended for use when you do not have a container
	to keep the stream headers for you and you want keyframes to be
	random access points.

.. vim: noet
