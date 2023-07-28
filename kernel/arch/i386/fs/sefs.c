#include <kernel/kernel.h>
 
sefs_header_t *sefs_header;			///< ...
sefs_file_header_t *file_headers;	///< ...
fs_node_t *sefs_root;				///< ...
fs_node_t *sefs_dev;				///< ...
fs_node_t *root_nodes;				///< ...
int nroot_nodes;                    ///< Количество файлов.
struct dirent dirent;				///< ...
size_t dirName[2048];              ///< Ссылка на названия папок по индексу
size_t diskUsed = 0;               ///< Количество используемого пространства
size_t diskSize = 0;               ///< Общие кол-во дисков
size_t dirCount = 0;               ///< Количество папок
void* sefs_root_impl; // Change the type of sefs_root_impl to void*


/**
 * @brief [SEFS] Полное чтение файла
 *
 * @param int node - Индекс файла
 * 
 * @warning IT's MEMORY LEAKY!!!
 * @return char* - Содержимое файла
 */
char* sefs_readChar(uint32_t node) {
    sefs_file_header_t header = file_headers[node];
    char* buf = (char*)kmalloc(header.length);
    memcpy(buf, (void*)header.offset, header.length);
    return buf;
}

/**
 * @brief [SEFS] Чтение файла
 *
 * @param int node - Индекс файла
 * @param int offset - С какой позиции читать файл
 * @param int size - Длина читаемого файла
 * @param void* buf - Буфер
 * 
 * @return uint32_t - Размер файла или отрицательное значение при ошибке
 */
uint32_t sefs_read(uint32_t node, size_t offset, size_t size, void *buffer){
    sefs_file_header_t header = file_headers[node];
    //qemu_log("[SEFS] [Read] Elem: %d | Off: %d | Size: %d", node, offset, size);

    // Did you mean: offset+size > header.length
    if (header.length < size) {
        size = header.length;
    }

    qemu_log("SEFS -> Read from: %x to (%x) (size %d)", header.offset + offset, header.offset + offset + size, size);

    memcpy(buffer, (char*)(header.offset+offset), size);
    
    // qemu_log("SEFS early buffer now: %s", (char*)(header.offset + offset));
    // qemu_log("SEFS buffer now: %s", buffer);
    return size;
}

/**
 * @brief [SEFS] запись в файл
 *
 * @param int node - Индекс файла
 * @param int offset - С какой позиции писать файл
 * @param int size - Сколько пишем
 * @param void* buf - Буфер
 *
 * @return uint32_t - Размер записаных байтов или отрицательное значение при ошибке
 */

uint32_t sefs_write(uint32_t node, size_t offset, size_t size, void* buffer) {
    sefs_file_header_t header = file_headers[node];
    if (offset > size) {
        return -2;
    }
    void* newfile = (void*)kmalloc((size + offset > header.length) ? (size + offset) : header.length);
    int w_tmp1 = 0;
    int w_tmp3 = 0;
    if (offset > 0) {
        void* tmp1 = (void*)kmalloc(offset);
        w_tmp1 = sefs_read(node, 0, offset, tmp1);
        memcpy(newfile, tmp1, offset);
        kfree(tmp1);
    }
    strcat(newfile, buffer);
    if ((size + offset < header.length)) {
        w_tmp3 = header.length - (size + offset);
        void* tmp3 = (void*)kmalloc(offset);
        w_tmp3 = sefs_read(node, 0, w_tmp3, tmp3);
        strcat(newfile, tmp3);
        kfree(tmp3);
    }
    return w_tmp1 + size + w_tmp3;
}

/**
 * @brief [SEFS] Получить размер файла (поиск по индексу)
 *
 * @param int node - Индекс файла
 * 
 * @return size_t - Размер файла или 0
 */
size_t sefs_getLengthFile(int node){
    //qemu_log("[SEFS] [gLF] Node: %d | Size: %d",node,root_nodes[node].length);

    return root_nodes[node].length;
}

/**
 * @brief [SEFS] Получить отступ в файловой системе у файла
 *
 * @param int node - Индекс файла
 * 
 * @return int - Позиция файла или отрицательное значение при ошибке
 */
size_t sefs_getOffsetFile(int node){
    return file_headers[node].offset;
}

/**
 * @brief [SEFS] Поиск файла на устройстве
 *
 * @param char* filename - Путь к файлу (виртуальный)
 * 
 * @return int - Индекс файла, или отрицательное значение при ошибке
 */
int32_t sefs_findFile(char* filename) {
    char* file = (char*)kmalloc(sizeof(char) * 256);
    char* sl = "/";
    strcpy(file, sl);
    strcat(file, filename);
    for (size_t i = 0; i < sefs_header->nfiles; i++) {
        if (strcmpn(root_nodes[i].path, file) == 0) {
            kfree(file);
            return i;
        }
    }
    kfree(file);
    return -1;
}

/**
 * @brief [SEFS] Поиск папки на устройстве
 *
 * @param char* filename - Путь к папке (виртуальный)
 *
 * @return int - Индекс папки, или отрицательное значение при ошибке
 */
int32_t sefs_findDir(char* path) {
    char* file = (char*)kmalloc(sizeof(char) * 256);
    char* sl = "/";
    strcpy(file, sl);
    strcat(file, path);
    for (size_t i = 0, a = 0; i < sefs_header->nfiles; i++) {
        if (root_nodes[i].flags != FS_DIRECTORY) continue;
        if (strcmpn(root_nodes[i].name, file) == 0) {
            kfree(file);
            return a;
        }
        a++;
    }
    kfree(file);
    return -1;
}

/**
 * @brief [SEFS] Считает количество элементов в папке
 */
