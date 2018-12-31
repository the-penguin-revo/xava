struct config_params {
	char *color, *bcolor, *raw_target, *audio_source, **gradient_colors, *shadow_color, *data_format;
	char bar_delim, frame_delim, oddoneout;
	double integral, gravity, ignore, sens, foreground_opacity, logScale, logBegin, logEnd, eqBalance; 
	unsigned int lowcf, highcf, shdw, shdw_col;
	double *smooth;
	int smcount, im, om, col, bgcol, autobars, stereo, is_bin, ascii_range,
 bit_format, gradient, fixedbars, framerate, bw, bs, autosens, overshoot, 
 set_win_props, w, h, fftsize, gradient_count;
};
void load_config(char configPath[255], char supportedInput[255], void* p);
