#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <elf.h>

long start_of_section_headers = 0;
int number_of_section_headers = 0;
int section_header_string_table_index = 0;

long start_of_segment_headers = 0;
int number_of_segment_headers = 0;
long size_of_segment_headers = 0;

//header
void read_header(const char* filename)
{
    FILE* file_pointer;
    Elf64_Ehdr header;

    file_pointer = fopen(filename, "r");
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

    printf("  Класс:\t\t\t\t");
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

    printf("  Данные:\t\t\t\tдополнение до 2, ");
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

    printf("  Версия:\t\t\t\t");
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

    printf("  OS/ABI:\t\t\t\t");
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

    printf("  Версия ABI:\t\t\t\t%d\n", header.e_ident[EI_ABIVERSION]);

    printf("  Начало заполнителя:\t\t\t%d\n", header.e_ident[EI_PAD]);

    printf("  Тип объектного файла:\t\t\t");
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

    printf("  Архитектура:\t\t\t\t");
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

        case EM_ECOG2: 
            printf("Cyan Technology eCOG2\n");
            break;

        case EM_SCORE7: 
            printf("Sunplus S+core7 RISC\n");
            break;

        case EM_DSP24: 
            printf("New Japan Radio (NJR) 24-bit DSPC\n");
            break;

        case EM_VIDEOCORE3: 
            printf("Broadcom VideoCore III\n");
            break;

        case EM_LATTICEMICO32: 
            printf("RISC for Lattice FPGA\n");
            break;

        case EM_SE_C17: 
            printf("Seiko Epson C17\n");
            break;

        case EM_TI_C6000: 
            printf("Texas Instruments TMS320C6000 DSP\n");
            break;

        case EM_TI_C2000: 
            printf("Texas Instruments TMS320C2000 DSP\n");
            break;

        case EM_TI_C5500: 
            printf("Texas Instruments TMS320C55x DSP\n");
            break;

        case EM_TI_ARP32: 
            printf("Texas Instruments App. Specific RISC\n");
            break;

        case EM_TI_PRU: 
            printf("Texas Instruments Prog. Realtime Unit\n");
            break;

        case EM_MMDSP_PLUS: 
            printf("STMicroelectronics 64bit VLIW DSP\n");
            break;

        case EM_CYPRESS_M8C: 
            printf("Cypress M8C\n");
            break;

        case EM_R32C: 
            printf("Renesas R32C\n");
            break;

        case EM_TRIMEDIA: 
            printf("NXP Semi. TriMedia\n");
            break;

        case EM_QDSP6: 
            printf("QUALCOMM DSP6\n");
            break;

        case EM_8051: 
            printf("Intel 8051 and variants\n");
            break;

        case EM_STXP7X: 
            printf("STMicroelectronics STxP7x\n");
            break;

        case EM_NDS32: 
            printf("Andes Tech. compact code emb. RISC\n");
            break;

        case EM_ECOG1X: 
            printf("Cyan Technology eCOG1X\n");
            break;

        case EM_MAXQ30: 
            printf("Dallas Semi. MAXQ30 mc\n");
            break;

        case EM_XIMO16: 
            printf("New Japan Radio (NJR) 16-bit DSP\n");
            break;

        case EM_MANIK: 
            printf("M2000 Reconfigurable RISC\n");
            break;

        case EM_CRAYNV2: 
            printf("Cray NV2 vector architecture\n");
            break;

        case EM_RX: 
            printf("Renesas RX\n");
            break;

        case EM_METAG: 
            printf("Imagination Tech. META\n");
            break;

        case EM_MCST_ELBRUS: 
            printf("MCST Elbrus\n");
            break;

        case EM_ECOG16: 
            printf("Cyan Technology eCOG16\n");
            break;

        case EM_CR16: 
            printf("National Semi. CompactRISC CR16\n");
            break;

        case EM_ETPU: 
            printf("Freescale Extended Time Processing Unit\n");
            break;

        case EM_SLE9X: 
            printf("Infineon Tech. SLE9X\n");
            break;

        case EM_L10M: 
            printf("Intel L10M\n");
            break;

        case EM_K10M: 
            printf("Intel K10M\n");
            break;

        case EM_AARCH64: 
            printf("ARM AARCH64\n");
            break;

        case EM_AVR32: 
            printf("Amtel 32-bit microprocessor\n");
            break;

        case EM_STM8: 
            printf("STMicroelectronics STM8\n");
            break;

        case EM_TILE64: 
            printf("Tilera TILE64\n");
            break;

        case EM_TILEPRO: 
            printf("Tilera TILEPro\n");
            break;

        case EM_MICROBLAZE: 
            printf("Xilinx MicroBlaze\n");
            break;

        case EM_CUDA: 
            printf("NVIDIA CUDA\n");
            break;

        case EM_TILEGX: 
            printf("Tilera TILE-Gx\n");
            break;

        case EM_CLOUDSHIELD: 
            printf("CloudShield\n");
            break;

        case EM_COREA_1ST: 
            printf("KIPO-KAIST Core-A 1st gen.\n");
            break;

        case EM_COREA_2ND: 
            printf("KIPO-KAIST Core-A 2nd gen.\n");
            break;

        case EM_ARCV2: 
            printf("Synopsys ARCv2 ISA.\n");
            break;

        case EM_OPEN8: 
            printf("Open8 RISC\n");
            break;

        case EM_RL78: 
            printf("Renesas RL78\n");
            break;

        case EM_VIDEOCORE5: 
            printf("Broadcom VideoCore V\n");
            break;

        case EM_78KOR: 
            printf("Renesas 78KOR\n");
            break;

        case EM_56800EX: 
            printf("Freescale 56800EX DSC\n");
            break;

        case EM_BA1: 
            printf("Beyond BA1\n");
            break;

        case EM_BA2: 
            printf("Beyond BA2\n");
            break;

        case EM_XCORE: 
            printf("XMOS xCORE\n");
            break;

        case EM_MCHP_PIC: 
            printf("Microchip 8-bit PIC(r)\n");
            break;

        case EM_INTELGT: 
            printf("Intel Graphics Technology\n");
            break;  

        case EM_KM32: 
            printf("KM211 KM32\n");
            break;

        case EM_KMX32: 
            printf("KM211 KMX32\n");
            break;

        case EM_EMX16: 
            printf("KM211 EMX16\n");
            break;

        case EM_EMX8: 
            printf("KM211 EMX8\n");
            break;

        case EM_KVARC: 
            printf("KM211 KVARC\n");
            break;

        case EM_CDP: 
            printf("Paneve CDP\n");
            break;

        case EM_COGE: 
            printf("Cognitive Smart Memory Processor\n");
            break;

        case EM_COOL: 
            printf("Bluechip CoolEngine\n");
            break;

        case EM_NORC: 
            printf("Nanoradio Optimized RISC\n");
            break;

        case EM_CSR_KALIMBA: 
            printf("CSR Kalimba\n");
            break;

        case EM_Z80: 
            printf("Zilog Z80\n");
            break;

        case EM_VISIUM: 
            printf("Controls and Data Services VISIUMcore\n");
            break;

        case EM_FT32: 
            printf("FTDI Chip FT32\n");
            break;

        case EM_MOXIE: 
            printf("Moxie processor\n");
            break;

        case EM_AMDGPU: 
            printf("AMD GPU\n");
            break;

        case EM_RISCV: 
            printf("RISC-V\n");
            break;

        case EM_BPF: 
            printf("Linux BPF -- in-kernel virtual machine\n");
            break;

        case EM_CSKY: 
            printf("C-SKY\n");
            break;

        default:
            perror("Не удалось определить версию спецификации ELF-формата");
            break;
    }

    printf("  Версия:\t\t\t\t0x%x\n", header.e_version);
    
    printf("  Адрес точки входа:\t\t\t0x%lx\n", header.e_entry);
    
    printf("  Начало заголовков программы:\t\t%lx (байтов в файле)\n", \
            header.e_phoff);
    start_of_segment_headers = header.e_phoff;

    printf("  Начало заголовков раздела:\t\t%lx (байтов в файле)\n", \
            header.e_shoff);
    start_of_section_headers = header.e_shoff;

    printf("  Флаги:\t\t\t\t0x%x\n", header.e_flags);

    printf("  Размер заголовка ELF-файла:\t\t%d (байтов)\n", header.e_ehsize);

    printf("  Размер программных заголовков:\t%d (байтов)\n", \
            header.e_phentsize);
    size_of_segment_headers = header.e_phentsize;

    printf("  Число заголовков программ:\t\t%d\n", header.e_phnum);
    number_of_segment_headers = header.e_phnum;

    printf("  Размер заголовков секций:\t\t%d (байтов)\n", header.e_shentsize);

    printf("  Число заголовков секций:\t\t%d\n", header.e_shnum);
    number_of_section_headers = header.e_shnum;

    printf("  Индекс заголовка секции, содержащей\n"
           "  имена остальных секций ELF-файла:\t%d\n", header.e_shstrndx);
    section_header_string_table_index = header.e_shstrndx;

    printf("\n");
    fclose(file_pointer);
}

