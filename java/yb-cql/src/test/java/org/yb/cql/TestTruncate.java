// Copyright (c) YugaByte, Inc.
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
package org.yb.cql;

import com.datastax.driver.core.Row;

import org.junit.Test;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;

public class TestTruncate extends BaseCQLTest {

  @Test
  public void testTruncate() throws Exception {

    // Create test table with rows.
    final int ROW_COUNT = 10;
    setupTable("test_truncate", ROW_COUNT);

    // Verify row count.
    int count = 0;
    for (Row row : session.execute("select * from test_truncate;")) {
      count++;
    }
    assertEquals(ROW_COUNT, count);

    // Truncate rows and verify the table is empty.
    session.execute("truncate test_truncate;");
    assertNull(session.execute("select * from test_truncate;").one());

    // Truncate again (an NOOP). Verify there is no issue.
    session.execute("truncate test_truncate;");
    assertNull(session.execute("select * from test_truncate;").one());

    // Truncate a newly created, empty table (an NOOP also). Verify there is no issue.
    setupTable("test_empty_truncate", 0);
    session.execute("truncate test_empty_truncate;");
    assertNull(session.execute("select * from test_empty_truncate;").one());
  }

  @Test
  public void testInvalidTruncate() throws Exception {
    runInvalidStmt("truncate table invalid_namespace.test_truncate;");
    runInvalidStmt("truncate table invalid_table;");
  }
}
