#ifdef INIPARSER
	#include "../lib/iniparser/src/iniparser.h"
#else
	#include <iniparser.h>
#endif

#ifdef SNDIO
	#include <sndio.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include <math.h>

#include "output/graphical.h"
#include "config.h"

char *inputMethod, *outputMethod, *channels;

int validate_color(char *checkColor, int om)
{
	int validColor = 0;
	if (checkColor[0] == '#' && strlen(checkColor) == 7) {
		// 0 to 9 and a to f
		for (int i = 1; checkColor[i]; ++i) {
			if (!isdigit(checkColor[i])) {
				if (tolower(checkColor[i]) >= 'a' && tolower(checkColor[i]) <= 'f') {
					validColor = 1;
				} else {
					validColor = 0;
					break;
				}
			} else {
				validColor = 1;
			}
		}
	} else {
		if ((strcmp(checkColor, "black") == 0) || \
			(strcmp(checkColor, "red") == 0) || \
			(strcmp(checkColor, "green") == 0) || \
			(strcmp(checkColor, "yellow") == 0) || \
			(strcmp(checkColor, "blue") == 0) || \
			(strcmp(checkColor, "magenta") == 0) || \
			(strcmp(checkColor, "cyan") == 0) || \
			(strcmp(checkColor, "white") == 0) || \
			(strcmp(checkColor, "default") == 0)) validColor = 1;
	}
	return validColor;
}

