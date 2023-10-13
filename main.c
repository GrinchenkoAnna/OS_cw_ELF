#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <elf.h>
#include <link.h>

long start_of_section_headers = 0;
int number_of_section_headers = 0;
int section_header_string_table_index = 0;

long start_of_segment_headers = 0;
int number_of_segment_headers = 0;
long size_of_segment_headers = 0;

long dynstr_offset = 0;
long dynstr_size = 0;
long dynstr_size_cur = 0; 
long dynstr_size_next = 0; 

long dynamic_offset = 0;
long dynamic_size = 0;

long rel_offset[4] = { 0, 0, 0, 0 };
long rel_size[4] = { 0, 0, 0, 0 };
int rel_index = 0;
int rel_index_in_section_headers[4] = {-1, -1, -1, -1};

long rela_offset[4] = { 0, 0, 0, 0 };
long rela_size[4] = { 0, 0, 0, 0 };
int rela_index = 0;
int rela_index_in_section_headers[4] = {-1, -1, -1, -1};
long relocs_arch = 0;

long dynsym_offset = 0;
long dynsym_size = 0;

long symtab_offset = 0;
long symtab_size = 0;

long gnu_verneed_offset = 0;
long gnu_verneed_size = 0;
int gnu_verneed_num = 0;

//header
void read_header(FILE* file_pointer)
{
    Elf64_Ehdr header;
    
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
            relocs_arch = EM_X86_64;
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
            symtab_offset = section_headers[i].sh_offset;
            symtab_size = section_headers[i].sh_size;
            break;
                
        case SHT_STRTAB:
            printf("STRTAB              ");
            break;
            
        case SHT_RELA:
            printf("RELA                ");            
            rela_offset[rela_index] = section_headers[i].sh_offset;  
            rela_size[rela_index] = section_headers[i].sh_size; 
            rela_index_in_section_headers[rela_index] = i;
            rela_index++;    
            break;
                
        case SHT_HASH:
            printf("HASH                ");
            break;
                
        case SHT_DYNAMIC:
            printf("DYNAMIC             ");
            dynamic_offset = section_headers[i].sh_offset;
            dynamic_size = section_headers[i].sh_size;
            break;
            
        case SHT_NOTE:
            printf("NOTE                ");
            break;
                
        case SHT_NOBITS:
            printf("NOBITS              ");
            break;
                
        case SHT_REL:
            printf("NOBITS              ");
            rel_offset[rel_index] = section_headers[i].sh_offset;  
            rel_size[rel_index] = section_headers[i].sh_size; 
            rel_index_in_section_headers[rel_index] = i;
            rel_index++;    
            break;
            
        case SHT_SHLIB:
            printf("NOBITS              ");
            break;
            
        case SHT_DYNSYM:
            printf("DYNSYM              ");
            dynsym_offset = section_headers[i].sh_offset;
            dynsym_size = section_headers[i].sh_size;
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
            //!!
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
            gnu_verneed_offset = section_headers[i].sh_offset;
            gnu_verneed_size = section_headers[i].sh_size;
            break;
            
        case SHT_GNU_versym:
            printf("GNU_versym          ");
            // gnu_versym_offset = section_headers[i].sh_offset;
            // gnu_versym_size = section_headers[i].sh_size;
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
            //dynstr_offset = section_headers[i].sh_offset;
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
    Elf64_Xword sh_flags = section_headers[i].sh_flags;
    if ((sh_flags & SHF_WRITE) >> 0 == 1) { printf("W"); }
    if ((sh_flags & SHF_ALLOC) >> 1 == 1) { printf("A"); }
    if ((sh_flags & SHF_EXECINSTR) >> 2 == 1) { printf("X"); }
    if ((sh_flags & SHF_MERGE) >> 4 == 1) { printf("M"); }
    if ((sh_flags & SHF_STRINGS) >> 5 == 1) { printf("S"); }
    if ((sh_flags & SHF_INFO_LINK) >> 6 == 1) { printf("I"); }
    if ((sh_flags & SHF_LINK_ORDER) >> 7 == 1) { printf("L"); }
    if ((sh_flags & SHF_OS_NONCONFORMING) >> 8 == 1) { printf("O"); }
    if ((sh_flags & SHF_GROUP) >> 9 == 1) { printf("G"); }
    if ((sh_flags & SHF_TLS) >> 10 == 1) { printf("T"); }
    if ((sh_flags & SHF_COMPRESSED) >> 11 == 1) { printf("C"); }
    if (sh_flags == SHF_MASKOS) { printf("o"); }
    if (sh_flags == SHF_MASKPROC) { printf("p"); }
    if ((sh_flags & SHF_GNU_RETAIN) >> 21 == 1) { printf("R"); }
    //SHF_ORDERED - это более старая версия функциональности, 
    //предоставляемой SHF_LINK_ORDER, и была заменена SHF_LINK_ORDER. 
    //SHF_ORDERED больше не поддерживается. 
    if ((sh_flags & SHF_ORDERED) >> 30 == 1) { printf("L"); }
    if (sh_flags == SHF_EXCLUDE) { printf("E"); }    

    printf("\t");
}

void read_sections(FILE* file_pointer)
{
    Elf64_Shdr *section_headers; //массив указателей на структуры
    section_headers = (Elf64_Shdr*)malloc(number_of_section_headers);
    
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
    fread(string_keeper, size_of_section, 1, file_pointer);   
    
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
}

void print_p_flags(int i, Elf64_Phdr *segment_headers)
{
    Elf64_Xword p_flags = segment_headers[i].p_flags;
    if ((p_flags & PF_R) >> 1 == 2) { printf("R"); }
    else { printf(" "); }
    if ((p_flags & PF_W) >> 1 == 1) { printf("W"); }
    else { printf(" "); }
    if ((p_flags & PF_X) >> 0 == 1) { printf("E"); }
    else { printf(" "); }   

    if (p_flags == PF_MASKOS) { printf("OS-sp"); }
    if (p_flags == PF_MASKPROC) { printf("Proc-sp"); }

    printf("\t");
}

