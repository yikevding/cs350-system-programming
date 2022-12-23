// THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING

// A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Kevin Ding

#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <string.h>
#include <utime.h>
#include <errno.h>
#include <string.h>
#include <ar.h>
#include <unistd.h>
#include <sys/types.h>
#include <libgen.h>
#include <math.h>

// define meta structure given by the problem
struct meta
{
    char name[16];
    int mode;
    int size;
    time_t mtime; // of data type long
};

// check archive format
int checkArchive(char *name)
{
    int fd = open(name, O_RDWR);
    // file does not exist
    if (fd == -1)
    {
        return -2;
    }
    int amount = SARMAG;
    char str[amount];
    char test[amount];
    read(fd, str, amount);
    char *magic = "!<arch>";
    for (int i = 0; i < amount - 1; i++)
        test[i] = str[i];
    // this means it is an archive format file
    if (strcmp(magic, test) == 0)
    {
        return 0;
    }
    // file is not in archive format
    else
    {
        return -1;
    }
};

// create an archive file
int createArchive(char *name)
{
    int archivefd = creat(name, 0666);
    int amount = SARMAG;
    write(archivefd, ARMAG, amount);
    printf("ar: creating %s\n", name);
    return archivefd;
};

// move info from header to meta
int fill_meta(struct ar_hdr hdr, struct meta *meta)
{
    // fill name
    int nameLength = sizeof(hdr.ar_name);
    for (int i = 0; i < nameLength; i++)
    {
        if (hdr.ar_name[i] == '/') // mark the end of the file
            break;
        meta->name[i] = hdr.ar_name[i];
    }

    // fill mode
    meta->mode = strtol(hdr.ar_mode, NULL, 8);

    // fill size
    meta->size = strtol(hdr.ar_size, NULL, 10);

    // fill modify time
    meta->mtime = strtol(hdr.ar_date, NULL, 10);

    return 0;
};

// move info from stat to header
int fill_ar_hdr(char *filename, struct ar_hdr *hdr)
{
    struct stat holder;
    stat(filename, &holder);

    // fill name
    int nameLength = strlen(filename);
    int capacity = sizeof(hdr->ar_name);
    for (int j = 0; j < nameLength; j++)
        hdr->ar_name[j] = filename[j];
    hdr->ar_name[nameLength] = '/'; // to fit archive header format, this is not part of the file name
    for (int i = nameLength + 1; i < capacity; i++)
        hdr->ar_name[i] = ' ';

    // file member date

    long date = holder.st_mtime;
    char *dateFormat = "%-lu";
    sprintf(hdr->ar_date, dateFormat, date);
    for (int i = 0; i < sizeof(hdr->ar_date); i++)
    {
        if (hdr->ar_date[i] == '\0')
            hdr->ar_date[i] = ' ';
    }

    // user id
    unsigned int uid = holder.st_uid;
    char *uidFormat = "%i";
    sprintf(hdr->ar_uid, uidFormat, uid);
    for (int i = 0; i < sizeof(hdr->ar_uid); i++)
        if (hdr->ar_uid[i] == '\0')
            hdr->ar_uid[i] = ' ';

    // group id
    unsigned int gid = holder.st_gid;
    char *gidFormat = "%i";
    sprintf(hdr->ar_gid, gidFormat, gid);
    for (int i = 0; i < sizeof(hdr->ar_gid); i++)
        if (hdr->ar_gid[i] == '\0')
            hdr->ar_gid[i] = ' ';

    // mode in octal
    unsigned short mode = holder.st_mode;
    char *modeFormat = "%o";
    sprintf(hdr->ar_mode, modeFormat, mode);
    for (int i = 0; i < sizeof(hdr->ar_mode); i++)
        if (hdr->ar_mode[i] == '\0')
            hdr->ar_mode[i] = ' ';

    // file size
    long long size = holder.st_size;
    char *sizeFormat = "%lld";
    sprintf(hdr->ar_size, sizeFormat, size);
    for (int i = 0; i < sizeof(hdr->ar_size); i++)
        if (hdr->ar_size[i] == '\0')
            hdr->ar_size[i] = ' ';

    // fill trailing string
    char *endFormat = "%s";
    sprintf(hdr->ar_fmag, endFormat, ARFMAG);

    return 0;
};

