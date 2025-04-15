# Copyright (C) 2025 Apple Inc. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import argparse
import os
import pkgutil
import shlex
import time
from fnmatch import fnmatch
from pathlib import Path
from typing import Optional
from types import ModuleType

import webkitapipy
from webkitapipy.sdkdb import SDKDB
from webkitapipy.macho import APIReport


# Some symbols, namely ones that are low-level parts of system libraries and
# runtimes, are implicitly available.
ALLOWED_SYMBOLS = {
    '_OBJC_METACLASS_$_NSObject',
    '_OBJC_EHTYPE_$_NSException',
    # Foundation APIs
    '_OBJC_CLASS_$_NSConstantArray',
    '_OBJC_CLASS_$_NSConstantDictionary',
    '_OBJC_CLASS_$_NSConstantDoubleNumber',
    '_OBJC_CLASS_$_NSConstantIntegerNumber',
    '___CFConstantStringClassReference',
    '___NSArray0__',
    '___NSDictionary0__',
    # rdar://79462292
    '___kCFBooleanFalse',
    '___kCFBooleanTrue',
    '___NSArray0__struct',
    '___NSDictionary0__struct',
    # C++ std
    '__ZdlPv',
    '__Znwm',
}

ALLOWED_SYMBOL_GLOBS = (
    # C++ std
    '__ZS*',
    '__ZT*',
    '__ZNS*',
    '__ZNK*',
    '__ZN9__gnu_cxx*',
    # We remove SwiftUI from SDK DBs due to rdar://143449950.
    '_$s*7SwiftUI*',
    # rdar://79109142
    '__swift_FORCE_LOAD_$_*',
)

# TBDs from the active SDK whose symbols are treated as implicitly available.
# Pattern strings on the right-hand side select individual libraries from the
# TBD.
SDK_ALLOWLIST = {
    'usr/lib/libobjc.tbd': (),
    'usr/lib/swift/lib*.tbd': (),
    'usr/lib/libc++*.tbd': (),
    'usr/lib/libSystem.B.tbd': ('/usr/lib/system/libsystem_*',
                                '/usr/lib/system/libcompiler_rt*',
                                '/usr/lib/system/libunwind*'),
    'usr/lib/libicucore.A.tbd': (),
}


class TSVReporter:
    def __init__(self, args: argparse.Namespace):
        self.n_issues = 0
        self.print_details = args.details

    def process_report(self, report: APIReport, db: SDKDB):
        for selref in sorted(report.selrefs):
            if not db.objc_selector(selref) and selref not in report.methods:
                self.missing_selector(selref)

        for symbol in sorted(report.imports):
            ignored = symbol in ALLOWED_SYMBOLS
            if not ignored:
                ignored = any(fnmatch(symbol, pattern)
                              for pattern in ALLOWED_SYMBOL_GLOBS)
            if symbol.startswith('_OBJC_CLASS_$_'):
                class_name = symbol.removeprefix('_OBJC_CLASS_$_')
                if not db.objc_class(class_name):
                    self.missing_class(class_name, ignored=ignored)
            elif not db.symbol(symbol):
                self.missing_symbol(symbol, ignored=ignored)

    def missing_selector(self, name: str, *, ignored=False):
        if not ignored:
            if self.print_details:
                print('selector:', name, sep='\t')
            self.n_issues += 1

    def missing_class(self, name: str, *, ignored=False):
        if not ignored:
            if self.print_details:
                print('class:', name, sep='\t')
            self.n_issues += 1

    def missing_symbol(self, name: str, *, ignored=False):
        if not ignored:
            if self.print_details:
                print('symbol:', name, sep='\t')
            self.n_issues += 1

    def finished(self):
        print(f'{self.n_issues} potential use{"s"[:self.n_issues^1]} of SPI.')
        if self.n_issues and not self.print_details:
            print('Rerun with --details to see each validation issue.')