void read_segments(FILE* file_pointer)
{    
    Elf64_Ehdr header;  

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
    printf("  Тип           Смещ.    Вирт.адр           Физ.адр " 
    "           Рзм.фйл  Рзм.пм   Флг    Выравн\n");

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
        printf("0x%06lx ", segment_headers[i].p_offset);
        printf("0x%016lx ", segment_headers[i].p_vaddr);
        printf("0x%016lx ", segment_headers[i].p_paddr);
        printf("0x%06lx ", segment_headers[i].p_filesz);
        printf("0x%06lx ", segment_headers[i].p_memsz);
        print_p_flags(i, segment_headers);
        printf("0x%lx \n", segment_headers[i].p_align);
    }    

    printf("\nСоответствие раздел-сегмент:\n  Сегмент Разделы...\n"); 

    Elf64_Shdr section_headers[number_of_section_headers]; 
    fseek(file_pointer, start_of_section_headers, SEEK_SET);
    fread(section_headers, sizeof(Elf64_Shdr), number_of_section_headers, \
          file_pointer);
    fseek(file_pointer, \
          section_headers[section_header_string_table_index].sh_offset, \
          SEEK_SET);
    int size_of_section = section_headers[section_header_string_table_index].sh_size;
    char string_keeper[size_of_section];    
    fread(string_keeper, 1, size_of_section, file_pointer);   

    for (int i = 0; i < number_of_segment_headers; i++)
    {
        printf("   %02d\t", i);
        for (int j = 0; j < number_of_section_headers; j++)
        {
            if (section_headers[j].sh_offset >= segment_headers[i].p_offset 
            && section_headers[j].sh_offset < segment_headers[i].p_offset 
            + segment_headers[i].p_filesz) 
            {      
                printf("%s ", &string_keeper[section_headers[j].sh_name]);
                //.bss не выводится
            }            
        }        
        printf("\n");
    }    
    printf("\n");
}

void read_needed_library(FILE* file_pointer, Elf64_Dyn *dynamic_arr, int i)
{
    printf("\t\tСовм. исп. библиотека: [");    
    char prev;
    fseek(file_pointer, dynstr_offset + dynamic_arr[i].d_un.d_val, SEEK_SET);    
    while (1)
    {
        prev = fgetc(file_pointer);
        printf("%c", prev);
        if (prev == '\0') { break; }
    }   
    printf("]"); 
}

