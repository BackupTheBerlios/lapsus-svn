/***************************************************************************
 *   Copyright (C) 2007 by Jakub Schmidtke                                 *
 *   sjakub@gmail.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.           *
 ***************************************************************************/

#include <qimage.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERSION			"0.02"

#define ACTION_HELP		0
#define ACTION_DISABLE		1
#define ACTION_ENABLE		2
#define ACTION_STATIC		3
#define ACTION_ROLL		4
#define ACTION_FLASH		5
#define ACTION_FLASH_ROLL	6

#define OUTPUT_TYPE_BIN		0
#define OUTPUT_TYPE_ASCII	1

#define MAX_WIDTH		1792
#define MAX_HEIGHT		32
#define DATA_SIZE		(MAX_WIDTH*MAX_HEIGHT)

#define COL_BLACK		0
#define COL_WHITE		1
#define COL_BLACK_CORR		2

/* What action should be performed? */
uint8_t what_action = ACTION_HELP;

/* Should there also be enable/disable action? */
uint8_t action_enable = ACTION_HELP;

/* Type of the output to be generated ( zeros and ones for BIN and # and spaces for ASCII) */
uint8_t output_type = OUTPUT_TYPE_BIN;

/* Optional width/height specified */
uint32_t width = 0;
uint32_t height = 0;

/* Special operations on data */
/* Invert values */
uint8_t do_invert = 0;

/* Do 'BlackBackground' operation */
uint8_t do_bb_l = 0;
uint8_t do_bb_r = 0;
uint8_t do_bb_t = 0;
uint8_t do_bb_b = 0;

/* Input file */
char *input_file = 0;

/* File where the output should be written instead of writing to /sys/... file */
char *output_file = 0;

/* Data buf used for our image map */
uint8_t data[DATA_SIZE];

#define DATA(x,y)		(data[(y)*MAX_WIDTH + (x)])

void show_help()
{
	printf("\nA program converting graphic files to the format\n");
	printf("recognized by Asus OLED kernel module.\n\n");

	printf("Usage: asus_oled [options]\n\n");
	printf("Possible options (more than one option is accepted at a time):\n");
	printf(" -e       - enables the display (may be used together with any other option like -s -r ...)\n");
	printf(" -d       - disables the display (may be used together with any other option like -s -r ...)\n");
	printf(" -s  file - creates static image from given graphic file\n");
	printf(" -r  file - creates rolling image from given graphic file\n");
	printf(" -f  file - creates flashing image from given graphic file\n");
	printf(" -fr file - creates flashing and rolling image from given graphic file\n");
	printf(" -h  num  - sets desired height of the picture to 'num'. Possible ranges:\n");
	printf("             <1; 32> in 's' and 'r' modes\n");
	printf("             <1; 16> in 'f' mode\n");
	printf("             <17; 32> in 'fr' mode\n");
	printf("             This parameter is optional\n");
	printf(" -w  num  - sets desired width of the picture to 'num'. Possible ranges:\n");
	printf("             <1; 128> in 's', 'f' and 'fr' modes\n");
	printf("             <1; 1792> in 'r' mode\n");
	printf("             This parameter is optional\n");
	printf(" -o  file - writes output data to 'file'. If output file is not specified, the\n");
	printf("             data will be written to /sys/... file  (if found)\n");
	printf("\n Modificators:\n");
	printf(" -a       - sets output data format to ASCII (# and ' ' - space)\n");
	printf(" -0       - sets output data format to BIN (1 and 0)\n");
	printf("\n Image manipulation:\n");
	printf(" -inv     - invert the values\n");
	printf("\n * BlackBackground operation - it sets background pixels to 'black', leaving only 1 white pixel\n");
	printf("     from one side between now-black background and the black target in center\n");
	printf(" -bb_l    - BlackBackground operation from left side of the picture\n");
	printf(" -bb_r    - BlackBackground operation from right side of the picture\n");
	printf(" -bb_t    - BlackBackground operation from top side of the picture\n");
	printf(" -bb_b    - BlackBackground operation from bottom side of the picture\n");
	printf(" -bb_h    - Alias for bb_l and bb_r - horizontal\n");
	printf(" -bb_v    - Alias for bb_t and bb_b - vertical\n");
	printf(" -bb      - Alias for all bb operations\n");
	printf("\n       Any set of -bb_* parameters might be used at a time\n");
	printf("\n");
	printf(" -V       - shows version and quits\n");
	printf(" -?       - shows this help and quits\n");
	
	printf("\nVisit http://lapsus.berlios.de/asus_oled.html for details.\n\n\n");
	printf("Copyright (c) 2007 Jakub Schmidtke\n\n");
	printf("This program is distributed under the terms of the GPL v2.\n");
	printf("Visit http://www.gnu.org/licenses/gpl.html for details.\n\n");
}

