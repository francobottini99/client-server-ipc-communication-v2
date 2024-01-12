#include "server_utils.h"

char* get_system_info(void) 
{
    double loadavg[3];

    long num_cpus = sysconf(_SC_NPROCESSORS_ONLN);

    if (num_cpus < 1) 
    {
        char *err_str = "Error: could not determine number of CPUs.\n";
        char *load_str = calloc(strlen(err_str) + 1, sizeof(char));

        strcpy(load_str, err_str);

        return load_str;
    }

    if (getloadavg(loadavg, 3) < 0) 
    {
        char *err_str = "Error: could not get load average.\n";
        char *load_str = calloc(strlen(err_str) + 1, sizeof(char));

        strcpy(load_str, err_str);

        return load_str;
    }

    struct sysinfo si;

    if (sysinfo(&si) != 0) 
    {
        char *err_str = "Error: could not get system info.\n";
        char *load_str = calloc(strlen(err_str) + 1, sizeof(char));

        strcpy(load_str, err_str);

        return load_str;
    }

    double free_mem = (double)(si.freeram * (unsigned long) si.mem_unit) / 1e9;

    double loadavg_norm = loadavg[0] / (double) num_cpus;

    char *format_str = "Load average: %.2f\nNumber of CPUs: %d\nNormalized load average: %.2f\nFree memory: %.2f GB";
    char *load_str = calloc(strlen(format_str) + 1, sizeof(char));

    sprintf(load_str, format_str, loadavg[0], num_cpus, loadavg_norm, free_mem);
    
    return load_str;
}

int compress_and_save_data(const char *data, const char* filename)
{
    FILE *file;
    gzFile gzfile;
    size_t length = strlen(data);
    size_t status;

    file = fopen(filename, "wb");

    if (!file) 
        return -1;
    
    gzfile = gzdopen(fileno(file), "wb");

    if (!gzfile) 
    {
        fclose(file);
        return -1;
    }

    status = (size_t) gzwrite(gzfile, data, (unsigned int) length);

    if (status != length) {
        gzclose(gzfile);
        fclose(file);
        return -1;
    }

    gzclose(gzfile);
    fclose(file);

    return 0;
}

char* journalctl_execute(const char* command, int client_fd)
{
    FILE *fp;
    char* result;

    char prompt[1024];
    char file_output[128];
    char file_err[128];
       
    sprintf(file_output, "%s_%d.log", JOURNAL_TMP_OUTPUT, client_fd);
    sprintf(file_err, "%s_%d.log", JOURNAL_TMP_ERROR, client_fd);

    sprintf(prompt, "journalctl %s > %s 2> %s", command, file_output, file_err);

    fp = popen(prompt, "r");

    if (fp == NULL) 
    {
        result = calloc(strlen(strerror(errno)) + 27, sizeof(char));
        sprintf(result, "Failed to run command: %s", strerror(errno));
        return result;
    }

    pclose(fp);

    if((result = read_file(file_err)) == NULL)
        if((result = read_file(file_output)) == NULL)
            result = NULL;

    remove(file_err);
    remove(file_output);

    return result;
}

char* read_file(const char* file)
{
    FILE *fp = fopen(file, "r");
    unsigned long size;
    char* result;

    if (fp == NULL)
        return NULL;

    fseek(fp, 0, SEEK_END);

    size = (unsigned long) ftell(fp);

    if(size < 1)
    {
        fclose(fp);
        return NULL;
    }

    rewind(fp);

    result = calloc(size, sizeof(char));

    fread(result, 1, size, fp);

    result[size - 1] = '\0';

    fclose(fp);

    return result;
}