void validate_config(char supportedInput[255], void* params)
{
	struct config_params *p = (struct config_params *)params;
	
	// validate: input method
	p->im = 0;
	if (strcmp(inputMethod, "alsa") == 0) {
		p->im = 1;
		#ifndef ALSA
		        fprintf(stderr,
	                        "xava was built without alsa support, install alsa dev files and run make clean && ./configure && make again\n");
	                exit(EXIT_FAILURE);
	        #endif
	}
	if (strcmp(inputMethod, "fifo") == 0) {
		p->im = 2;
	}
	if (strcmp(inputMethod, "pulse") == 0) {
		p->im = 3;
		#ifndef PULSE
		        fprintf(stderr,
	                        "xava was built without pulseaudio support, install pulseaudio dev files and run make clean && ./configure && make again\n");
	                exit(EXIT_FAILURE);
	        #endif
	
	}
	if (strcmp(inputMethod, "sndio") == 0) {
		p->im = 4;
		#ifndef SNDIO
			fprintf(stderr, "xava was built without sndio support\n");
			exit(EXIT_FAILURE);
		#endif
	}
	if (strcmp(inputMethod, "portaudio") == 0) {
		p->im = 5;
		#ifndef PORTAUDIO
			fprintf(stderr, "xava was built without portaudio support\n");
			exit(EXIT_FAILURE);
		#endif
	}
	if (strcmp(inputMethod, "shmem") == 0) {
		p->im = 6;
		#ifndef SHMEM
			fprintf(stderr, "xava was built without shmem support\n");
			exit(EXIT_FAILURE);
		#endif
	}
	if (p->im == 0) {
		fprintf(stderr,
			"input method '%s' is not supported, supported methods are: %s\n",
						inputMethod, supportedInput);
		exit(EXIT_FAILURE);
	}
	
	// validate: output method
	p->om = 0;
	if (strcmp(outputMethod, "raw") == 0) {//raw:
		p->om = 4;
		p->bs = 0;
		p->bw = 1;

		//checking data format
		p->is_bin = -1;
		if (strcmp(p->data_format, "binary") == 0) {
			p->is_bin = 1;
			//checking bit format:
			if (p->bit_format != 8 && p->bit_format != 16 ) {
			fprintf(stderr,
				"bit format  %d is not supported, supported data formats are: '8' and '16'\n",
							p->bit_format );
			exit(EXIT_FAILURE);
		
			}
		} else if (strcmp(p->data_format, "ascii") == 0) {
			p->is_bin = 0;
			if (p->ascii_range < 1 ) {
			fprintf(stderr,
				"ascii max value must be a positive integer\n");
			exit(EXIT_FAILURE);
			}
		} else {
		fprintf(stderr,
			"data format %s is not supported, supported data formats are: 'binary' and 'ascii'\n",
						p->data_format);
		exit(EXIT_FAILURE);
		
		}
	
	}
	if(strcmp(outputMethod, "x") == 0)
	{
		p->om = 5;
		#ifndef XLIB
			fprintf(stderr,
				"xava was built without Xlib support, install Xlib dev files and run make clean && ./configure && make again\n");
			exit(EXIT_FAILURE);
		#endif
	}
	if(strcmp(outputMethod, "sdl") == 0)
	{
		p->om = 6;
		#ifndef SDL
			fprintf(stderr,
				"xava was build without SDL2 support, install SDL2 dev files and run make clean && ./configure && make again\n");
			exit(EXIT_FAILURE);
		#endif
	}
	if(strcmp(outputMethod, "win") == 0)
	{
		p->om = 7;
		#ifndef WIN
			fprintf(stderr,
				"xava was build without win32 support, you need to be running windows in order for win32 to work :P\n");
			exit(EXIT_FAILURE);
		#endif
	}
	if (p->om == 0) {
		fprintf(stderr,
			"output method %s is not supported, supported methods are: 'raw'",
						outputMethod);
		#ifdef XLIB
			fprintf(stderr, ", 'x'");
		#endif
		#ifdef SDL
			fprintf(stderr, ", 'sdl'");
		#endif
		#ifdef WIN
			fprintf(stderr, ", 'win'");
		#endif
		// Just a quick question, should you add 'circle' and 'raw' here?
		fprintf(stderr, "\n");
	
		exit(EXIT_FAILURE);
	}
	// validate: output channels
	p->stereo = -1;
	if (strcmp(channels, "mono") == 0) p->stereo = 0;
	if (strcmp(channels, "stereo") == 0) p->stereo = 1;
	if (p->stereo == -1) {
		fprintf(stderr,
			"output channels %s is not supported, supported channelss are: 'mono' and 'stereo'\n",
						channels);
		exit(EXIT_FAILURE);
	}
	
	// validate: bars
	p->autobars = 1;
	if (p->fixedbars > 0) p->autobars = 0;
	if (p->fixedbars > 200) p->fixedbars = 200;
	if (p->bw > 200) p->bw = 200;
	if (p->bw < 1) p->bw = 1;
	
	// validate: framerate
	if (p->framerate <= 0) {
		fprintf(stderr,
			"framerate can't equal or lower than 0!\n");
		exit(EXIT_FAILURE);
	}
	
	// validate: color
	if (!validate_color(p->color, p->om)) {
		fprintf(stderr, "The value for 'foreground' is invalid. It can be either one of the 7 named colors or a HTML color of the form '#xxxxxx'.\n");
		exit(EXIT_FAILURE);
	}
	
	// validate: background color
	if (!validate_color(p->bcolor, p->om)) {
		fprintf(stderr, "The value for 'background' is invalid. It can be either one of the 7 named colors or a HTML color of the form '#xxxxxx'.\n");
		exit(EXIT_FAILURE);
	}
	
	if (p->gradient) { 
		for(int i = 0;i < p->gradient_count;i++){
			if (!validate_color(p->gradient_colors[i], p->om)) {
				fprintf(stderr, "The first gradient color is invalid. It must be HTML color of the form '#xxxxxx'.\n");
				exit(EXIT_FAILURE);
			}
		}
	}
	
	// In case color is not html format set bgcol and col to predefinedint values
	p->col = 6;
	if (strcmp(p->color, "black") == 0) p->col = 0;
	if (strcmp(p->color, "red") == 0) p->col = 1;
	if (strcmp(p->color, "green") == 0) p->col = 2;
	if (strcmp(p->color, "yellow") == 0) p->col = 3;
	if (strcmp(p->color, "blue") == 0) p->col = 4;
	if (strcmp(p->color, "magenta") == 0) p->col = 5;
	if (strcmp(p->color, "cyan") == 0) p->col = 6;
	if (strcmp(p->color, "white") == 0) p->col = 7;
	// default if invalid
	
	// validate: background color
	if (strcmp(p->bcolor, "black") == 0) p->bgcol = 0;
	if (strcmp(p->bcolor, "red") == 0) p->bgcol = 1;
	if (strcmp(p->bcolor, "green") == 0) p->bgcol = 2;
	if (strcmp(p->bcolor, "yellow") == 0) p->bgcol = 3;
	if (strcmp(p->bcolor, "blue") == 0) p->bgcol = 4;
	if (strcmp(p->bcolor, "magenta") == 0) p->bgcol = 5;
	if (strcmp(p->bcolor, "cyan") == 0) p->bgcol = 6;
	if (strcmp(p->bcolor, "white") == 0) p->bgcol = 7;
	// default if invalid
	
	
	// validate: gravity
	p->gravity = p->gravity / 100;
	if (p->gravity < 0) {
		p->gravity = 0;
	} 

	// validate: oddoneout
	if(p->stereo&&p->oddoneout) {
		fprintf(stderr, "oddoneout cannot work with stereo mode on\n");
		exit(EXIT_FAILURE);
	}
	
	// validate: integral
	p->integral = p->integral / 100;
	if (p->integral < 0) {
		p->integral = 0;
	} else if (p->integral > 1) {
		p->integral = 1;
	}
	
	// validate: cutoff
	if (p->lowcf == 0 ) p->lowcf++;
	if (p->lowcf > p->highcf) {
		fprintf(stderr,
			"lower cutoff frequency can't be higher than higher cutoff frequency\n");
		exit(EXIT_FAILURE);
	}
	
	//setting sens
	p->sens = p->sens / 100;
	
	// validate: window settings
	if(p->om == 5 || p->om == 6 || p->om == 7) {
		// validate: alignment
		if(strcmp(windowAlignment, "top_left"))
		if(strcmp(windowAlignment, "top_right"))
		if(strcmp(windowAlignment, "bottom_left"))
		if(strcmp(windowAlignment, "bottom_right"))
		if(strcmp(windowAlignment, "left"))
		if(strcmp(windowAlignment, "right"))
		if(strcmp(windowAlignment, "top"))
		if(strcmp(windowAlignment, "bottom"))
		if(strcmp(windowAlignment, "center"))
		if(strcmp(windowAlignment, "none"))
			fprintf(stderr, "The value for alignment is invalid, '%s'!", windowAlignment);
		
		if(p->foreground_opacity > 1.0) {
			fprintf(stderr, "foreground_opacity cannot be above 1.0\n");
			exit(EXIT_FAILURE);
		}
	
		// TUI used x8 height, let's fix that
		p->sens /= 8;
	}
	
	// validate: shadow
	if(sscanf(p->shadow_color, "#%x", &p->shdw_col) != 1)
	{
		fprintf(stderr, "shadow color is improperly formatted!\n");
		exit(EXIT_FAILURE);
	}
}