void print_d_tag(int i, Elf64_Dyn *dynamic_arr, int number_of_elements, FILE* file_pointer)
{
    switch (dynamic_arr[i].d_tag)
    {
        case DT_NULL:
            printf("(NULL)");
            printf("\t\t0x0");
            break;

        case DT_NEEDED:
            printf("(NEEDED)");
            read_needed_library(file_pointer, dynamic_arr, i);
            break;

        case DT_PLTRELSZ:
            printf("(PLTRELSZ)");
            printf("\t\t%ld (байт)", dynamic_arr[i].d_un.d_val);
            break;

        case DT_PLTGOT:
            printf("(PLTGOT)");
            printf("\t\t0x%04lx", dynamic_arr[i].d_un.d_ptr);
            break;

        case DT_HASH:
            printf("(HASH)");
            printf("\t\t0x%04lx", dynamic_arr[i].d_un.d_ptr);
            break;

        case DT_STRTAB:
            printf("(STRTAB)");
            printf("\t\t0x%04lx", dynamic_arr[i].d_un.d_ptr);
            dynstr_offset = dynamic_arr[i].d_un.d_ptr;
            break;

        case DT_SYMTAB:
            printf("(SYMTAB)");
            printf("\t\t0x%04lx", dynamic_arr[i].d_un.d_ptr);
            break;

        case DT_RELA:
            printf("(RELA)");
            printf("\t\t0x%04lx", dynamic_arr[i].d_un.d_ptr);
            break;
        
        case DT_RELASZ:
            printf("(RELASZ)");
            printf("\t\t%ld (байт)", dynamic_arr[i].d_un.d_val);
            break;

        case DT_RELAENT:
            printf("(RELAENT)");
            printf("\t\t%ld (байт)", dynamic_arr[i].d_un.d_val);
            break;
            
        case DT_STRSZ:
            printf("(STRSZ)");
            printf("\t\t%ld (байт)", dynamic_arr[i].d_un.d_val);
            break;

        case DT_SYMENT:
            printf("(SYMENT)");
            printf("\t\t%ld (байт)", dynamic_arr[i].d_un.d_val);
            break;

        case DT_INIT:
            printf("(INIT)");
            printf("\t\t0x%04lx", dynamic_arr[i].d_un.d_ptr);
            break;

        case DT_FINI:
            printf("(FINI)");
            printf("\t\t0x%04lx", dynamic_arr[i].d_un.d_ptr);
            break;

        case DT_SONAME:
            printf("(SONAME)");
            printf("\t\t%ld", dynamic_arr[i].d_un.d_val);
            break;

        case DT_RPATH:
            printf("(RPATH)");
            printf("\t\t%ld", dynamic_arr[i].d_un.d_val);
            break;

        case DT_SYMBOLIC:
            printf("(SYMBOLIC)");
            printf("\t\t0x%04lx", dynamic_arr[i].d_un.d_val);            
            break;

        case DT_REL:
            printf("(REL)");
            printf("\t\t0x%04lx", dynamic_arr[i].d_un.d_ptr);
            break;

        case DT_RELSZ:
            printf("(RELSZ)");
            printf("\t\t%ld (байт)", dynamic_arr[i].d_un.d_val);
            break;

        case DT_RELENT:
            printf("(RELENT)");
            printf("\t\t%ld (байт)", dynamic_arr[i].d_un.d_val);
            break;

        case DT_PLTREL:
            printf("(PLTREL)");
            for (int j = 0; j < number_of_elements; j++)
            {
                if (dynamic_arr[j].d_tag == dynamic_arr[i].d_un.d_val)
                {
                    switch (dynamic_arr[j].d_tag)
                    {
                        case DT_REL:
                            printf("\t\tREL"); break;

                        case DT_RELA:
                            printf("\t\tRELA"); break;

                        default:
                            printf("\t\tOther"); break;
                    }
                }
            }
            break;

        case DT_DEBUG:
            printf("(DEBUG)");
            printf("\t\t0x%01lx", dynamic_arr[i].d_un.d_ptr);
            break;

        case DT_TEXTREL:
            printf("(TEXTREL)");
            break;

        case DT_JMPREL:
            printf("(JMPREL)");
            printf("\t\t0x%03lx", dynamic_arr[i].d_un.d_ptr);
            break;

        case DT_BIND_NOW:
            printf("(BIND_NOW)");
            break;

        case DT_INIT_ARRAY:
            printf("(INIT_ARRAY)");
            printf("\t\t0x%04lx", dynamic_arr[i].d_un.d_ptr);
            break;

        case DT_FINI_ARRAY:
            printf("(FINI_ARRAY)");
            printf("\t\t0x%04lx", dynamic_arr[i].d_un.d_ptr);
            break;

        case DT_INIT_ARRAYSZ:
            printf("(INIT_ARRAYSZ)");
            printf("\t%ld (байт)", dynamic_arr[i].d_un.d_val);
            break;

        case DT_FINI_ARRAYSZ:
            printf("(FINI_ARRAYSZ)");
            printf("\t%ld (байт)", dynamic_arr[i].d_un.d_val);
            break;

        case DT_RUNPATH:
            printf("(RUNPATH)");
            break;

        case DT_FLAGS:
            printf("(FLAGS)");
            switch (dynamic_arr[i].d_un.d_val)
            {
                case DF_ORIGIN:
                    printf("\t\tORIGIN");
                    break;
                
                case DF_SYMBOLIC:
                    printf("\t\tSYMBOLIC");
                    break;

                case DF_TEXTREL:
                    printf("\t\tTEXTREL");
                    break;

                case DF_BIND_NOW:
                    printf("\t\tBIND_NOW");
                    break;

                case DF_STATIC_TLS:
                    printf("\t\tSTATIC_TLS");
                    break;

                default:
                    printf("\t\tНе опр.");
            }
            break;

        case DT_ENCODING:
            printf("(ENCODING)");
            printf("\t\t0x%04lx", dynamic_arr[i].d_un.d_ptr);
            break;

        // case DT_PREINIT_ARRAY:
        //     printf("(PREINIT_ARRAY)");
        //     break;

        case DT_PREINIT_ARRAYSZ:
            printf("(PREINIT_ARRAYSZ)");
            printf("\t%ld (байт)", dynamic_arr[i].d_un.d_val);
            break;
        
        case DT_SYMTAB_SHNDX:
            printf("(SYMTAB_SHNDX)");
            printf("\t\t0x%04lx", dynamic_arr[i].d_un.d_ptr);
            break;

        case DT_NUM:
            printf("(NUM)");
            printf("\t%ld", dynamic_arr[i].d_un.d_val);
            break;

        case DT_LOOS:
            printf("(LOOS)");
            printf("\t\t0x%04lx", dynamic_arr[i].d_un.d_ptr);
            break;

        case DT_HIOS:
            printf("(HIOS)");
            printf("\t\t0x%04lx", dynamic_arr[i].d_un.d_ptr);
            break;

        case DT_LOPROC:
            printf("(LOPROC)");
            printf("\t\t0x%04lx", dynamic_arr[i].d_un.d_ptr);
            break;

        case DT_HIPROC:
            printf("(HIPROC)");
            printf("\t\t0x%04lx", dynamic_arr[i].d_un.d_ptr);
            break;

        case DT_PROCNUM:
            printf("(PROCNUM)");
            printf("\t\t0x%ld", dynamic_arr[i].d_un.d_val);
            break;

        // 1
        case DT_VALRNGLO:
            printf("(VALRNGLO)");
            break;

        case DT_GNU_PRELINKED:
            printf("(GNU_PRELINKED)");
            break;

        case DT_GNU_CONFLICTSZ:
            printf("(GNU_CONFLICTSZ)");
            printf("\t%ld (байт)", dynamic_arr[i].d_un.d_val);
            break;

        case DT_GNU_LIBLISTSZ:
            printf("(GNU_LIBLISTSZ)");
            printf("\t%ld (байт)", dynamic_arr[i].d_un.d_val);
            break;

        case DT_CHECKSUM:
            printf("(CHECKSUM)");
            printf("\t%ld", dynamic_arr[i].d_un.d_val);
            break;

        case DT_PLTPADSZ:
            printf("(PLTPADSZ)");
            printf("\t%ld (байт)", dynamic_arr[i].d_un.d_val);
            break;

        case DT_MOVEENT:
            printf("(MOVEENT)");
            printf("\t%ld (байт)", dynamic_arr[i].d_un.d_val);
            break;

        case DT_MOVESZ:
            printf("(MOVESZ)");
            printf("\t%ld (байт)", dynamic_arr[i].d_un.d_val);
            break;

        case DT_FEATURE_1:
            printf("(FEATURE_1)");
            switch (dynamic_arr[i].d_un.d_val)
            {
                case DTF_1_PARINIT:
                    printf("\t\tPARINIT");
                    break;
                
                case DTF_1_CONFEXP:
                    printf("\t\tCONFEXP");
                    break;

                default:
                    printf("\t\tНе опр.");
            }
            break;

        case DT_POSFLAG_1:
            printf("(POSFLAG_1)");
            switch (dynamic_arr[i].d_un.d_val)
            {
                case DF_P1_LAZYLOAD:
                    printf("\t\tLAZYLOAD");
                    break;
                
                case DF_P1_GROUPPERM:
                    printf("\t\tGROUPPERM");
                    break;

                default:
                    printf("\t\tНе опр.");
            }
            break;

        case DT_SYMINSZ:
            printf("(SYMINSZ)");
            printf("\t%ld (байт)", dynamic_arr[i].d_un.d_val);
            break;

        case DT_SYMINENT:
            printf("(SYMINENT)");
            printf("\t%ld (байт)", dynamic_arr[i].d_un.d_val);
            break;

        // case DT_VALRNGHI:
        //     printf("(VALRNGHI)");
        //     break;

        // case DT_VALTAGIDX:
        //     printf("(VALTAGIDX)");
        //     break;

        // case DT_VALNUM:
        //     printf("(VALNUM)");
        //     break;

        // 2
        case DT_ADDRRNGLO:
            printf("(ADDRRNGLO)");
            break;

        case DT_GNU_HASH:
            printf("(GNU_HASH)");
            printf("\t\t0x%04lx", dynamic_arr[i].d_un.d_ptr);
            break;

        case DT_TLSDESC_PLT:
            printf("(TLSDESC_PLT)");
            break;

        case DT_TLSDESC_GOT:
            printf("(TLSDESC_GOT)");
            break;

        case DT_GNU_CONFLICT:
            printf("(GNU_CONFLICT)");
            printf("\t\t0x%04lx", dynamic_arr[i].d_un.d_ptr);
            break;

        case DT_GNU_LIBLIST:
            printf("(GNU_LIBLIST)");
            break;

        case DT_CONFIG:
            printf("(CONFIG)");
            printf("\t\t0x%04lx", dynamic_arr[i].d_un.d_ptr);
            break;
        
        case DT_DEPAUDIT:
            printf("(DEPAUDIT)");
            printf("\t\t0x%04lx", dynamic_arr[i].d_un.d_ptr);
            break;

        case DT_AUDIT:
            printf("(AUDIT)");
            printf("\t\t0x%04lx", dynamic_arr[i].d_un.d_ptr);
            break;

        case DT_PLTPAD:
            printf("(PLTPAD)");
            printf("\t\t0x%04lx", dynamic_arr[i].d_un.d_ptr);
            break;

        case DT_MOVETAB:
            printf("(MOVETAB)");
            printf("\t\t0x%04lx", dynamic_arr[i].d_un.d_ptr);
            break;

        case DT_SYMINFO:
            printf("(SYMINFO)");
            printf("\t\t0x%04lx", dynamic_arr[i].d_un.d_ptr);
            break;

        // case DT_ADDRRNGHI:
        //     printf("(ADDRRNGHI)");
        //     break;

        // case DT_ADDRTAGIDX(tag): //(DT_VALRNGHI - (tag))
        //     printf("(ADDRTAGIDX)");
        //     break;

        // case DT_ADDRNUM:
        //     printf("(ADDRNUM)");
        //     break;

        // 3
        case DT_VERSYM:
            printf("(VERSYM)");
            printf("\t\t0x%03lx", dynamic_arr[i].d_un.d_ptr);
            break;

        case DT_RELACOUNT:
            printf("(RELACOUNT)");
            printf("\t\t%ld", dynamic_arr[i].d_un.d_val);
            break;

        case DT_RELCOUNT:
            printf("(RELCOUNT)");
            printf("\t%ld", dynamic_arr[i].d_un.d_val);
            break;

        // 4
        case DT_FLAGS_1:
            printf("(FLAGS_1)\t\tФлаги: ");
            Elf64_Xword d_val = dynamic_arr[i].d_un.d_val;
            if ((d_val & DF_1_NOW) == DF_1_NOW) { printf("NOW "); }
            if ((d_val & DF_1_GLOBAL) == DF_1_GLOBAL) { printf("GLOBAL "); }
            if ((d_val & DF_1_GROUP) == DF_1_GROUP) { printf("GROUP "); }
            if ((d_val & DF_1_NODELETE) == DF_1_NODELETE) { printf("NODELETE "); }
            if ((d_val & DF_1_LOADFLTR) == DF_1_LOADFLTR) { printf("LOADFLTR "); }
            if ((d_val & DF_1_INITFIRST) == DF_1_INITFIRST) { printf("INITFIRST "); }
            if ((d_val & DF_1_NOOPEN) == DF_1_NOOPEN) { printf("NOOPEN "); }
            if ((d_val & DF_1_ORIGIN) == DF_1_ORIGIN) { printf("ORIGIN "); }
            if ((d_val & DF_1_DIRECT) == DF_1_DIRECT) { printf("DIRECT "); }
            if ((d_val & DF_1_TRANS) == DF_1_TRANS) { printf("TRANS "); }
            if ((d_val & DF_1_INTERPOSE) == DF_1_INTERPOSE) { printf("INTERPOSE "); }
            if ((d_val & DF_1_NODEFLIB) == DF_1_NODEFLIB) { printf("NODEFLIB "); }
            if ((d_val & DF_1_NODUMP) == DF_1_NODUMP) { printf("NODUMP "); }
            if ((d_val & DF_1_CONFALT) == DF_1_CONFALT) { printf("CONFALT "); }
            if ((d_val & DF_1_ENDFILTEE) == DF_1_ENDFILTEE) { printf("ENDFILTEE "); }
            if ((d_val & DF_1_DISPRELDNE) == DF_1_DISPRELDNE) { printf("DISPRELDNE "); }
            if ((d_val & DF_1_DISPRELPND) == DF_1_DISPRELPND) { printf("DISPRELPND "); }
            if ((d_val & DF_1_NODIRECT) == DF_1_NODIRECT) { printf("NODIRECT "); }
            if ((d_val & DF_1_IGNMULDEF) == DF_1_IGNMULDEF) { printf("IGNMULDEF "); }
            if ((d_val & DF_1_NOKSYMS) == DF_1_NOKSYMS) { printf("NOKSYMS "); }
            if ((d_val & DF_1_NOHDR) == DF_1_NOHDR) { printf("NOHDR "); }
            if ((d_val & DF_1_EDITED) == DF_1_EDITED) { printf("EDITED "); }
            if ((d_val & DF_1_NORELOC) == DF_1_NORELOC) { printf("NORELOC "); }
            if ((d_val & DF_1_SYMINTPOSE) == DF_1_SYMINTPOSE) { printf("SYMINTPOSE "); }
            if ((d_val & DF_1_GLOBAUDIT) == DF_1_GLOBAUDIT) { printf("GLOBAUDIT "); }
            if ((d_val & DF_1_SINGLETON) == DF_1_SINGLETON) { printf("SINGLETON "); }
            if ((d_val & DF_1_STUB) == DF_1_STUB) { printf("STUB "); }
            if ((d_val & DF_1_PIE) == DF_1_PIE) { printf("PIE "); }  
            if ((d_val & DF_1_KMOD) == DF_1_KMOD) { printf("KMOD "); }
            if ((d_val & DF_1_WEAKFILTER) == DF_1_WEAKFILTER) { printf("WEAKFILTER "); }
            if ((d_val & DF_1_NOCOMMON) == DF_1_NOCOMMON) { printf("NOCOMMON "); }
            break;

        case DT_VERDEF:
            printf("(VERDEF)");
            printf("\t\t0x%04lx", dynamic_arr[i].d_un.d_ptr);
            break;

        case DT_VERDEFNUM:
            printf("(VERDEFNUM)");
            printf("\t\t0x%ld", dynamic_arr[i].d_un.d_val);
            break;

        case DT_VERNEED:
            printf("(VERNEED)");
            printf("\t\t0x%04lx", dynamic_arr[i].d_un.d_ptr);
            break;

        case DT_VERNEEDNUM:
            printf("(VERNEEDNUM)");
            printf("\t\t%ld", dynamic_arr[i].d_un.d_val);
            gnu_verneed_num = dynamic_arr[i].d_un.d_val;
            break;

        // case DT_VERSIONTAGIDX(tag): //(DT_ADDRRNGHI - (tag))
        //     printf("(VERSIONTAGIDX)");
        //     break;

        // case DT_VERSIONTAGNUM:
        //     printf("(VERSIONTAGNUM)");
        //     break;

        // 5
        case DT_AUXILIARY:
            printf("(AUXILIARY)");
            printf("\t\t0x%ld", dynamic_arr[i].d_un.d_val);
            break;

        // case DT_FILTER:
        //     printf("(FILTER)");
        //     break;

        // case DT_EXTRATAGIDX(tag): //(DT_VERNEEDNUM - (tag))
        //     printf("(EXTRATAGIDX)");
        //     break;

        // case DT_EXTRANUM: 
        //     printf("(EXTRANUM)");
        //     break;

        default:
            printf("(Не опр.)");
            break;
    }
}