//sections
void print_sh_type(int i, Elf64_Shdr *section_headers)
{
    switch (section_headers[i].sh_type)
    {
        case SHT_NULL:
            printf("NULL                ");
            break;
                
        case SHT_PROGBITS:
            printf("PROGBITS            ");
            break;
            
        case SHT_SYMTAB:
            printf("SYMTAB              ");
            break;
                
        case SHT_STRTAB:
            printf("STRTAB              ");
            break;
            
        case SHT_RELA:
            printf("RELA                ");
            break;
                
        case SHT_HASH:
            printf("HASH                ");
            break;
                
        case SHT_DYNAMIC:
            printf("DYNAMIC             ");
            break;
            
        case SHT_NOTE:
            printf("NOTE                ");
            break;
                
        case SHT_NOBITS:
            printf("NOBITS              ");
            break;
                
        case SHT_REL:
            printf("NOBITS              ");
            break;
            
        case SHT_SHLIB:
            printf("NOBITS              ");
            break;
            
        case SHT_DYNSYM:
            printf("DYNSYM              ");
            break;
                
        case SHT_INIT_ARRAY:
            printf("INIT_ARRAY          ");
            break;
            
        case SHT_FINI_ARRAY:
            printf("FINI_ARRAY          ");
            break;
                
        case SHT_PREINIT_ARRAY:
            printf("PREINIT_ARRAY       ");
            break;
                
        case SHT_GROUP:
            printf("GROUP               ");
            break;
                
        case SHT_SYMTAB_SHNDX:
            printf("SYMTAB_SHNDX        ");
            break;
                
        case SHT_NUM:
            printf("NUM                 ");
            break;

        case SHT_LOOS:
            printf("LOOS                ");
            break;
                
        case SHT_GNU_ATTRIBUTES:
            printf("GNU_ATTRIBUTES      ");
            break;
                
        case SHT_GNU_HASH:
            printf("GNU_HASH            ");
            break;
            
        case SHT_GNU_LIBLIST:
            printf("GNU_LIBLIST         ");
            break;
                
        case SHT_CHECKSUM:
            printf("GNU_CHECKSUM        ");
            break;
                
        case SHT_LOSUNW:
            printf("LOSUNW              ");
            break;
        
        //Дублирует по значению SHT_LOSUNW
        // case SHT_SUNW_move:
        //     printf("SUNW_move\n");
        //     break;
        
        case SHT_SUNW_COMDAT:
            printf("SUNW_COMDAT         ");
            break;
                
        case SHT_SUNW_syminfo:
            printf("SUNW_syminfo        ");
            break;
            
        case SHT_GNU_verdef:
            printf("GNU_verdef          ");
            break;
            
        case SHT_GNU_verneed:
            printf("GNU_verneed         ");
            break;
            
        case SHT_GNU_versym:
            printf("GNU_versym          ");
            break;
        
        //Дублируют по значению SHT_GNU_versym 
        // case SHT_HISUNW:
        //     printf("HISUNW\n");
        //     break;

        // case SHT_HIOS:
        //     printf("HIOS\n");
        //     break;
        
        case SHT_LOPROC:
            printf("LOPROC              ");
            break;
            
        case SHT_HIPROC:
            printf("HIPROC              ");
            break;
                        
        case SHT_LOUSER:
            printf("LOUSER              ");
            break;
            
        case SHT_HIUSER:
            printf("LOUSER              ");
            break;

        default:
            printf("Не опр.             ");
            break;
    }
}

