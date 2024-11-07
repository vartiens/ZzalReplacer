#include<iostream>
#include<filesystem>
#include<cstdlib>
#include<format>
#include<urlmon.h>

#define X_PICTURE_FILENAME_LEN 15
#define DOWNLOAD_MODE 1

using namespace std::filesystem;
using namespace std;

vector<string> stems; // Stem: filename w/o extensions
vector<string> exts;
vector<string> oldNames;

#if !DOWNLOAD_MODE
void downloadCurl()
{
	// No error checking whatsoever
	for (int i = 0; i < stems.size(); i++)
	{
		string cmd = format("curl -o {}{} https://pbs.twimg.com/media/{}{}:orig", stems[i], exts[i], stems[i], exts[i]);
		system(cmd.c_str());
	}
}
#else
int downloadUrlmon()
{
	int fail = 0;
	for (int i = 0; i < stems.size(); i++)
	{
		string name = stems[i] + exts[i];
		string url = format("https://pbs.twimg.com/media/{}:orig", name);
		HRESULT hr = URLDownloadToFileA(nullptr, url.c_str(), name.c_str(), BINDF_GETNEWESTVERSION, nullptr);
		
		if (hr != S_OK) // Failure
		{
			cout << "Failed to download " << name << endl;
			fail++;
		}
		else // Success
		{
			cout << "(" << i + 1 << "/" << stems.size() << ") " << name << endl;
			
			// Only delete if old name is different from new name
			// fuck
			if (oldNames[i].length() != name.length())
			{
				path oldFile(oldNames[i]);
				remove(oldFile); // Delete old (low res) file
			}
		}
	}

	return fail;
}
#endif

int main()
{
	cout << "Put exe in the same folder with target pictures" << endl;
	cout << "Replaced pictures will be deleted automatically" << endl;
	cout << "Current path: ";

	path current(current_path());
	cout << current.root_path().string() << current.relative_path().string() << endl;
	directory_iterator iter("."); // Current path

	string stem, ext;
	for (const auto& entry : iter)
	{
		// filename(): path object with only the name
		// Use string() to turn it into string
		// push_back(): Adds element to the back
		// If error occurs because of non-ascii char, just skip it
		try { stem = entry.path().stem().string(); }
		catch (exception e) { continue; }

		if (stem.length() == X_PICTURE_FILENAME_LEN || stem.length() == X_PICTURE_FILENAME_LEN + 4)
		{
			stems.push_back(stem.substr(0, X_PICTURE_FILENAME_LEN)); // To handle (1), (2), etc.
			ext = entry.path().extension().string();

			// Turn jpegs, jfifs into jpgs
			// Why do they even exist
			// Also save old filenames to delete later
			exts.push_back(
				(ext == ".jpeg" || ext == ".jfif") ? ".jpg" : ext);
			oldNames.push_back(entry.path().filename().string());
		}
	}

#if !DOWNLOAD_MODE
	downloadCurl();
#else
	int fails = downloadUrlmon();
	cout << stems.size() << " images total, " << fails << " failed downloads" << endl;
#endif

	system("pause >> nul");
	return 0;
}