void read_section_dynamic(FILE* file_pointer) //не считает кол-во записей
{
    Elf64_Dyn dynamic;

    fseek(file_pointer, dynamic_offset, SEEK_SET);
    fread(&dynamic, sizeof(Elf64_Dyn), 1, file_pointer);    
    
    int count = 1;
    while (1)
    {
        fread(&dynamic, sizeof(Elf64_Dyn), 1, file_pointer);
        if (dynamic.d_tag != 0) 
        { 
            count++;
            switch (dynamic.d_tag)
            {
                case DT_STRTAB:
                    dynstr_offset = dynamic.d_un.d_ptr;                    
                    break;

                case DT_STRSZ:
                    dynstr_size = dynamic.d_un.d_val;
                
                default: break;
            } 
        }
        else 
        {
            count++; 
            break; 
        }
    }

    Elf64_Dyn dynamic_arr[count];
    fseek(file_pointer, dynamic_offset, SEEK_SET);       
    
    printf("Динамический раздел .dynamic со смещением 0x%lx "
    "содержит %d элементов:\n", dynamic_offset, count);
    printf("  Тег               Тип\t\t\tИмя/Знач\n");
    
    fseek(file_pointer, dynamic_offset, SEEK_SET);   
    for (int i = 0; i < count; i++)
    {
        fread(&dynamic_arr[i], sizeof(Elf64_Dyn), 1, file_pointer);        
    }  
    for (int i = 0; i < count; i++)
    {        
        printf("0x%016lx ", dynamic_arr[i].d_tag);
        print_d_tag(i, dynamic_arr, count, file_pointer);
        //print_d_tag(i, dynamic_arr, count, file_pointer);

        printf("\n");
    }  

    printf("\n");
}

