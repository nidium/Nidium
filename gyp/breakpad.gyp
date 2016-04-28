# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This is a modified version of 
# https://chromium.googlesource.com/chromium/chromium/+/trunk/breakpad/breakpad.gyp 
# Revision : 5df1e5f3f458ae9b228a399545a4610fa4010684
# Removed : 
# - iOS without ninja generator
# - All tests target
# - 

{
    'conditions': [
        # minidump_stackwalk and minidump_dump are tool-type executables that do
        # not build on iOS.
        ['OS!="ios" and OS!="win"', {
            'targets': [
                {
                    'target_name': 'minidump_stackwalk',
                    'type': 'executable',
                    'product_dir': '../tools/',
                    'includes': ['breakpad_tools.gypi'],
                    'xcode_settings': {
                        "OTHER_LDFLAGS": [
                            '-stdlib=libc++'
                        ]
                    },
                    'defines': ['BPLOG_MINIMUM_SEVERITY=SEVERITY_ERROR'],
                    'sources': [
                        '<(third_party_path)/breakpad/src/processor/basic_code_module.h',
                        '<(third_party_path)/breakpad/src/processor/basic_code_modules.cc',
                        '<(third_party_path)/breakpad/src/processor/basic_code_modules.h',
                        '<(third_party_path)/breakpad/src/processor/basic_source_line_resolver.cc',
                        '<(third_party_path)/breakpad/src/processor/binarystream.cc',
                        '<(third_party_path)/breakpad/src/processor/binarystream.h',
                        '<(third_party_path)/breakpad/src/processor/call_stack.cc',
                        '<(third_party_path)/breakpad/src/processor/cfi_frame_info.cc',
                        '<(third_party_path)/breakpad/src/processor/cfi_frame_info.h',
                        '<(third_party_path)/breakpad/src/processor/disassembler_x86.cc',
                        '<(third_party_path)/breakpad/src/processor/disassembler_x86.h',
                        '<(third_party_path)/breakpad/src/processor/exploitability.cc',
                        '<(third_party_path)/breakpad/src/processor/exploitability_linux.cc',
                        '<(third_party_path)/breakpad/src/processor/exploitability_linux.h',
                        '<(third_party_path)/breakpad/src/processor/exploitability_win.cc',
                        '<(third_party_path)/breakpad/src/processor/exploitability_win.h',
                        '<(third_party_path)/breakpad/src/processor/logging.cc',
                        '<(third_party_path)/breakpad/src/processor/logging.h',
                        '<(third_party_path)/breakpad/src/processor/minidump.cc',
                        '<(third_party_path)/breakpad/src/processor/minidump_processor.cc',
                        '<(third_party_path)/breakpad/src/processor/minidump_stackwalk.cc',
                        '<(third_party_path)/breakpad/src/processor/pathname_stripper.cc',
                        '<(third_party_path)/breakpad/src/processor/pathname_stripper.h',
                        '<(third_party_path)/breakpad/src/processor/process_state.cc',
                        '<(third_party_path)/breakpad/src/processor/simple_symbol_supplier.cc',
                        '<(third_party_path)/breakpad/src/processor/simple_symbol_supplier.h',
                        '<(third_party_path)/breakpad/src/processor/source_line_resolver_base.cc',
                        '<(third_party_path)/breakpad/src/processor/stack_frame_symbolizer.cc',
                        '<(third_party_path)/breakpad/src/processor/stackwalker.cc',
                        '<(third_party_path)/breakpad/src/processor/stackwalker_amd64.cc',
                        '<(third_party_path)/breakpad/src/processor/stackwalker_amd64.h',
                        '<(third_party_path)/breakpad/src/processor/stackwalker_arm.cc',
                        '<(third_party_path)/breakpad/src/processor/stackwalker_arm.h',
                        '<(third_party_path)/breakpad/src/processor/stackwalker_mips.cc',
                        '<(third_party_path)/breakpad/src/processor/stackwalker_mips.h',
                        '<(third_party_path)/breakpad/src/processor/stackwalker_ppc.cc',
                        '<(third_party_path)/breakpad/src/processor/stackwalker_ppc.h',
                        '<(third_party_path)/breakpad/src/processor/stackwalker_ppc64.cc',
                        '<(third_party_path)/breakpad/src/processor/stackwalker_ppc64.h',
                        '<(third_party_path)/breakpad/src/processor/stackwalker_sparc.cc',
                        '<(third_party_path)/breakpad/src/processor/stackwalker_sparc.h',
                        '<(third_party_path)/breakpad/src/processor/stackwalker_x86.cc',
                        '<(third_party_path)/breakpad/src/processor/stackwalker_x86.h',
                        '<(third_party_path)/breakpad/src/processor/tokenize.cc',
                        '<(third_party_path)/breakpad/src/processor/tokenize.h',
                        # libdisasm
                        '<(third_party_path)/breakpad/src/third_party/libdisasm/ia32_implicit.c',
                        '<(third_party_path)/breakpad/src/third_party/libdisasm/ia32_implicit.h',
                        '<(third_party_path)/breakpad/src/third_party/libdisasm/ia32_insn.c',
                        '<(third_party_path)/breakpad/src/third_party/libdisasm/ia32_insn.h',
                        '<(third_party_path)/breakpad/src/third_party/libdisasm/ia32_invariant.c',
                        '<(third_party_path)/breakpad/src/third_party/libdisasm/ia32_invariant.h',
                        '<(third_party_path)/breakpad/src/third_party/libdisasm/ia32_modrm.c',
                        '<(third_party_path)/breakpad/src/third_party/libdisasm/ia32_modrm.h',
                        '<(third_party_path)/breakpad/src/third_party/libdisasm/ia32_opcode_tables.c',
                        '<(third_party_path)/breakpad/src/third_party/libdisasm/ia32_opcode_tables.h',
                        '<(third_party_path)/breakpad/src/third_party/libdisasm/ia32_operand.c',
                        '<(third_party_path)/breakpad/src/third_party/libdisasm/ia32_operand.h',
                        '<(third_party_path)/breakpad/src/third_party/libdisasm/ia32_reg.c',
                        '<(third_party_path)/breakpad/src/third_party/libdisasm/ia32_reg.h',
                        '<(third_party_path)/breakpad/src/third_party/libdisasm/ia32_settings.c',
                        '<(third_party_path)/breakpad/src/third_party/libdisasm/ia32_settings.h',
                        '<(third_party_path)/breakpad/src/third_party/libdisasm/libdis.h',
                        '<(third_party_path)/breakpad/src/third_party/libdisasm/qword.h',
                        '<(third_party_path)/breakpad/src/third_party/libdisasm/x86_disasm.c',
                        '<(third_party_path)/breakpad/src/third_party/libdisasm/x86_format.c',
                        '<(third_party_path)/breakpad/src/third_party/libdisasm/x86_imm.c',
                        '<(third_party_path)/breakpad/src/third_party/libdisasm/x86_imm.h',
                        '<(third_party_path)/breakpad/src/third_party/libdisasm/x86_insn.c',
                        '<(third_party_path)/breakpad/src/third_party/libdisasm/x86_misc.c',
                        '<(third_party_path)/breakpad/src/third_party/libdisasm/x86_operand_list.c',
                        '<(third_party_path)/breakpad/src/third_party/libdisasm/x86_operand_list.h',
                    ],
                },
                {
                    'target_name': 'minidump_dump',
                    'type': 'executable',
                    'product_dir': '../tools/',
                    'includes': ['breakpad_tools.gypi'],
                    'xcode_settings': {
                        "OTHER_LDFLAGS": [
                            '-stdlib=libc++'
                        ]
                    },
                    'sources': [
                        '<(third_party_path)/breakpad/src/processor/basic_code_module.h',
                        '<(third_party_path)/breakpad/src/processor/basic_code_modules.cc',
                        '<(third_party_path)/breakpad/src/processor/basic_code_modules.h',
                        '<(third_party_path)/breakpad/src/processor/logging.cc',
                        '<(third_party_path)/breakpad/src/processor/logging.h',
                        '<(third_party_path)/breakpad/src/processor/minidump.cc',
                        '<(third_party_path)/breakpad/src/processor/minidump_dump.cc',
                        '<(third_party_path)/breakpad/src/processor/pathname_stripper.cc',
                        '<(third_party_path)/breakpad/src/processor/pathname_stripper.h',
                    ],
                },
            ],
        }],
        ['OS=="mac" or (OS=="ios" and "<(GENERATOR)"=="ninja")', {
            'target_defaults': {
                'include_dirs': [
                    '<(third_party_path)/breakpad/src/',
                ],
            },
            'targets': [
                {
                    'target_name': 'dump_syms',
                    'type': 'executable',
                    'product_dir': '../tools/',
                    'toolsets': ['host'],
                    'include_dirs': [
                        '<(third_party_path)/breakpad/src/common/mac',
                    ],
                    'sources': [
                        '<(third_party_path)/breakpad/src/common/dwarf/bytereader.cc',
                        '<(third_party_path)/breakpad/src/common/dwarf_cfi_to_module.cc',
                        '<(third_party_path)/breakpad/src/common/dwarf_cu_to_module.cc',
                        '<(third_party_path)/breakpad/src/common/dwarf/dwarf2diehandler.cc',
                        '<(third_party_path)/breakpad/src/common/dwarf/dwarf2reader.cc',
                        '<(third_party_path)/breakpad/src/common/dwarf_line_to_module.cc',
                        '<(third_party_path)/breakpad/src/common/language.cc',
                        '<(third_party_path)/breakpad/src/common/mac/arch_utilities.cc',
                        '<(third_party_path)/breakpad/src/common/mac/arch_utilities.h',
                        '<(third_party_path)/breakpad/src/common/mac/dump_syms.mm',
                        '<(third_party_path)/breakpad/src/common/mac/file_id.cc',
                        '<(third_party_path)/breakpad/src/common/mac/macho_id.cc',
                        '<(third_party_path)/breakpad/src/common/mac/macho_reader.cc',
                        '<(third_party_path)/breakpad/src/common/mac/macho_utilities.cc',
                        '<(third_party_path)/breakpad/src/common/mac/macho_walker.cc',
                        '<(third_party_path)/breakpad/src/common/md5.cc',
                        '<(third_party_path)/breakpad/src/common/module.cc',
                        '<(third_party_path)/breakpad/src/common/stabs_reader.cc',
                        '<(third_party_path)/breakpad/src/common/stabs_to_module.cc',
                        '<(third_party_path)/breakpad/src/tools/mac/dump_syms/dump_syms_tool.mm',
                    ],
                    'defines': [
                        # For src/common/stabs_reader.h.
                        'HAVE_MACH_O_NLIST_H',
                    ],
                    'xcode_settings': {
                        # Like ld, dump_syms needs to operate on enough data that it may
                        # actually need to be able to address more than 4GB. Use x86_64.
                        # Don't worry! An x86_64 dump_syms is perfectly able to dump
                        # 32-bit files.
                        'ARCHS': [
                            'x86_64',
                        ],

                        # The DWARF utilities require -funsigned-char.
                        'GCC_CHAR_IS_UNSIGNED_CHAR': 'YES',

                        # dwarf2reader.cc uses dynamic_cast.
                        'GCC_ENABLE_CPP_RTTI': 'YES',
                        "OTHER_LDFLAGS": [
                            '-stdlib=libc++'
                        ]
                    },
                    'link_settings': {
                        'libraries': [
                            '$(SDKROOT)/System/Library/Frameworks/Foundation.framework',
                        ],
                    },
                    'configurations': {
                        'Release': {
                            'xcode_settings': {
                                # dump_syms crashes when built at -O1, -O2, and -O3.    It does
                                # not crash at -Os.    To play it safe, dump_syms is always built
                                # at -O0 until this can be sorted out.
                                # http:/code.google.com/p/google-breakpad/issues/detail?id=329
                                'GCC_OPTIMIZATION_LEVEL': '0',    # -O0
                             },
                         },
                    },
                },
            ],
        }],
        ['OS=="mac"', {
            'target_defaults': {
                'include_dirs': [
                    '<(third_party_path)/breakpad/src/',
                ],
            },
            'targets': [
                {
                    'target_name': 'breakpad_utilities',
                    'type': 'static_library',
                    'sources': [
                        '<(third_party_path)/breakpad/src/client/mac/handler/breakpad_nlist_64.cc',
                        '<(third_party_path)/breakpad/src/client/mac/handler/dynamic_images.cc',
                        '<(third_party_path)/breakpad/src/client/mac/handler/minidump_generator.cc',
                        '<(third_party_path)/breakpad/src/client/minidump_file_writer.cc',
                        '<(third_party_path)/breakpad/src/common/convert_UTF.c',
                        '<(third_party_path)/breakpad/src/common/mac/MachIPC.mm',
                        '<(third_party_path)/breakpad/src/common/mac/arch_utilities.cc',
                        '<(third_party_path)/breakpad/src/common/mac/bootstrap_compat.cc',
                        '<(third_party_path)/breakpad/src/common/mac/file_id.cc',
                        '<(third_party_path)/breakpad/src/common/mac/macho_id.cc',
                        '<(third_party_path)/breakpad/src/common/mac/macho_utilities.cc',
                        '<(third_party_path)/breakpad/src/common/mac/macho_walker.cc',
                        '<(third_party_path)/breakpad/src/common/mac/string_utilities.cc',
                        '<(third_party_path)/breakpad/src/common/md5.cc',
                        '<(third_party_path)/breakpad/src/common/simple_string_dictionary.cc',
                        '<(third_party_path)/breakpad/src/common/string_conversion.cc',
                    ],
                },
                {
                    'target_name': 'crash_inspector',
                    'type': 'executable',
                    'product_dir': '<(nidium_resources_path)/osx/',
                    'variables': {
                        'mac_real_dsym': 1,
                    },
                    'dependencies': [
                        'breakpad_utilities',
                    ],
                    'include_dirs': [
                        '<(third_party_path)/breakpad/src/client/apple/Framework',
                        '<(third_party_path)/breakpad/src/common/mac',
                    ],
                    'sources': [
                        '<(third_party_path)/breakpad/src/client/mac/crash_generation/ConfigFile.mm',
                        '<(third_party_path)/breakpad/src/client/mac/crash_generation/Inspector.mm',
                        '<(third_party_path)/breakpad/src/client/mac/crash_generation/InspectorMain.mm',
                        '<(third_party_path)/breakpad/src/common/mac/bootstrap_compat.cc',
                    ],
                    'link_settings': {
                        'libraries': [
                            '$(SDKROOT)/System/Library/Frameworks/CoreServices.framework',
                            '$(SDKROOT)/System/Library/Frameworks/Foundation.framework',
                        ],
                    },
                    'xcode_settings': {
                        "OTHER_LDFLAGS": [
                            '-stdlib=libc++'
                        ]
                    },
                },
                {
                    'target_name': 'crash_report_sender',
                    'type': 'executable',
                    'product_dir': '<(nidium_resources_path)/osx/',
                    'mac_bundle': 1,
                    'variables': {
                        'mac_real_dsym': 1,
                    },
                    'include_dirs': [
                        '<(third_party_path)/breakpad/src/common/mac',
                    ],
                    'sources': [
                        '<(third_party_path)/breakpad/src/common/mac/HTTPMultipartUpload.m',
                        '<(third_party_path)/breakpad/src/client/mac/sender/crash_report_sender.m',
                        '<(third_party_path)/breakpad/src/client/mac/sender/uploader.mm',
                        '<(third_party_path)/breakpad/src/common/mac/GTMLogger.m',
                    ],
                    'mac_bundle_resources': [
                        '<(third_party_path)/breakpad/src/client/mac/sender/English.lproj/Localizable.strings',
                        '<(third_party_path)/breakpad/src/client/mac/sender/crash_report_sender.icns',
                        '<(third_party_path)/breakpad/src/client/mac/sender/Breakpad.xib',
                        '<(third_party_path)/breakpad/src/client/mac/sender/crash_report_sender-Info.plist',
                    ],
                    'mac_bundle_resources!': [
                         '<(third_party_path)/breakpad/src/client/mac/sender/crash_report_sender-Info.plist',
                    ],
                    'xcode_settings': {
                         'INFOPLIST_FILE': '<(third_party_path)/breakpad/src/client/mac/sender/crash_report_sender-Info.plist',
                    },
                    'link_settings': {
                        'libraries': [
                            '$(SDKROOT)/System/Library/Frameworks/AppKit.framework',
                            '$(SDKROOT)/System/Library/Frameworks/Foundation.framework',
                            '$(SDKROOT)/System/Library/Frameworks/SystemConfiguration.framework',
                        ],
                    }
                },
                {
                    'target_name': 'breakpad',
                    'type': 'static_library',
                    'dependencies': [
                        'breakpad_utilities',
                        'crash_inspector',
                        'crash_report_sender',
                    ],
                    'include_dirs': [
                        '<(third_party_path)/breakpad/src/client/apple/Framework',
                    ],
                    'direct_dependent_settings': {
                        'include_dirs': [
                            '<(third_party_path)/breakpad/src/client/apple/Framework',
                        ],
                    },
                    'defines': [
                        'USE_PROTECTED_ALLOCATIONS=1',
                    ],
                    'sources': [
                        '<(third_party_path)/breakpad/src/client/mac/crash_generation/crash_generation_client.cc',
                        '<(third_party_path)/breakpad/src/client/mac/crash_generation/crash_generation_client.h',
                        '<(third_party_path)/breakpad/src/client/mac/handler/protected_memory_allocator.cc',
                        '<(third_party_path)/breakpad/src/client/mac/handler/exception_handler.cc',
                        '<(third_party_path)/breakpad/src/client/mac/Framework/Breakpad.mm',
                        '<(third_party_path)/breakpad/src/client/mac/Framework/OnDemandServer.mm',
                    ],
                },
            ],
        }],
        [ 'OS=="linux" or OS=="android"', {
            'conditions': [
                ['OS=="android"', {
                    'defines': [
                        '__ANDROID__',
                    ],
                }],
            ],
            # Tools needed for archiving build symbols.
            'targets': [
                {
                    'target_name': 'symupload',
                    'type': 'executable',

                    'includes': ['breakpad_tools.gypi'],

                    'sources': [
                        '<(third_party_path)/breakpad/src/tools/linux/symupload/sym_upload.cc',
                        '<(third_party_path)/breakpad/src/common/linux/http_upload.cc',
                        '<(third_party_path)/breakpad/src/common/linux/http_upload.h',
                    ],
                    'include_dirs': [
                        '<(third_party_path)/breakpad/src/',
                        '<(third_party_path)/breakpad/src/third_party',
                    ],
                    'link_settings': {
                        'libraries': [
                            '-ldl',
                        ],
                    },
                },
                {
                    'target_name': 'dump_syms',
                    'type': 'executable',
                    'product_dir': '../tools/',
                    'conditions': [
                        ['OS=="android"', {
                            'toolsets': [ 'host' ],
                        }],
                    ],

                    # dwarf2reader.cc uses dynamic_cast. Because we don't typically
                    # don't support RTTI, we enable it for this single target. Since
                    # dump_syms doesn't share any object files with anything else,
                    # this doesn't end up polluting Chrome itself.
                    'cflags_cc!': ['-fno-rtti'],

                    'sources': [
                        '<(third_party_path)/breakpad/src/common/dwarf/bytereader.cc',
                        '<(third_party_path)/breakpad/src/common/dwarf_cfi_to_module.cc',
                        '<(third_party_path)/breakpad/src/common/dwarf_cfi_to_module.h',
                        '<(third_party_path)/breakpad/src/common/dwarf_cu_to_module.cc',
                        '<(third_party_path)/breakpad/src/common/dwarf_cu_to_module.h',
                        '<(third_party_path)/breakpad/src/common/dwarf/dwarf2diehandler.cc',
                        '<(third_party_path)/breakpad/src/common/dwarf/dwarf2reader.cc',
                        '<(third_party_path)/breakpad/src/common/dwarf_line_to_module.cc',
                        '<(third_party_path)/breakpad/src/common/dwarf_line_to_module.h',
                        '<(third_party_path)/breakpad/src/common/language.cc',
                        '<(third_party_path)/breakpad/src/common/language.h',
                        '<(third_party_path)/breakpad/src/common/linux/dump_symbols.cc',
                        '<(third_party_path)/breakpad/src/common/linux/dump_symbols.h',
                        '<(third_party_path)/breakpad/src/common/linux/elf_symbols_to_module.cc',
                        '<(third_party_path)/breakpad/src/common/linux/elf_symbols_to_module.h',
                        '<(third_party_path)/breakpad/src/common/linux/elfutils.cc',
                        '<(third_party_path)/breakpad/src/common/linux/elfutils.h',
                        '<(third_party_path)/breakpad/src/common/linux/file_id.cc',
                        '<(third_party_path)/breakpad/src/common/linux/file_id.h',
                        '<(third_party_path)/breakpad/src/common/linux/linux_libc_support.cc',
                        '<(third_party_path)/breakpad/src/common/linux/linux_libc_support.h',
                        '<(third_party_path)/breakpad/src/common/linux/memory_mapped_file.cc',
                        '<(third_party_path)/breakpad/src/common/linux/memory_mapped_file.h',
                        '<(third_party_path)/breakpad/src/common/linux/guid_creator.h',
                        '<(third_party_path)/breakpad/src/common/module.cc',
                        '<(third_party_path)/breakpad/src/common/module.h',
                        '<(third_party_path)/breakpad/src/common/stabs_reader.cc',
                        '<(third_party_path)/breakpad/src/common/stabs_reader.h',
                        '<(third_party_path)/breakpad/src/common/stabs_to_module.cc',
                        '<(third_party_path)/breakpad/src/common/stabs_to_module.h',
                        '<(third_party_path)/breakpad/src/tools/linux/dump_syms/dump_syms.cc',
                    ],

                    # Breakpad rev 583 introduced this flag.
                    # Using this define, stabs_reader.h will include a.out.h to
                    # build on Linux.
                    'defines': [
                        'HAVE_A_OUT_H',
                    ],

                    'include_dirs': [
                        '<(third_party_path)/breakpad/src/',
                    ],
                },
                {
                    'target_name': 'breakpad_client',
                    'type': 'static_library',

                    'sources': [
                        '<(third_party_path)/breakpad/src/client/linux/crash_generation/crash_generation_client.cc',
                        '<(third_party_path)/breakpad/src/client/linux/crash_generation/crash_generation_client.h',
                        '<(third_party_path)/breakpad/src/client/linux/handler/exception_handler.cc',
                        '<(third_party_path)/breakpad/src/client/linux/handler/exception_handler.h',
                        '<(third_party_path)/breakpad/src/client/linux/handler/minidump_descriptor.cc',
                        '<(third_party_path)/breakpad/src/client/linux/handler/minidump_descriptor.h',
                        '<(third_party_path)/breakpad/src/client/linux/log/log.cc',
                        '<(third_party_path)/breakpad/src/client/linux/log/log.h',
                        '<(third_party_path)/breakpad/src/client/linux/minidump_writer/cpu_set.h',
                        '<(third_party_path)/breakpad/src/client/linux/minidump_writer/directory_reader.h',
                        '<(third_party_path)/breakpad/src/client/linux/minidump_writer/line_reader.h',
                        '<(third_party_path)/breakpad/src/client/linux/minidump_writer/linux_core_dumper.cc',
                        '<(third_party_path)/breakpad/src/client/linux/minidump_writer/linux_core_dumper.h',
                        '<(third_party_path)/breakpad/src/client/linux/minidump_writer/linux_dumper.cc',
                        '<(third_party_path)/breakpad/src/client/linux/minidump_writer/linux_dumper.h',
                        '<(third_party_path)/breakpad/src/client/linux/minidump_writer/linux_ptrace_dumper.cc',
                        '<(third_party_path)/breakpad/src/client/linux/minidump_writer/linux_ptrace_dumper.h',
                        '<(third_party_path)/breakpad/src/client/linux/minidump_writer/minidump_writer.cc',
                        '<(third_party_path)/breakpad/src/client/linux/minidump_writer/minidump_writer.h',
                        '<(third_party_path)/breakpad/src/client/linux/minidump_writer/proc_cpuinfo_reader.h',
                        '<(third_party_path)/breakpad/src/client/minidump_file_writer-inl.h',
                        '<(third_party_path)/breakpad/src/client/minidump_file_writer.cc',
                        '<(third_party_path)/breakpad/src/client/minidump_file_writer.h',
                        '<(third_party_path)/breakpad/src/common/convert_UTF.c',
                        '<(third_party_path)/breakpad/src/common/convert_UTF.h',
                        '<(third_party_path)/breakpad/src/common/linux/elf_core_dump.cc',
                        '<(third_party_path)/breakpad/src/common/linux/elf_core_dump.h',
                        '<(third_party_path)/breakpad/src/common/linux/elfutils.cc',
                        '<(third_party_path)/breakpad/src/common/linux/elfutils.h',
                        '<(third_party_path)/breakpad/src/common/linux/file_id.cc',
                        '<(third_party_path)/breakpad/src/common/linux/file_id.h',
                        '<(third_party_path)/breakpad/src/common/linux/google_crashdump_uploader.cc',
                        '<(third_party_path)/breakpad/src/common/linux/google_crashdump_uploader.h',
                        '<(third_party_path)/breakpad/src/common/linux/guid_creator.cc',
                        '<(third_party_path)/breakpad/src/common/linux/guid_creator.h',
                        '<(third_party_path)/breakpad/src/common/linux/libcurl_wrapper.cc',
                        '<(third_party_path)/breakpad/src/common/linux/libcurl_wrapper.h',
                        '<(third_party_path)/breakpad/src/common/linux/linux_libc_support.cc',
                        '<(third_party_path)/breakpad/src/common/linux/linux_libc_support.h',
                        '<(third_party_path)/breakpad/src/common/linux/memory_mapped_file.cc',
                        '<(third_party_path)/breakpad/src/common/linux/memory_mapped_file.h',
                        '<(third_party_path)/breakpad/src/common/linux/safe_readlink.cc',
                        '<(third_party_path)/breakpad/src/common/linux/safe_readlink.h',
                        '<(third_party_path)/breakpad/src/common/memory.h',
                        '<(third_party_path)/breakpad/src/common/simple_string_dictionary.cc',
                        '<(third_party_path)/breakpad/src/common/simple_string_dictionary.h',
                        '<(third_party_path)/breakpad/src/common/string_conversion.cc',
                        '<(third_party_path)/breakpad/src/common/string_conversion.h',
                    ],

                    'conditions': [
#                        # Android NDK toolchain doesn't support -mimplicit-it=always
#                        ['target_arch=="arm" and OS!="android"', {
#                            'cflags': ['-Wa,-mimplicit-it=always'],
#                        }],
#                        ['target_arch=="arm" and chromeos==1', {
#                            # Avoid running out of registers in
#                            # linux_syscall_support.h:sys_clone()'s inline assembly.
#                            'cflags': ['-marm'],
#                        }],
#                        ['OS=="android"', {
#                            'include_dirs': [
#                                '<(third_party_path)/breakpad/src/common/android/include',
#                            ],
#                            'direct_dependent_settings': {
#                                'include_dirs': [
#                                    '<(third_party_path)/breakpad/src/common/android/include',
#                                ],
#                            },
#                            'sources': [
#                                '<(third_party_path)/breakpad/src/common/android/breakpad_getcontext.S',
#                            ],
#                        }],
                        ['OS!="android"', {
                            'link_settings': {
                                'libraries': [
                                    # In case of Android, '-ldl' is added in common.gypi, since it
                                    # is needed for stlport_static. For LD, the order of libraries
                                    # is important, and thus we skip to add it here.
                                    '-ldl',
                                ],
                            },
                        }],
                    ],

                    'include_dirs': [
                        '<(third_party_path)/breakpad/src/',
                        '<(third_party_path)/breakpad/src/client',
                        '<(third_party_path)/breakpad/src/third_party/linux/include',
                        #'..',
                        #'.',
                    ],
                },
                {
                    # Breakpad r693 uses some files from src/processor in unit tests.
                    'target_name': 'breakpad_processor_support',
                    'type': 'static_library',

                    'sources': [
                        '<(third_party_path)/breakpad/src/common/scoped_ptr.h',
                        '<(third_party_path)/breakpad/src/processor/basic_code_modules.cc',
                        '<(third_party_path)/breakpad/src/processor/basic_code_modules.h',
                        '<(third_party_path)/breakpad/src/processor/logging.cc',
                        '<(third_party_path)/breakpad/src/processor/logging.h',
                        '<(third_party_path)/breakpad/src/processor/minidump.cc',
                        '<(third_party_path)/breakpad/src/processor/pathname_stripper.cc',
                        '<(third_party_path)/breakpad/src/processor/pathname_stripper.h',
                    ],

                    'include_dirs': [
                        '<(third_party_path)/breakpad/src/',
                        '<(third_party_path)/breakpad/src/client',
                        '<(third_party_path)/breakpad/src/third_party/linux/include',
                        #'..',
                        #'.',
                    ],
                },
                {
                    'target_name': 'minidump-2-core',
                    'type': 'executable',

                    'sources': [
                        '<(third_party_path)/breakpad/src/tools/linux/md2core/minidump-2-core.cc'
                    ],

                    'dependencies': [
                        'breakpad_client',
                    ],

                    'include_dirs': [
                        '..',
                        '<(third_party_path)/breakpad/src/',
                    ],
                },
                {
                    'target_name': 'core-2-minidump',
                    'type': 'executable',

                    'sources': [
                        '<(third_party_path)/breakpad/src/tools/linux/core2md/core2md.cc'
                    ],

                    'dependencies': [
                        'breakpad_client',
                    ],

                    'include_dirs': [
                        #'..',
                        '<(third_party_path)/breakpad/src/',
                    ],
                },
            ],
        }],
    ],
}
