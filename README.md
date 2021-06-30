# youtube-dl-audio-gui
**A very simple GUI for youtube-dl, specialized in downloading audio files or audio file playlists with only one click**

### Requirements
 
- Windows
- Visual Studio 2017 with CMake

### Download 

You can download the latest version of *youtube-dl-audio-gui* by cloning the GitHub repository:

	git clone https://github.com/HighDefinist/youtube-dl-audio-gui.git
	
### Usage

* Download and compile
* Get youtube-dl, and put youtube-dl-audio-gui.exe and youtube-dl.exe into the same folder
  * Alternatively, put youtube-dl in any other folder, and edit "settings.json" (it is created when first starting youtube-dl-audio-gui.exe, in the same folder), and set "YoutubeDlPath" to the location of youtube-dl.exe
* Execute youtube-dl-audio-gui.exe to get the following functionality:
  * When you copy a valid Youtube web address into the clipboard, the GUI will show a video and/or playlist string
  * Then, click "Download single" or "Download playlist" to download the music of the referenced video (or of all videos in the referenced playlist) into the folder (or a subfolder) of youtube-dl-audio-gui.exe
  * "Download anything" attempts to download the audio contained in the link, even if it cannot successfully be parsed as a Youtube link, by passing it directly to youtube-dl
  * "Open Download Folder" opens an explorer window of the folder containing youtube-dl-audio-gui.exe
  * "To Ogg" attempts to losslessly convert any downloaded .webm files to .ogg, or .m4a. If successful, the original webm file is deleted (you can edit "settings.json" and set "MkvExtractPath" to the location of mkvextract.exe, if it is not in the same folder as youtube-dl-audio-gui.exe)
* Multiple downloads are done sequentially. When it says "Actions pending: 0", all downloads have finished
* Closing the GUI window will not cancel any remaining actions. Instead, the program termination is delayed until all actions are finished
* Closing the console window will instantly close the application, and cancel any remaining actions