void show_version()
{
	printf("Asus OLED data converter and control utility ver. " VERSION "\n");
	return;
}


uint8_t get_filename(int argc, char *argv[], int i, char **file)
{
	if (i >= argc || strlen(argv[i]) < 1)
	{
		fprintf(stderr, "Missing filename for option '%s'!\n"
			"Try running the program with '--help' option to see all parameters.\n",
			argv[i - 1]);
		return 0;
	}
	
	*file = argv[i];
	
	return 1;
}

uint8_t get_int(int argc, char *argv[], int i, uint32_t *val)
{
	if (i >= argc || strlen(argv[i]) < 1)
	{
		fprintf(stderr, "Missing parameter for option '%s'!\n"
			"Try running the program with '--help' option to see all parameters.\n",
			argv[i - 1]);
		return 0;
	}
	
	int v = atoi(argv[i]);
	
	if (v < 1)
	{
		fprintf(stderr, "Illegal parameter for option '%s'!\n"
			"Try running the program with '--help' option to see all parameters.\n",
			argv[i - 1]);
		return 0;
	}
		
	*val = (uint32_t) v;
	
	return 1;
}

uint8_t correct_pixel(int x, int y, int width, int height)
{
	if (x >= width || y >= height) return 0;
	
	if (DATA(x,y) == COL_BLACK) return 0;

	if (DATA(x,y) == COL_BLACK_CORR) return 1;
	
	if ((x+1 >= width || DATA(x+1,y) != COL_BLACK)
		&& (x-1 < 0 || DATA(x-1,y) != COL_BLACK)
		&& (y+1 >= height || DATA(x,y+1) != COL_BLACK)
		&& (y-1 < 0 || DATA(x,y-1) != COL_BLACK))
	{
		
		DATA(x,y) = COL_BLACK_CORR;
		return 1;
	}
	
	return 0;
	
}

