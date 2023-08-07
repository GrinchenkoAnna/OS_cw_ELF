#include <stdio.h>
#include <stdlib.h>
#include <elf.h>

void read_header(const char* filename)
{
    FILE* file_pointer;
    Elf64_Ehdr header;

    file_pointer = fopen("example", "r");
    if (!file_pointer)
    {
        perror ("Невозможно открыть ELF-файл");
        exit(1);
    }
    
    fread(&header, sizeof(Elf64_Ehdr), 1, file_pointer);
    if (header.e_ident[EI_MAG0] != 0x7f
        || header.e_ident[EI_MAG1] != 'E' 
        && header.e_ident[EI_MAG2] != 'L'
        && header.e_ident[EI_MAG3] != 'F')
    {
        perror("Файл не является ELF-файлом");
        exit(1);
    }

    printf("Заголовок ELF:\n");
    printf("  Magic:\t");
    for (int i = 0; i < EI_NIDENT; i++) 
    {
        printf("%02x ", header.e_ident[i]);
    }
    printf("\n");

    printf("  Класс:\t");
    switch (header.e_ident[EI_CLASS])
    {
        case ELFCLASS32:
            printf("ELF32\n");
            break;

        case ELFCLASS64:
            printf("ELF64\n");
            break;

        case ELFCLASSNONE:
            printf("Invalid class\n");
            break;

        default:
            perror("Не удалось определить разрядность ELF-файла");
            break;
    }

    printf("  Данные:\tдополнение до 2, ");
    switch (header.e_ident[EI_DATA])
    {
        case ELFDATA2LSB:
            printf
            ("от младшего к старшему (little endian)\n");
            break;

        case ELFDATA2MSB:
            printf("от старшего к младшему (big endian)\n");
            break;

        case ELFDATANONE:
            printf("от старшего к младшему (big endian)\n");
            break;
        
        default:
            perror("Не удалось определить порядок следования байтов данных");
            break;
    }

    printf("  Version:\t");
    switch (header.e_ident[EI_VERSION])
    {
        case EV_CURRENT:
            printf("1 (current)\n");
            break;
        
        default:
            perror("Не удалось определить версию спецификации ELF-формата");
            break;
    }

    printf("  OS/ABI:\t");
    switch (header.e_ident[EI_OSABI])
    {
        case 0x01:
            printf("1 (current)\n");
            break;
        
        default:
            perror("Не удалось определить версию спецификации ELF-формата");
            break;
    }

    fclose(file_pointer);
}

int main()
{    
    read_header("example");

    return 0;
}