int qOption(char *archive, char *input)
{
    // the file is not in archive format
    if (checkArchive(archive) == -1)
    {
        printf("ar: %s: File format not recognized\n", archive);
        exit(1);
    }

    int archivefd;

    // file does not exist
    if (checkArchive(archive) == -2)
        archivefd = createArchive(archive);

    // if an archive exists
    if (checkArchive(archive) == 0)
    {
        archivefd = open(archive, O_RDWR | O_APPEND, 0666);
    }

    // now append file in archive
    int filefd = open(input, O_RDWR);
    // input file is missing
    if (filefd == -1)
    {
        printf("ar: %s: No such file or directory\n", input);
        exit(1);
    }
    struct ar_hdr *header = malloc(sizeof(struct ar_hdr));

    // write header
    fill_ar_hdr(input, header);

    int amount = sizeof(struct ar_hdr);
    int amount_write = write(archivefd, header, amount);
    if (amount_write != amount)
    {
        printf("Writing archive header failed.\n");
        exit(1);
    }

    // // now write content
    struct meta meta;
    fill_meta(*header, &meta);

    int contentSize = meta.size;
    char contentBuff[contentSize];
    int readAmount = read(filefd, contentBuff, contentSize);
    if (readAmount != contentSize)
    {
        printf("Reading file failed.\n");
        exit(1);
    }
    int writeAmount = write(archivefd, contentBuff, contentSize);
    if (writeAmount != contentSize)
    {
        printf("Writing file content failed.\n");
        exit(1);
    }

    // check even padding to make sure alignment is good
    // this step is specific to archive format file
    int endPosition = lseek(archivefd, 0, SEEK_END);
    if (endPosition % 2 == 1)
        write(archivefd, "\n", 1);

    free(header);
    return 0;
};

int tOption(char *archive)
{
    int archivefd = open(archive, O_RDWR);
    struct stat holder;
    stat(archive, &holder);
    int total = holder.st_size;
    int count = 8;
    lseek(archivefd, 8, SEEK_SET);
    while (count < total)
    {
        char buffer[16];
        read(archivefd, buffer, 16);
        int length = 0;
        for (int i = 0; i < 16; i++)
        {
            if (buffer[i] == '/')
                break;
            length++;
        }
        char text[length];
        strncpy(text, buffer, length);
        text[length] = 0;
        printf("%s\n", text);
        lseek(archivefd, 32, SEEK_CUR);
        char temp[10];
        read(archivefd, temp, 10);
        lseek(archivefd, 2, SEEK_CUR);

        count += 60;
        int size = strtol(temp, NULL, 10);
        if (size % 2 == 1)
        {
            count += 1;
            lseek(archivefd, 1, SEEK_CUR);
        }
        count += size;
        lseek(archivefd, size, SEEK_CUR);
    }

    return 0;
}

int xOption(char *archive, char *fileName, int oflag)
{
    int archivefd = open(archive, O_RDWR);
    struct stat holder;
    stat(archive, &holder);
    int total = holder.st_size;
    int count = 8;
    lseek(archivefd, 8, SEEK_SET);
    while (count < total)
    {
        char buffer[16];
        read(archivefd, buffer, 16);
        int length = 0;
        for (int i = 0; i < 16; i++)
        {
            if (buffer[i] == '/')
                break;
            length++;
        }
        char text[length];
        strncpy(text, buffer, length);
        text[length] = 0;
        int target = strlen(fileName);
        if (target == length)
        {
            char goal[target];
            for (int i = 0; i < target; i++)
                goal[i] = fileName[i];
            int match = 0;
            for (int i = 0; i < target; i++)
            {
                if (goal[i] != text[i])
                    match = 1;
            }
            if (match == 0) // if we find extract files
            {

                // get last modification time
                char dates[12];
                read(archivefd, dates, 12);
                int date = strtol(dates, NULL, 10);

                // skip user and group id
                lseek(archivefd, 12, SEEK_CUR);

                // get mode
                char modes[8];
                read(archivefd, modes, 8);
                int mode = strtol(modes, NULL, 8);

                // get size
                char temp[10];
                read(archivefd, temp, 10);
                int size = strtol(temp, NULL, 10);
                lseek(archivefd, 2, SEEK_CUR);
                char content[size];
                read(archivefd, content, size);

                // create file and write content
                int fd = open(text, O_RDWR | O_CREAT, mode);
                write(fd, content, size);
                // restore last modification time, only for O option
                if (oflag == 1)
                {
                    struct stat record;
                    struct utimbuf updated;
                    stat(text, &record);
                    updated.actime = record.st_atime; // keep the accessing time
                    updated.modtime = date;
                    utime(text, &updated);
                }
                return 0;
            }
        }
        // if we do not find file in this header, we keep going
        lseek(archivefd, 32, SEEK_CUR);
        char sizes[10];
        read(archivefd, sizes, 10);
        int size = strtol(sizes, NULL, 10);
        if (size % 2 == 1)
        {
            count += 1;
            lseek(archivefd, 1, SEEK_CUR);
        }
        lseek(archivefd, size, SEEK_CUR);
        lseek(archivefd, 2, SEEK_CUR);
        count += 60;
        count += size;
    }
    printf("ar: %s: not found in archive.\n", fileName);
    exit(1);
};

