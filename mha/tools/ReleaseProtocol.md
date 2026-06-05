# Branch policy
A new release is prepared in branch development. At release, branch master is fast-forwarded to the state of branch development. No squash merges are performed anymore.

# Release environment
Releases can only be made from an Ubuntu computer which can build the PDF manuals. I.e. it must have the necessary openMHA build environment for compilation and documentation generation already installed and must have a working Jack installation. Sound hardware (Headphones, microphones, sound card) is required.

Manual tests before a release involve MHA instances using Jack and alsa. It is unlikely that a Videocall session with screen sharing can run in parallel as this also wants access to audio hardware.

# Release procedure
1. Create a clean worktree
2. `./configure --prefix=<arbitrary non-standard prefix>`
3. `make test unit-tests`
4. Perform manual pre-release tests (see below)
5. `make release`
6. News Release

# Manual pre-release tests

All of these tests run on Linux and require jack and the user being a member of group "audio".

## test_mhaioalsa.m and other automated live tests
test_mhaioalsa.m needs the alsa loopback module, aplay, and arecord installed.
To do this, execute:
```
sudo apt-get install alsa-utils
grep snd-aloop /etc/modules || echo snd-aloop | sudo tee -a /etc/modules
grep "alias snd-card 0 snd-aloop" /etc/snd-aloop.conf || (echo "alias snd-card 0 snd-aloop"; echo  "options snd-aloop index=0 pcm_substreams=2") | sudo tee -a /etc/snd-aloop.conf
```
Then run:
```
cd mha/mhatest; octave --no-gui --no-window-system --eval "set_environment;global execute_live_tests=true;exit(~run_mha_tests())"
```
or
```
cd mha/mhatest; matlab -nodesktop -nosplash -nodisplay -r"set_environment;global execute_live_tests=true;exit(~run_mha_tests())"
```

**If the alsa loop device is not installed as detailed above, then this test may still signal success, but it does not count for us as a successful test because the MHAIOalsa test was skipped.** It says 
> warning: ALSA Loopback device is not available, can not test ALSA IO

in this case. To fix it: `sudo modprobe snd-aloop`

## Run gain_live example, dynamic compressor live example
`cd` back to the git root directory, run make install, add freshly installed prefix/bin dir to PATH and the lib dir to LD_LIBRARY_PATH. Then run the examples as detailed in the table below.

<table>
  <tr>
    <th>setting</th>
    <th>gain_live</th>
    <th>dynamic_compression</th>
  </tr>
  <tr>
    <th>Jack period size</th>
    <td>multiple of 64</td>
    <td>multiple of 64</td>
  </tr>
  <tr>
    <th>Jack sampling rate</th>
    <td>44100</td>
    <td>44100</td>
  </tr>
  <tr>
    <th>HW audio channels</th>
    <td>2 in/2 out</td>
    <td>2 in/2 out</td>
  </tr>
  <tr>
    <th>required HW</th>
    <td>mics, headphones</td>
    <td>mics, headphones</td>
  </tr>
  <tr>
    <th>Start command</th>
    <td><tt>mha ?read:examples/00-gain/gain_live.cfg cmd=start --interactive</tt></td>
    <td><tt>mha ?read:examples/01-dynamic-compression/example_dc_live.cfg cmd=start --interactive</tt></td>
  </tr>
  <tr>
    <th>Expected effect 0</th>
    <td>Left output softer than right output</td>
    <td>Hardly anything can be heard because the initial gaintable setting attenuates mostly</td>
  </tr>
  <tr>
    <th>Interaction 1</th>
    <td><tt>mha.gain.gains=[-10 -10]</tt></td>
    <td><tt>mha.overlapadd.mhachain.dc.gtdata = [[60 20 -20];[60 20 -20];[60 20 -20];[60 20 -20]]</tt></td>
  </tr>
  <tr>
    <th>Expected Effect 1</th>
    <td>equal loudness on both sides</td>
    <td>pantonal compression, constant output level</td>
  </tr>
  <tr>
    <th>Interaction 2</th>
    <td><tt>mha.gain.gains=[10 10]</tt></td>
    <td>Start matlab fitting GUI. Apply two different fitting rules. Adjust fine tuning.</td>
  </tr>
  <tr>
    <th>Expected Effect 1</th>
    <td>higher equal loudness on both sides</td>
    <td>Changing fitting or fine tuning has desired effect. Not all finetuning frequencies have an effect because we have very few bands.</td>
  </tr>
</table>

## Offline fitting gui
Test the offline fitting gui with sound files and a variable compression rate fitting rule
## Additional manual tests
Compile the openMHA on a Windows desktop machine with openMHA build tools installed. Edit `openMHA/mha/mhatest/test_mhaioportaudio.m` on Windows. The test ignores an "Internal PortAudio error" which is expected on Windows build servers without a sound device. Remove the ignore and check that the test succeeds on a Windows machine with sound hardware.