void read_symbol_name(FILE* file_pointer, Elf64_Rela *rela, int i)
{       
    char prev;
    fseek(file_pointer, dynstr_offset, SEEK_SET);    
    while (prev != EOF)
    {
        prev = fgetc(file_pointer);
        printf("%c", prev);
        if (prev == '\0') { break; }
    }   
    printf(" + "); 
}

void reloc_x86_64(Elf64_Xword info)
{
    switch (ELF64_R_TYPE(info))
    {
        case R_X86_64_NONE:
            printf("R_X86_64_NONE");
            break;

        case R_X86_64_64:
            printf("R_X86_64_64");
            break;

        case R_X86_64_PC32:
            printf("R_X86_64_PC32");
            break;

        case R_X86_64_GOT32:
            printf("R_X86_64_GOT32");
            break;

        case R_X86_64_PLT32:
            printf("R_X86_64_PLT32");
            break;

        case R_X86_64_COPY:
            printf("R_X86_64_COPY");
            break;

        case R_X86_64_GLOB_DAT:
            printf("R_X86_64_GLOB_DAT");
            break;

        case R_X86_64_JUMP_SLOT:
            printf("R_X86_64_JUMP_SLOT");
            break;

        case R_X86_64_RELATIVE:
            printf("R_X86_64_RELATIVE");
            break;

        case R_X86_64_GOTPCREL:
            printf("R_X86_64_GOTPCREL");
            break;

        case R_X86_64_32:
            printf("R_X86_64_32");
            break;

        case R_X86_64_32S:
            printf("R_X86_64_32S");
            break;

        case R_X86_64_16:
            printf("R_X86_64_16");
            break;

        case R_X86_64_PC16:
            printf("R_X86_64_PC16");
            break;

        case R_X86_64_8:
            printf("R_X86_64_8");
            break;

        case R_X86_64_PC8:
            printf("R_X86_64_PC8");
            break;

        case R_X86_64_DTPMOD64:
            printf("R_X86_64_DTPMOD64");
            break;

        case R_X86_64_DTPOFF64:
            printf("R_X86_64_DTPOFF64");
            break;

        case R_X86_64_TPOFF64:
            printf("R_X86_64_TPOFF64");
            break;

        case R_X86_64_TLSGD:
            printf("R_X86_64_TLSGD");
            break;

        case R_X86_64_TLSLD:
            printf("R_X86_64_TLSLD");
            break;

        case R_X86_64_DTPOFF32:
            printf("R_X86_64_DTPOFF32");
            break;

        case R_X86_64_GOTTPOFF:
            printf("R_X86_64_GOTTPOFF");
            break;

        case R_X86_64_TPOFF32:
            printf("R_X86_64_TPOFF32");
            break;

        case R_X86_64_PC64:
            printf("R_X86_64_PC64");
            break;

        case R_X86_64_GOTOFF64:
            printf("R_X86_64_GOTOFF64");
            break;

        case R_X86_64_GOTPC32:
            printf("R_X86_64_GOTPC32");
            break;

        case R_X86_64_GOT64:
            printf("R_X86_64_GOT64");
            break;

        case R_X86_64_GOTPCREL64:
            printf("R_X86_64_GOTPCREL64");
            break;

        case R_X86_64_GOTPC64:
            printf("R_X86_64_GOTPC64");
            break;

        case R_X86_64_GOTPLT64:
            printf("R_X86_64_GOTPLT64");
            break;

        case R_X86_64_PLTOFF64:
            printf("R_X86_64_PLTOFF64");
            break;

        case R_X86_64_SIZE32:
            printf("R_X86_64_SIZE32");
            break;

        case R_X86_64_SIZE64:
            printf("R_X86_64_SIZE64");
            break;

        case R_X86_64_GOTPC32_TLSDESC:
            printf("R_X86_64_GOTPC32_TLSDESC");
            break;

        case R_X86_64_TLSDESC_CALL:
            printf("R_X86_64_TLSDESC_CALL");
            break;

        case R_X86_64_TLSDESC:
            printf("R_X86_64_TLSDESC");
            break;

        case R_X86_64_IRELATIVE:
            printf("R_X86_64_IRELATIVE");
            break;

        case R_X86_64_RELATIVE64:
            printf("R_X86_64_RELATIVE64");
            break;

        case R_X86_64_GOTPCRELX:
            printf("R_X86_64_GOTPCRELX");
            break;      

        case R_X86_64_REX_GOTPCRELX:
            printf("R_X86_64_REX_GOTPCRELX");
            break;   

        case R_X86_64_NUM:
            printf("R_X86_64_NUM");
            break;   

        default:
            break;        
    }
    printf("\t");
}

