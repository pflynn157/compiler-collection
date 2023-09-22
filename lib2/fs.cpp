#include <iostream>
#include <map>
#include <fstream>
#include <cstring>

std::map<int, std::ifstream *> file_map;
std::map<int, FILE *> writer_map;
int idx = 1;

extern "C" {

int fs_open(const char *path, int mode) {
    if (mode == 1) {
        std::ifstream *reader = new std::ifstream(path);
        if (reader->is_open()) {
            ++idx;
            file_map[idx] = reader;
            return idx;
        }
    } else if (mode == 2) {
        FILE *file = fopen(path, "w");
        if (file != NULL) {
            ++idx;
            writer_map[idx] = file;
            return idx;
        }
    }
    return 0;
}

bool fs_eof(int fd) {
    std::ifstream *reader = file_map[fd];
    if (reader == nullptr) return true;
    return reader->eof();
}

int fs_get(int fd) {
    if (fs_eof(fd)) return 0;
    std::ifstream *reader = file_map[fd];
    return reader->get();
}

void fs_writeln(int fd, const char *input) {
    FILE *writer = writer_map[fd];
    if (writer == NULL) return;
    fputs(input, writer);
    fputc('\n', writer);
}

void fs_write(int fd, const unsigned char *input, int size, int count) {
    FILE *writer = writer_map[fd];
    if (writer == NULL) return;
    fwrite(input, size, count, writer);
}

void fs_close(int fd) {
    std::ifstream *reader = file_map[fd];
    if (reader == nullptr) {
        FILE *writer = writer_map[fd];
        if (writer == NULL) return;
        fclose(writer);
        return;
    }
    reader->close();
    file_map[fd] = nullptr;
}

}