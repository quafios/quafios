/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 1.0.2.                               | |
 *        | |  -> ELF header.                                      | |
 *        | +------------------------------------------------------+ |
 *        +----------------------------------------------------------+
 *
 * This file is part of Quafios 1.0.2 source code.
 * Copyright (C) 2014  Mostafa Abd El-Aziz Mohamed.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Quafios.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Visit http://www.quafios.com/ for contact information.
 *
 */

#ifndef ELF_H
#define ELF_H

#include <arch/type.h>

typedef uint32_t Elf32_Addr;
typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Off;
typedef  int32_t Elf32_Sword;
typedef uint32_t Elf32_Word;

#define EI_NIDENT 16

typedef struct {
    /* Identification Indexes: */
    #define     EI_MAG0         0
    #define     EI_MAG1         1
    #define     EI_MAG2         2
    #define     EI_MAG3         3
    #define     EI_CLASS        4
    /* ELF Classes: */
    #define     ELFCLASSNONE    0
    #define     ELFCLASS32      1
    #define     ELFCLASS64      2
    #define     EI_DATA         5
    /* Data Encoding: */
    #define     ELFDATANONE     0
    #define     ELFDATA2LSB     1
    #define     ELFDATA2MSB     2
    #define     EI_VERSION      6
    #define     EI_PAD          7
    uint8_t e_ident[EI_NIDENT];

    /* Object File Type: */
    #define     ET_NONE         0
    #define     ET_REL          1
    #define     ET_EXEC         2
    #define     ET_DYN          3
    #define     ET_CORE         4
    #define     ET_LOPROC       0xFF00
    #define     ET_HIPROC       0xFFFF
    Elf32_Half    e_type;

    /* Machine Architectures: */
    #define     EM_NONE         0
    #define     EM_M32          1
    #define     EM_SPARC        2
    #define     EM_386          3
    #define     EM_68K          4
    #define     EM_88K          5
    #define     EM_860          7
    #define     EM_MIPS         8
    #define     EM_MIPS_RS4_BE  10
    Elf32_Half    e_machine;

    Elf32_Word    e_version;
    Elf32_Addr    e_entry;

    Elf32_Off     e_phoff;
    Elf32_Off     e_shoff;

    Elf32_Word    e_flags;

    Elf32_Half    e_ehsize;

    Elf32_Half    e_phentsize;
    Elf32_Half    e_phnum;

    Elf32_Half    e_shentsize;
    Elf32_Half    e_shnum;

    Elf32_Half    e_shstrndx;
} __attribute__ ((packed)) Elf32_Ehdr;


/* Special Section Header Indexes: */
#define SHN_UNDEF       0
#define SHN_LORESERVE   0xff00
#define SHN_LOPROC      0xff00
#define SHN_HIPROC      0xff1f
#define SHN_ABS         0xfff1
#define SHN_COMMON      0xfff2
#define SHN_HIRESERVE   0xffff

typedef struct {
    Elf32_Word  sh_name;

    /* Section Types: */
    #define SHT_NULL            0
    #define SHT_PROGBITS        1
    #define SHT_SYMTAB          2
    #define SHT_STRTAB          3
    #define SHT_RELA            4
    #define SHT_HASH            5
    #define SHT_DYNAMIC         6
    #define SHT_NOTE            7
    #define SHT_NOBITS          8
    #define SHT_REL             9
    #define SHT_SHLIB           10
    #define SHT_DYNSYM          11
    #define SHT_LOPROC          0x70000000
    #define SHT_HIPROC          0x7fffffff
    #define SHT_LOUSER          0x80000000
    #define SHT_HIUSER          0xffffffff
    Elf32_Word  sh_type;

    /* Attribute Flags: */
    #define SHF_WRITE           0x1
    #define SHF_ALLOC           0x2
    #define SHF_EXECINSTR       0x4
    #define SHF_MASKPROC        0xf0000000
    Elf32_Word  sh_flags;

    Elf32_Addr  sh_addr;
    Elf32_Off   sh_offset;

    Elf32_Word  sh_size;
    Elf32_Word  sh_link;
    Elf32_Word  sh_info;
    Elf32_Word  sh_addralign;

    Elf32_Word  sh_entsize;
} Elf32_Shdr;


typedef struct {
    Elf32_Word    st_name;
    Elf32_Addr    st_value;
    Elf32_Word    st_size;

    #define ELF32_ST_BIND(i)    ((i)>>4)
    /* Symbol Binding: */
    #define STB_LOCAL           0
    #define STB_GLOBAL          1
    #define STB_WEAK            2
    #define STB_LOPROC          13
    #define STB_HIPROC          15
    #define ELF32_ST_TYPE(i)    ((i)&0xf)
    /* Symbol Type: */
    #define STT_NOTYPE          0
    #define STT_OBJECT          1
    #define STT_FUNC            2
    #define STT_SECTION         3
    #define STT_FILE            4
    #define STT_LOPROC          13
    #define STT_HIPROC          15
    #define ELF32_ST_INFO(b,t)  (((b)<<4)+((t)&0xf))
    uint8_t st_info;

    uint8_t st_other;
    Elf32_Half    st_shndx;
} Elf32_Sym;


typedef struct {

    #define PT_NULL             0
    #define PT_LOAD             1
    #define PT_DYNAMIC          2
    #define PT_INTERP           3
    #define PT_NOTE             4
    #define PT_SHLIB            5
    #define PT_PHDR             6
    #define PT_LOPROC           0x70000000
    #define PT_HIPROC           0x7fffffff
    Elf32_Word p_type;

    Elf32_Off  p_offset;
    Elf32_Addr p_vaddr;

    Elf32_Addr p_paddr;

    Elf32_Word p_filesz;
    Elf32_Word p_memsz;

    #define PF_X                0x1
    #define PF_W                0x2
    #define PF_R                0x4
    #define PF_MASKPROC         0xf0000000
    Elf32_Word p_flags;

    Elf32_Word p_align;
} Elf32_Phdr;

#endif