void load_config(char configPath[255], char supportedInput[255], void* params)
{
	struct config_params *p = (struct config_params *)params;
	FILE *fp;
		
	//config: creating path to default config file
	if (configPath[0] == '\0') {
		char *configFile = "config";
		#if defined(__unix__)||defined(__APPLE__)
			char *configHome = getenv("XDG_CONFIG_HOME");
		#elif defined(WIN)
			char *configHome = getenv("APPDATA");
		#endif
		if (configHome != NULL) {
			sprintf(configPath,"%s/%s/", configHome, PACKAGE);
		} else {
			configHome = getenv("HOME");
			if (configHome != NULL) {
				sprintf(configPath,"%s/%s/%s/", configHome, ".config", PACKAGE);
			} else {
				printf("No HOME found (ERR_HOMELESS), exiting...");
				exit(EXIT_FAILURE);
			}
		}
	
		// config: create directory
		#if defined(__unix__)||defined(__APPLE__)
			mkdir(configPath, 0777);
		#else
			mkdir(configPath);
		#endif	
	
		// config: adding default filename file
		strcat(configPath, configFile);
		
		fp = fopen(configPath, "ab+");
		if (fp) {
			fclose(fp);
		} else {
			printf("Unable to access config '%s', exiting...\n", configPath);
			exit(EXIT_FAILURE);
		}
	
	
	} else { //opening specified file
	
		fp = fopen(configPath, "rb+");	
		if (fp) {
			fclose(fp);
		} else {
			printf("Unable to open file '%s', exiting...\n", configPath);
			exit(EXIT_FAILURE);
		}
	}
	
	// config: parse ini
	dictionary* ini;
	ini = iniparser_load(configPath);
	
	// setting fifo to default if no other input modes supported
	inputMethod = (char *)iniparser_getstring(ini, "input:method", "fifo"); 
	
	// setting alsa to default if supported
	#ifdef ALSA
		inputMethod = (char *)iniparser_getstring(ini, "input:method", "alsa"); 
	#endif
	
	// portaudio is priority 2 (sorted by difficulty)
	#ifdef PORTAUDIO
		inputMethod = (char *)iniparser_getstring(ini, "input:method", "portaudio");
	#endif

	// pulse is basically autoconfig (so priority 1)
	#ifdef PULSE
		inputMethod = (char *)iniparser_getstring(ini, "input:method", "pulse");
	#endif

	#if defined(__linux__)||defined(__APPLE__)||defined(__unix__)
		outputMethod = (char *)iniparser_getstring(ini, "output:method", "x");
	#elif defined(__WIN32__)
		outputMethod = (char *)iniparser_getstring(ini, "output:method", "win");
	#else	
		outputMethod = (char *)iniparser_getstring(ini, "output:method", "sdl");
	#endif

	p->fftsize = (int)exp2((float)iniparser_getint(ini, "smoothing:fft_size", 16));
	p->integral = iniparser_getdouble(ini, "smoothing:integral", 90);
	p->gravity = iniparser_getdouble(ini, "smoothing:gravity", 100);
	p->ignore = iniparser_getdouble(ini, "smoothing:ignore", 0);
	p->logScale = iniparser_getdouble(ini, "smoothing:log", 1.0);
	p->oddoneout = iniparser_getdouble(ini, "smoothing:oddoneout", 1);
	p->eqBalance = iniparser_getdouble(ini, "smoothing:eq_balance", 0.64);
	
	p->color = (char *)iniparser_getstring(ini, "color:foreground", "default");
	p->bcolor = (char *)iniparser_getstring(ini, "color:background", "default");
	p->foreground_opacity = iniparser_getdouble(ini, "color:foreground_opacity", 1.0);
	
	p->gradient = iniparser_getint(ini, "color:gradient", 0);
	if (p->gradient) {
	    p->gradient_count = iniparser_getint(ini, "color:gradient_count", 2);
		if(p->gradient_count < 2){
		    printf("\nAtleast two colors must be given as gradient!\n");
		    exit(EXIT_FAILURE);
		}
		if(p->gradient_count > 8){
		    printf("\nMaximum 8 colors can be specified as gradient!\n");
		    exit(EXIT_FAILURE);
		}
		p->gradient_colors = (char **)malloc(sizeof(char*) * p->gradient_count);
	    for(int i = 0;i < p->gradient_count;i++){
	        char ini_config[23];
	        sprintf(ini_config, "color:gradient_color_%d", (i + 1));
	        p->gradient_colors[i] = (char *)iniparser_getstring(ini, ini_config, NULL);
	        if(p->gradient_colors[i] == NULL){
	            printf("\nGradient color not specified : gradient_color_%d\n", (i + 1));
	            exit(EXIT_FAILURE);
	        }
	    }
	    //p->gradient_color_1 = (char *)iniparser_getstring(ini, "color:gradient_color_1", "#0099ff");
		//p->gradient_color_2 = (char *)iniparser_getstring(ini, "color:gradient_color_2", "#ff3399");
	}
	
	p->fixedbars = iniparser_getint(ini, "general:bars", 0);
	p->bw = iniparser_getint(ini, "general:bar_width", 13);
	p->bs = iniparser_getint(ini, "general:bar_spacing", 5);
	p->framerate = iniparser_getint(ini, "general:framerate", 60);
	p->sens = iniparser_getint(ini, "general:sensitivity", 100);
	p->autosens = iniparser_getint(ini, "general:autosens", 1);
	p->overshoot = iniparser_getint(ini, "general:overshoot", 0);
	p->lowcf = iniparser_getint(ini, "general:lower_cutoff_freq", 20);
	p->highcf = iniparser_getint(ini, "general:higher_cutoff_freq", 20000);
	
	// config: window
	#ifdef GLX
		GLXmode = iniparser_getint(ini, "window:opengl", 1);
	#endif
	
	p->w = iniparser_getint(ini, "window:width", 1180);
	p->h = iniparser_getint(ini, "window:height", 300);
	
	windowAlignment = (char *)iniparser_getstring(ini, "window:alignment", "none");
	windowX = iniparser_getint(ini, "window:x_padding", 0);
	windowY = iniparser_getint(ini, "window:y_padding", 0);
	fs = iniparser_getboolean(ini, "window:fullscreen", 0);
	transparentFlag = iniparser_getboolean(ini, "window:transparency", 0);
	borderFlag = iniparser_getboolean(ini, "window:border", 1);
	keepInBottom = iniparser_getboolean(ini, "window:keep_below", 0);
	interactable = iniparser_getboolean(ini, "window:interactable", 1);
	p->set_win_props = iniparser_getint(ini, "window:set_win_props", 0);
	
	// config: output
	channels =  (char *)iniparser_getstring(ini, "output:channels", "mono");
	p->raw_target = (char *)iniparser_getstring(ini, "output:raw_target", "/dev/stdout");
	p->data_format = (char *)iniparser_getstring(ini, "output:data_format", "binary");
	p->bar_delim = (char)iniparser_getint(ini, "output:bar_delimiter", 59);
	p->frame_delim = (char)iniparser_getint(ini, "output:frame_delimiter", 10);
	p->ascii_range = iniparser_getint(ini, "output:ascii_max_range", 1000);
	p->bit_format = iniparser_getint(ini, "output:bit_format", 16);
	
	// config: shadow
	p->shdw = iniparser_getint(ini, "shadow:size", 0);
	p->shadow_color = (char *)iniparser_getstring(ini, "shadow:color", "#ff000000");	
	if(sscanf(p->shadow_color, "#%x", &p->shdw_col) != 1)
	{
		fprintf(stderr, "shadow color is improperly formatted!\n");
		exit(EXIT_FAILURE);
	}
	
	
	// read & validate: eq
	p->smcount = iniparser_getsecnkeys(ini, "eq");
	if (p->smcount > 0) {
		p->smooth = malloc(p->smcount*sizeof(double));
		#ifndef LEGACYINIPARSER
		const char *keys[p->smcount];
		iniparser_getseckeys(ini, "eq", keys);
		#else
		char **keys = iniparser_getseckeys(ini, "eq");
		#endif
		for (int sk = 0; sk < p->smcount; sk++) {
			p->smooth[sk] = iniparser_getdouble(ini, keys[sk], 1);
		}
	} else {
		p->smcount = 64; //back to the default one
		p->smooth = malloc(p->smcount*sizeof(double));
		for(int i=0; i<64; i++) p->smooth[i]=1.0f;
	}
	
	// config: input
	p->im = 0;
	if (strcmp(inputMethod, "alsa") == 0) {
		p->im = 1;
		p->audio_source = (char *)iniparser_getstring(ini, "input:source", "hw:Loopback,1");
	}
	if (strcmp(inputMethod, "fifo") == 0) {
		p->im = 2;
		p->audio_source = (char *)iniparser_getstring(ini, "input:source", "/tmp/mpd.fifo");
	}
	if (strcmp(inputMethod, "pulse") == 0) {
		p->im = 3;
		p->audio_source = (char *)iniparser_getstring(ini, "input:source", "auto");
	}
	#ifdef SNDIO
	if (strcmp(inputMethod, "sndio") == 0) {
		p->im = 4;
		p->audio_source = (char *)iniparser_getstring(ini, "input:source", SIO_DEVANY);
	}
	#endif
	#ifdef PORTAUDIO
	if (strcmp(inputMethod, "portaudio") == 0) {
		p->im = 5;
		p->audio_source = (char *)iniparser_getstring(ini, "input:source", "auto");
	}
	#endif
	#ifdef SHMEM
	if (strcmp(inputMethod, "shmem") == 0) {
		p->im = 6;
		p->audio_source = (char *)iniparser_getstring(ini, "input:source", "/squeezelite-00:00:00:00:00:00");
	}
	#endif
		
	validate_config(supportedInput, params);
	//iniparser_freedict(ini);
}
