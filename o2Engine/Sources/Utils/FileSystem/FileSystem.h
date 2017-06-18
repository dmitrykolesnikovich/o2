#pragma once

#include "Utils/Containers/Dictionary.h"
#include "Utils/Containers/Vector.h"
#include "Utils/FileSystem/FileInfo.h"

#include "Utils/Singleton.h"
#include "Utils/String.h"


namespace o2
{
	class LogStream;

	// File system access macros
#define o2FileSystem o2::FileSystem::Instance()

	// ---------------------------------------------------
	// File system. Using for working with files and paths
	// ---------------------------------------------------
	class FileSystem: public Singleton<FileSystem>
	{
		typedef Vector<String> StringsVec;
		typedef Dictionary<FileType, StringsVec> ExtnsionsDict;

		LogStream*     mLog;           // File system log stream
		ExtnsionsDict  mExtensions;    // Extensions arrays dictionary
		String         mResourcesPath; // Basic application resources path

	public:
		// Default constructor
		FileSystem();

		// Destructor
		~FileSystem();

		// Returns resource path
		const String& GetResourcesPath() const;

		// Returns info of paths and files of specified path
		FolderInfo GetFolderInfo(const String& path) const;

		// Returns file info
		FileInfo GetFileInfo(const String& path) const;

		// Sets file edited date
		bool SetFileEditDate(const String& path, const TimeStamp& time) const;

		// Copying file
		bool FileCopy(const String& source, const String& dest) const;

		// Deletion file
		bool FileDelete(const String& file) const;

		// Moving file
		bool FileMove(const String& source, const String& dest) const;

		// Creates folder
		bool FolderCreate(const String& path, bool recursive = true) const;

		// Moves folder
		bool FolderCopy(const String& from, const String& to) const;

		// Removes directory
		bool FolderRemove(const String& path, bool recursive = true) const;

		// Renames file or directory
		bool Rename(const String& old, const String& newPath) const;

		// Returns true if specified directory exist
		bool IsFolderExist(const String& path) const;

		// Returns true if specified file exist
		bool IsFileExist(const String& path) const;

		// Returns path from string
		String ExtractPathStr(const String& path) const;

		// Returns file extension
		static String GetFileExtension(const String& filePath);

		// Returns file name without extension
		static String GetFileNameWithoutExtension(const String& filePath);

		// Returns end path without parent directories
		static String GetPathWithoutDirectories(const String& path);

		// Returns parent path
		static String GetParentPath(const String& path);

		// Read file and returns result
		static String ReadFile(const String& path);

		// Writes file data
		static void WriteFile(const String& path, const String& data);
	};
}