void read_section_rel_a(FILE* file_pointer) //пока только для rela (для rel - аналогично)
{
    int number_of_rs = 0; 
    int index = 0;

    Elf64_Shdr section_headers[number_of_section_headers]; 
    fseek(file_pointer, start_of_section_headers, SEEK_SET);
    fread(section_headers, sizeof(Elf64_Shdr), number_of_section_headers, \
        file_pointer);
    fseek(file_pointer, \
        section_headers[section_header_string_table_index].sh_offset, \
        SEEK_SET);
    int size_of_section = section_headers[section_header_string_table_index].sh_size;

    char string_keeper[size_of_section];    
    fread(string_keeper, 1, size_of_section, file_pointer); 

    for (int i = 0; i < rela_index; i++)
    {   
        fseek(file_pointer, rela_offset[i], SEEK_SET);

        number_of_rs = rela_size[i]/sizeof(Elf64_Rela);  
        index = rela_index_in_section_headers[i];
        Elf64_Rela rela[number_of_rs];   

        fread(rela, sizeof(Elf64_Rela), number_of_rs, file_pointer); 

        if ( index > 0 && (section_headers[index].sh_type == SHT_RELA
                           || section_headers[index].sh_type == SHT_REL)) 
        {      
            printf("Раздел перемещения '%s' по смещению 0x%lx "
                "содержит %d элемент:\n", \
                &string_keeper[section_headers[index].sh_name],\
                rela_offset[i], number_of_rs);
            printf("  Смещение       Инфо           Тип"
            "                 Знач.симв.   Имя симв. + Addend\n");
        } 

        for (int j = 0; j < number_of_rs; j++)
        {              
            printf("%012lx ", rela[j].r_offset);
            printf("%012lx ", rela[j].r_info); 
            switch (relocs_arch) //only for AMD x86-64 (others define similarly)
            {
                case EM_X86_64:
                    reloc_x86_64(rela[j].r_info);
                    break;
                
                default:
                    break;
            }

            long unsigned int flag = rela[j].r_info >> 32;
            if (flag != 0x0) 
            {                
                printf("%016lx ", ELF64_R_SYM(rela[j].r_addend));
                read_symbol_name(file_pointer, rela, j); 
            }
            else if (flag >> 32 == 0x0)
            {                
                printf("                  ");
            }
            printf("%lx", rela[j].r_addend);
            
            printf("\n");
        }
        printf("\n");       
    }    
}