int AOption(char *archive, int days)
{
    struct dirent *file;
    DIR *directory;
    directory = opendir("."); // we open current directory
    if (directory == NULL)
    {
        printf("Open current directory failed.\n");
        exit(1);
    }
    file = readdir(directory);
    while (file != NULL)
    {
        // only append regular files
        if (file->d_type == DT_REG)
        {
            int targetDiff = days * 24 * 60 * 60;
            struct stat lastRecords;
            char *name;
            name = file->d_name;
            stat(name, &lastRecords);
            int diff = time(NULL) - lastRecords.st_mtime;
            if (diff > targetDiff)
                qOption(archive, name);
        }
        file = readdir(directory);
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int option;
    int qflag = 0;
    int xflag = 0;
    int oflag = 0;
    int tflag = 0;
    int Aflag = 0;
    int archivefd;
    char *archive;
    char *inputFile;
    int Ndays;

    while ((option = getopt(argc, argv, "qxotvdA:")) != -1)
    {
        switch (option)
        {
        case 'q':
        {
            qflag++;
            break;
        }
        case 'x':
        {
            xflag++;
            break;
        }
        case 'o':
        {
            oflag++;
            break;
        }
        case 't':
        {
            tflag++;
            break;
        }
        case 'A':
        {
            Aflag++;
            Ndays = strtol(optarg, &optarg, 10);
            break;
        }
        }
    }

    // archive is always this position regardless of which option
    archive = argv[optind];
    if (qflag == 1)
    {
        int fileIndex = optind + 1;
        inputFile = argv[fileIndex];
        if (archive == NULL)
        {
            printf("Usage error, refer to documentation\n");
            exit(1);
        }
        if (archive != NULL && inputFile == NULL)
        {
            createArchive(archive);
            exit(0);
        }
        if (archive != NULL && inputFile != NULL)
        {
            qOption(archive, inputFile);
            exit(0);
        }
    }

    if (tflag == 1)
    {
        if (archive == NULL)
        {
            printf("Usage error, refer to documentation\n");
            exit(1);
        }
        if (checkArchive(archive) == -2)
        {
            printf("ar: %s: No such file or directory\n", archive);
            exit(1);
        }
        if (checkArchive(archive) == -1)
        {
            printf("ar: %s: File format not recognized\n", archive);
            exit(1);
        }
        tOption(archive);
        exit(0);
    }

    if (xflag == 1)
    {
        if (archive == NULL)
        {
            printf("Usage error, refer to documentation\n");
            exit(1);
        }
        if (checkArchive(archive) == -2)
        {
            printf("ar: %s: No such file or directory\n", archive);
            exit(1);
        }
        if (checkArchive(archive) == -1)
        {
            printf("ar: %s: File format not recognized\n", archive);
            exit(1);
        }
        if (oflag == 0)
        {
            for (int i = 3; i < argc; i++)
            {
                char *fileName = argv[i];
                xOption(archive, fileName, oflag);
            }
            exit(0);
        }
    }
    if (oflag == 1 && xflag == 0)
    {
        printf("Usage error, refer to documentation.\n");
        exit(1);
    }
    if (xflag == 1 && oflag == 1)
    {
        if (archive == NULL)
        {
            printf("Usage error, refer to documentation\n");
            exit(1);
        }
        if (checkArchive(archive) == -2)
        {
            printf("ar: %s: No such file or directory\n", archive);
            exit(1);
        }
        if (checkArchive(archive) == -1)
        {
            printf("ar: %s: File format not recognized\n", archive);
            exit(1);
        }
        for (int i = 3; i < argc; i++)
        {
            char *fileName = argv[i];
            xOption(archive, fileName, oflag);
        }
        exit(0);
    }
    if (Aflag == 1)
    {
        if (archive == NULL)
        {
            printf("Usage error, refer to documentation\n");
            exit(1);
        }
        // if no archive existed yet
        if (checkArchive(archive) == -2)
        {
            createArchive(archive);
        }
        AOption(archive, Ndays);
        exit(0);
    }
}