void print_sh_type_in_detail(int i, Elf64_Shdr *section_headers)
{
    switch (section_headers[i].sh_type)
    {  
        case SHT_PROGBITS:
            printf("                            ");
            printf("Program data\n");
            break;

        case SHT_SYMTAB:
            printf("                            ");
            printf("Symbol table\n");
            break;

        case SHT_STRTAB:
            printf("                            ");
            printf("String table\n");
            break;
            
        case SHT_RELA:
            printf("                            ");
            printf("Relocation entries\n");
            printf("                            ");
            printf("with addends\n");
            break;
            
        case SHT_HASH:
            printf("                            ");
            printf("Symbol hash table\n");
            break;
            
        case SHT_DYNAMIC:
            printf("                            ");
            printf("Dynamic linking\n");
            printf("                            ");
            printf("information\n");
            break;
            
        case SHT_NOTE:
            printf("                            ");
            printf("Notes\n");
            break;
            
        case SHT_NOBITS:
            printf("                            ");
            printf("Program space with\n");
            printf("                            ");
            printf("no data (bss)\n");
            break;
            
        case SHT_REL:
            printf("                            ");
            printf("Relocation entries,\n");
            printf("                            ");
            printf("no addends\n");
            break;
            
        case SHT_SHLIB:
            printf("                            ");
            printf("Reserved\n");
            break;
            
        case SHT_DYNSYM:
            printf("                            ");
            printf("Dynamic linker\n");
            printf("                            ");
            printf("symbol table\n");
            break;
            
        case SHT_INIT_ARRAY:
            printf("                            ");
            printf("Array of\n");
            printf("                            ");
            printf("constructors\n");
            break;
            
        case SHT_FINI_ARRAY:
            printf("                            ");
            printf("Array of\n");
            printf("                            ");
            printf("destructors\n");
            break;
            
        case SHT_PREINIT_ARRAY:
            printf("                            ");
            printf("Array of\n");
            printf("                            ");
            printf("pre-constructors\n");
            break;
            
        case SHT_GROUP:
            printf("                            ");
            printf("Section group\n");
            break;
            
        case SHT_SYMTAB_SHNDX:
            printf("                            ");
            printf("Extended section\n");
            printf("                            ");
            printf("indices\n");
            break;
            
        case SHT_NUM:
            printf("                            ");
            printf("Number of defined\n");
            printf("                            ");
            printf("types\n");
            break;
            
        case SHT_LOOS:
            printf("                            ");
            printf("Start OS-specific\n");
            break;
            
        case SHT_GNU_ATTRIBUTES:
            printf("                            ");
            printf("Object attributes\n");
            break;
            
        case SHT_GNU_HASH:
            printf("                            ");
            printf("GNU-style hash\n");
            printf("                            ");
            printf("table\n");
            break;
            
        case SHT_GNU_LIBLIST:
            printf("                            ");
            printf("Prelink library\n");
            printf("                            ");
            printf("list\n");
            break;
            
        case SHT_CHECKSUM:
            printf("                            ");
            printf("Checksum for DSO\n");
            printf("                            ");
            printf("content\n");
            break;
            
        case SHT_LOSUNW:
            printf("                            ");
            printf("Sun-specific low\n");
            printf("                            ");
            printf("bound\n");
            break;

        case SHT_GNU_verdef:
            printf("                            ");
            printf("Version definition\n");
            printf("                            ");
            printf("section\n");
            break;
            
        case SHT_GNU_verneed:
            printf("                            ");
            printf("Version needs\n");
            printf("                            ");
            printf("section\n");
            break;
            
        case SHT_GNU_versym:
            printf("                            ");
            printf("Version symbol\n");
            printf("                            ");
            printf("table\n");
            break;
            
        case SHT_LOPROC:
            printf("                            ");
            printf("Start of\n");
            printf("                            ");
            printf("processor-specific\n");
            break;

        case SHT_HIPROC:
            printf("                            ");
            printf("End of\n");
            printf("                            ");
            printf("processor-specific\n");
            break;
            
        case SHT_LOUSER:
            printf("                            ");
            printf("Start of\n");
            printf("                            ");
            printf("application-\n");
            printf("                            ");
            printf("specific\n");
            break;
            
        case SHT_HIUSER:
            printf("                            ");
            printf("End of\n");
            printf("                            ");
            printf("application\n");
            printf("                            ");
            printf("specific\n");
            break;

        default:
            break;
    }
}

