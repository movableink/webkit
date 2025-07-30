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

import json
import os
import sqlite3
import typing
from pathlib import Path

from .macho import APIReport, objc_fully_qualified_method
from .tbd import TBD
from fnmatch import fnmatch

# Increment this number to force clients to rebuild from scratch, to
# accomodate schema changes or fix caching bugs.
VERSION = 1


class SDKDB:
    """
    A sqlite-backed cache of API availability data. Composed from a variety of
    sources:

    - Symbols and Objective-C runtime metadata from partial SDKDB (.sdkdb)
      records produced by TAPI
    - Symbols from text-based library stubs (.tbd).
    - Symbols from Mach-O binaries.

    The cache keeps track of the file name which each record comes from, and
    the modification time of each input file. The add_* methods take a path
    and short circuit if that file is already in the cache with the same mtime.
    When an input file changes, all the records associated with it are deleted
    and it is re-added.

    To manage concurrent access to a cache, enter the sdkdb as a context
    manager before using the add_* methods.
    """

    def __init__(self, db_file: Path):
        user_version = None
        while user_version != VERSION:
            self.con = sqlite3.connect(db_file, isolation_level='IMMEDIATE')
            self.con.execute('PRAGMA busy_timeout = 30000')
            user_version, = self.con.execute('PRAGMA user_version').fetchone()
            if user_version == 0:
                try:
                    self._initialize_db()
                except sqlite3.OperationalError:
                    # Delete the database on initialization errors, in case
                    # rebuilding fixes it.
                    self.con.close()
                    db_file.unlink()
                    raise
            elif user_version != VERSION:
                print(f'Rebuilding {db_file} due to version change')
                self.con.close()
                db_file.unlink()

    def _initialize_db(self):
        cur = self.con.cursor()
        # Python's sqlite3 module does not start a transaction for DDL
        # statements. Explicitly BEGIN to prevent concurrent processes from
        # trying to initialize the database simultaneously.
        cur.execute('BEGIN IMMEDIATE TRANSACTION')
        cur.execute('PRAGMA user_version')
        if cur.fetchone() == (VERSION,):
            # The database was initialized while we were waiting.
            return
        cur.execute('CREATE TABLE input_file(name PRIMARY KEY, hash)')
        cur.execute('CREATE TABLE symbol(name, input_file '
                    'REFERENCES input_file(name) ON DELETE CASCADE)')
        cur.execute('CREATE TABLE objc_class(name, input_file '
                    'REFERENCES input_file(name) ON DELETE CASCADE)')
        cur.execute('CREATE TABLE objc_selector(name, class, input_file '
                    'REFERENCES input_file(name) ON DELETE CASCADE)')
        cur.execute('CREATE TABLE meta(key, value)')
        cur.execute('CREATE INDEX symbol_names ON symbol (name)')
        cur.execute('CREATE INDEX selector_names ON objc_selector (name)')
        cur.execute(f'PRAGMA user_version = {VERSION}')
        self.con.commit()

    def __del__(self):
        if not hasattr(self, 'con'):
            return
        # May fail if the connection is closed (due to failure to initialize).
        try:
            self.con.execute('PRAGMA optimize')
        except sqlite3.ProgrammingError:
            pass

    def __enter__(self):
        # Python's sqlite3 module will issue a BEGIN before the first data
        # modification. No need to explicitly start a transaction earlier.
        pass

    def __exit__(self, exc_type, exc_value, traceback):
        if exc_type:
            self.con.rollback()
        else:
            self.con.commit()

    def _cache_hit_preparing_to_insert(self, name: str, hash_: int) -> bool:
        cur = self.con.cursor()
        cur.execute('SELECT hash from input_file where name = ?', (name,))
        if cur.fetchone() == (hash_,):
            return True
        cur.execute('INSERT OR REPLACE INTO input_file VALUES (?, ?)',
                    (name, hash_))
        return False

    def add_partial_sdkdb(self, sdkdb_file: Path, *, spi=False, abi=False,
                          ) -> bool:
        fd = open(sdkdb_file)
        sdkdb_hash = os.fstat(fd.fileno()).st_mtime_ns

        if self._cache_hit_preparing_to_insert(sdkdb_file.name,
                                               sdkdb_hash):
            return False

        doc = json.load(fd)
        criteria = [
            ('PublicSDKContentRoot', lambda _: True),
            ('SDKContentRoot',
             lambda x: spi or x.get('access') == 'public'),
        ]
        if abi:
            criteria.append(('RuntimeRoot', lambda _: True))
        for key, pred in criteria:
            root = doc.get(key, ())
            for ent in root:
                for category in ent.get('categories', []):
                    class_name = f'{category["interface"]}'\
                                 f'({category["name"]})'
                    self._add_objc_interface(category, class_name,
                                             sdkdb_file, pred)
                for symbol in ent.get('globals', []):
                    if pred(symbol):
                        self._add_symbol(symbol['name'], sdkdb_file)
                for iface in ent.get('interfaces', []):
                    if pred(iface):
                        self._add_objc_interface(iface, iface['name'],
                                                 sdkdb_file, pred)
                        self._add_objc_class(iface['name'], sdkdb_file)
                for proto in ent.get('protocols', []):
                    if pred(proto):
                        self._add_objc_interface(proto, proto['name'],
                                                 sdkdb_file, pred)
        return True

    def add_binary(self, binary: Path, arch: str) -> bool:
        stat_hash = binary.stat().st_mtime_ns
        if self._cache_hit_preparing_to_insert(binary.name, stat_hash):
            return False
        report = APIReport.from_binary(binary, arch=arch, exports_only=True)
        for selector in report.methods:
            self._add_objc_selector(selector, None, binary)
        for symbol in report.exports:
            m = objc_fully_qualified_method.match(symbol)
            if m:
                self._add_objc_selector(m.group('selector'),
                                        m.group('class'), binary)
            else:
                self._add_symbol(symbol, binary)
        return True

    def add_tbd(self, tbd_file: Path,
                only_including: typing.Optional[typing.Iterable[str]]) -> bool:
        fd = open(tbd_file)
        stat_hash = os.fstat(fd.fileno()).st_mtime_ns

        if self._cache_hit_preparing_to_insert(tbd_file.name, stat_hash):
            return False
        for tbd in TBD.from_file(tbd_file):
            if only_including and all(not fnmatch(tbd.install_name, pattern)
                                      for pattern in only_including):
                continue
            for export_list in tbd.exports + tbd.reexports:
                for symbol in export_list.symbols:
                    self._add_symbol(symbol, tbd_file)
                for symbol in export_list.weak_symbols:
                    self._add_symbol(symbol, tbd_file)
                for class_ in export_list.objc_classes:
                    self._add_objc_class(class_, tbd_file)
        return True

    def symbol(self, name: str):
        cur = self.con.cursor()
        cur.execute('SELECT * FROM symbol WHERE name = ? LIMIT 1', (name,))
        row = cur.fetchone()
        if row:
            return dict(name=row[0], input_file=row[1])

    def objc_class(self, name: str):
        cur = self.con.cursor()
        cur.execute('SELECT * FROM objc_class WHERE name = ?', (name,))
        row = cur.fetchone()
        if row:
            return dict(name=row[0], input_file=row[1])

    def objc_selector(self, selector: str):
        cur = self.con.cursor()
        cur.execute('SELECT * FROM objc_selector WHERE name = ?', (selector,))
        row = cur.fetchone()
        if row:
            return dict(name=row[0], class_name=row[1], input_file=row[2])

    def stats(self):
        cur = self.con.cursor()
        cur.execute('SELECT (SELECT COUNT(name) FROM symbol) AS symbols, '
                    '(SELECT COUNT(name) FROM objc_class) AS classes, '
                    '(SELECT COUNT(name) FROM objc_selector) AS selectors')
        row = cur.fetchone()
        return row

    def _add_objc_interface(self, ent: dict, class_name: str, file: Path,
                            pred: typing.Callable[[dict], bool]):
        for key in 'instanceMethods', 'classMethods':
            for method in ent.get(key, []):
                if pred(method):
                    self._add_objc_selector(method['name'], class_name, file)
        for prop in ent.get('properties', []):
            if pred(prop):
                self._add_objc_selector(prop['getter'], class_name, file)
                if 'readonly' not in prop.get('attr', {}):
                    self._add_objc_selector(prop['setter'], class_name, file)
        for ivar in ent.get('ivars', []):
            if pred(ivar):
                self._add_symbol(
                    f'_OBJC_IVAR_$_{class_name}.{ivar["name"]}', file)

    def _add_symbol(self, name: str, file: Path):
        cur = self.con.cursor()
        cur.execute('INSERT INTO symbol VALUES (?, ?)', (name, file.name))

    def _add_objc_class(self, name: str, file: Path):
        cur = self.con.cursor()
        cur.execute('INSERT INTO objc_class VALUES (?, ?)', (name, file.name))
        cur.execute('INSERT INTO symbol VALUES (?, ?)',
                    (f'_OBJC_CLASS_$_{name}', file.name))
        cur.execute('INSERT INTO symbol VALUES (?, ?)',
                    (f'_OBJC_METACLASS_$_{name}', file.name))

    def _add_objc_selector(self, name: str, class_name: str, file: Path):
        cur = self.con.cursor()
        cur.execute('INSERT INTO objc_selector VALUES (?, ?, ?)',
                    (name, class_name, file.name))