void read_section_dynsym(FILE* file_pointer)
{
    int number_of_ds = dynsym_size/sizeof(Elf64_Sym);
    Elf64_Sym dynsym[number_of_ds];       
    fseek(file_pointer, dynsym_offset, SEEK_SET);
    fread(dynsym, sizeof(Elf64_Sym), number_of_ds, file_pointer); 
    char dynsym_string_keeper[dynsym_size];
    fread (dynsym_string_keeper, 1, dynsym_size, file_pointer);
    

    printf("Таблица символов \".dynsym\" содержит %d элементов:\n", number_of_ds);
    printf("   Чис:    Знач           Разм Тип      Связ    Vis     Индекс имени\n");

    for (int j = 0; j < number_of_ds; j++)
    {
        printf("     %2d: ", j);
        printf("%016lx ", dynsym[j].st_value);
        printf("%4ld ", dynsym[j].st_size); 
        switch (ELF64_ST_TYPE(dynsym[j].st_info))
        {
            case STT_NOTYPE:
                printf("NOTYPE\t");
                break;

            case STT_OBJECT:
                printf("OBJECT\t");
                break;

            case STT_FUNC:
                printf("FUNC\t");
                break;

            case STT_SECTION:
                printf("SECTION\t");
                break;

            case STT_FILE:
                printf("FILE\t");
                break;

            case STT_COMMON:
                printf("COMMON\t");
                break;

            case STT_TLS:
                printf("TLS\t");
                break;

            case STT_NUM:
                printf("NUM\t");
                break;

            case STT_LOOS:
                printf("LOOS\t");
                break;

            //значение = STT_LOOS
            // case STT_GNU_IFUNC:
            //     printf("GNU_IFUNC\t");
            //     break;

            case STT_HIOS:
                printf("HIOS\t");
                break;

            case STT_LOPROC:
                printf("LOPROC\t");
                break;

            case STT_HIPROC:
                printf("HIPROC\t");
                break;

            default:
                printf("Не опр.");
                break;
        }
        switch (ELF64_ST_BIND(dynsym[j].st_info))
        {
            case STB_LOCAL:
                printf("LOCAL\t");
                break;

            case STB_GLOBAL:
                printf("GLOBAL\t");
                break;

            case STB_WEAK:
                printf("WEAK\t");
                break;

            case STB_NUM:
                printf("NUM\t");
                break;

            case STB_LOOS:
                printf("LOOS\t");
                break;

            //значение = STB_LOOS
            // case STB_GNU_UNIQUE:
            //     printf("GNU_UNIQUE\t");
            //     break;

            case STB_HIOS:
                printf("HIOS\t");
                break;

            case STB_LOPROC:
                printf("LOPROC\t");
                break;

            case STB_HIPROC:
                printf("HIPROC\t");
                break;

            default:
                printf("Не опр.\t");
                break;
        }
        switch (ELF64_ST_VISIBILITY(dynsym[j].st_other))
        {
            case STV_DEFAULT:
                printf("DEFAULT\t");
                break;

            case STV_INTERNAL:
                printf("INTERNAL\t");
                break;

            case STV_HIDDEN:
                printf("HIDDEN\t");
                break;

            case STV_PROTECTED:
                printf("PROTECTED\t");
                break;

            default:
                printf("Не опр.\t");
                break;
        }
        printf("%s ", &dynsym_string_keeper[dynsym[j].st_name]);  

        printf("\n"); 
        fread(dynsym, sizeof(Elf64_Sym), 1, file_pointer);       
    }
    printf("\n");   
}