void print_sh_flags(int i, Elf64_Shdr *section_headers)
{
    if ((section_headers[i].sh_flags & SHF_WRITE) >> 0 == 1) { printf("W"); }
    if ((section_headers[i].sh_flags & SHF_ALLOC) >> 1 == 1) { printf("A"); }
    if ((section_headers[i].sh_flags & SHF_EXECINSTR) >> 2 == 1) { printf("X"); }
    if ((section_headers[i].sh_flags & SHF_MERGE) >> 4 == 1) { printf("M"); }
    if ((section_headers[i].sh_flags & SHF_STRINGS) >> 5 == 1) { printf("S"); }
    if ((section_headers[i].sh_flags & SHF_INFO_LINK) >> 6 == 1) { printf("I"); }
    if ((section_headers[i].sh_flags & SHF_LINK_ORDER) >> 7 == 1) { printf("L"); }
    if ((section_headers[i].sh_flags & SHF_OS_NONCONFORMING) >> 8 == 1) { printf("O"); }
    if ((section_headers[i].sh_flags & SHF_GROUP) >> 9 == 1) { printf("G"); }
    if ((section_headers[i].sh_flags & SHF_TLS) >> 10 == 1) { printf("T"); }
    if ((section_headers[i].sh_flags & SHF_COMPRESSED) >> 11 == 1) { printf("C"); }
    if (section_headers[i].sh_flags == SHF_MASKOS) { printf("o"); }
    if (section_headers[i].sh_flags == SHF_MASKPROC) { printf("p"); }
    if ((section_headers[i].sh_flags & SHF_GNU_RETAIN) >> 21 == 1) { printf("R"); }
    //SHF_ORDERED - это более старая версия функциональности, 
    //предоставляемой SHF_LINK_ORDER, и была заменена SHF_LINK_ORDER. 
    //SHF_ORDERED больше не поддерживается. 
    if ((section_headers[i].sh_flags & SHF_ORDERED) >> 30 == 1) { printf("L"); }
    if (section_headers[i].sh_flags == SHF_EXCLUDE) { printf("E"); }    

    printf("\t");
}

