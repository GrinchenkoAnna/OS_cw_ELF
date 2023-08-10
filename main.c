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
    printf("  Magic: ");
    for (int i = 0; i < EI_NIDENT; i++) 
    {
        printf("%02x ", header.e_ident[i]);
    }
    printf("\n");

    printf("  Класс:\t\t\t");
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

    printf("  Данные:\t\t\tдополнение до 2, ");
    switch (header.e_ident[EI_DATA])
    {
        case ELFDATA2LSB:
            printf("от младшего к старшему (little endian)\n");
            break;

        case ELFDATA2MSB:
            printf("от старшего к младшему (big endian)\n");
            break;

        case ELFDATANONE:
            printf("некорректный класс\n");
            break;
        
        default:
            perror("Не удалось определить порядок следования байтов данных");
            break;
    }

    printf("  Версия:\t\t\t");
    switch (header.e_ident[EI_VERSION])
    {
        case EV_CURRENT:
            printf("1 (текущая версия)\n");
            break;
        
        case EV_NONE:
            printf("Неправильный номер версии\n");
            break;
        
        default:
            perror("Не удалось определить версию спецификации ELF-формата");
            break;
    }

    printf("  OS/ABI:\t\t\t");
    switch (header.e_ident[EI_OSABI])
    {
        case ELFOSABI_NONE:
            printf("UNIX System V ABI\n");
            break;

        // case ELFOSABI_SYSV:
        //     printf("Alias. \n");
        //     break;

        case ELFOSABI_HPUX:
            printf("HP-UX\n");
            break;
        
        case ELFOSABI_NETBSD:
            printf("NetBSD. \n");
            break;

        case ELFOSABI_GNU:
            printf("Object uses GNU ELF extensions. \n");
            break;

        // case ELFOSABI_LINUX:
        //     printf("Compatibility alias. \n");
        //     break;

        case ELFOSABI_SOLARIS:
            printf("Sun Solaris. \n");
            break;

        case ELFOSABI_AIX:
            printf("IBM AIX. \n");
            break;

        case ELFOSABI_IRIX:
            printf("SGI Irix. \n");
            break;

        case ELFOSABI_FREEBSD:
            printf("FreeBSD. \n");
            break;

        case ELFOSABI_TRU64:
            printf("Compaq TRU64 UNIX. \n");
            break;

        case ELFOSABI_MODESTO:
            printf("Novell Modesto. \n");
            break;

        case ELFOSABI_OPENBSD:
            printf("OpenBSD. \n");
            break;

        case ELFOSABI_ARM_AEABI:
            printf("ARM EABI\n");
            break;

        case ELFOSABI_ARM:
            printf("ARM\n");
            break;

        case ELFOSABI_STANDALONE:
            printf("Автономный (встраиваемый) ABI\n");
            break;

        default:
            perror("Не удалось определить OS/ABI");
            break;
    }

    printf("  Версия ABI:\t\t\t%d\n", header.e_ident[EI_ABIVERSION]);

    printf("  Начало заполнителя:\t\t%d\n", header.e_ident[EI_PAD]);

    printf("  Тип объектного файла:\t\t");
    switch (header.e_type)
    {
        case ET_NONE:
            printf("Неизвестный тип\n");
            break;

        case ET_REL:
            printf("Перемещаемый файл\n");
            break;

        case ET_EXEC:
            printf("Исполняемый файл\n");
            break;

        case ET_DYN:
            printf("Совместно используемый объектный файл\n");
            break;

        case ET_CORE:
            printf("Файл типа core\n");
            break;

        // case ET_NUM:
        //     printf("Number of defined types\n");
        //     break;

        case ET_LOOS:
            printf("Зависимые от операционной системы значения (start)\n");
            break;

        case ET_HIOS:
            printf("Зависимые от операционной системы значения (end)\n");
            break;

        case ET_LOPROC:
            printf("Зависимые от процессора значения (start)\n");
            break;

        case ET_HIPROC:
            printf("Зависимые от процессора значения (end)\n");
            break;
        
        default:
            perror("Не удалось определить тип ELF-файла");
            break;
    }

    printf("  Архитектура:\t\t\t");
    switch (header.e_machine)
    {
        case EM_NONE:
            printf("Не определена\n");
            break;
        
        case EM_M32:
            printf("AT&T WE 32100\n");
            break;

        case EM_SPARC:
            printf("SUN SPARC\n");
            break;

        case EM_386:
            printf("Intel 80386\n");
            break;

        case EM_68K:
            printf("Motorola m68k family\n");
            break;

        case EM_88K:
            printf("Motorola m88k family\n");
            break;

        case EM_IAMCU:
            printf("Intel MCU\n");
            break;

        case EM_860:
            printf("Intel 80860\n");
            break;

        case EM_MIPS:
            printf("MIPS R3000 big-endian\n");
            break;

        case EM_S370:
            printf("IBM System/370\n");
            break;

        case EM_MIPS_RS3_LE:
            printf("MIPS R3000 little-endian\n");
            break;

        case EM_PARISC:
            printf("Hewlett-Packard PA-RISC\n");
            break;

        case EM_VPP500:
            printf("Fujitsu VPP500\n");
            break;

        case EM_SPARC32PLUS:
            printf("Sun's \"v8plus\"\n");
            break;

        case EM_960:
            printf("Intel 80960\n");
            break;

        case EM_PPC:
            printf("PowerPC\n");
            break;

        case EM_PPC64:
            printf("PowerPC 64-bit\n");
            break;

        case EM_S390:
            printf("IBM S390\n");
            break;

        case EM_SPU:
            printf("IBM SPU/SPC\n");
            break;

        case EM_V800:
            printf("NEC V800 series\n");
            break;

        case EM_FR20:
            printf("Fujitsu FR20\n");
            break;
        
        case EM_RH32:
            printf("TRW RH-32\n");
            break;

        case EM_RCE:
            printf("Motorola RCE\n");
            break;

        case EM_ARM:
            printf("ARM (up to ARMv7/Aarch32)\n");
            break;

        case EM_FAKE_ALPHA:
            printf("Digital Alpha\n");
            break;

        case EM_SH:
            printf("Hitachi SH\n");
            break;
        
        case EM_SPARCV9:
            printf("SPARC v9 64-bit\n");
            break;

        case EM_TRICORE:
            printf("Siemens TriCore embedded processor\n");
            break;

        case EM_ARC:
            printf("Argonaut RISC Core\n");
            break;

        case EM_H8_300:
            printf("Hitachi H8/300\n");
            break;

        case EM_H8_300H:
            printf("Hitachi H8/300H\n");
            break;

        case EM_H8S:
            printf("Hitachi H8S\n");
            break;

        case EM_H8_500:
            printf("Hitachi H8/500\n");
            break;

        case EM_IA_64:
            printf("Intel Merced (IA-64)\n");
            break;

        case EM_MIPS_X:
            printf("Stanford MIPS-X\n");
            break;
        
        case EM_COLDFIRE:
            printf("Motorola Coldfire\n");
            break;

        case EM_68HC12:
            printf("Motorola M68HC12\n");
            break;

        case EM_MMA:
            printf("Fujitsu MMA Multimedia Accelerator\n");
            break;

        case EM_PCP:
            printf("Siemens PCP\n");
            break;

        case EM_NCPU:
            printf("Sony nCPU embeeded RISC\n");
            break;

        case EM_NDR1:
            printf("Denso NDR1 microprocessor\n");
            break;

        case EM_STARCORE:
            printf("Motorola Start*Core processor\n");
            break;

        case EM_ME16:
            printf("Toyota ME16 processor\n");
            break;

        case EM_ST100:
            printf("STMicroelectronic ST100 processor\n");
            break;

        case EM_TINYJ:
            printf("Advanced Logic Corp. TinyJ embedded processor family\n");
            break;

        case EM_X86_64:
            printf("AMD x86-64 architecture\n");
            break;

        case EM_PDSP:
            printf("Sony DSP Processor\n");
            break;

        case EM_PDP10:
            printf("Digital PDP-10\n");
            break;

        case EM_PDP11:
            printf("Digital PDP-11\n");
            break;

        case EM_FX66:
            printf("Siemens FX66 microcontroller\n");
            break;

        case EM_ST9PLUS:
            printf("STMicroelectronics ST9+ 8/16 mc\n");
            break;

        case EM_ST7:
            printf("STMicroelectronics ST7 8 bit mc\n");
            break;

        case EM_68HC16:
            printf("Motorola MC68HC16 microcontroller\n");
            break;

        case EM_68HC11:
            printf("Motorola MC68HC11 microcontroller\n");
            break;

        case EM_68HC08:
            printf("Motorola MC68HC08 microcontroller\n");
            break;

        case EM_68HC05:
            printf("Motorola MC68HC05 microcontroller\n");
            break;

        case EM_SVX:
            printf("Silicon Graphics SVx\n");
            break;

        case EM_ST19:
            printf("STMicroelectronics ST19 8 bit mc\n");
            break;

        case EM_VAX:
            printf("Digital VAX\n");
            break;

        case EM_CRIS:
            printf("Axis Communications 32-bit emb.proc\n");
            break;

        case EM_JAVELIN:
            printf("Infineon Technologies 32-bit emb.proc\n");
            break;

        case EM_FIREPATH:
            printf("Element 14 64-bit DSP Processor\n");
            break;

        case EM_ZSP:
            printf("LSI Logic 16-bit DSP Processor\n");
            break;

        case EM_MMIX:
            printf("Donald Knuth's educational 64-bit proc\n");
            break;

        case EM_HUANY:
            printf("Harvard University machine-independent object files\n");
            break;

        case EM_PRISM:
            printf("SiTera Prism\n");
            break;

        case EM_AVR:
            printf("Atmel AVR 8-bit microcontroller\n");
            break;

        case EM_FR30:
            printf("Fujitsu FR30\n");
            break;

        case EM_D10V:
            printf("Mitsubishi D10V\n");
            break;

        case EM_D30V:
            printf("Mitsubishi D30V\n");
            break;

        case EM_V850:
            printf("NEC v850\n");
            break;

        case EM_M32R:
            printf("Mitsubishi M32R\n");
            break;

        case EM_MN10300:
            printf("Matsushita MN10300\n");
            break;

        case EM_MN10200:
            printf("Matsushita MN10200\n");
            break;

        case EM_PJ:
            printf("picoJava\n");
            break;

        case EM_OPENRISC:
            printf("OpenRISC 32-bit embedded processor\n");
            break;

        case EM_ARC_COMPACT:
            printf("ARC International ARCompact\n");
            break;

        case EM_XTENSA:
            printf("Tensilica Xtensa Architecture\n");
            break;

        case EM_VIDEOCORE:
            printf("Alphamosaic VideoCore\n");
            break;

        case EM_TMM_GPP:
            printf("Thompson Multimedia General Purpose Proc\n");
            break;

        case EM_NS32K:
            printf("National Semi. 32000\n");
            break;

        case EM_TPC:
            printf("Tenor Network TPC\n");
            break;

        case EM_SNP1K:
            printf("Trebia SNP 1000\n");
            break;

        case EM_ST200:
            printf("STMicroelectronics ST200\n");
            break;

        case EM_IP2K:
            printf("Ubicom IP2xxx\n");
            break;

        case EM_MAX:
            printf("MAX processor\n");
            break;

        case EM_CR:
            printf("National Semi. CompactRISC\n");
            break;

        case EM_F2MC16:
            printf("Fujitsu F2MC16\n");
            break;

        case EM_MSP430:
            printf("Texas Instruments msp430\n");
            break;

        case EM_BLACKFIN:
            printf("Analog Devices Blackfin DSP\n");
            break;

        case EM_SE_C33:
            printf("Seiko Epson S1C33 family\n");
            break;

        case EM_SEP:
            printf("Sharp embedded microprocessor\n");
            break;

        case EM_ARCA:
            printf("Arca RISC\n");
            break;

        case EM_UNICORE:
            printf("PKU-Unity & MPRC Peking Uni. mc series\n");
            break;

        case EM_EXCESS:
            printf("eXcess configurable cpu\n");
            break;

        case EM_DXP:
            printf("Icera Semi. Deep Execution Processor\n");
            break;

        case EM_ALTERA_NIOS2:
            printf("Altera Nios II\n");
            break;

        case EM_CRX:
            printf("National Semi. CompactRISC CRX\n");
            break;

        case EM_XGATE:
            printf("Motorola XGATE\n");
            break;

        case EM_C166:
            printf("Infineon C16x/XC16x\n");
            break;

        case EM_M16C:
            printf("Renesas M16C\n");
            break;

        case EM_DSPIC30F:
            printf("Microchip Technology dsPIC30F\n");
            break;

        case EM_CE:
            printf("Freescale Communication Engine RISC\n");
            break;

        case EM_M32C:
            printf("Renesas M32C\n");
            break;

        case EM_TSK3000:
            printf("Altium TSK3000\n");
            break;

        case EM_RS08:
            printf("Freescale RS08\n");
            break;

        case EM_SHARC:
            printf("Analog Devices SHARC family\n");
            break;

        case EM_SCORE7: //135/253
            printf("Sunplus S+core7 RISC\n");
            break;

        default:
            perror("Не удалось определить версию спецификации ELF-формата");
            break;
    }


    printf("\n");
    fclose(file_pointer);
}

int main()
{    
    read_header("example");

    return 0;
}