/*
Benchmark plugin for AviUtl ver. 0.1.0

The MIT License

Copyright (c) 2012 Oka Motofumi (chikuzen.mo at gmail dot com)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <stdio.h>
#include <string.h>
#include <sys/timeb.h>
#include <time.h>
#include <windows.h>
#include "output.h"

OUTPUT_PLUGIN_TABLE output_plugin_table = {
    0,
    "benchmark plugin for aviutl",
    "Text file (*.txt)\0*.txt\0All file (*.*)\0*.*\0",
    "Benchmark pulugin version 0.0.1",
    NULL,
    NULL,
    func_output,
    NULL,
    NULL,
    NULL
};

EXTERN_C OUTPUT_PLUGIN_TABLE __declspec(dllexport) * __stdcall GetOutputPluginTable(void)
{
  return &output_plugin_table;
}

static __int64 get_current_time(void)
{
    struct timeb tb;
    ftime(&tb);
    return ((__int64)tb.time * 1000 + (__int64)tb.millitm) * 1000;
}

#define CONFIG_FILE "benchmark.ini"
static int generate_default_config()
{
    FILE *config = fopen(CONFIG_FILE, "w");
    if (!config)
        return -1;
    fprintf(config, "# output_format - 0:RGB 1:YUY2 2:YC48\n"
                    "output_format=1\n"
                    "# repeat number of times\n"
                    "repeat=1\n");
    fclose(config);
    return 0;
}

static int get_config(DWORD *format, int *repeat, char *color)
{
    FILE *config = NULL;
    while (!config) {
        config = fopen(CONFIG_FILE, "r");
        if (!config && generate_default_config())
            return -1;
    }

    DWORD fourcc[] = { 0, MAKEFOURCC('Y', 'U', 'Y', '2'), MAKEFOURCC('Y', 'C', '4', '8') };
    char *color_name[] = { "RGB", "YUY2", "YC48" };
    int tmp_format = 1;

    struct {
        const char *prefix;
        size_t length;
        int *address;
    } config_table[] = {
        { "output_format=", 14, &tmp_format },
        { "repeat=",         7, repeat      },
        { NULL }
    };

    char buf[64];
    while (fgets(buf, sizeof buf, config)) {
        if (buf[0] == '#')
            continue;
        for (int i = 0; config_table[i].prefix; i++) {
            if (strncmp(buf, config_table[i].prefix, config_table[i].length) == 0) {
                sscanf(buf + config_table[i].length, "%d", config_table[i].address);
                break;
            }
        }
    }

    fclose(config);

    if (tmp_format < 0 || tmp_format > 2)
        tmp_format = 1;
    *format = fourcc[tmp_format];
    strcpy(color, color_name[tmp_format]);

    if (*repeat < 1)
        *repeat = 1;

    return 0;
}
#undef CONFIG_FILE

BOOL func_output(OUTPUT_INFO *oip)
{
    DWORD format;
    int repeat = 1;
    char color[5];

    if (get_config(&format, &repeat, color))
        return FALSE;

    int frames = oip->n;

    while (repeat--) {
        __int64 elapsed = get_current_time();

        for (int n = 0; n < frames; n++) {
            if(oip->func_is_abort())
                return FALSE;
            oip->func_get_video_ex(n, format);
            oip->func_rest_time_disp(n, frames);
            oip->func_update_preview();
        }

        elapsed = get_current_time() - elapsed;

        FILE *output = fopen(oip->savefile, "a");
        if(!output)
            return FALSE;

        time_t time_now;
        time(&time_now);

        fprintf(output,
                "date                      : %s"
                "output colorspace         : %s\n"
                "resolution(width x height): %d x %d\n"
                "frames                    : %d\n"
                "total proc time(msec)     : %I64u\n"
                "average proc rate(fps)    : %.3f\n\n\n",
                ctime(&time_now), color, oip->w, oip->h, frames,
                elapsed / 1000, frames * 1000000.0 / elapsed);

        fclose(output);
    }

    return TRUE;
}
