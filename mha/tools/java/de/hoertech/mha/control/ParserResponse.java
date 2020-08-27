// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2008 2011 2013 2020 HörTech gGmbH
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
import java.util.*;

/**
 * Encapsulates the response from the MHA parser to a single request.
 * Also helps accumulating a complete response.
 */
public class ParserResponse {
  /** MHA prompt after successful command execution */
  private String promptSuccess = "(MHA:success)";

  /** MHA prompt after unsuccessful command execution */
  private String promptFailure = "(MHA:failure)";

  /**
   * Indicates whether the last line of the response has been
   * (MHA:success) or (MHA:failure) (or altered prompts);
   */
  private boolean success = false;

  /**
   * Indicates whether this parser response is already complete, or if we
   * are still waiting for the final prompt.
   */
  private boolean complete = false;
  
  /**
   * Individual lines of text from the response, without the newline, are
   * collected in this array. The final prompt is not stored here.
   */
  private ArrayList<String> lines = new ArrayList<String>();

  /**
   * Alter what prompts to expect from MHA to signal successful or unsuccessful
   * command execution.
   * @param promptSuccess
   *   Prompt to expect after successful command execution, without trailing
   *   newline.
   * @param promptFailure
   *   Prompt to expect after unsuccessful command execution, without trailing
   *   newline.
   */
  public void setExpectedPrompts(String promptSuccess, String promptFailure) {
    this.promptSuccess = promptSuccess;
    this.promptFailure = promptFailure;
  }

  /**
   * Build-support for the parser response object. Individual lines may
   * be added as they are received.
   * If this parser response is already complete, then this method has no
   * effect.
   * @param line
   *   The next line of the response. The prompts (MHA:success) and
   *   (MHA:failure) are recognised and not stored in the array #lines,
   *   but rather cause the success flag to be set appropriately and
   *   the complete flag gets set to true.
   * @return
   *   !{@link #complete}. The network receiver can use this return value
   *   to determine when the response to a single request is completed.
   */
  public boolean appendLine(String line) {
    if (complete) {
      return false;
    }
    if (line.equals(promptSuccess)) {
      success = complete = true;
      return false;
    }
    if (line.equals(promptFailure)) {
      complete = true;
      success = false;
      return false;
    }
    lines.add(line);
    return true;
  }

  /**
   * @return
   *   The text lines that make up the response, as string array.
   *   Never contains the final prompt.
   */
  public String[] getLines() {
    return lines.toArray(new String[]{});
    
  }

  /**
   * @return
   *   Which prompt completed this response.
   */
  public boolean getSuccess(){
    return success;
  }

  /**
   * Resets this parser response object to the state that it has after 
   * construction: {@link #complete} = false, {@link #success} = false,
   * {@link #lines} empty.
   */
  public void clear(){
    complete = success = false;
    lines.clear();
  }

  /**
   * combines all response lines. Last line does not have an EOL.
   */
  public String toString() {
    StringBuilder sb = new StringBuilder();
    for (String line : lines) {
      sb.append(line);
      sb.append("\n");
    }
    while (sb.length() > 0 && 
           (sb.charAt(sb.length()-1) == '\n' || 
            sb.charAt(sb.length()-1) == '\r'))
      sb.deleteCharAt(sb.length()-1);
    return sb.toString();
  }
}