void read_section_symtab(FILE* file_pointer)
{
    int number_of_ds = symtab_size/sizeof(Elf64_Sym);
    Elf64_Sym symtab[number_of_ds];       
    fseek(file_pointer, symtab_offset, SEEK_SET);
    fread(symtab, sizeof(Elf64_Sym), number_of_ds, file_pointer); 
    char symtab_string_keeper[symtab_size];
    fread(symtab_string_keeper, 1, symtab_size, file_pointer);

    printf("Таблица символов \".symtab\" содержит %d элементов:\n", number_of_ds);
    printf("   Чис:    Знач           Разм Тип      Связ    Vis     Индекс имени\n");

    for (int j = 0; j < number_of_ds; j++)
    {
        printf("     %2d: ", j);
        printf("%016lx ", symtab[j].st_value);
        printf("%4ld ", symtab[j].st_size); 
        switch (ELF64_ST_TYPE(symtab[j].st_info))
        {
            case STT_NOTYPE:
                printf("NOTYPE\t");
                break;

            case STT_OBJECT:
                printf("OBJECT\t");
                break;

            case STT_FUNC:
                printf("FUNC\t");
                break;

            case STT_SECTION:
                printf("SECTION\t");
                break;

            case STT_FILE:
                printf("FILE\t");
                break;

            case STT_COMMON:
                printf("COMMON\t");
                break;

            case STT_TLS:
                printf("TLS\t");
                break;

            case STT_NUM:
                printf("NUM\t");
                break;

            case STT_LOOS:
                printf("LOOS\t");
                break;

            //значение = STT_LOOS
            // case STT_GNU_IFUNC:
            //     printf("GNU_IFUNC\t");
            //     break;

            case STT_HIOS:
                printf("HIOS\t");
                break;

            case STT_LOPROC:
                printf("LOPROC\t");
                break;

            case STT_HIPROC:
                printf("HIPROC\t");
                break;

            default:
                printf("Не опр.");
                break;
        }
        switch (ELF64_ST_BIND(symtab[j].st_info))
        {
            case STB_LOCAL:
                printf("LOCAL\t");
                break;

            case STB_GLOBAL:
                printf("GLOBAL\t");
                break;

            case STB_WEAK:
                printf("WEAK\t");
                break;

            case STB_NUM:
                printf("NUM\t");
                break;

            case STB_LOOS:
                printf("LOOS\t");
                break;

            //значение = STB_LOOS
            // case STB_GNU_UNIQUE:
            //     printf("GNU_UNIQUE\t");
            //     break;

            case STB_HIOS:
                printf("HIOS\t");
                break;

            case STB_LOPROC:
                printf("LOPROC\t");
                break;

            case STB_HIPROC:
                printf("HIPROC\t");
                break;

            default:
                printf("Не опр.\t");
                break;
        }
        switch (ELF64_ST_VISIBILITY(symtab[j].st_other))
        {
            case STV_DEFAULT:
                printf("DEFAULT\t");
                break;

            case STV_INTERNAL:
                printf("INTERNAL\t");
                break;

            case STV_HIDDEN:
                printf("HIDDEN\t");
                break;

            case STV_PROTECTED:
                printf("PROTECTED\t");
                break;

            default:
                printf("Не опр.\t");
                break;
        }
        printf("%s ", &symtab_string_keeper[symtab[j].st_name]);  

        printf("\n"); 
        fread(symtab, sizeof(Elf64_Sym), 1, file_pointer);       
    }
    printf("\n");  
}

void read_section_gnu_vernaux(FILE* file_pointer, Elf64_Word offset, Elf64_Half num)
{
    Elf64_Vernaux vernaux[num];
    fseek(file_pointer, offset, SEEK_SET);
    fread(vernaux, sizeof(Elf64_Vernaux), 1, file_pointer);

    // fseek(file_pointer, offset, SEEK_SET); 
    // char vernaux_string_keeper[sizeof(Elf64_Vernaux)*num];
    // fread(vernaux_string_keeper, sizeof(Elf64_Vernaux)*num, 1, file_pointer);    

    for (int i = 0; i < num; i++)
    {
        printf("  0x%04x:   ", offset);
        offset = vernaux[i].vna_next;
        //printf("Имя: %s ", &vernaux_string_keeper[vernaux[i].vna_name]);
        printf("Флаги: ");
        switch (vernaux[i].vna_flags)
        {
            case VER_FLG_BASE:
                printf("BASE ");
                break;

            case VER_FLG_WEAK:
                printf("WEAK ");
                break;

            default:
                printf("нет ");
                break;
        }        
        printf("Версия: \n");
    }
}

void read_file_name_verneed(FILE* file_pointer, Elf64_Verneed *verneed, int i)
{
    printf("Файл: ");
    fseek(file_pointer, dynstr_offset + verneed[i].vn_file + 1, SEEK_SET);
    while (fgetc(file_pointer) != '\0')
    {
        printf("%c", fgetc(file_pointer));
    }
}

void read_section_gnu_version(FILE* file_pointer)
{
    Elf64_Verneed verneed[gnu_verneed_num];    
    fseek(file_pointer, gnu_verneed_offset, SEEK_SET);
    fread(verneed, sizeof(Elf64_Verneed), gnu_verneed_num, file_pointer); 

    fseek(file_pointer, gnu_verneed_offset, SEEK_SET); 
    char verneed_string_keeper[gnu_verneed_size];
    fread(verneed_string_keeper, gnu_verneed_size, 1, file_pointer);  

    printf("Раздел Version needs \".gnu.version_r\", содержащий %d элементов:\n",
            gnu_verneed_num);
    printf(" Адрес: 0x%016lx ", gnu_verneed_offset);
    printf(" Смещение: 0x%06lx ", gnu_verneed_offset);
    printf(" Ссылка: \n");

    for (int i = 0; i < gnu_verneed_num; i++)
    {
        printf("  %06x: ", verneed[i].vn_next);
        printf("Версия: %d ", verneed[i].vn_version); 
        read_file_name_verneed(file_pointer, verneed, i);               
        //printf("Файл: %s ", &verneed_string_keeper[verneed[i].vn_file]);
        printf(" Счетчик: %x\n", verneed[i].vn_cnt);

        read_section_gnu_vernaux(file_pointer, verneed[i].vn_aux, verneed[i].vn_cnt);       
    }
    printf("\n");  
}

// void read_strtab(FILE* file_pointer)
// {
//     printf("strtab:\n");
//     fseek(file_pointer, dynstr_offset, SEEK_SET);
            
    
//     printf("\nend of strtab\n");
// }

int main()
{   
    char filename[] = "example";
    FILE* file_pointer;
    file_pointer = fopen(filename, "r");
    if (!file_pointer)
    {
        perror ("Невозможно открыть ELF-файл");
        exit(1);
    }
    
    read_header(file_pointer);
    read_sections(file_pointer);
    read_segments(file_pointer);

    read_section_dynamic(file_pointer);
    read_section_rel_a(file_pointer);
    //read_section_dynsym(file_pointer);
    //read_section_symtab(file_pointer);
    //read_section_gnu_version(file_pointer);

    //read_strtab(file_pointer);

    fclose(file_pointer);
    return 0;
}