void read_sections(const char* filename)
{
    Elf64_Shdr *section_headers; //массив указателей на структуры
    section_headers = (Elf64_Shdr*)malloc(number_of_section_headers);

    FILE* file_pointer; 
    file_pointer = fopen(filename, "r");
    fseek(file_pointer, start_of_section_headers, SEEK_SET); 
    /*в массив считывается определенное ранее число указателей на структуры*/   
    fread(section_headers, sizeof(Elf64_Shdr), number_of_section_headers, \
          file_pointer);

    if (number_of_section_headers%10 == 1)
    {
        printf("Имеется %d заголовок разделов, начиная со смещения 0x%lx:\n", \
                number_of_section_headers, start_of_section_headers);
    }
    else if (number_of_section_headers%10 == 2 
             || number_of_section_headers%10 == 3
             || number_of_section_headers%10 == 4)
    {
        printf("Имеется %d заголовка разделов, начиная со смещения 0x%lx:\n", \
                number_of_section_headers, start_of_section_headers);
    }
    else
    {
        printf("Имеется %d заголовков разделов, начиная со смещения 0x%lx:\n", \
                number_of_section_headers, start_of_section_headers);
    }

    printf("\nЗаголовки разделов:\n");
    printf("  [Нм] Имя                  Тип\t\t\tАдрес      Смещ   Размер ES "
           "Флг\tЛк Инф Al\n");

    fseek(file_pointer, \
          section_headers[section_header_string_table_index].sh_offset, \
          SEEK_SET);
    int size_of_section = section_headers[section_header_string_table_index].sh_size;
    char string_keeper[size_of_section];    
    fread(string_keeper, 1, size_of_section, file_pointer);   
    
    printf("  ----|--------------------|-------------------|----------|-"
           "-----|------|--|---|--|---|--\n");
    for (int i = 0; i < number_of_section_headers; i++)
    {
        printf("  [%2d]\t", i);
        printf("%-20s", &string_keeper[section_headers[i].sh_name]);
        print_sh_type(i, section_headers);
        printf("0x%08lx ", section_headers[i].sh_addr);
        printf("%06lx ", section_headers[i].sh_offset);
        printf("%06lx ", section_headers[i].sh_size);
        printf("%02lx ", section_headers[i].sh_entsize);
        print_sh_flags(i, section_headers);
        printf("%-2d ", section_headers[i].sh_link);
        printf("%-2d  ", section_headers[i].sh_info);
        printf("%ld\n", section_headers[i].sh_addralign);

        print_sh_type_in_detail(i, section_headers); 
        printf("  ----|--------------------|-------------------|----------|-"
               "-----|------|--|---|--|---|--\n");          
        
    }
    printf("Обозначения флагов:\n");
    printf("  W (запись), A (назнач), X (исполняемый), M (слияние), S (строки),\n");
    printf("  I (инфо), L (порядок ссылок), O (требуется дополнительная работа ОС),\n");
    printf("  G (группа), T (TLS), C (сжат), o (специфич. для ОС)\n");
    printf("  R (не исп. GC при компоновке), E (исключен), p (processor specific)\n\n");

    fclose(file_pointer);
}

