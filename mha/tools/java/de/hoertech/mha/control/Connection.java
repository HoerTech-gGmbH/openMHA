// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2008 2011 2012 2013 2016 2020 HörTech gGmbH
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

import java.io.*;
import java.net.*;

/**
 * This class establishes a connection to the MHA (Master-Hearing-Aid) server
 * on demand and performs the communication.
 */

public class Connection implements Parser {

  private InetSocketAddress mhaAddress;
  private int timeoutMs = 7000; // 7 seconds
  private Socket mhaServerSocket;
  private DataOutputStream outToServer;
  private BufferedReader inFromServer;

  public Connection() {}

  public Connection(String host, int port) {
    setAddress(host, port);
  }
  
  @Override
  public String toString() {
    if (mhaAddress == null) return "mha://null/";
    return "mha:/" + mhaAddress.toString() + "/"; 
  }
  
  @Override
  public int hashCode() {
    return mhaAddress.hashCode() + timeoutMs*3 + 
        super.hashCode()*2 - 2;
  }
  
  @Override
  public boolean equals(Object obj) {
    if (this == obj) return true;
    if (!(obj instanceof Connection)) return false;
    Connection other = (Connection) obj;
    return super.equals(obj) && 
        mhaAddress.equals(other.mhaAddress) &&
        (timeoutMs == other.timeoutMs);
  }
  
  /**
   * sets the timeout value for communication (default: 7s)
   * 
   * @param timeout
   */
  public void setTimeout(int timeout) {
    timeoutMs = timeout;
  }

  /**
   * This Method sets the hostname and port to the MHA Server.
   * It translates the hostname to an IP address and opens
   * the TCP connection to the Server. 
   * If a TCP connection to an MHA server was already established
   * when this method is called, the existing TCP connection is closed
   * before opening the new TCP connection.
   * @param hostname
   *            hostname or ip address of the computer where the MHA runs
   * @param port
   *            TCP port number where the MHA listens to incoming connections
   * @return true if setting the address was successful, false if hostname
   *              lookup or TCP connection did not succeed
   */

  public boolean setAddress(String hostname, int mhaPort) {
    InetSocketAddress newAddress;
    try {
      newAddress = new InetSocketAddress(InetAddress.getByName(hostname),
          mhaPort);
    } catch (UnknownHostException e) {
      return false;
    }
    try {
      if (mhaServerSocket != null)
        connect(false);
    } catch (Exception e) {}
    try {
      mhaAddress = newAddress;
      connect(true);
    } catch (Exception e) {
      return false;
    }
    return true;
  }

  /**
   * This method sends a command to the MHA and returns the response.
   * This method will block no more than timeoutMs.
   * 
   * @param command
   *            the command to send to the MHA Server
   * @return answer from the MHA Server
   * @throws InterruptedException 
   * @throws Exception
   * @throws SocketTimeoutException If the timeout occurs.
   */
  public ParserResponse parse(String command)
      throws InterruptedException, Exception {
      String [] commands = new String[1];
      commands[0] = command;
      ParserResponse[] responses = parse(commands);
      return responses[0];
  }
  
  /** Method for sending multiple commands and receiving multiple responses.
   * @param commands Array of commands
   * @return Array of mha responses.
   * @throws InterruptedException 
   * @throws Exception
   * @throws SocketTimeoutException If the timeout occurs.
   */
  public ParserResponse[] parse(String[] commands)
      throws InterruptedException, Exception {
    ParserResponse[] responses = new ParserResponse[commands.length];
    int i = 0;
    try {
      StringBuilder sb = new StringBuilder();
      for (String command : commands) {
        sb.append(command);
        sb.append("\n");
      }
      outToServer.writeBytes(sb.toString());
      outToServer.flush();
      
      for (; i < responses.length; ++i) {
        responses[i] = receiveResponse();
      }
    } catch (Exception e) {
      for (; i < responses.length; ++i) {
        responses[i] = new ParserResponse();
        responses[i].appendLine(e.toString());
        responses[i].appendLine("(MHA:failure)");
      }
    }
    return responses;
  }
  
  /**
   * Collects the response to a single command.
   * @return The collected response
   * @throws IOException
   */
  private ParserResponse receiveResponse()
      throws IOException {
    ParserResponse response = new ParserResponse();
    String line = "";

    // The loop may throw a timeout Exception TODO: Test this!
    while ((line = inFromServer.readLine()) != null) {
      if (response.appendLine(line) == false) {
        break;
      }
    }
    return response;
  }

  public String hostname() {
    return mhaAddress.getHostName();
  }

  public int port() {
    return mhaAddress.getPort();
  }

  public void connect(boolean onOff) throws Exception {
    if (onOff) {
      mhaServerSocket = new Socket();
      mhaServerSocket.connect(mhaAddress, timeoutMs);
      mhaServerSocket.setSoTimeout(timeoutMs);
      mhaServerSocket.setTcpNoDelay(true);
      outToServer = new DataOutputStream(mhaServerSocket
          .getOutputStream());
      inFromServer = new BufferedReader(new InputStreamReader(
          mhaServerSocket.getInputStream()));
    } else if (mhaServerSocket != null) {
      try {
        mhaServerSocket.close();
      } catch (IOException e) {}
      outToServer.close();
      outToServer = null;
      inFromServer.close();
      inFromServer = null;
      mhaServerSocket = null;
    }
  }
}
