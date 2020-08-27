// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2020 HörTech gGmbH
//
// openMHA is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, version 3 of the License.
//
// openMHA is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License, version 3 for more details.
//
// You should have received a copy of the GNU Affero General Public License, 
// version 3 along with openMHA.  If not, see <http://www.gnu.org/licenses/>.

package de.hoertech.mha.control;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

/** Input stream reader class.  Reads from input stream in separate
 * thread.  Received input can either be discarded or stored in
 * a string object accessible through the get() method.  Input is read
 * line by line, therefore this class is intended for text streams and
 * not suitable for binary input streams. */
public class StreamGobbler extends Thread {
  /** Input stream to read from */
  private InputStream is;

  /** Received input is stored here if store == true */
  private String storage;

  /** Whether to store received input in storage or discard it */
  private boolean store;

  /** Initialize StreamGobbler and start reader thread.  The reader thread
   * terminates when the input stream reaches EOF. 
   * @param is The input stream to read from
   * @param store When true, then the received input will be stored and
   *              is accessible through the get() method.  When false,
   *              then the received input is discarded. */
  public StreamGobbler(InputStream is, boolean store) {
    this.is = is;
    this.store = store;
    storage = store ? "" : null;
    start();
  }
  
  @Override
  public void run() {
    try {
      InputStreamReader isr = new InputStreamReader(is);
      BufferedReader br = new BufferedReader(isr);
      String line = null;
      while ((line = br.readLine()) != null) {
        if (store)
          storage += line + "\n";
      }
    } catch (IOException e) {
      e.printStackTrace();
    }
  }

  /** Returns received text if it was stored. */
  public String get() {
    return storage;
  }
}
