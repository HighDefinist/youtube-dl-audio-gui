# youtube-dl-audio-gui
**A very simple GUI for youtube-dl, specialized in downloading audio files or audio file playlists with only one click**

### Requirements
 
- Windows
- Visual Studio 2017
- [CMake](https://cmake.org/)

### Download 

You can download the latest version of *youtube-dl-audio-gui* by cloning the GitHub repository:

	git clone https://github.com/HighDefinist/youtube-dl-audio-gui.git
	
### Usage

* Download and compile
* Get youtube-dl, and put youtube-dl-audio-gui.exe and youtube-dl.exe into the same folder
* Execute youtube-dl-audio-gui.exe
** When you copy a valid Youtube web address into the clipboard, the GUI will show a video and/or playlist string
** Then, click "Download single" or "Download playlist" to download the music of the referenced video, or of all videos in the referenced playlist into the folder containing youtube-dl-audio-gui.exe
* Multiple downloads are done sequentially. When it sais "Actions pending: 0", all downloads have finished
* Closing the GUI window will not cancel any remaining actions. Instead, the program termination is delayed until all actions are finished
* Closing the console window will instantly close the application, and cancel any remaining actions