int main( int argc, char *argv[] )
{
	for (int i = 1; i < argc; ++i)
	{
		if (!strcmp("-?", argv[i]) || !strcmp("--help", argv[i]))
		{
			show_help();
			return EXIT_SUCCESS;
		}
		else if (!strcmp("-V", argv[i]))
		{
			show_version();
			return EXIT_SUCCESS;
		}
		else if (!strcmp("-e", argv[i]))
		{
			action_enable = what_action = ACTION_ENABLE;
		}
		else if (!strcmp("-d", argv[i]))
		{
			action_enable = what_action = ACTION_DISABLE;
		}
		else if (!strcmp("-a", argv[i]))
		{
			output_type = OUTPUT_TYPE_ASCII;
		}
		else if (!strcmp("-0", argv[i]))
		{
			output_type = OUTPUT_TYPE_BIN;
		}
		else if (!strcmp("-inv", argv[i]))
		{
			do_invert = 1;
		}
		else if (!strcmp("-bb_l", argv[i]))
		{
			do_bb_l = 1;
		}
		else if (!strcmp("-bb_r", argv[i]))
		{
			do_bb_r = 1;
		}
		else if (!strcmp("-bb_t", argv[i]))
		{
			do_bb_t = 1;
		}
		else if (!strcmp("-bb_b", argv[i]))
		{
			do_bb_b = 1;
		}
		else if (!strcmp("-bb_h", argv[i]))
		{
			do_bb_r = 1;
			do_bb_l = 1;
		}
		else if (!strcmp("-bb_v", argv[i]))
		{
			do_bb_t = 1;
			do_bb_b = 1;
		}
		else if (!strcmp("-bb", argv[i]))
		{
			do_bb_t = 1;
			do_bb_b = 1;
			do_bb_l = 1;
			do_bb_r = 1;
		}
		else if (!strcmp("-s", argv[i]))
		{
			what_action = ACTION_STATIC;
			i++;
			if (!get_filename(argc, argv, i, &input_file))
				return EXIT_FAILURE;
		}
		else if (!strcmp("-r", argv[i]))
		{
			what_action = ACTION_ROLL;
			i++;
			if (!get_filename(argc, argv, i, &input_file))
				return EXIT_FAILURE;
		}
		else if (!strcmp("-f", argv[i]))
		{
			what_action = ACTION_FLASH;
			i++;
			if (!get_filename(argc, argv, i, &input_file))
				return EXIT_FAILURE;
		}
		else if (!strcmp("-fr", argv[i]) || !strcasecmp("-rf", argv[i]))
		{
			what_action = ACTION_FLASH_ROLL;
			i++;
			if (!get_filename(argc, argv, i, &input_file))
				return EXIT_FAILURE;
		}
		else if (!strcmp("-o", argv[i]))
		{
			i++;
			if (!get_filename(argc, argv, i, &output_file))
				return EXIT_FAILURE;
		}
		else if (!strcmp("-w", argv[i]))
		{
			i++;
			if (!get_int(argc, argv, i, &width))
				return EXIT_FAILURE;
		}
		else if (!strcmp("-h", argv[i]))
		{
			i++;
			if (!get_int(argc, argv, i, &height))
				return EXIT_FAILURE;
		}
		else
		{
			fprintf(stderr, "Incorrect parameter: '%s'\n", argv[i]);
			show_help();
			return EXIT_FAILURE;
		}
	}

	if (what_action == ACTION_HELP)
	{
		fprintf(stderr, "No action specified.\n");
		show_help();
		return EXIT_FAILURE;
	}

	if (what_action != ACTION_ENABLE && what_action != ACTION_DISABLE)
	{
		if (height > 0)
		{
			if (what_action == ACTION_FLASH && height > 16)
			{
				fprintf(stderr, "Incorrect height for selected picture mode (>16)\n");
				return EXIT_FAILURE;
			}
			else if (what_action == ACTION_FLASH_ROLL && height < 17)
			{
				fprintf(stderr, "Incorrect height for selected picture mode (<17)\n");
				return EXIT_FAILURE;
			}
			else if (height > 32)
			{
				fprintf(stderr, "Incorrect height (> 32)\n");
				return EXIT_FAILURE;
			}
		}
		
		if (width > 0)
		{
			if (width > 128 && what_action != ACTION_ROLL)
			{
				fprintf(stderr, "Incorrect width for selected picture mode (>128)\n");
				return EXIT_FAILURE;
			}
			else if (width > 1792)
			{
				fprintf(stderr, "Incorrect width (>1792)\n");
				return EXIT_FAILURE;
			}
		}
		
		if (!input_file)
		{
			fprintf(stderr, "No input file specified!\n");
			return EXIT_FAILURE;
		}
	
		QImage img = QImage(input_file);
	
		if (img.isNull())
		{
			fprintf(stderr, "Opening file '%s' failed, or unsupported file format.\n", input_file);
			return EXIT_FAILURE;
		}
		
		int w, h;
		
		w = img.width();
		h = img.height();
		
		/* not set */
		if (height < 1)
		{
			/* User didn't specify the height - lets find something reasonable */
			
			/* But there is width - lets try to calculate height from that */
			if (width > 0)
			{
				height = (h * width) / w;
				
				if (height > 32) height = 32;
				
				if (height > 16 && what_action == ACTION_FLASH) height = 16;
			}
			else if (what_action == ACTION_FLASH) height = 16;
			else height = 32;
			
			if (h < height) height = h;
			
			if (height < 17 && what_action == ACTION_FLASH_ROLL) height = 32;
		}
		
		/* not set */
		if (width < 1)
		{
			/* User didn't specify width. We already have desired height,
				so we can calculate width for that height. */
			width = (w * height) / h;

			/* Now check if it is correct for chosen picture mode, and if not,
				correct it. */			
			if (width > 1792) width = 1792;
			
			if (what_action != ACTION_ROLL && width > 128) width = 128;
		}
		
		/* Lets scale the image */
		QImage scaled(img.smoothScale(width, height, QImage::ScaleFree));

		if (scaled.isNull())
		{
			fprintf(stderr, "Scaling the image failed!\n");
			return EXIT_FAILURE;
		}

		/* And convert it to B&W */
		img = scaled.convertDepth(1, Qt::MonoOnly | Qt::DiffuseDither);
		
		if (img.isNull())
		{
			fprintf(stderr, "Converting image to Black&White failed!\n");
			return EXIT_FAILURE;
		}
		
		if (img.width() != width || img.height() != height)
		{
			fprintf(stderr, "Converted image has different than expected dimensions: %d, %d vs %d, %d!\n",
					img.width(), img.height(), width, height);
			return EXIT_FAILURE;
		}
		
		memset(data, do_invert?COL_WHITE:COL_BLACK, DATA_SIZE);
		
		for (int x = 0; x < width; ++x)
			for (int y = 0; y < height; ++y)
			{
				if (!img.valid(x, y))
				{
					fprintf(stderr, "Pixel %d, %d is invalid!\n", x, y);
					return EXIT_FAILURE;
				}
				
				if (qGray(img.pixel(x, y)))
				{
					DATA(x,y) = do_invert?COL_BLACK:COL_WHITE;
				}
			}
		
		if (do_bb_t || do_bb_b)
		{
			int x, y;
			
			for (x = 0; x < width; ++x)
			{
				if (do_bb_t)
					for (y = 0; y < height; ++y)
						if (!correct_pixel(x, y, width, height)) break;
			
				if (do_bb_b)
					for (y = height-1; y >= 0; --y)
						if (!correct_pixel(x, y, width, height)) break;
			}
		}
		
		if (do_bb_l || do_bb_r)
		{
			int x, y;
			
			for (y = 0; y < height; ++y)
			{
				if (do_bb_l)
					for (x = 0; x < width; ++x)
						if (!correct_pixel(x, y, width, height)) break;
				
				if (do_bb_r)
					for (x = width-1; x >= 0; --x)
						if (!correct_pixel(x, y, width, height)) break;
			}
		}
		
		/* Our data is prepared */
	}
	
	/* If requested, disable the display */
	if (action_enable == ACTION_DISABLE)
	{
		// This will not show any errors:
		//const char *cmd = "f=$( find /sys -name asus_oled_enabled ); if [ -e \"$f\" ]; then (echo 0 > $f ); fi";
		
		// This will:
		const char *cmd = "echo 0 > $( find /sys -name asus_oled_enabled )";
		
		int ret = system(cmd);
		
		if (ret < 0 || WEXITSTATUS(ret) != 0)
		{
			fprintf(stderr, "\nsystem(%s) failed\n", cmd);
			return EXIT_FAILURE;
		}
	}
	
	if (what_action != ACTION_ENABLE && what_action != ACTION_DISABLE)
	{
		const char *fn = output_file;
		QString qStrName;
		
		if (!fn)
		{
			qStrName = QString("/tmp/asus_oled_%1").arg(QString::number(getpid()));
			
			fn = qStrName.ascii();
		}
		
		FILE *fout = fopen(fn, "w");
		
		if (!fout)
		{
			fprintf(stderr, "Error opening file '%s' for writing", fn);
			perror("fopen");
			
			return EXIT_FAILURE;
		}
		
		char mode;
		
		switch(what_action)
		{
			case ACTION_STATIC: mode = 's';
			break;
			case ACTION_ROLL: mode = 'r';
			break;
			
			/* Difference between flashing and flashing&rolling is only in image size */
			case ACTION_FLASH:
			case ACTION_FLASH_ROLL: mode = 'f';
			break;
			
			default: mode = 'x';
			break;
		}
		
		fprintf(fout, "<%c:%dx%d>\n", mode, width, height);
		
		for(int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				if (data[y*MAX_WIDTH + x] == COL_WHITE)
					fprintf(fout, (output_type == OUTPUT_TYPE_BIN)?"1":"#");
				else
					fprintf(fout, (output_type == OUTPUT_TYPE_BIN)?"0":" ");
			}
			
			fprintf(fout, "\n");
		}
		
		fclose(fout);
		
		if (!output_file)
		{
			QString str = QString("cat %1 > $( find /sys -name asus_oled_picture )").arg(qStrName);
			
			const char *cmd = str.ascii();
		
			int ret = system(cmd);
		
			if (ret < 0 || WEXITSTATUS(ret) != 0)
			{
				fprintf(stderr, "\nsystem(%s) failed\n", cmd);
				return EXIT_FAILURE;
			}
			
			unlink(fn);
		}
	}
	
	/* If requested, enable the display */
	if (action_enable == ACTION_ENABLE)
	{
		// This won't show any errors:
		//const char *cmd = "f=$( find /sys -name asus_oled_enabled ); if [ -e \"$f\" ]; then (echo 1 > $f ); fi";
		
		// This will:
		const char *cmd = "echo 1 > $( find /sys -name asus_oled_enabled )";
		
		int ret = system(cmd);
		
		if (ret < 0 || WEXITSTATUS(ret) != 0)
		{
			fprintf(stderr, "\nsystem(%s) failed\n", cmd);
			return EXIT_FAILURE;
		}
	}
	
	return EXIT_SUCCESS;
}
