//  Copyright (c) 2011-present, Facebook, Inc.  All rights reserved.
//  This source code is licensed under the BSD-style license found in the
//  LICENSE file in the root directory of this source tree. An additional grant
//  of patent rights can be found in the PATENTS file in the same directory.
//
// The following only applies to changes made to this file as part of YugaByte development.
//
// Portions Copyright (c) YugaByte, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
// in compliance with the License.  You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software distributed under the License
// is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
// or implied.  See the License for the specific language governing permissions and limitations
// under the License.
//
// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// Thread-safe (provides internal synchronization)
#ifndef ROCKSDB_DB_TABLE_CACHE_H
#define ROCKSDB_DB_TABLE_CACHE_H

#pragma once
#include <stdint.h>

#include <string>
#include <vector>

#include "yb/rocksdb/db/dbformat.h"
#include "yb/rocksdb/port/port.h"
#include "yb/rocksdb/cache.h"
#include "yb/rocksdb/env.h"
#include "yb/rocksdb/table.h"
#include "yb/rocksdb/options.h"
#include "yb/rocksdb/table/table_reader.h"

namespace rocksdb {

class Env;
class Arena;
struct FileDescriptor;
class GetContext;
class HistogramImpl;
class InternalIterator;

class TableCache {
 public:
  TableCache(const ImmutableCFOptions& ioptions,
             const EnvOptions& storage_options, Cache* cache);
  ~TableCache();

  // Return an iterator for the specified file number (the corresponding
  // file length must be exactly "total_file_size" bytes).  If "tableptr" is
  // non-nullptr, also sets "*tableptr" to point to the Table object
  // underlying the returned iterator, or nullptr if no Table object underlies
  // the returned iterator.  The returned "*tableptr" object is owned by
  // the cache and should not be deleted, and is valid for as long as the
  // returned iterator is live.
  // @param skip_filters Disables loading/accessing the filter block
  InternalIterator* NewIterator(
      const ReadOptions& options, const EnvOptions& toptions,
      const InternalKeyComparator& internal_comparator,
      const FileDescriptor& file_fd, TableReader** table_reader_ptr = nullptr,
      HistogramImpl* file_read_hist = nullptr, bool for_compaction = false,
      Arena* arena = nullptr, bool skip_filters = false);

  // If a seek to internal key "k" in specified file finds an entry,
  // call (*handle_result)(arg, found_key, found_value) repeatedly until
  // it returns false.
  // @param skip_filters Disables loading/accessing the filter block
  Status Get(const ReadOptions& options,
             const InternalKeyComparator& internal_comparator,
             const FileDescriptor& file_fd, const Slice& k,
             GetContext* get_context, HistogramImpl* file_read_hist = nullptr,
             bool skip_filters = false);

  // Evict any entry for the specified file number
  static void Evict(Cache* cache, uint64_t file_number);

  // Find table reader
  // @param skip_filters Disables loading/accessing the filter block
  Status FindTable(const EnvOptions& toptions,
                   const InternalKeyComparator& internal_comparator,
                   const FileDescriptor& file_fd, Cache::Handle**,
                   const QueryId query_id,
                   const bool no_io = false, bool record_read_stats = true,
                   HistogramImpl* file_read_hist = nullptr,
                   bool skip_filters = false);

  // Get TableReader from a cache handle.
  TableReader* GetTableReaderFromHandle(Cache::Handle* handle);

  // Get the table properties of a given table.
  // @no_io: indicates if we should load table to the cache if it is not present
  //         in table cache yet.
  // @returns: `properties` will be reset on success. Please note that we will
  //            return STATUS(Incomplete, ) if table is not present in cache and
  //            we set `no_io` to be true.
  Status GetTableProperties(const EnvOptions& toptions,
                            const InternalKeyComparator& internal_comparator,
                            const FileDescriptor& file_meta,
                            std::shared_ptr<const TableProperties>* properties,
                            bool no_io = false);

  // Return total memory usage of the table reader of the file.
  // 0 if table reader of the file is not loaded.
  size_t GetMemoryUsageByTableReader(
      const EnvOptions& toptions,
      const InternalKeyComparator& internal_comparator,
      const FileDescriptor& fd);

  // Release the handle from a cache
  void ReleaseHandle(Cache::Handle* handle);

 private:
  // Build a table reader
  Status GetTableReader(const EnvOptions& env_options,
                        const InternalKeyComparator& internal_comparator,
                        const FileDescriptor& fd, bool sequential_mode,
                        bool record_read_stats, HistogramImpl* file_read_hist,
                        unique_ptr<TableReader>* table_reader,
                        bool skip_filters = false);

  const ImmutableCFOptions& ioptions_;
  const EnvOptions& env_options_;
  Cache* const cache_;
  std::string row_cache_id_;
};

}  // namespace rocksdb

#endif // ROCKSDB_DB_TABLE_CACHE_H