def get_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description='''\
    Using API availability information from a directory of SDKDB records,
    scan a Mach-O binary for use of unknown symbols or Objective-C selectors.
    ''')
    parser.add_argument('input_files', nargs='*', type=Path,
                        help='binaries and SDKDBs to allow arbitrary use of')
    parser.add_argument('-a', '--arch-name', required=True,
                        help='which architecture to analyze binary with')
    parser.add_argument('--primary-file', type=Path, required=True,
                        help='file to analyze')
    parser.add_argument('--sdkdb-dir', type=Path, required=True,
                        help='directory of partial SDKDB records for an SDK')
    parser.add_argument('--sdkdb-cache', type=Path, required=True,
                        help='database file to store SDKDB availabilities')
    parser.add_argument('--sdk-dir', type=Path, required=True,
                        help='Xcode SDK the binary is built against')
    parser.add_argument('--depfile', type=Path,
                        help='write inputs used for incremental rebuilds')
    parser.add_argument('--details', action='store_true',
                        help='print a line for each unknown symbol')
    return parser


def main(argv=None):
    webkitapipy_additions: Optional[ModuleType]
    try:
        import webkitapipy_additions
    except ImportError:
        webkitapipy_additions = None

    if webkitapipy_additions:
        parser = webkitapipy_additions.program.get_parser()
    else:
        parser = get_parser()
    args = parser.parse_args(argv)

    inputs = []
    # For the depfile, start with the paths of all the modules in webkitapipy
    # since this library is part of WebKit source code.
    for package in (webkitapipy, webkitapipy_additions):
        if not package:
            continue
        for info in pkgutil.walk_packages(package.__path__):
            spec = info.module_finder.find_spec(info.name, None)
            if spec and spec.origin:
                inputs.append(spec.origin)

    def use_input(path):
        inputs.append(path)
        return path

    # Do not `use_input` on the sdkdb cache, because we may write to it, and
    # that would invalidate audit-spi invocations in other projects.
    db = SDKDB(args.sdkdb_cache)

    # Initializing the SDKDB cache from scratch takes some time (~ 15-20 sec).
    # Print progress updates and measure execution time to indicate how much
    # build time the cache will save.
    n_changes = 0

    def increment_changes():
        nonlocal n_changes
        if n_changes == 0:
            print(f'Building SDKDB cache from {args.sdkdb_dir}...')
        n_changes += 1
        if n_changes % 10 == 0:
            print(f'{n_changes} projects...')

    db_initialization_start = time.monotonic()
    with db:
        for file in args.sdkdb_dir.iterdir():
            if file.suffix != '.sdkdb':
                continue
            if db.add_partial_sdkdb(use_input(file)):
                increment_changes()
        for file_pattern, library_patterns in SDK_ALLOWLIST.items():
            for tbd_path in args.sdk_dir.glob(file_pattern):
                if db.add_tbd(use_input(tbd_path),
                              only_including=library_patterns):
                    increment_changes()
    if n_changes:
        symbols, classes, selectors = db.stats()
        db_initialization_duration = time.monotonic() - db_initialization_start
        print(f'Done. Took {db_initialization_duration:.2f} sec.',
              f'{symbols=} {classes=} {selectors=}')

    for path in args.input_files:
        with db:
            if path.suffix == '.sdkdb':
                db.add_partial_sdkdb(use_input(path), spi=True, abi=True)
            else:
                db.add_binary(use_input(path), arch=args.arch_name)

    report = APIReport.from_binary(args.primary_file, arch=args.arch_name)

    if webkitapipy_additions:
        reporter = webkitapipy_additions.program.configure_reporter(args, db)
    else:
        reporter = TSVReporter(args)

    reporter.process_report(report, db)
    reporter.finished()

    if args.depfile:
        with open(args.depfile, 'w') as fd:
            fd.write('dependencies: ')
            fd.write(' \\\n  '.join(shlex.quote(os.path.abspath(path))
                                    for path in inputs))
            fd.write('\n')
