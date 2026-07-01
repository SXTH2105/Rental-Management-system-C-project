#ifndef UTILS_H
#define UTILS_H

void  utils_trim(char *str);
int   utils_str_to_int(const char *str);
float utils_str_to_float(const char *str);
void  utils_today(char out_date[20]);

int   utils_next_id(const int ids[], int count);
int   utils_file_exists(const char *path);
int   utils_ensure_dir(const char *path);

#endif