void read_segments(const char* filename)
{
    FILE* file_pointer;
    Elf64_Ehdr header;      

    file_pointer = fopen(filename, "r"); 
    fread(&header, sizeof(Elf64_Ehdr), 1, file_pointer); 

    printf("Тип файла ELF - ");
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
    printf("Точка входа 0x%lx\n", header.e_entry);
    printf("Имеется %d заголовков программы, начиная со смещения %ld\n\n", \
    number_of_segment_headers, start_of_segment_headers);   
    printf("Заголовки программы:\n");
    printf("  Тип           Смещ.    Вирт.адр Физ.адр Рзм.фйл Рзм.пм Флг Выравн\n");

    fseek(file_pointer, start_of_segment_headers, SEEK_SET); 
    
    Elf64_Phdr segment_headers[number_of_segment_headers]; 

    for (int i = 0; i < number_of_segment_headers; i++)
    {
        fread(&segment_headers[i], sizeof(Elf64_Phdr), 1, file_pointer);

        switch (segment_headers[i].p_type) 
        {
            case PT_NULL:
                printf("  NULL\t\t");
                break;
            
            case PT_LOAD:
                printf("  LOAD\t\t");
                break;

            case PT_DYNAMIC:
                printf("  DYNAMIC\t");
                break;

            case PT_INTERP:
                printf("  INTERP\t");
                break;

            case PT_NOTE:
                printf("  NOTE\t\t");
                break;

            case PT_SHLIB:
                printf("  SHLIB\t");
                break;
            
            case PT_PHDR:
                printf("  PHDR\t\t");
                break;

            case PT_TLS:
                printf("  TLS\t\t");
                break;

            case PT_NUM:
                printf("  NUM\t\t");
                break;

            case PT_LOOS:
                printf("  LOOS\t\t");
                break;

            case PT_GNU_EH_FRAME:
                printf("  GNU_EH_FRAME\t");
                break;

            case PT_GNU_STACK:
                printf("  GNU_STACK\t");
                break;

            case PT_GNU_RELRO:
                printf("  GNU_RELRO\t");
                break;

            case PT_GNU_PROPERTY:
                printf("  GNU_PROPERTY\t");
                break;

            case PT_LOSUNW:
                printf("  LOSUNW\t");
                break;

            // case PT_SUNWBSS:
            //     printf("  SUNWBSS\t");
            //     break;

            case PT_SUNWSTACK:
                printf("  SUNWSTACK\t");
                break;

            case PT_HISUNW:
                printf("  HISUNW\t");
                break;

            // case PT_HIOS:
            //     printf("  HIOS\t\t");
            //     break;

            case PT_LOPROC:
                printf("  LOPROC\t");
                break;

            case PT_HIPROC:
                printf("  HIPROC\t");
                break;

            default:
                break;
        }    

        printf("%08lx ", segment_headers[i].p_offset);
    }    
    
}

int main()
{   
    char filename[] = "example";
    read_header(filename);
    read_sections(filename);
    read_segments(filename);

    return 0;
}