size_t sefs_countElemFolder(char* path) {
    int32_t inxDir = sefs_findDir(path);
    if (inxDir >= 0) {
        size_t count = 0;
        for (size_t i = 0; i < sefs_header->nfiles; i++) {
            if (root_nodes[i].root == (size_t)inxDir) {
                count++;
            }
        }
        return count;
    }
    return 0;
}

/**
 * @brief [SEFS] Выводит список файлов
 */
struct dirent* sefs_list(char* path) {
    int32_t inxDir = sefs_findDir(path);
    if (inxDir >= 0) {
        struct dirent* testFS = (struct dirent*)kcalloc(sefs_header->nfiles, sizeof(struct dirent));
        size_t inxFile = 0;
        for (size_t i = 0; i < sefs_header->nfiles; i++) {
            if (root_nodes[i].root != (size_t)inxDir) {
                continue;
            }
            testFS[inxFile].type = root_nodes[i].flags;
            testFS[inxFile].ino = i;
            testFS[inxFile].next = i + 1;
            testFS[inxFile].length = root_nodes[i].length;
            strcpy(testFS[inxFile].name, root_nodes[i].name);
            inxFile++;
        }
        testFS[inxFile].next = 0;
        return testFS;
    }
    return NULL;
}
/**
 * @brief [SEFS] Количество используемого места устройства
 *
 * @param int node - Нода
 *
 * @return size_t - Количество используемого места устройства
 */
size_t sefs_diskUsed(int node) {
    (void)node; // Avoid unused parameter warning
    return diskUsed;
}

size_t sefs_diskSpace(int node) {
    (void)node; // Avoid unused parameter warning
    return 0;
}

size_t sefs_diskSize(int node) {
    (void)node; // Avoid unused parameter warning
    return diskSize;
}

char* sefs_getDevName(int node) {
    (void)node; // Avoid unused parameter warning
    return sefs_root->devName;
}

void sefs_dirfree(struct dirent* ptr) {
    if (ptr) {
        kfree(ptr);
        ptr = 0;
    }
}

/**
 * @brief [SEFS] Инициализация Sayori Easy File System
 *
 * @param uint32_t location - Точка монтирования
 * 
 * @return fs_node_t - Структура с файлами
 */
fs_node_t* sefs_initrd(uint32_t location) {
    qemu_log("[SEFS] [Init] loc: %x", location);

    qemu_log("SEFS INIT SCOPE =====================================");
    // Инициализирует указатели main и заголовке файлов и заполняет корневой директорий.
	qemu_log("Step:%d",1);
    sefs_header = (sefs_header_t *)&location;
	qemu_log("Step:%d",2);
    file_headers = (sefs_file_header_t *) (location + sizeof(sefs_header_t));
	qemu_log("Step:%d",3);
    sefs_root = (fs_node_t*)kcalloc(1, sizeof(fs_node_t));
	
	qemu_log("Step:%d",4);
    strcpy(sefs_root->name, "/");
    strcpy(sefs_root->devName, "SayoriDisk RDv2");
	
	qemu_log("Step:%d",sefs_header->nfiles);
	qemu_log("Step:%d",5);
    sefs_root->mask = sefs_root->uid = sefs_root->gid = sefs_root->inode = sefs_root->length = 0;
    sefs_root->flags = FS_DIRECTORY;
    sefs_root->open = NULL;
    sefs_root->close = NULL;
    sefs_root->findFile = &sefs_findFile;
    sefs_root->findDir = &sefs_findDir;
    sefs_root->getLengthFile = &sefs_getLengthFile;
    sefs_root->getOffsetFile = &sefs_getOffsetFile;
    sefs_root->list = &sefs_list;
    sefs_root->unlistElem = &sefs_dirfree; // Fixing unused warning for sefs_list
	qemu_log("Step:%d",6);
    root_nodes = (fs_node_t*)kcalloc(sefs_header->nfiles, sizeof(fs_node_t));
	qemu_log("Step:%d|%d",6,1);
    nroot_nodes = sefs_header->nfiles;
	qemu_log("Step:%d|%d",6,2);
    for (size_t i = 0; i < sefs_header->nfiles; i++) {
        qemu_log("Step:%d|%d",7,i);
        root_nodes[i].root = file_headers[i].parentDir;
        file_headers[i].offset += location;
        strcpy(root_nodes[i].name, file_headers[i].name);
        root_nodes[i].mask = root_nodes[i].uid = root_nodes[i].gid = 0;
        root_nodes[i].length = (size_t)file_headers[i].length;
        root_nodes[i].inode = i;
        root_nodes[i].flags = (!file_headers[i].types ? FS_FILE : FS_DIRECTORY);
        if (root_nodes[i].flags == FS_FILE) {
            diskUsed += root_nodes[i].length;
            diskSize += root_nodes[i].length;
        }
    }
    for (size_t i = 0; i < sefs_header->nfiles; i++) {
        qemu_log("Step:%d|%d",8,i);
        if (root_nodes[i].flags != FS_FILE) {
            continue;
        }
        strcpy(root_nodes[i].path, root_nodes[(sefs_header->nfiles - dirCount) + root_nodes[i].root].name);
        strcat(root_nodes[i].path, root_nodes[i].name);
        size_t fpath_len = strlen(root_nodes[i].path);
        root_nodes[i].path[fpath_len + 1] = '\0';
    }
    sefs_root_impl = NULL; // Properly assign the NULL pointer to void*
    sefs_root->impl = (uint32_t)sefs_root_impl; // Cast void* to uint32_t before assignment
    qemu_log("Step:%d",9);
    return sefs_root;
}
