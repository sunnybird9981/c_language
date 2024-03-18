#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pulse/error.h>  /* pulseaudio */
#include <pulse/simple.h> /* pulseaudio */

#define APP_NAME "pulseaudio_sample"
#define STREAM_NAME "play"
#define DATA_SIZE 1024

int main() {
    int pa_errno, pa_result, read_bytes;

    pa_sample_spec ss;
    ss.format = PA_SAMPLE_S16LE;
    ss.rate = 48000;
    ss.channels = 1;

    pa_simple *pa = pa_simple_new(NULL, APP_NAME, PA_STREAM_PLAYBACK, NULL, STREAM_NAME, &ss, NULL, NULL, &pa_errno);
    if (pa == NULL) {
        fprintf(stderr, "ERROR: Failed to connect pulseaudio server: %s\n", pa_strerror(pa_errno));
        return 1;
    }

    char data[DATA_SIZE];
    while (1) {
        read_bytes = read(STDIN_FILENO, data, DATA_SIZE);
        if (read_bytes == 0) {
            break;
        } else if (read_bytes < 0) {
            fprintf(stderr, "ERROR: Failed to read data from stdin: %s\n", strerror(errno));
            return 1;
        }
        pa_result = pa_simple_write(pa, data, read_bytes, &pa_errno);
        if (pa_result < 0) {
            fprintf(stderr, "ERROR: Failed to write data to pulseaudio: %s\n", pa_strerror(pa_errno));
            return 1;
        }
    }

    pa_simple_free(pa);
    return 0;
}
